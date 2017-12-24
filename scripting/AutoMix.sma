//#define SMALL_GAME_FOR_LOCAL_TESTS

/** Уровень дебага:
	0 - отключить дебаг
	1 - + Лог не правильных значений switch-case и вкл/откл сервера
	2 - + Включение тестовых команд для отладки и Показ дебаг информации каждый раунд и по командам.
	3 - + Лог всего
*/
enum _:DebugLevels (+=1)
{
	D_LEVEL_0 = 0,
	D_LEVEL_1,
	D_LEVEL_2,
	D_LEVEL_3
}

new g_iDebugLevel = D_LEVEL_3,	// Стандартный уровень дебага
	g_pcvar_DebugLvl;

#pragma semicolon 1

#include <amxmodx>
#include <amxmisc>
#include <sqlx>
#include <reapi>

new const plVERSION[] = "0.1 b17";

const MAX_AUTHID_LENGTH = 32;
const MAX_IP_LENGTH = 22;

const TimeForPAUSE = 60;
const TimeForPAUSE_Limit = 300;

/* Временное хранение */
new g_szSteamId[MAX_CLIENTS][MAX_AUTHID_LENGTH],	// SteamId игрока
	g_szIPAddress[MAX_CLIENTS][MAX_IP_LENGTH],		// IP игрока
	bool: g_bVoted[MAX_CLIENTS];						// Cостояние готовности игрока
	
new Float:g_fPlayerPingEMA[MAX_CLIENTS] = { 1.0, ... },		// Средний пинг
	g_iPlayerCheckTimes[MAX_CLIENTS];					// Кол-во измерений пинга

new g_szServerIP[MAX_IP_LENGTH],		// IP сервера
	g_szServerName[32],					// Имя сервера
	g_szServerPort[10],					// Port сервера
	g_iFreezeTime,						// Freezetime сервера
	g_iGameSlots,						// Кол-во слотов для игры/игроков
	g_iGameId,							// ID игры
	g_szMapName[32];					// Имя карты
	
new g_iGameStateCurrent,				// Статус игры
	g_iVotedPlayersNum,					// Кол-во отдавших голос за готовность.
	g_iVotedPlayersForPause,
	g_iConnectedCount;					// Кол-во присоеденённых игроков на сервере.
	
new g_iVotePauseRequesterId,
	g_bIsPause,
	Float: g_fLastPauseVote,
	bool: g_bIsOvertime;

new g_iOvertimeRoundCounter,
	g_iOvertimeRoundCounterPart;

/* Стадия игры */
enum _:GAME_STATE
{
	WARMUP,
	HALF_1,
	HALF_2,
	OVERTIME,
	FINISHED
}

/* Названия стадии игры */
new stock g_szGameStates_Name[GAME_STATE][] = {
	"WARMUP",
	"HALF_1",
	"HALF_2",
	"OVERTIME",
	"FINISHED"
};

enum _:HC_TYPES
{
	__SV_DropClient,
	// __CBasePlayer_AddAccount,
	__RG_CBasePlayer_Spawn,
	__CBasePlayer_Killed,
	__RG_CSGameRules_GiveC4,
	__RG_CSGameRules_FPlayerCanRespawn,
	__RG_RoundEnd,
	__RG_CSGameRules_RestartRound,
	__RG_CSGameRules_DeadPlayerWeapons
}

new HookChain:HC_Types[HC_TYPES];

enum _:eTeams (+=1)
{
	Team_A,
	Team_B,
	NoLeaderTeam
}

/** Ключи команд для LANG */
new g_szTeamsNames[eTeams][] = {
	"Team_A",
	"Team_B",
	"No_Team_Leader"
};

new g_iTeamWins[eTeams];

new	g_iRoundPlayed,				// Общее кол-во сыгранных раундов
	bool: g_bTeamSwapped;		// Поменяны ли команды местами?

#define PercentSub(%1,%2)    (%1 - (%1 * %2) / 100)
#define IsParityOfTeams		(g_iTeamWins[Team_A] == g_iTeamWins[Team_B])

/* Отсутсвующий процент игроков для начала запуска игры */
const PercentToReady = 10;

#define IsPlayer(%1)		(1 <= (%1) <= MaxClients)

// new g_iNeedSlotsToPreReady;

/* Время для отложенных задач */
enum Float:TimersForTasks
{
	PingRepeatTime = 3.0,			// Интервал проверки пинга игрока
	DelayForForceStartGame = 180.0,	// Через сколько секунд игра начнётся принудительно
	MsgTime_HLTV = 20.0,			// Интервал обновления сообщения для HLTV
	TimeForPauseVote = 20.0,		// Время на голосование за паузу игры
	ShowNotReadyPlayers = 20.0,		// Показ списка не готовых к игре игроков
	DalayBeforeSwitch = 10.0,		// Задержка перед сменой сторон в середине игры
	Delay_ServerShutdown = 30.0		// ВЫдержка времени перед выключением сервера по окончанию игры.
}

enum _:TASKS (+=3)
{
	TASK_ForceStartGame = 1345,
	TASK_ShowMsgHLTV,
	TASK_GetPing,
	TASK_CheckWarmupEnd,
	TASK_ShutdownServer_Force,
	TASK_ProtectionId,
	TASK_ShowNotReadyPlayersList,
	TASK_PlayerVoice
}

enum _:Tries {
	Trie: tTeam,
	Trie: tMoney,
	Trie: tFrags,
	Trie: tDeaths,
	Trie: tPing,
	Trie: tName
}

new Trie: g_tPlayerData[Tries];

new g_iModelIndexRadio;

/* Общие правила для запрета оружий */
new const ItemID:g_rRestrictWeapons[] = {
	// ITEM_SHIELDGUN,
	// ITEM_P228,
	// ITEM_GLOCK,
	// ITEM_SCOUT,
	// ITEM_HEGRENADE,
	// ITEM_XM1014,
	// ITEM_C4,
	// ITEM_MAC10,
	// ITEM_AUG,
	// ITEM_SMOKEGRENADE,
	// ITEM_ELITE,
	// ITEM_FIVESEVEN,
	// ITEM_UMP45,
	// ITEM_SG550,
	// ITEM_GALIL,
	// ITEM_FAMAS,
	// ITEM_USP,
	// ITEM_GLOCK18,
	// ITEM_AWP,
	// ITEM_MP5N,
	// ITEM_M249,
	// ITEM_M3,
	// ITEM_M4A1,
	// ITEM_TMP,
	// ITEM_G3SG1,
	// ITEM_FLASHBANG,
	// ITEM_DEAGLE,
	// ITEM_SG552,
	// ITEM_AK47,
	// ITEM_KNIFE,
	// ITEM_P90,
	
	// don't touch it!!
	ITEM_NONE
};

new const ItemID:g_rRestrictItems[] = {
	ITEM_NVG,
	// ITEM_DEFUSEKIT,
	// ITEM_KEVLAR,
	// ITEM_ASSAULT,
	// ITEM_LONGJUMP,
	// ITEM_HEALTHKIT,
	// ITEM_ANTIDOTE,
	// ITEM_SECURITY,
	// ITEM_BATTERY,
	// ITEM_SUIT,
	
	// don't touch it!!
	ITEM_NONE
};

enum SectionBits { SECTION_WEAPONS,	SECTION_ITEMS }
new g_bitsRestrict[SectionBits];

enum _:Name_Sql_Cvar
{
	Sql_Host,
	Sql_User,
	Sql_Pass,
	Sql_Db,
	Sql_Table,
	Sql_Driver,
	Sql_Max_Error
};

new g_Sql_Cvar[Name_Sql_Cvar];
new g_Sql_Error_Count;

new Handle:g_Sql;

public plugin_precache()
{
	/* GameId берётся из квара */
	g_iGameId = get_cvar_num("violence_hgibs");
	g_iGameSlots = MaxClients - 1;
	
	Init__Logger();
	Init__Tries();
	Init__BugFixes();
}

#include "AutoMix_System/Logger.inl"
#include "AutoMix_System/BugFixes.inl"
#include "AutoMix_System/Game_Settings.inl"
#include "AutoMix_System/Sockets.inl"
#include "AutoMix_System/Connect.inl"
#include "AutoMix_System/Menus.inl"
#include "AutoMix_System/Commands.inl"
#include "AutoMix_System/Messages.inl"
#include "AutoMix_System/Warmup.inl"

public plugin_init()
{
	register_plugin("AutoMix System", plVERSION, "FastMix.net");
	register_dictionary("AutoMix_System.txt");
	
	Init__Cvars();
	Init__Sql_Cvar();
	Init__Sql_Db();	

	Init__ReAPI_Hooks();
	HC_Disable_All();

	ToggleMods(FIRSTWARMUP);

	Init__WeaponRestricts();

	// debug
	register_clcmd("fm_test1", "test1");
	register_clcmd("fm_test2", "test2");

	set_task(PingRepeatTime, "Get_PlayersPing", .flags = "b");

	if(MaxClients != get_maxplayers())
	{
		Logging("\
			plugin_init() -> Error get MaxPlayers.^n^t\
			MaxClients='%d'^n^t\
			get_maxplayers='%d'", MaxClients, get_maxplayers()
		);
	}
}

// public OnConfigsExecuted() - не работает, или я не понял как пользоваться =)
public plugin_cfg()
{
	GameInfo_Get();
	Register_PlayerCommands();
	RegisterMenu();

	#if defined SMALL_GAME_FOR_LOCAL_TESTS
	Fake__AddNicknames();
	#endif
}

/*
public Changed_DebugLvl(pcvar, const old_value[], const new_value[])
{
	g_iDebugLevel = get_pcvar_num(pcvar);

	// log_amx("Changed_DebugLvl -> g_iDebugLevel = %i", g_iDebugLevel);
}

public ReloadDebugLevel(pPlayerId, AccessLevel, iCommandId)
{
	if(~get_user_flags(pPlayerId) & AccessLevel)
	{
		return PLUGIN_HANDLED;
	}

	g_iDebugLevel = get_cvar_num("debug_level");

	return PLUGIN_CONTINUE;
}
*/

public Get_PlayersPing()
{
	for(new pPlayerId = 1; pPlayerId <= MaxClients; pPlayerId++)
	{
		if(!is_user_connected(pPlayerId))
		{
			continue;
		}

/*
		if(g_iDebugLevel >= D_LEVEL_3)
		{
			// client_print(pPlayerId, print_chat, "Your EMA ping: %.0f", Get__AveragePing(pPlayerId));
		}
*/

		Get__AveragePing(pPlayerId);
	}
}

// Получение экспоненциального скользящего среднего произвольного порядка пинга игрока.
stock Float: Get__AveragePing(const pPlayerId)
{
	static iPing, iLoss, Float: fAlpha;
	get_user_ping(pPlayerId, iPing, iLoss);

	fAlpha = 2.0 / ++g_iPlayerCheckTimes[pPlayerId];
	g_fPlayerPingEMA[pPlayerId] = (fAlpha * iPing) + (1.0 - fAlpha) * g_fPlayerPingEMA[pPlayerId];
	
	return g_fPlayerPingEMA[pPlayerId];
}

public Init__WeaponRestricts()
{
	new i;
	for(i = 0; g_rRestrictWeapons[i] != ITEM_NONE; i++)
	{
		g_bitsRestrict[SECTION_WEAPONS] |= (1 << any: g_rRestrictWeapons[i]);
	}

	for(i = 0; g_rRestrictItems[i] != ITEM_NONE; i++)
	{
		g_bitsRestrict[SECTION_ITEMS] |= (1 << any: (g_rRestrictItems[i] % ITEM_NVG));
	}
}

public Init__Cvars()
{
	ExecCMDS(g_Cmds_Default, sizeof g_Cmds_Default);

	formatex(g_szServerName, charsmax(g_szServerName), "FastMix.Net");
	// get_cvar_string("hostname", g_szServerName, charsmax(g_szServerName));

	get_cvar_string("port", g_szServerPort, charsmax(g_szServerPort));
	g_iFreezeTime = /*get_cvar_num("mp_freezetime")*/ 10;
	
	// get_cvar_string("net_address", g_szServerIP, charsmax(g_szServerIP));
	formatex(g_szServerIP, charsmax(g_szServerIP), "cs.FastMix.net:%s", g_szServerPort);

	new szDbgLVL[2];
	num_to_str(g_iDebugLevel, szDbgLVL, charsmax(szDbgLVL));
	
	g_pcvar_DebugLvl = create_cvar(
		.name = "debug_level",
		.string = szDbgLVL,
		.flags = FCVAR_PROTECTED,
		.description = "Set Debug level",
		.has_min = true,
		.min_val = float(D_LEVEL_0),
		.has_max = true,
		.max_val = float(DebugLevels) - 1.0
	);

	// hook_cvar_change(g_pcvar_DebugLvl, "Changed_DebugLvl");
	bind_pcvar_num(g_pcvar_DebugLvl, g_iDebugLevel);
}

public test1(id)
{
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		// g_iConnectedCount = g_iGameSlots;
		// client_print(id, print_chat, "g_iConnectedCount='%i', g_iGameSlots='%i'", g_iConnectedCount, g_iGameSlots);
		
		// client_print_color(id, print_team_default,
			// "g_iConnectedCount='^4%i^1', g_iGameSlots='^4%i^1'",
			// g_iConnectedCount, g_iGameSlots
		// );
		
		if(g_iGameStateCurrent == WARMUP)
		{
			client_print(id, print_chat, "%L%s", id, "NotReadyList", Get_NickNames_NotReady());
		}
	}

	return PLUGIN_HANDLED;
}

public test2(id)
{
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		g_iVotedPlayersNum = g_iGameSlots;
	
		Check__Counts_Ready();
	}

	return PLUGIN_HANDLED;
}

public ShutdownServer_Force()
{
	if(g_iDebugLevel >= D_LEVEL_1)
	{
		Logging("ShutdownServer_Force -> [ Server Stop: 'QUIT']");
	}
	
	server_cmd("quit");
}

public plugin_end()
{
	CloseSockets();

	if(g_Sql != Empty_Handle)
	{
		SQL_FreeHandle(g_Sql);
	}
}

public ShowVGUIMenu(const pPlayerId, VGUIMenu:menuType, const bitsSlots, szOldMenu[])
{	
	if(menuType == VGUI_Menu_Team)
	{
		SetHookChainArg(4, ATYPE_STRING, " ");
		
		// Не вызывается...
		//log_amx("ShowVGUIMenu");
		
		return HC_SUPERCEDE;
	}

	return HC_CONTINUE;
}

stock OnPlayer_ResetData(pPlayerId)
{
	g_szIPAddress[pPlayerId][0] = EOS;

	// g_iAccount[pPlayerId] = 0;

	if(--g_iConnectedCount < 0)
	{
		g_iConnectedCount = 0;
	}

	// log_amx("g_iConnectedCount = %i", g_iConnectedCount);

	if(g_bVoted[pPlayerId] == true)
	{
		if(--g_iVotedPlayersNum < 0)
		{
			g_iVotedPlayersNum = 0;
		}
	}

	g_bVoted[pPlayerId] = false;
}

// Во время Warmup мы проверяем кол-во готовых к этапу игры.
stock Check__Counts_Ready()
{	
	if(g_iGameStateCurrent == WARMUP)
	{
		/* Если осталось 10% игроков до готовности,
			то запускаем таймер до автоматической готовности */
		// if(g_iVotedPlayersNum == g_iNeedSlotsToPreReady)
		// {
		/* Не забыть установить RoundTime в нужное время - 3 минуты */
			// set_task(DelayForForceStartGame, "ForceStartGame", .id = TASK_ForceStartGame);
			
			// set_member_game(m_iRoundTime, floatround(DelayForForceStartGame));
		
			// client_print(0, print_chat, "%L",
				// LANG_PLAYER, "GamePreReady",
				// DelayForForceStartGame / 60.0
			// );
		// }

		/* Warmup оканчивается только по набору всех игроков !r готовности */
		if(g_iVotedPlayersNum >= g_iGameSlots)
		{
			//remove_task(TASK_ForceStartGame);

			switch(g_iRoundPlayed)
			{
				case 0: g_iGameStateCurrent = HALF_1;
				case Round_ToSwitch: g_iGameStateCurrent = HALF_2;
			}
			
			if(g_bIsOvertime) {
				g_iGameStateCurrent = OVERTIME;
			}

			// DisableHookChain(HookChain:HC_Types[__RG_RoundEnd]);
			
			Stop_WarMup_Toggle();

			g_iPrepareRestarts = PREPARE_RESTARTS;

			DoRestarts();
		}
	}
	
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging("\
			Check__Counts_Ready ->^n^t\
			g_iRoundPlayed='%i'^n^t\
			g_iGameStateCurrent='%s'^n^t\
			g_iVotedPlayersNum='%i'^n^t\
			g_iGameSlots='%i'",
			g_iRoundPlayed, g_szGameStates_Name[ g_iGameStateCurrent ], g_iVotedPlayersNum, g_iGameSlots
		);
	}
}

/*
public ForceStartGame()
{
	Stop_WarMup_Toggle();

	g_iPrepareRestarts = PREPARE_RESTARTS;

	DoRestarts();
}
*/

stock ToggleMods(iMode)
{
	switch(iMode)
	{
		case WARMUP_START:
		{
			Warmup_Controller(WARMUP_START_BTWGAME);

			HC_Controller_Do(WARMUP_START);

			WeaponRestricts_Mode(WARMUP_START);
			
			set_task(180.0 + 5.0, "Check__OnWarmupTimeout", TASK_CheckWarmupEnd);
			set_task(ShowNotReadyPlayers, "MsgChat__NotReadyPlayers", TASK_ShowNotReadyPlayersList);
		}
		
		case WARMUP_STOP:
		{
			Warmup_Controller(WARMUP_STOP);

			HC_Controller_Do(WARMUP_STOP);

			WeaponRestricts_Mode(WARMUP_STOP);
			
			remove_task(TASK_CheckWarmupEnd);
		}
		
		case FIRSTWARMUP:
		{
			Warmup_Controller(WARMUP_START);

			HC_Controller_Do(WARMUP_START);

			WeaponRestricts_Mode(WARMUP_START);
			
			set_task(300.0 + 5.0, "Check__OnWarmupTimeout", TASK_CheckWarmupEnd);
		}
		
		default:
		{
			if(g_iDebugLevel >= D_LEVEL_2)
			{
				Logging("ToggleMods -> iMode='%i'", iMode);
			}
		}
	}
}

stock TeamSwapALL()
{
	rg_swap_all_players();

	g_bTeamSwapped = !g_bTeamSwapped;
}

public RoundEnd(WinStatus:status, ScenarioEventEndRound:event, Float:tmDelay)
{
	/**
		Если установлена пауза - в конце раунда выставляем параметры для паузы
	*/
	if(g_bIsPause)
	{
		SetGame_To_Pause();
	}
	
	switch(event)
	{
		case ROUND_GAME_COMMENCE:
		{
			set_member_game(m_bGameStarted, true);

			SetHookChainReturn(ATYPE_INTEGER, false);

			return HC_SUPERCEDE;
		}
		
		case ROUND_GAME_RESTART:
		{
			return HC_CONTINUE;
		}
	}

	switch(g_iGameStateCurrent)
	{		
		case FINISHED, WARMUP:
		{
			return HC_CONTINUE;
		}
		
		default:
		{
			if(g_iDebugLevel >= D_LEVEL_2
				&& g_iGameStateCurrent != HALF_1
				&& g_iGameStateCurrent != HALF_2
				&& g_iGameStateCurrent != OVERTIME)
			{
				Logging("RoundEnd -> g_iGameStateCurrent = %i", g_iGameStateCurrent);
			}
		}
	}
	
	++g_iRoundPlayed;

	AddPoints_ToTeam(status);
	
	if(g_iDebugLevel >= D_LEVEL_2)
	{
		Logging("RoundEnd -> [Round: %i] [A='%i'][B='%i']", g_iRoundPlayed, g_iTeamWins[Team_A], g_iTeamWins[Team_B]);
	}

	/**
		Обработчик овертайма
	*/
	if(g_bIsOvertime)
	{
		g_iGameStateCurrent = OVERTIME;
		
		const OverTimePart = 3; // Половина овертайма
		const FullOvertimePart = 6; // Цикл овертайма
		const iRoundDiffsToWin = 3; // Разница в счёте

		++g_iOvertimeRoundCounter; // Общее кол-во овертайм раундов.
		++g_iOvertimeRoundCounterPart; // каунтер половины овертайма

		if(g_iOvertimeRoundCounter > OverTimePart && GetTeamLeader_ScoreDiff(iRoundDiffsToWin + 1))
		{
			g_iGameStateCurrent = FINISHED;

			ToCompleteTheGame();

			MsgChat__Status();

			Logging("GameEnd WinTeam: %s", g_szTeamsNames[Get_TeamLeader()]);

			return HC_CONTINUE;
		}
		
		if(!(g_iOvertimeRoundCounter % FullOvertimePart))
		{
			if(IsParityOfTeams)
			{
				PlayerProps_SaveALL();

				TeamSwapALL();

				g_iOvertimeRoundCounter = 0;

				server_cmd("sv_restart 1");

				MsgChat__Overtime();

				Logging("RoundEnd() -> TeamSwapALL() Overtime Retry - TeamScores on parity");
			}
			else
			{
				g_iGameStateCurrent = FINISHED;

				ToCompleteTheGame();

				MsgChat__Status();
				
				Logging("GameEnd WinTeam: %s", g_szTeamsNames[Get_TeamLeader()]);
				
				return HC_CONTINUE;
			}
			
			// return HC_CONTINUE;
		}
		else if(!(g_iOvertimeRoundCounterPart % OverTimePart))
		{
			PlayerProps_SaveALL();

			TeamSwapALL();

			g_iOvertimeRoundCounterPart = 0;

			server_cmd("sv_restart 1");
						
			// MsgChat__Overtime();
			
			Logging("RoundEnd() -> TeamSwapALL() 1 part Overtime been finished");
		}
			
		// log_amx("g_iOvertimeRoundCounter = %i", g_iOvertimeRoundCounter);
	}	
	
	/**
		Проверка на окончание игры (команды достигли Score 15)
	*/
	if((g_iTeamWins[Team_A] == Score_ToWin - 1) && (g_iTeamWins[Team_B] == Score_ToWin - 1) && !g_bIsOvertime)
	{
		/**
			Если счёт команд равен - включаем перед OVERTIME разминку
		*/
		if(IsParityOfTeams)
		{
			g_iGameStateCurrent = WARMUP;
			g_bIsOvertime = true;
			
			PlayerProps_SaveALL();
			
			set_task(DalayBeforeSwitch, "DelayedSwapTeams");
			SetHookChainArg(3, ATYPE_FLOAT, DalayBeforeSwitch);

			/*	Должен быть отложенный свап.
			TeamSwapALL();
			ToggleMods(WARMUP_START);
			g_iPrepareRestarts = PREPARE_RESTARTS;
			DoRestarts();
			*/

			MsgChat__Overtime();
			
			return HC_CONTINUE;
		}
	}
	
	/** 
		По достижении Score = 16 ПОбеждает одна из команд - игра завершается
	*/
	if(g_iTeamWins[Get_TeamLeader()] == Score_ToWin && !g_bIsOvertime)
	{
		g_iGameStateCurrent = FINISHED;

		ToCompleteTheGame();

		MsgChat__Status();
		
		return HC_CONTINUE;
	}
	
	switch(g_iRoundPlayed)
	{
		// 1 половина игры (Round: 1-14)
		case 1 .. (Round_ToSwitch - 1):
		{
			g_iGameStateCurrent = HALF_1;
		}
		
		/**
			1 половина игры прошла, включаем разминку меж периодную (Round 15)
		*/
		case Round_ToSwitch:
		{
			g_iGameStateCurrent = WARMUP;
			
			PlayerProps_SaveALL();

			set_task(DalayBeforeSwitch, "DelayedSwapTeams");
			SetHookChainArg(3, ATYPE_FLOAT, DalayBeforeSwitch);
			
			/*	Должен быть отложенный свап.
			TeamSwapALL();
			ToggleMods(WARMUP_START);
			g_iPrepareRestarts = PREPARE_RESTARTS;
			DoRestarts();
			*/
			
			MsgChat__Status();

			return HC_CONTINUE;
		}
		
		// 2 половина игры (16-29)
		case (Round_ToSwitch + 1) .. (Round_OverTime - 1):
		{
			g_iGameStateCurrent = HALF_2;
		}

		default:
		{
			if(g_iDebugLevel >= D_LEVEL_2)
			{
				Logging("RoundEnd -> g_iRoundPlayed = %i", g_iRoundPlayed);
			}
		}
	}
	
	/*
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging("RoundEnd ->^n^t\
			g_iRoundPlayed: %i^n^t\
			Team_A='%i'^n^t\
			Team_B='%i'",
			g_iRoundPlayed,
			g_iTeamWins[Team_A], g_iTeamWins[Team_B]
		);
	}
	*/

	MsgChat__Status();

	return HC_CONTINUE;
}

public DelayedSwapTeams()
{
	TeamSwapALL();
	ToggleMods(WARMUP_START);
	g_iPrepareRestarts = PREPARE_RESTARTS;
	DoRestarts();
}

stock GetTeamLeader_ScoreDiff(iNum)
{
	new iWins_A = g_iTeamWins[Team_A],
		iWins_B = g_iTeamWins[Team_B];
	
	if(iWins_A > iWins_B)
	{
		return ((iWins_A - iWins_B) == iNum);
	}
	else if(iWins_A < iWins_B)
	{
		return ((iWins_B - iWins_A) == iNum);
	}
	else
	{
		return false;
	}
}

stock AddPoints_ToTeam(WinStatus:status)
{
	switch(status)
	{
		case WINSTATUS_CTS: ++g_iTeamWins[g_bTeamSwapped ? Team_A : Team_B];
		case WINSTATUS_TERRORISTS: ++g_iTeamWins[g_bTeamSwapped ? Team_B : Team_A];
		
		default:
		{
			if(g_iDebugLevel >= D_LEVEL_2)
			{
				Logging("AddPoints_ToTeam -> status = %i", status);
			}
		}
	}
}

/* Начало нового раунда */
public CSGameRules_RestartRound()
{
	switch(g_iGameStateCurrent)
	{
		case HALF_1, HALF_2, OVERTIME:
		{
			if((g_bIsOvertime && g_iOvertimeRoundCounter) || (g_iRoundPlayed || g_iRoundPlayed == Round_ToSwitch))
			{
				MsgChat_TeammatesMoney();
			}
		}
	}
	
	if(g_bIsPause)
	{
		RestoreGame();

		g_bIsPause = false;
	}

	if(g_bIsOvertime && g_iGameStateCurrent == OVERTIME)
	{
		set_task(1.0, "TASK_RestoreScores");
	}
}

stock ToCompleteTheGame()
{
	if(g_iRoundPlayed)
	{
		PlayerProps_SaveALL();
	}
	else if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging("ToCompleteTheGame() -> [- Game Not Started -]");
	}
	
	DisableHookChain(HookChain:HC_Types[__RG_RoundEnd]);
	
	g_iPrepareRestarts = PREPARE_RESTARTS;

	DoRestarts();

	/* Выставляем FreezeTime на 60 секунд и не даём денег */
	ExecCMDS(g_Cmds_GameEnd, charsmax(g_Cmds_GameEnd));

	/* Отправляем инфу о состоявшейся игре */
	GameInfo_Send();
	
	/* Через 30 секунд сервер выключится */
	set_task(Delay_ServerShutdown + 2.0, "ShutdownServer_Force");

	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging("ToCompleteTheGame() -> [- The server stops after 30 seconds -]");
	}
}

public CBasePlayer_Killed(const pPlayerId, const pKillerId, iGib)
{	
	switch(g_iGameStateCurrent)
	{
		case WARMUP, FINISHED:
		{
			/* Удаление defuser при разминке */
			if(get_member(pPlayerId, m_bHasDefuser))
			{
				set_member(pPlayerId, m_bHasDefuser, 0);
				set_entvar(pPlayerId, var_body, 0);
			}
		}
		
		/**
			Сбор кол-ва убийств и смертей
		*/
		case HALF_1, HALF_2, OVERTIME:
		{
			new szSteamId[MAX_AUTHID_LENGTH], iLeveling;
			
			new bool:bIsTeamKill = (get_member(pPlayerId, m_iTeam) == get_member(pKillerId, m_iTeam)) ? true : false;
			
			if(IsPlayer(pKillerId))
			{
				// Для убийцы
				copy(szSteamId, charsmax(szSteamId), g_szSteamId[pKillerId]);
				
				TrieGetCell(g_tPlayerData[tFrags], szSteamId, iLeveling);
				
				TrieSetCell(
					g_tPlayerData[tFrags],
					szSteamId,
					/* При тимкилл минусовать фраги */
					bIsTeamKill ? --iLeveling : ++iLeveling
				);
			}

			if(!bIsTeamKill)
			{
				// Для жертвы
				copy(szSteamId, charsmax(szSteamId), g_szSteamId[pPlayerId]);
	
				TrieGetCell(g_tPlayerData[tDeaths], szSteamId, iLeveling);

				TrieSetCell(g_tPlayerData[tDeaths], szSteamId, ++iLeveling);
			}
		}
		
		default:
		{
			if(g_iDebugLevel >= D_LEVEL_2)
			{
				Logging("CBasePlayer_Killed -> g_iGameStateCurrent = %i", g_iGameStateCurrent);
			}
		}
	}

	return HC_CONTINUE;
}

stock Do_OverTime()
{
	ExecCMDS(g_Cmds_OverTime, sizeof g_Cmds_OverTime);
}

/* Выполнение блока команд сервера */
stock ExecCMDS(szBuffer[][], const iLen)
{
	for(new i; i < iLen; i++)
	{
		server_cmd(szBuffer[i]);
	}
}

stock Do_ResetVotes()
{
	for(new i; i < sizeof g_bVoted; i++)
	{
		g_bVoted[i] = false;
	}
	
	g_iVotedPlayersNum = 0;
}

stock PlayerProps_Save(const pPlayerId)
{
	new szSteamId[MAX_AUTHID_LENGTH];
	copy(szSteamId, charsmax(szSteamId), g_szSteamId[pPlayerId]);

	if(g_iDebugLevel >= D_LEVEL_3)
	{
		new Float: fFrags = get_entvar(pPlayerId, var_frags);
		new iDeaths = get_member(pPlayerId, m_iDeaths);
		
		TrieSetCell(g_tPlayerData[tFrags], szSteamId, fFrags);
		TrieSetCell(g_tPlayerData[tDeaths], szSteamId, iDeaths);

		// TrieSetCell(g_tPlayerData[tMoney], szSteamId, get_member(pPlayerId, m_iAccount));

		Logging("\
			PlayerProps_Save ->^n^t\
			pPlayerId='%i'^n^t\
			szSteamId='%s'^n^t\
			var_frags='%1.f'^n^t\
			m_iDeaths='%i'^n^t\",
			pPlayerId, szSteamId, fFrags, iDeaths
		);
	}
	else
	{
		TrieSetCell(g_tPlayerData[tFrags], szSteamId, get_entvar(pPlayerId, var_frags));
		TrieSetCell(g_tPlayerData[tDeaths], szSteamId, get_member(pPlayerId, m_iDeaths));

		// TrieSetCell(g_tPlayerData[tMoney], szSteamId, get_member(pPlayerId, m_iAccount));
	}
}

stock PlayerProps_Restore(const pPlayerId)
{
	new Float: fFrags, iDeaths,/* iAccount,*/ szSteamId[MAX_AUTHID_LENGTH];
	copy(szSteamId, charsmax(szSteamId), g_szSteamId[pPlayerId]);
	
	TrieGetCell(g_tPlayerData[tFrags], szSteamId, fFrags);
	TrieGetCell(g_tPlayerData[tDeaths], szSteamId, iDeaths);

	// TrieGetCell(g_tPlayerData[tMoney], szSteamId, iAccount);
	
	set_entvar(pPlayerId, var_frags, fFrags);
	set_member(pPlayerId, m_iDeaths, iDeaths);

	// set_member(pPlayerId, m_iAccount, iAccount);
		
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging("\
			PlayerProps_Restore -> ^n^t\
			pPlayerId='%i'^n^t\
			szSteamId='%s'^n^t\
			var_frags='%1.f'^n^t\
			m_iDeaths='%i'^n^t\",
			pPlayerId, szSteamId, fFrags, iDeaths
		);
	}	
	
	Msg_UpdateScoreInfo(pPlayerId, fFrags, iDeaths);
}

stock PlayerProps_SaveALL()
{
	for(new pPlayerId = 1; pPlayerId <= MaxClients; pPlayerId++)
	{
		if(!is_user_connected(pPlayerId))
		{
			continue;
		}

		PlayerProps_Save(pPlayerId);
	}
}

stock PlayerProps_RestoreALL()
{
	for(new pPlayerId = 1; pPlayerId <= MaxClients; pPlayerId++)
	{
		if(!is_user_connected(pPlayerId))
		{
			continue;
		}

		PlayerProps_Restore(pPlayerId);
	}
}

stock Msg_UpdateScoreInfo(const pPlayerId, Float: fFrags, iDeaths)
{	
	/* https://wiki.alliedmods.net/Half-life_1_game_events#ScoreInfo */
	const iMsg_ScoreInfo = 85; // get_user_msgid("ScoreInfo")
	new iTeam = get_member(pPlayerId, m_iTeam);

	message_begin(MSG_ALL, iMsg_ScoreInfo);
	write_byte(pPlayerId);
	write_short(floatround(fFrags));
	write_short(iDeaths);
	write_short(0);
	write_short(iTeam);
	message_end();
	
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging("\
			Msg_UpdateScoreInfo -> ^n^t\
			pPlayerId='%i'^n^t\
			var_frags='%1.f'^n^t\
			m_iDeaths='%i'^n^t\
			m_iTeam='%i'^n^t\",
			pPlayerId, fFrags, iDeaths, iTeam
		);
	}
}

public Stop_WarMup_Toggle()
{
	switch(g_iGameStateCurrent)
	{
		case HALF_1, HALF_2, OVERTIME:
		{
			ToggleMods(WARMUP_STOP);
		}
	}
}

public DoRestarts()
{
	if(--g_iPrepareRestarts >= 0)
	{
		server_cmd("sv_restart 1");
		set_task(1.0, "DoRestarts");
	}
	else
	{		
		switch(g_iGameStateCurrent)
		{
			case HALF_1, HALF_2, OVERTIME:
			{
				// Если это продолжение игры (OverTime)
				if(g_bIsOvertime)
				{
					g_iGameStateCurrent = OVERTIME;

					Do_OverTime();
				}
				
				if(g_iRoundPlayed)
				{
					set_task(1.0, "TASK_RestoreScores");
				}
			}
		}
				
		if(g_iDebugLevel >= D_LEVEL_3)
		{
			Logging("\
				DoRestarts() -> Done 3 Restarts.^n^t\
				g_iGameStateCurrent='%s'", g_szGameStates_Name[g_iGameStateCurrent]
			);
		}
	}
}

public TASK_RestoreScores()
{
	// Возвращаем игрокам их K-D
	PlayerProps_RestoreALL();

	// И счёт команд.
	/* На второй половине игры команды порменялись местами,
		Поэтому сохраняем инверсированно, ОДНАКО, на овертайме
		они снова могут поменяться.
	*/

	if(g_bTeamSwapped)
	{
		rg_update_teamscores(
			.iCtsWins = g_iTeamWins[Team_A],
			.iTsWins = g_iTeamWins[Team_B],
			.bAdd = false
		);
	}
	else
	{
		rg_update_teamscores(
			.iCtsWins = g_iTeamWins[Team_B],
			.iTsWins = g_iTeamWins[Team_A],
			.bAdd = false
		);
	}

	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging("\
			TASK_RestoreScores -> rg_update_teamscores ->^n^t\
			g_iTeamWins[Team_A]='%i'^n^t\
			g_iTeamWins[Team_B]='%i'",
			g_iTeamWins[Team_A], g_iTeamWins[Team_B]
		);
	}
}

stock Init__ReAPI_Hooks()
{
	HC_Types[__SV_DropClient] = RegisterHookChain(
		RH_SV_DropClient,
		"SV_DropClient",
		.post = false
	);

	/*
	HC_Types[__CBasePlayer_AddAccount] = RegisterHookChain(
		RG_CBasePlayer_AddAccount,
		"CBasePlayer_AddAccount",
		.post = false
	);
	*/
	
	HC_Types[__RG_CBasePlayer_Spawn] = RegisterHookChain(
		RG_CBasePlayer_Spawn,
		"CBasePlayer_Spawn",
		.post = true
	);
	
	HC_Types[__CBasePlayer_Killed] = RegisterHookChain(
		RG_CBasePlayer_Killed,
		"CBasePlayer_Killed",
		.post = false
	);
	
	HC_Types[__RG_CSGameRules_GiveC4] = RegisterHookChain(
		RG_CSGameRules_GiveC4,
		"CSGameRules_GiveC4",
		.post = false
	);
	
	HC_Types[__RG_CSGameRules_FPlayerCanRespawn] = RegisterHookChain(
		RG_CSGameRules_FPlayerCanRespawn,
		"CSGameRules_FPlayerCanRespawn",
		.post = false
	);
	
	HC_Types[__RG_RoundEnd] = RegisterHookChain(
		RG_RoundEnd,
		"RoundEnd",
		.post = false
	);
	
	HC_Types[__RG_CSGameRules_RestartRound] = RegisterHookChain(
		RG_CSGameRules_RestartRound,
		"CSGameRules_RestartRound",
		.post = true
	);
	
	HC_Types[__RG_CSGameRules_DeadPlayerWeapons] = RegisterHookChain(
		RG_CSGameRules_DeadPlayerWeapons,
		"CSGameRules_DeadPlayerWeapons",
		.post = false
	);

	RegisterHookChain(
		RG_ShowVGUIMenu,
		"ShowVGUIMenu",
		.post = false
	);
	
	RegisterHookChain(
		RG_CBasePlayer_SetClientUserInfoName,
		"CBasePlayer_SetClientUserInfoName",
		.post = true
	);
	
	RegisterHookChain(
		RG_CBasePlayer_HasRestrictItem,
		"CBasePlayer_HasRestrictItem",
		.post = true
	);
}

public CBasePlayer_HasRestrictItem(const pPlayerId, const ItemID:item, const ItemRestType:type)
{
	if((item < ITEM_NVG)
		? g_bitsRestrict[SECTION_WEAPONS] & (1 << any:item)
		: g_bitsRestrict[SECTION_ITEMS] & (1 << any:(item % ITEM_NVG)))
	{
		if(type == ITEM_TYPE_BUYING) {
			client_print(pPlayerId, print_center, "%L", pPlayerId, "WeaponRestrict");
		}

		// return 1, let's restrict up this item
		SetHookChainReturn(ATYPE_INTEGER, 1);

		return HC_SUPERCEDE;
	}

	return HC_CONTINUE;
}

stock WeaponRestricts_Mode(iMode)
{
	switch(iMode)
	{
		case WARMUP_START:
		{
			g_bitsRestrict[SECTION_WEAPONS] |= (1 << any:ITEM_AWP);
			g_bitsRestrict[SECTION_WEAPONS] |= (1 << any:ITEM_SMOKEGRENADE);
			g_bitsRestrict[SECTION_WEAPONS] |= (1 << any:ITEM_HEGRENADE);
			g_bitsRestrict[SECTION_WEAPONS] |= (1 << any:ITEM_FLASHBANG);
			g_bitsRestrict[SECTION_WEAPONS] |= (1 << any:ITEM_DEFUSEKIT);
			g_bitsRestrict[SECTION_WEAPONS] |= (1 << any:ITEM_SHIELDGUN);
		}

		case WARMUP_STOP:
		{
			g_bitsRestrict[SECTION_WEAPONS] &= ~(1 << any:ITEM_AWP); 
			g_bitsRestrict[SECTION_WEAPONS] &= ~(1 << any:ITEM_SMOKEGRENADE);
			g_bitsRestrict[SECTION_WEAPONS] &= ~(1 << any:ITEM_HEGRENADE);
			g_bitsRestrict[SECTION_WEAPONS] &= ~(1 << any:ITEM_FLASHBANG);
			g_bitsRestrict[SECTION_WEAPONS] &= ~(1 << any:ITEM_DEFUSEKIT);
			g_bitsRestrict[SECTION_WEAPONS] &= ~(1 << any:ITEM_SHIELDGUN);
		}

		default:
		{
			if(g_iDebugLevel >= D_LEVEL_3)
			{
				Logging("WeaponRestricts_Mode -> iMode='%i'", iMode);
			}
		}
	}
}

public CBasePlayer_SetClientUserInfoName(const pPlayerId, infobuffer[], szNewName[])
{
	TrieSetString(
		g_tPlayerData[tName],
		g_szSteamId[pPlayerId],
		szNewName
	);
}

stock HC_Disable_All()
{
	for(new i; i < sizeof(HC_Types); i++)
	{
		if(!HC_Types[i] || i == __SV_DropClient || i == __CBasePlayer_Killed)
		{
			continue;
		}
		
		DisableHookChain(HC_Types[i]);
	}
}

/*
stock ExecCFG(const State_CFG)
{
	new szCfg[256];
	formatex(szCfg, charsmax(szCfg), "%s%s", g_szConfigsPath,  g_szGameConfigs_Name[State_CFG]);
	
	server_cmd("exec %s", szCfg);
	
	// log_amx("ExecCFG -> exec %s", szCfg);
}
*/

/* DEBUG
new stock RewardTypes[RewardType][] = 
{
	"RT_NONE",
	"RT_ROUND_BONUS",
	"RT_PLAYER_RESET",
	"RT_PLAYER_JOIN",
	"RT_PLAYER_SPEC_JOIN",
	"RT_PLAYER_BOUGHT_SOMETHING",
	"RT_HOSTAGE_TOOK",
	"RT_HOSTAGE_RESCUED",
	"RT_HOSTAGE_DAMAGED",
	"RT_HOSTAGE_KILLED",
	"RT_TEAMMATES_KILLED",
	"RT_ENEMY_KILLED",
	"RT_INTO_GAME",
	"RT_VIP_KILLED",
	"RT_VIP_RESCUED_MYSELF"
};
*/

/*
enum _:GAME_CONFIGS {
	CFG__WARMUP_START,
	CFG__WARMUP_END,
	CFG__PREPARE,
	CFG__GAME_STARTED,
	CFG__GAME_FINISHED,
	CFG__GAME_OVERTIME,
	MSG__ALIASES
}

new g_szGameConfigs_Name[GAME_CONFIGS][] = {
	"/AutoMix_System/CFG__Warmup_Start.cfg",
	"/AutoMix_System/CFG__Warmup_End.cfg",
	"/AutoMix_System/CFG__Prepare.cfg",
	"/AutoMix_System/CFG__Game_Start.cfg",
	"/AutoMix_System/CFG__Game_Finished.cfg",
	"/AutoMix_System/CFG__Game_Overtime.cfg",
	"/AutoMix_System/MSG_Aliases.cfg"	
};
*/

stock eTeams: Get_TeamLeader()
{
	if(g_iTeamWins[Team_A] > g_iTeamWins[Team_B])
	{
		return eTeams: Team_A;
	}
	else if(g_iTeamWins[Team_A] < g_iTeamWins[Team_B])
	{
		return eTeams: Team_B;
	}
	else
	{
		return eTeams: NoLeaderTeam;
	}
}

/*
	Подсчёт не зашедших игроков
*/
stock CountingMissingPlayers()
{
	return;
}

public Check__OnWarmupTimeout()
{
	switch(g_iGameStateCurrent)
	{
		case WARMUP:
		{
			/**
				Первая разминка
			*/
			if(g_iRoundPlayed < 1)
			{
				/**
					Если в игре находятся все, но кто-то не нажал !R
				*/
				if(g_iConnectedCount >= g_iGameSlots)
				{
					switch(g_iRoundPlayed)
					{
						case 0: g_iGameStateCurrent = HALF_1;
						case Round_ToSwitch: g_iGameStateCurrent = HALF_2;
					}
					
					if(g_bIsOvertime) {
						g_iGameStateCurrent = OVERTIME;
					}

					Stop_WarMup_Toggle();

					g_iPrepareRestarts = PREPARE_RESTARTS;

					DoRestarts();

					MsgChat__GameForceStart();
				}
				else
				{
					g_iGameStateCurrent = FINISHED;

					CountingMissingPlayers();	
					
					/**
						ЗАВЕРШАЕМ ИГРУ, если игроки не набрались на сервере 
					*/
					ToCompleteTheGame();					
					
					MsgChat__NotEnoughPlayers();
				}
			}
			else
			{
				switch(g_iRoundPlayed)
				{
					case Round_ToSwitch: g_iGameStateCurrent = HALF_2;
				}
				
				if(g_bIsOvertime) {
					g_iGameStateCurrent = OVERTIME;
				}

				Stop_WarMup_Toggle();

				g_iPrepareRestarts = PREPARE_RESTARTS;

				DoRestarts();
			}
		}
		
		default:
		{
			if(g_iDebugLevel >= D_LEVEL_3)
			{
				Logging("\
					Check__OnWarmupTimeout ->^n^t\
					g_iGameStateCurrent='%s'^n^t\
					g_iConnectedCount='%i'^n^t\
					g_iGameSlots='%i'",
					g_szGameStates_Name[g_iGameStateCurrent], g_iConnectedCount, g_iGameSlots
				);
			}
		}
	}
}

/**
	PAUSE
*/
public SetGame_To_Pause()
{
	server_cmd("mp_freezetime %i", g_iFreezeTime + TimeForPAUSE);
}

public RestoreGame()
{
	server_cmd("mp_freezetime %i", g_iFreezeTime);
}
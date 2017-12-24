/* Errors Codes
 Socket API Error:
 #01 - Кол-во полученных SteamId с сервера не равно кол-ву слотов
*/

/* SERVER SETTINGS */

#if defined SMALL_GAME_FOR_LOCAL_TESTS

const Round_ToSwitch = 3;
// const Round_ToWin = 6;
const Round_OverTime = 7;
const Round_Maximum = 16;
const Score_ToWin = 6;

#else

const Round_ToSwitch = 15;
// const Round_ToWin = 16;
const Round_OverTime = 31;
const Round_Maximum = 46;
const Score_ToWin = 16;

#endif

const PREPARE_RESTARTS = 3;
new g_iPrepareRestarts = PREPARE_RESTARTS;

new const g_szPreCmd[][] = {"say ", "say_team ", ""};
new const g_szCtrlChar[][] = {"!","/","\", "."};

new const g_szBlockCommands[][] =  {
	"chooseteam",
	"jointeam",
	"joinclass",
	"votemap",
	"vote"
};

new const g_szCmds_Ready[][] = {"r"};
new const g_szCmds_NotReady[][] = {"n"};
new const g_szCmds_Status[][] = {"s"};
new const g_szCmds_Help[][] = {"h", "help"};
new const g_szCmds_Pause[][] = {"p"};

new g_Cmds_Default[][] = {
	"hostname FastMix.Net",

#if defined SMALL_GAME_FOR_LOCAL_TESTS
	"bot_stop 1",
	"bot_join_after_player 0",
	// "bot_quota 1",
#endif

	// "log on",
	"mp_autoteambalance 0",
	"mp_limitteams 0",
	"mp_autokick 0",
	"mp_consistency 1",
	"mp_c4timer 35",
	"mp_flashlight 0",
	// "mp_fadetoblack 1",
	"mp_forcechasecam 2",
	"mp_forcecamera 3",
	"mp_freezetime 0",
	"mp_footsteps 1",
	"mp_logecho 1",
	"mp_logdetail 3",
	"mp_logfile 1",
	"mp_logmessages 1",
	"mp_playerid 0",
	"mp_timelimit 0",	
	"mp_nadedrops 0",
	"mp_tkpunish 0",
	"sv_airaccelerate 10",
	"sv_allowdownload 0",
	"sv_allowupload 0",
	"sv_alltalk 0",
	"sv_proxies 1",
	"mp_maxmoney 16000",
	"mp_startmoney 16000"
};

new g_Cmds_WarmupStart[][] = {
	"mp_buytime -1",
	"mp_forcerespawn 1",
	"mp_freezetime 2",
	"mp_friendlyfire 0",
	"mp_round_infinite bcdefg",
	"mp_roundrespawn_time 0",
	
	"mp_roundtime 5",
	"mp_startmoney 16000",
	
	"sv_alltalk 1"
};

new g_Cmds_WarmupStop[][] = {
	"mp_buytime 0.25",
	"mp_forcerespawn 0",
	"mp_freezetime 10",
	"mp_friendlyfire 1",
	"mp_round_infinite 0",
	"mp_roundrespawn_time 5",
	"mp_roundtime 1.75",
	"mp_startmoney 800",
	
	"sv_alltalk 0"
};

new g_Cmds_WarmupBtwGame[][] = {
	"mp_buytime -1",
	"mp_forcerespawn 1",
	"mp_freezetime 0",
	"mp_friendlyfire 0",
	"mp_round_infinite bcdefg",
	"mp_roundrespawn_time 0",
	
	"mp_roundtime 3",
	
	"mp_startmoney 16000",
	
	"sv_alltalk 1"
};

new g_Cmds_OverTime[][] =  {
	"mp_startmoney 10000"
};

new g_Cmds_GameEnd[][] = {
	"mp_freezetime 30",
	"mp_roundtime 1",
	"mp_round_infinite bcdefg",
	"mp_startmoney 0"
};

/* SERVER SETTINGS */
enum _:CMDS_TYPE
{
	READY,
	STATUS
}

/* Надо сделать одной функцией. Но позже.
new g_szCommands_Ready[][]	= {"r","ready"};
new g_szCommands_Status[][]	= {"s","status"};
new g_szCmmands[CMDS_TYPE][] = 
{
	g_szCommands_Ready,
	g_szCommands_Status
}

new g_szCMDSFuncs[][] = 
{
	"Player__Say_Ready",
	"Player__Say_Status",
	"Player__Say_Help",
	"Player__Say_Pause"
}
*/

stock Register_PlayerCommands()
{
	Register__Cmd_Ready();
	Register__Cmd_NotReady();
	Register__Cmd_Status();
	Register__Cmd_Help();
	Register__Cmd_Pause();
	
	Register__Block_Commands();
}

stock Register__Cmds()
{
	new iLen = 0;
	new szMsg[512];
	
	for(new i; i <= charsmax(g_szPreCmd); i++)
	{
		for(new k; k <= charsmax(g_szCtrlChar); k++)
		{
			for(new j; j <= charsmax(g_szCmds_Ready); j++)
			{
				new szCmd[16];

				formatex(szCmd, charsmax(szCmd),
					"%s%s%s",
					g_szPreCmd[i],
					g_szCtrlChar[k],
					g_szCmds_Ready[j]
				);
	
				register_clcmd(szCmd, "Player__Say_Ready");

				if(g_iDebugLevel >= D_LEVEL_3)
				{
					if(!iLen)
					{
						iLen = formatex(szMsg, charsmax(szMsg), "Register__Cmd_Ready ->^n^t");
					}
	
					iLen += formatex(szMsg[iLen], charsmax(szMsg) - iLen, "► Reg: '%s'^n^t", szCmd);
				}
			}
		}
	}
	
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging(szMsg);
	}
}

stock Register__Cmd_Ready()
{
	new iLen = 0;
	new szMsg[512];
	
	for(new i; i <= charsmax(g_szPreCmd); i++)
	{
		for(new k; k <= charsmax(g_szCtrlChar); k++)
		{
			for(new j; j <= charsmax(g_szCmds_Ready); j++)
			{
				new szCmd[16];

				formatex(szCmd, charsmax(szCmd),
					"%s%s%s",
					g_szPreCmd[i],
					g_szCtrlChar[k],
					g_szCmds_Ready[j]
				);
	
				register_clcmd(szCmd, "Player__Say_Ready");

				if(g_iDebugLevel >= D_LEVEL_3)
				{
					if(!iLen)
					{
						iLen = formatex(szMsg, charsmax(szMsg), "Register__Cmd_Ready ->^n^t");
					}
	
					iLen += formatex(szMsg[iLen], charsmax(szMsg) - iLen, "► Reg: '%s'^n^t", szCmd);
				}
			}
		}
	}
	
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging(szMsg);
	}
}

stock Register__Cmd_NotReady()
{
	new iLen = 0;
	new szMsg[512];
	
	for(new i; i <= charsmax(g_szPreCmd); i++)
	{
		for(new k; k <= charsmax(g_szCtrlChar); k++)
		{
			for(new j; j <= charsmax(g_szCmds_NotReady); j++)
			{
				new szCmd[16];

				formatex(szCmd, charsmax(szCmd),
					"%s%s%s",
					g_szPreCmd[i],
					g_szCtrlChar[k],
					g_szCmds_NotReady[j]
				);
	
				register_clcmd(szCmd, "Player__Say_NotReady");

				if(g_iDebugLevel >= D_LEVEL_3)
				{
					if(!iLen)
					{
						iLen = formatex(szMsg, charsmax(szMsg), "Register__Cmd_NotReady ->^n^t");
					}
	
					iLen += formatex(szMsg[iLen], charsmax(szMsg) - iLen, "► Reg: '%s'^n^t", szCmd);
				}
			}
		}
	}
	
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging(szMsg);
	}
}

stock Register__Cmd_Status()
{
	new iLen = 0;
	new szMsg[512];
	
	for(new i; i <= charsmax(g_szPreCmd); i++)
	{
		for(new k; k <= charsmax(g_szCtrlChar); k++)
		{
			for(new j; j <= charsmax(g_szCmds_Status); j++)
			{
				new szCmd[16];

				formatex(szCmd, charsmax(szCmd),
					"%s%s%s",
					g_szPreCmd[i],
					g_szCtrlChar[k],
					g_szCmds_Status[j]
				);
	
				register_clcmd(szCmd, "Player__Say_Status");
				
				if(g_iDebugLevel >= D_LEVEL_3)
				{
					if(!iLen)
					{
						iLen = formatex(szMsg, charsmax(szMsg), "Register__Cmd_Status ->^n^t");
					}
					
					iLen += formatex(szMsg[iLen], charsmax(szMsg) - iLen, "► Reg: '%s'^n^t", szCmd);
				}
			}
		}
	}
	
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging(szMsg);
	}
}

stock Register__Cmd_Help()
{
	for(new i; i <= charsmax(g_szPreCmd); i++)
	{
		for(new k; k <= charsmax(g_szCtrlChar); k++)
		{
			for(new j; j <= charsmax(g_szCmds_Help); j++)
			{
				new szCmd[16];

				formatex(szCmd, charsmax(szCmd),
					"%s%s%s",
					g_szPreCmd[i],
					g_szCtrlChar[k],
					g_szCmds_Help[j]
				);
	
				register_clcmd(szCmd, "Player__Say_Help");

				// log_amx("Register__Cmd_Help -> szCmd='%s'", szCmd);
			}
		}
	}
}

stock Register__Cmd_Pause()
{
	for(new i; i <= charsmax(g_szPreCmd); i++)
	{
		for(new k; k <= charsmax(g_szCtrlChar); k++)
		{
			for(new j; j <= charsmax(g_szCmds_Pause); j++)
			{
				new szCmd[16];

				formatex(szCmd, charsmax(szCmd),
					"%s%s%s",
					g_szPreCmd[i],
					g_szCtrlChar[k],
					g_szCmds_Pause[j]
				);
	
				register_clcmd(szCmd, "Player__Say_Pause");
				
				// log_amx("Register__Cmd_Pause -> szCmd='%s'", szCmd);
			}
		}
	}
}

stock Register__Block_Commands()
{
	for(new i; i < sizeof g_szBlockCommands; i++)
	{
		register_clcmd(g_szBlockCommands[i], "BlockCommand");

		// log_amx("Register__Block_Commands -> g_szBlockCommands[i]='%s'", g_szBlockCommands[i]);
	}
}

public Player__Say_Ready(pPlayerId)
{
	if(g_iGameStateCurrent == WARMUP)
	{

		// Build_NickNames();

		if(!g_bVoted[pPlayerId])
		{
			g_bVoted[pPlayerId] = true;

			if(++g_iVotedPlayersNum > g_iGameSlots)
			{
				g_iVotedPlayersNum = g_iGameSlots;
			}
		
			new szSteamId[MAX_AUTHID_LENGTH], szName[MAX_NAME_LENGTH];
	
			copy(szSteamId, charsmax(szSteamId), g_szSteamId[pPlayerId]);
			TrieGetString(g_tPlayerData[tName], szSteamId, szName, charsmax(szName));
	
			client_print(0, print_chat, "%L",
				LANG_PLAYER, "CMD_Ready",
				szName,
				LANG_PLAYER, "READY",
				g_iVotedPlayersNum,
				g_iGameSlots
			);
		}
		else
		{
			client_print(pPlayerId, print_chat, "%L", LANG_PLAYER, "ALREADY_READY");
		}
		
		Check__Counts_Ready();
	}
}

enum any: Teams_s { TT, CT }
enum any: PlayersInOneTeam_s { P1, P2, P3, P4, P5 }
enum any: Names_s { szName1[32] }

#if defined SMALL_GAME_FOR_LOCAL_TESTS
new const g_szNickNames[10][Names_s] = 
{
	{"Player #1"},
	{"Player #2"},
	{"Player #3"},
	{"Player #4"},
	{"Player #5"},
	{"Player #6"},
	{"Player #7"},
	{"Player #8"},
	{"Player #9"},
	{"Player #10"}
};

public Fake__AddNicknames()
{
	new szLog[512], iLen;
	
	for(new pPlayerId; pPlayerId <= MaxClients; pPlayerId++)
	{
		if(TrieKeyExists(g_tPlayerData[tTeam], g_szSteamId[pPlayerId]))
		{
			TrieSetString(g_tPlayerData[tName], g_szSteamId[pPlayerId], g_szNickNames[pPlayerId]);
			
			iLen += formatex(szLog[iLen], charsmax(szLog) - iLen, "id='%i', Name='%s', SteamId='%s'^n^t", pPlayerId, g_szNickNames[pPlayerId], g_szSteamId[pPlayerId]);
		}
	}
	
	// Logging("Fake__AddNicknames ->^n^t%s", szLog);
	log_amx("Fake__AddNicknames ->^n^t%s", szLog);
}
#endif

stock Get_NickNames_NotReady()
{	
	new szNicknamesMsg[190],
		iLen,
		szName[32],
		iNicksCount;

	for(new pPlayerId = 1; pPlayerId <= MaxClients; pPlayerId++)
	{
		if(g_bVoted[pPlayerId])
		{
			continue;
		}

		if(TrieKeyExists(g_tPlayerData[tTeam], g_szSteamId[pPlayerId]))
		{
			TrieGetString(g_tPlayerData[tName], g_szSteamId[pPlayerId], szName, charsmax(szName));

			iLen += formatex(szNicknamesMsg[iLen], charsmax(szNicknamesMsg) - iLen, " %s,", szName);

			//log_amx("Get_NickNames_NotReady -> pPlayerId='%i', szNicknamesMsg:'%s'", pPlayerId, szName);
			
			if(++iNicksCount > 3)
			{
				break;
			}
		}
	}
	
	szNicknamesMsg[iLen - 1] = '.';

	//log_amx("'%s'", szNicknamesMsg);

	return szNicknamesMsg;	
}

public Player__Say_NotReady(pPlayerId)
{
	if(g_iGameStateCurrent == WARMUP)
	{
		if(g_bVoted[pPlayerId])
		{
			g_bVoted[pPlayerId] = false;

			if(--g_iVotedPlayersNum < 0)
			{
				g_iVotedPlayersNum = 0;
			}
		
			new szSteamId[MAX_AUTHID_LENGTH], szName[MAX_NAME_LENGTH];
	
			copy(szSteamId, charsmax(szSteamId), g_szSteamId[pPlayerId]);
			TrieGetString(g_tPlayerData[tName], szSteamId, szName, charsmax(szName));
	
			client_print(0, print_chat, "%L",
				LANG_PLAYER, "CMD_Ready",
				szName,
				LANG_PLAYER, "NOTREADY",
				g_iVotedPlayersNum,
				g_iGameSlots
			);
		}
	}
}

public Player__Say_Status(pPlayerId)
{	
	switch(g_iGameStateCurrent)
	{
		case WARMUP:
		{
			MsgChat__StatusOnWarmup();
		}

		case HALF_1, HALF_2, OVERTIME:
		{
			MsgChat__Status();
		}
/*
		default:
		{
			if(g_iDebugLevel >= D_LEVEL_3)
			{
				new szMsg[192];
				
				formatex(szMsg, charsmax(szMsg), "Player__Say_Status -> g_iGameStateCurrent='%i'", g_iGameStateCurrent);
				SaveLogFile(szMsg);
			}
		}
*/
	}
}

public Player__Say_Help(pPlayerId)
{
	return;

	// show_motd(pPlayerId, "http://FastMix.net/MOTD_help", "HELP");
}

public Player__Say_Pause(pPlayerId)
{
	/**
		Если режим разминки или игра окончена
	*/
	switch(g_iGameStateCurrent)
	{
		case WARMUP, FINISHED:
		{
			client_print(pPlayerId, print_chat, "%L", pPlayerId, "PauseAllowedOnlyInGame");
			return;
		}
	}
	
	/**
		Если время запуска последнего голосования (+ 5min) больше текущего времени
	*/
	if((g_fLastPauseVote + TimeForPAUSE_Limit) > get_gametime())
	{
		client_print(pPlayerId, print_chat, "%L", pPlayerId, "DontPauseFlood");
		return;
	}
	
	/**
		Если Игра уже в паузе или будет поставлена на паузу
	*/
	if(g_bIsPause)
	{
		client_print(pPlayerId, print_chat, "%L", pPlayerId, "PauseAlreadyEnabled");
		return;
	}
	
	/**
		Если меню голосования за паузу уже запущено 
	*/
	if(g_iVotedPlayersForPause)
	{
		client_print(pPlayerId, print_chat, "%L", pPlayerId, "VoteForPauseAlreadyStarted");
		return;
	}

	++g_iVotedPlayersForPause;

	g_iVotePauseRequesterId = pPlayerId;

	if(g_iGameSlots > 3)
	{
		new iTeamRequester = get_member(pPlayerId, m_iTeam);
		
		for(new i = 1; i <= MaxClients; i++)
		{
			if(!is_user_connected(i) || is_user_hltv(i) || i == pPlayerId)
			{
				continue;
			}

			if(iTeamRequester == get_member(i, m_iTeam))
			{
				ShowMenu_Pause(i);
			}
			else
			{
				client_print(i, print_chat, "%L", i, "OpponentTeamNeedAPause");
			}
		}

		set_task(TimeForPauseVote, "Check__PauseVoteCounts");
		
		client_print(pPlayerId, print_chat, "%L", pPlayerId, "VoteForPauseStarted");
	}
	else
	{
		client_print(0, print_chat, "%L", LANG_PLAYER, "WillBePause");
		
		g_bIsPause = true;
		g_iVotedPlayersForPause = 0;
	}
	
	g_fLastPauseVote = get_gametime();
}

public BlockCommand()
{
	return PLUGIN_HANDLED;
}
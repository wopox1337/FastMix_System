enum _:
{
    Plural,			// минут
    Singular,		// минуты
    Nominative	// минута
};

new stock g_szMinutesWordEnding[][] = {
	"MinutePlural",
	"MinuteSingular",
	"MinuteNominative"
};

/* Вычисление окончания слова в зависимости от множ числа. */
stock get_numerical_noun_form(iNum)
{
    if (iNum > 10 && ((iNum % 100) / 10) == 1)
        return Plural;                   

    switch (iNum % 10)
    {
        case 1: return Nominative;       
        case 2, 3, 4: return Singular;   
    }

    return Plural;                       
}

/** Показывает игрокам сообщение о начале игры 
	НЕ ИСПОЛЬЗУЕТСЯ
*/
stock Msg__Hud_GameStarted()
{
	set_hudmessage(
		.red = 0,
		.green = 255,
		.blue = 0,
		.x = -1.0,
		.y = 0.4,
		.effects = 0,
		.fxtime = 0.0,
		.holdtime = 5.0
		// .fadeintime = 0.1,
		// .fadeouttime = 0.2,
		// .channel = 4
	);

	show_hudmessage(0, "Fight started!^nGood luck & Have fun!");
}

/* Показ игрокам Денег тиммейтов */
stock MsgHud_TeammatesMoney()
{
	set_hudmessage(
		.red = 0,
		.green = 200,
		.blue = 0,
		.x = 0.05,
		.y = -1.0,
		.effects = 0,
		.holdtime = g_iFreezeTime / 1.1
	);
	
	for(new iDestPlayer = 1; iDestPlayer <= MaxClients; iDestPlayer++)
	{
		if(!is_user_connected(iDestPlayer))
		{
			continue;
		}

		new szMsg[512], iLen;

		for(new pPlayerId = 1; pPlayerId <= MaxClients; pPlayerId++)
		{
			if(!is_user_connected(pPlayerId) || get_member(iDestPlayer, m_iTeam) != get_member(pPlayerId, m_iTeam))
			{
				continue;
			}

			if(!iLen)
			{
				iLen = formatex(szMsg[iLen], charsmax(szMsg), "%s: %i$^n", g_szName[pPlayerId], get_member(pPlayerId, m_iAccount));
			}
			else
			{
				iLen += formatex(szMsg[iLen], charsmax(szMsg) - iLen, "%s: %i$^n", g_szName[pPlayerId], get_member(pPlayerId, m_iAccount));
			}
		}

		show_hudmessage(iDestPlayer, "%s", szMsg);
	}
}

stock MsgChat_TeammatesMoney()
{
	for(new pPlayerId = 1; pPlayerId <= MaxClients; pPlayerId++)
	{
		if(!is_user_connected(pPlayerId) || is_user_hltv(pPlayerId))
		{
			continue;
		}

		new szMoney[10];	
		formatex(szMoney, charsmax(szMoney),"%i$", get_member(pPlayerId, m_iAccount));
		engclient_cmd(pPlayerId, "say_team", szMoney);
	}
}

/* Показывает HLTV клиенту переодически информацию.
	Включает: Дату и время.
*/
public MsgHud__HLTVInformer(pHLTVid)
{
	pHLTVid -= TASK_ShowMsgHLTV;
	
	static szTime[32];
	get_time("%m/%d/%Y^n%H:%M:%S", szTime, charsmax(szTime));
	
	set_hudmessage(
		.red = 0,
		.green = 255,
		.blue = 0,
		.x = -1.0,
		.y = 0.2,
		.effects = 0,
		.fxtime = 0.0,
		.holdtime = 1.2,
		.fadeintime = 0.0,
		.fadeouttime = 0.0,
		.channel = 4
	);

	show_hudmessage(pHLTVid, szTime);
}

/* Показывает HLTV клиенту сообщение приветствия.
	Включает: Game id, IP, HostName.
*/
stock MsgHud__HLTVFirst(pHLTVId)
{
	set_hudmessage(
		.red = 0,
		.green = 255,
		.blue = 0,
		.x = -1.0,
		.y = 0.4,
		.effects = 1,
		.fxtime = 5.0,
		.holdtime = 10.0,
		.fadeintime = 0.0,
		.fadeouttime = 0.0,
		.channel = 3
	);

	show_hudmessage(pHLTVId, "\
		Game id: %i^n^nIP: %s^nHostname: %s",
		g_iGameId,
		g_szServerIP,
		g_szServerName
	);
}

stock MsgChat__Overtime()
{
	client_print(0, print_chat, "%L",
		LANG_PLAYER, "WillBeOVERTIME",
		g_iTeamWins[Team_A],
		g_iTeamWins[Team_B]
	);
}

stock MsgChat__Status()
{
	client_print(0, print_chat, "%L",
		LANG_PLAYER, "CMD_StatusInGame",
		LANG_PLAYER, g_szGameStates_Name[ g_iGameStateCurrent ],
		g_iTeamWins[Team_A],
		g_iTeamWins[Team_B],
		LANG_PLAYER, g_szTeamsNames[ Get_TeamLeader() ]
	);
}

stock MsgChat__StatusOnWarmup()
{
	client_print(0, print_chat, "%L",
		LANG_PLAYER, "CMD_Status",
		LANG_PLAYER, g_szGameStates_Name[ g_iGameStateCurrent ],
		g_iVotedPlayersNum,
		g_iGameSlots
	);

	client_print(0, print_chat, "%L%s", LANG_PLAYER, "NotReadyList", Get_NickNames_NotReady());
}

stock MsgChat__NotEnoughPlayers()
{
	client_print(0, print_chat, "%L", LANG_PLAYER, "NotEnoughPlayersToStart", g_iConnectedCount, g_iGameSlots);
}

stock MsgChat__GameForceStart()
{
	client_print(0, print_chat, "%L", LANG_PLAYER, "GameForceStart", g_iConnectedCount, g_iGameSlots);
}

public MsgChat__NotReadyPlayers()
{
	if(g_iGameStateCurrent == WARMUP)
	{
		client_print(0, print_chat, "%L%s", LANG_PLAYER, "NotReadyList", Get_NickNames_NotReady());
	}
	else
	{
		remove_task(TASK_ShowNotReadyPlayersList);
	}
}
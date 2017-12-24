const bitsKeys = MENU_KEY_1 | MENU_KEY_2;

public RegisterMenu()
{
	register_menu("Menu_VoteToPause", bitsKeys, "MenuHandler_Pause");
}

public ShowMenu_Pause(pPlayerId)
{
	new szMenu[512],
		iLen,
		iKeys = MENU_KEY_1 | MENU_KEY_2,
		szName[MAX_NAME_LENGTH];

	TrieGetString(g_tPlayerData[tName], g_szSteamId[g_iVotePauseRequesterId], szName, charsmax(szName));

	iLen = formatex(szMenu[iLen], charsmax(szMenu), "\w%L^n", pPlayerId, "NeedPause", szName);

	iLen += formatex(szMenu[iLen], charsmax(szMenu) - iLen, "\r1. \w%L^n", pPlayerId, "YES");
	iLen += formatex(szMenu[iLen], charsmax(szMenu) - iLen, "\r2. \w%L^n", pPlayerId, "NO");

	show_menu(pPlayerId, iKeys, szMenu, floatround(TimeForPauseVote), "Menu_VoteToPause");

	return PLUGIN_HANDLED;
}

public MenuHandler_Pause(pPlayerId, key)
{
	switch(++key)
	{
		case 1: ++g_iVotedPlayersForPause;
		case 2: show_menu(pPlayerId, 0, "^n", 1);

		default:
		{
			if(g_iDebugLevel >= D_LEVEL_3)
			{
				Logging("MenuHandler_Pause -> pPlayerId='%i', key='%i'", pPlayerId, key);
			}
		}
	}
}

public Check__PauseVoteCounts()
{
	show_menu(0, 0, "^n", 1);

	new NumVoteSlots = g_iGameSlots / 2;
	new NumCheckVote = 0;

	if(NumVoteSlots == 5)
	{
		NumCheckVote = 3;
	}
	else if(NumVoteSlots == 4 || NumVoteSlots == 3)
	{
		NumCheckVote = 2;
	}
	else if(NumVoteSlots == 2)
	{
		NumCheckVote = 1;
	}

	if(g_iVotedPlayersForPause <= NumCheckVote)
	{
		client_print(0, print_chat, "%L", LANG_PLAYER, "PauseCanceled");
	}
	else
	{
		g_fLastPauseVote = get_gametime();
		g_bIsPause = true;

		client_print(0, print_chat, "%L", LANG_PLAYER, "WillBePause");
	}

	g_iVotedPlayersForPause = 0;
}
public client_authorized(pPlayerId)
{
	g_bVoted[pPlayerId] = false;

	OnPlayer_CatchInfo(pPlayerId);
}

public client_putinserver(pPlayerId)
{
	if(g_iDebugLevel >= D_LEVEL_2 && is_user_bot(pPlayerId))
	{
		return;
	}
	
	if(is_user_hltv(pPlayerId))
	{
		MsgHud__HLTVFirst(pPlayerId);
	
		new iTask = pPlayerId + TASK_ShowMsgHLTV;
		remove_task(iTask);

		set_task(MsgTime_HLTV, "MsgHud__HLTVInformer", .id = iTask, .flags = "b");
		
		return;
	}

	if(++g_iConnectedCount > g_iGameSlots)
	{
		g_iConnectedCount = g_iGameSlots;
	}

	// log_amx("g_iConnectedCount = %i", g_iConnectedCount);

	new szSteamId[MAX_AUTHID_LENGTH];
	copy(szSteamId, charsmax(szSteamId), g_szSteamId[pPlayerId]);

	if(TrieKeyExists(g_tPlayerData[tTeam], szSteamId))
	{
		new iTeam;

		TrieGetCell(g_tPlayerData[tTeam], szSteamId, iTeam);

		if(g_bTeamSwapped)
		{
			rg_join_team(pPlayerId, (TeamName:iTeam == TEAM_TERRORIST) ?  TEAM_CT : TEAM_TERRORIST);
		}
		else
		{
			rg_join_team(pPlayerId, (TeamName:iTeam == TEAM_CT) ?  TEAM_CT : TEAM_TERRORIST);
		}

		switch(g_iGameStateCurrent)
		{
			case HALF_1, HALF_2, OVERTIME:
			{
				PlayerProps_Restore(pPlayerId);
			}
		}
		
		if(g_iDebugLevel >= D_LEVEL_3)
		{
			Logging("\
				client_putinserver ->^n^t\
				g_iConnectedCount='%i'^n^t\
				pPlayerId='%i'^n^t\
				szSteamId='%s'^n^t\
				iTeam='%i'^n^t",
				g_iConnectedCount, pPlayerId, szSteamId, iTeam
			);
		}
	}
	else
	{
		server_cmd("\
			kick #%d ^"%L^"",
			get_user_userid(pPlayerId),
			pPlayerId, "KICK_NotYouGame",
			g_iGameId
		);
		
		return;
	}
	
	remove_task(TASK_ShutdownServer_Force);

	set_task(3.0, "Cmd_SendVoice", TASK_PlayerVoice + pPlayerId);
}

public SV_DropClient(const pPlayerId, bool:crash, const szReason[])
{	
	if(is_user_hltv(pPlayerId))
	{
		remove_task(TASK_ShowMsgHLTV + pPlayerId);
	}
	else
	{
		remove_task(TASK_PlayerVoice + pPlayerId);

		if(is_user_connected(pPlayerId))
		{
			switch(g_iGameStateCurrent)
			{
				case HALF_1, HALF_2, OVERTIME:
				{
					PlayerProps_Save(pPlayerId);
				}
			}

			OnPlayer_ResetData(pPlayerId);
		}

		if(TrieKeyExists(g_tPlayerData[tTeam], g_szSteamId[pPlayerId]))
		{
			TrieSetCell(g_tPlayerData[tPing], g_szSteamId[pPlayerId], g_fPlayerPingEMA[pPlayerId]);
		}
		
		g_fPlayerPingEMA[pPlayerId] = 1.0;
		g_iPlayerCheckTimes[pPlayerId] = 0;
		
		if(g_iConnectedCount <= 0)
		{
			remove_task(TASK_ShutdownServer_Force);
			set_task(35.0, "ShutdownServer_Force", TASK_ShutdownServer_Force);
		}
	}
	
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		ShowPlayerLog(pPlayerId);
	}
}

stock OnPlayer_CatchInfo(const pPlayerId)
{
	get_user_authid(pPlayerId, g_szSteamId[pPlayerId], MAX_AUTHID_LENGTH - 1);
	
	get_user_ip(pPlayerId, g_szIPAddress[pPlayerId], MAX_IP_LENGTH - 1, .without_port = true);
	
	new szName[MAX_NAME_LENGTH];
	get_user_name(pPlayerId, szName, MAX_NAME_LENGTH - 1);

	TrieSetString(g_tPlayerData[tName], g_szSteamId[pPlayerId], szName);
	
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging("\
			OnPlayer_CatchInfo -> ^n^t\
			pPlayerId='%i' ^n^t\
			g_szSteamId='%s' ^n^t\
			g_szIPAddress='%s' ^n^t\
			szName='%s'^n^t",
			pPlayerId, g_szSteamId[pPlayerId], g_szIPAddress[pPlayerId], szName
		);
	}
}

public Cmd_SendVoice(id)
{
	id -= TASK_PlayerVoice;

	if(is_user_connected(id))
	{
		message_begin(MSG_ONE, SVC_VOICEINIT, .player = id);
		write_string("voice_speex");
		write_byte(5);
		message_end();
	}
}
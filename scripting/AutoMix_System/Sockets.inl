new const szBase[] = "76561197960265728";

stock my_strval(const iNum)
{
	return '0' + ((iNum >= 0 && iNum <= 9) ? iNum : 0);
}

stock my_intval(cNum)
{
	return (cNum >= '0' && cNum <= '9') ? (cNum - '0') : 0;
}

stock getSteam2(const szSteam64[], szSteam2[], iLen)
{
	new iBorrow = 0;
	new szSteam[18];
	new szAccount[18];
	new iY = 0;
	new iZ = 0;
	new iTemp = 0;
	
	arrayset(szAccount, '0', charsmax(szAccount));
	copy(szSteam, charsmax(szSteam), szSteam64);
	
	if (my_intval(szSteam[16]) % 2 == 1)
	{
		iY = 1;
		szSteam[16] = my_strval(my_intval(szSteam[16]) - 1);
	}
	
	for (new k = 16; k >= 0; k--)
	{
		if (iBorrow > 0)
		{
			iTemp = my_intval(szSteam[k]) - 1;
			
			if (iTemp >= my_intval(szBase[k]))
			{
				iBorrow = 0;
				szAccount[k] = my_strval(iTemp - my_intval(szBase[k]));
			}
			else
			{
				iBorrow = 1;
				szAccount[k] = my_strval((iTemp + 10) - my_intval(szBase[k]));
			}
		}
		else
		{
			if (my_intval(szSteam[k]) >= my_intval(szBase[k]))
			{
				iBorrow = 0;
				szAccount[k] = my_strval(my_intval(szSteam[k]) - my_intval(szBase[k]));
			}
			else
			{
				iBorrow = 1;
				szAccount[k] = my_strval((my_intval(szSteam[k]) + 10) - my_intval(szBase[k]));
			}
		}
	}
	
	iZ = str_to_num(szAccount);
	iZ /= 2;
	
	formatex(szSteam2, iLen, "STEAM_0:%d:%d", iY, iZ);
}

stock getSteam64(const szSteam2[], szSteam64[18])
{
	new iCarry = 0;
	new szAccount[18];
	new iTemp = 0;
	
	copy(szSteam64, charsmax(szSteam64), szBase);
	formatex(szAccount, charsmax(szAccount), "%s", szSteam2[10]);
	formatex(szAccount, charsmax(szAccount), "%017d", str_to_num(szAccount));
	
	szSteam64[16] = my_strval(my_intval(szSteam64[16]) + my_intval(szSteam2[8]));
	
	for (new j = 0; j < 2; j++)
	{
		for (new k = 16; k >= 0; k--)
		{
			if (iCarry > 0)
			{
				iTemp = my_intval(szSteam64[k - iCarry + 1]) + 1;
				
				if (iTemp > 9)
				{
					iTemp -= 10;
					szSteam64[k - iCarry + 1] = my_strval(iTemp);
					iCarry += 1;
				}
				else
				{
					szSteam64[k - iCarry + 1] = my_strval(iTemp);
					iCarry = 0;
				}
				
				k++;
			}
			else
			{
				iTemp = my_intval(szSteam64[k]) + my_intval(szAccount[k]);
				
				if (iTemp > 9)
				{
					iCarry = 1;
					iTemp -= 10;
				}
				
				szSteam64[k] = my_strval(iTemp);
			}
		}
	}
}

public Init__Sql_Cvar()
{
	g_Sql_Cvar[Sql_Host] = register_cvar("fastmix_sql_host", "127.0.0.1");
	g_Sql_Cvar[Sql_User] = register_cvar("fastmix_sql_user", "user_fastmix");
	g_Sql_Cvar[Sql_Pass] = register_cvar("fastmix_sql_pass", "mjubWS33Q7OeDHpi");
	g_Sql_Cvar[Sql_Db] = register_cvar("fastmix_sql_db", "bd_fastmix");
	g_Sql_Cvar[Sql_Table] = register_cvar("fastmix_sql_table", "users");
	g_Sql_Cvar[Sql_Driver] = register_cvar("fastmix_sql_driver", "mysql");
	g_Sql_Cvar[Sql_Max_Error] = register_cvar("fastmix_sql_max_error", "3");
}

public Init__Sql_Db()
{
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging("Init__Sql_Db -> game_id [%d]", g_iGameId);
	}

	new db_host[64], db_user[64], db_pass[64], db_name[32], db_table[32], db_driver[10];
	
	get_pcvar_string(g_Sql_Cvar[Sql_Host], db_host, charsmax(db_host));
	get_pcvar_string(g_Sql_Cvar[Sql_User], db_user, charsmax(db_user));
	get_pcvar_string(g_Sql_Cvar[Sql_Pass], db_pass, charsmax(db_pass));
	get_pcvar_string(g_Sql_Cvar[Sql_Db], db_name, charsmax(db_name));
	get_pcvar_string(g_Sql_Cvar[Sql_Table], db_table, charsmax(db_table));
	get_pcvar_string(g_Sql_Cvar[Sql_Driver], db_driver, charsmax(db_driver));
	
	SQL_SetAffinity(db_driver);

	g_Sql = SQL_MakeDbTuple(db_host, db_user, db_pass, db_name, 3);
	
	if(g_Sql == Empty_Handle)
	{
		Logging("Failed to initialize database | game_id [%d]", g_iGameId);
		
		/* ����� 15 ������ ������ ���������� */
		set_task(15.0, "ShutdownServer_Force");

		return;
	}
	
	SQL_SetCharset(g_Sql, "utf8");

	new query[512];

	formatex(query, charsmax(query), "\
		SELECT aa.id, \
		aa.name, \
		CONVERT(aa.steamid, BINARY) steamid, \
		aa.is_admin, \
		aa.ready, \
		aa.game_id, \
		aa.status, \
		aa.hash, \
		aa.team, \
		aa.time \
		FROM %s aa \
		WHERE aa.game_id = '%d' AND aa.ready = 1 AND (aa.team = 1 OR aa.team = 2)",
		db_table, g_iGameId
	);

	SQL_ThreadQuery(g_Sql, "Sql_Handler", query);
}

public Sql_Handler(failstate, Handle:query, const error[], error_num, const data[], size, Float:queuetime)
{
	if(g_Sql_Error_Count >= get_pcvar_num(g_Sql_Cvar[Sql_Max_Error]))
	{
		if(g_iDebugLevel >= D_LEVEL_1)
		{
			Logging("Sql error max | game_id [%d]", g_iGameId);
		}

		return PLUGIN_HANDLED;
	}
	
	switch(failstate)
	{
		case TQUERY_CONNECT_FAILED:
		{
			if(g_iDebugLevel >= D_LEVEL_1)
			{
				new szMsg[192];
				
				formatex(szMsg, charsmax(szMsg), "\
					Sql_Handler ->^n^t\
					Sql connection failed | game_id [%d]^n^t\
					[ %d ] %s^n^t\
					Query state: %d^n^t",
					g_iGameId,
					error_num, error,
					data[1]
				);
				
				Logging(szMsg);
			}
			
			g_Sql_Error_Count += 1;
			
			return PLUGIN_CONTINUE;
		}

		case TQUERY_QUERY_FAILED:
		{			
			if(g_iDebugLevel >= D_LEVEL_1)
			{
				new szMsg[192];
				
				formatex(szMsg, charsmax(szMsg), "\
					Sql_Handler ->^n^t\
					Sql query failed | game_id [%d]^n^t\
					[ %d ] %s^n^t\
					Query state: %d^n^t",
					g_iGameId,
					error_num, error,
					data[1]
				);
				
				Logging(szMsg);
			}
			
			g_Sql_Error_Count += 1;
			
			return PLUGIN_CONTINUE;
		}
	}
	
	if(SQL_NumResults(query) > 0)
	{
		new qcolId = SQL_FieldNameToNum(query, "id");
		new qcolName = SQL_FieldNameToNum(query, "name");
		new qcolSteamid = SQL_FieldNameToNum(query, "steamid");
		new qcolTeam = SQL_FieldNameToNum(query, "team");

		new iNum;
		new UserId;
		new Data_Name[64];
		new Data_Team[10];
		new Data_Steam[32];
		new Convert_Steam[32];
		
		while (SQL_MoreResults(query))
		{
			iNum += 1;

			setc(Data_Name, sizeof Data_Name, 0);
			setc(Data_Steam, sizeof Data_Steam, 0);
			setc(Convert_Steam, sizeof Convert_Steam, 0);
			setc(Data_Team, sizeof Data_Team, 0);

			UserId = SQL_ReadResult(query, qcolId);

			SQL_ReadResult(query, qcolName, Data_Name, charsmax(Data_Name));

			SQL_ReadResult(query, qcolSteamid, Data_Steam, charsmax(Data_Steam));

			SQL_ReadResult(query, qcolTeam, Data_Team, charsmax(Data_Team));

			getSteam2(Data_Steam, Convert_Steam, charsmax(Convert_Steam));

			TrieSetCell(g_tPlayerData[tTeam], Convert_Steam, str_to_num(Data_Team) == 1 ? TEAM_TERRORIST : TEAM_CT);

			TrieSetString(g_tPlayerData[tName], Convert_Steam, Data_Name);

			SQL_NextRow(query);

			if(g_iDebugLevel >= D_LEVEL_3)
			{
				Logging("id [%d] | steam64 [%s] | steam [%s] | team [%d] | name [%s]",
					UserId, Data_Steam, Convert_Steam, str_to_num(Data_Team), Data_Name);
			}
		}
	}
	else
	{
		if(g_iDebugLevel >= D_LEVEL_1)
		{
			Logging("Query fail | game_id [%d]", g_iGameId);
		}
	}

	SQL_FreeHandle(query);
	
	return PLUGIN_HANDLED;
}

/* Получение инфы с сокета при загрузке сервера */
stock GameInfo_Get()
{
	g_iGameSlots = MaxClients - 1;
	get_mapname(g_szMapName, charsmax(g_szMapName));
	
	// if(g_iGameSlots >= 4)
	// {
		// g_iNeedSlotsToPreReady = PercentSub(g_iGameSlots, PercentToReady);
	// }
	// else
	// {
		// g_iNeedSlotsToPreReady = -10;
	// }
	

	if(g_iDebugLevel >= D_LEVEL_2)
	{
		new szPassword[10];
		get_cvar_string("sv_password", szPassword, charsmax(szPassword));

		Logging("\
			GameInfo_Get ->^n^t\
			g_iGameId='%i',^n^t\
			g_iGameSlots='%i'^n^t\
			g_szMapName='%s'^n^t\
			g_szServerIP='%s'^n^t\
			g_szServerName='%s'^n^t\
			Password='%s'^n\",
			g_iGameId, g_iGameSlots, g_szMapName, g_szServerIP, g_szServerName, szPassword
		);
		
	#if defined SMALL_GAME_FOR_LOCAL_TESTS
		// фейк данные для локальных тестов
		GameInfo_Get_TeamsFake();
	#endif
	}
}

/**
	При завершении игры - отправляем данные в БД и сайт.
*/
stock GameInfo_Send()
{
	/**
		g_iGameId - GameId игры
		g_iTeamWins[Team_A | Team_B] - Счета команд.
		g_iRoundPlayed - Общее кол-во сыгранных раундов
		
		(g_tPlayerData[tTeam], Team) - SteamID и его команда
		(g_tPlayerData[tFrags], iFrags) - SteamID и его фраги
		(g_tPlayerData[tDeaths], iDeaths) - SteamID и его смерти
		(g_tPlayerData[tPing], iPing) - SteamID и его средний пинг
	*/
	
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		new szMsg[192];
		formatex(szMsg, charsmax(szMsg), "GameInfo_Send ->");
	}
}

public CloseSockets()
{
	Destroy__Tries();
}

stock Init__Tries()
{
	/* Содержит маску типа SteamID = Team|Money|Frags|Deaths */
	for(new i; i < Tries; i++)
	{
		g_tPlayerData[i] = TrieCreate();
	}
}

stock Destroy__Tries()
{
	for(new i; i < Tries; i++)
	{
		if(g_tPlayerData[i])
		{
			TrieDestroy(g_tPlayerData[i]);
		}
	}
}

stock GameInfo_Get_TeamsFake()
{
	// К примеру, мы получили по JSON список игроков и их команды ( A и B ).
	new szSteams[2][][] = 
	{
		{ /* Team_A - - изначально TT*/
			{"STEAM_1:0:56777063"},
			{"STEAM_1:1:12290715"},
			{"STEAM_1:1:80388844"},
			{"STEAM_0:0:4"},
			{"STEAM_0:0:5"}
		},
		{ /* Team_B - - изначально КТ*/
			{"STEAM_1:1:2536172"},
			{"STEAM_3:0:134860499"},
			{"STEAM_0:0:8"},
			{"STEAM_0:0:9"},
			{"STEAM_0:0:10"}
		}
	};

	new iLen = 0;
	new szMsg[1024];
	new pPlayerId;

	for(new iTeam = Team_A; iTeam <= Team_B; iTeam++)
	{
		for(new i; i <= (g_iGameSlots / 2) - 1; i++)
		{
			// if(TrieKeyExists(g_tPlayerData[tTeam], g_szSteamId[pPlayerId]))
			{
				TrieSetCell(g_tPlayerData[tTeam], szSteams[iTeam][i], iTeam ? TEAM_CT : TEAM_TERRORIST);
				
				copy(g_szSteamId[pPlayerId++], MAX_AUTHID_LENGTH - 1, szSteams[iTeam][i]);
				
				if(g_iDebugLevel >= D_LEVEL_3)
				{				
					if(!iLen)
					{
						iLen = formatex(szMsg, charsmax(szMsg), "GameInfo_Get_TeamsFake -> ^n^t");
					}
					
					iLen += formatex(szMsg[iLen], charsmax(szMsg) - iLen, "► Team:'%s', AuthId:'%s'^n^t",
								iTeam ? "TEAM_CT" : "TEAM_TERRORIST", szSteams[iTeam][i]
							);
				}
			}
		}
	}
	
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging(szMsg);
	}
}
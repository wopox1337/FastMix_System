stock ShowPlayerLog(const pPlayerId)
{
	new Float: fFrags,
		iDeaths,
		iAccount,
		fPing,
		szSteamId[MAX_AUTHID_LENGTH],
		szName[MAX_NAME_LENGTH];

	copy(szSteamId, charsmax(szSteamId), g_szSteamId[pPlayerId]);
	
	TrieGetCell(g_tPlayerData[tFrags], szSteamId, fFrags);
	TrieGetCell(g_tPlayerData[tDeaths], szSteamId, iDeaths);
	TrieGetCell(g_tPlayerData[tMoney], szSteamId, iAccount);
	TrieGetCell(g_tPlayerData[tPing], szSteamId, fPing);
	TrieGetString(g_tPlayerData[tName], szSteamId, szName, charsmax(szName));
	
	new szMsg[512];

	formatex(szMsg, charsmax(szMsg),
		"ShowPlayerLog -> ^n^t\
		g_iConnectedCount='%i'^n^t\
		PLAYER INFO (Disconnect):^n^n\
		^tName:'%s'^n^t\
		^tid:'%d'^n^t\
		^tSteamId:'%s'^n^t\
		^tIP:'%s'^n^t\
		^tMoney:'%d'^n^t\
		^tFrags: '%d'^n^t\
		^tDeaths: '%d'^n^t\
		^tPing: '%.0f'\
		",
		g_iConnectedCount,
		szName,
		pPlayerId,
		szSteamId,
		g_szIPAddress[pPlayerId],
		iAccount,
		floatround(fFrags),
		iDeaths,
		fPing
	);
	
	Logging(szMsg);
}

new g_szFilePath[64], g_szLogDir[264];

stock Init__Logger()
{
	get_localinfo("amxx_logs", g_szFilePath, charsmax(g_szFilePath));
	formatex(g_szLogDir, charsmax(g_szLogDir), "%s/AutoMix_System", g_szFilePath);

	if(!dir_exists(g_szLogDir))
	{
		mkdir(g_szLogDir);
	}
	
	if(g_iDebugLevel >= D_LEVEL_3)
	{
		Logging("Init__Logger -> [- SERVER START -]");
	}
}

/*
stock SaveLogFile(LogText[])
{
	new LogFileTime[32], LogTime[32], LogFile[128], LogMsg[1536];

	get_time("20%y.%m.%d", LogFileTime, charsmax(LogFileTime));
	get_time("%H:%M:%S", LogTime, charsmax(LogTime));

	formatex(LogFile, charsmax(LogFile), "%s/%s.log", g_szLogDir, LogFileTime);
	formatex(LogMsg, charsmax(LogMsg), "[%s] [%s] %s^n", LogFileTime, LogTime, LogText);

	write_file(LogFile, LogMsg, .line = -1);
}
*/

stock Logging(const Message[], any:...)
{
	new szMsg[1024], szTime[22], szLogFileTime[32], szFile[256], pFile;
	vformat(szMsg, charsmax(szMsg), Message, 2);
	
	get_time("20%y.%m.%d", szLogFileTime, charsmax(szLogFileTime));
	get_time("%H:%M:%S", szTime, charsmax(szTime));

	formatex(szFile, charsmax(szFile), "%s/gId_%i.log", g_szLogDir, g_iGameId);

	pFile = fopen(szFile, "at");
	fprintf(pFile, "[%s] [%s] %s^n", szLogFileTime, szTime, szMsg);
	fclose(pFile);
}
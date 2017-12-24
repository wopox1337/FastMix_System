enum {
	WARMUP_START,
	WARMUP_START_BTWGAME,
	WARMUP_STOP,
	FIRSTWARMUP
};

const FREE_BUY_MODE_MONEY = 16000;

#define PROTECTION_TIME 2
// #define DISABLE_ATTACK

stock Warmup_Controller(iMode)
{
	switch(iMode)
	{
		case WARMUP_START: ExecCMDS(g_Cmds_WarmupStart, sizeof g_Cmds_WarmupStart);			
		case WARMUP_START_BTWGAME: ExecCMDS(g_Cmds_WarmupBtwGame, sizeof g_Cmds_WarmupBtwGame);
		case WARMUP_STOP: ExecCMDS(g_Cmds_WarmupStop, sizeof g_Cmds_WarmupStop);

		default:
		{
			if(g_iDebugLevel >= D_LEVEL_3)
			{
				Logging("Warmup_Controller -> iMode='%i'", iMode);
			}
		}
	}

	Do_ResetVotes();
}

stock HC_Controller_Do(iMode)
{
	switch(iMode)
	{
		case WARMUP_START:
		{
			/* Нужно для разминки */
			EnableHookChain(HookChain:HC_Types[__RG_CSGameRules_GiveC4]);
			EnableHookChain(HookChain:HC_Types[__RG_CSGameRules_FPlayerCanRespawn]);
			EnableHookChain(HookChain:HC_Types[__RG_CSGameRules_DeadPlayerWeapons]);
			EnableHookChain(HookChain:HC_Types[__RG_CBasePlayer_Spawn]);

			/* Не нужно на разминке */
			DisableHookChain(HookChain:HC_Types[__RG_RoundEnd]);
			DisableHookChain(HookChain:HC_Types[__RG_CSGameRules_RestartRound]);
		}
		case WARMUP_STOP:
		{
			/* Не нужно на LIVE */
			DisableHookChain(HookChain:HC_Types[__RG_CSGameRules_GiveC4]);
			DisableHookChain(HookChain:HC_Types[__RG_CSGameRules_FPlayerCanRespawn]);
			DisableHookChain(HookChain:HC_Types[__RG_CSGameRules_DeadPlayerWeapons]);
			DisableHookChain(HookChain:HC_Types[__RG_CBasePlayer_Spawn]);
			
			/* Нужно на LIVE */
			EnableHookChain(HookChain:HC_Types[__RG_RoundEnd]);
			EnableHookChain(HookChain:HC_Types[__RG_CSGameRules_RestartRound]);
		}
		
		default:
		{
			if(g_iDebugLevel >= D_LEVEL_3)
			{
				Logging("HC_Controller_Do -> iMode='%i'", iMode);
			}
		}
	}
}

public CSGameRules_FPlayerCanRespawn(const index)
{
	SetHookChainReturn(ATYPE_INTEGER, true);
}

public CSGameRules_GiveC4()
{
	return HC_SUPERCEDE;
}

public CBasePlayer_Spawn(pPlayerId)
{
	if(!is_user_alive(pPlayerId))
	{
		return;
	}
	
	rg_add_account(pPlayerId, FREE_BUY_MODE_MONEY, AS_SET, .bTrackChange = true);

	SetProtection(pPlayerId);
}

#if PROTECTION_TIME > 0
public SetProtection(pPlayerId)
{
	set_entvar(pPlayerId, var_takedamage, DAMAGE_NO);

#if defined DISABLE_ATTACK && PROTECTION_TIME > 1
	set_member(pPlayerId, m_bIsDefusing, true);
#endif

	rg_set_rendering(pPlayerId, kRenderFxGlowShell, 0, 0, 255, 10);

	remove_task(TASK_ProtectionId + pPlayerId);
	set_task(PROTECTION_TIME.0, "EndProtection", TASK_ProtectionId + pPlayerId);
}

public EndProtection(pPlayerId)
{
	pPlayerId -= TASK_ProtectionId;

	if(!is_user_alive(pPlayerId))
	{
		return;
	}
	
#if defined DISABLE_ATTACK && PROTECTION_TIME > 1
	set_member(pPlayerId, m_bIsDefusing, false);
#endif

	set_entvar(pPlayerId, var_takedamage, DAMAGE_AIM);

	rg_set_rendering(pPlayerId);
}

stock rg_set_rendering(pPlayerId, fx = kRenderFxNone, r=255, g=255, b=255, amount=16)
{
	new Float: RenderColor[3];
	RenderColor[0] = float(r);
	RenderColor[1] = float(g);
	RenderColor[2] = float(b);
	
	set_entvar(pPlayerId, var_renderfx, fx);
	set_entvar(pPlayerId, var_rendercolor, RenderColor);
	set_entvar(pPlayerId, var_renderamt, float(amount));
}
#endif	// PROTECTION_TIME

public CSGameRules_DeadPlayerWeapons(const pPlayer)
{
	SetHookChainReturn(ATYPE_INTEGER, GR_PLR_DROP_GUN_NO);

	return HC_SUPERCEDE;
}
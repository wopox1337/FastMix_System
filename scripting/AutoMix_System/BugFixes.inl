public Init__BugFixes()
{
	/* This won't precache the model(sprite) twice, it will return
		the ID of an already precached model
	*/
	g_iModelIndexRadio = precache_model("sprites/radio.spr");
	register_message(SVC_TEMPENTITY, "Msg_SVC_TempEntity");
}

/* Сквозь спрайт Radio.spr можно видеть через дым.
	ПОПРАВЛЕНО
*/
public Msg_SVC_TempEntity(iMsgId, iDest, id)
{
	if(get_msg_arg_int(1) == TE_PLAYERATTACHMENT)
	{
		if(get_msg_arg_int(4) == g_iModelIndexRadio)
		{
			return PLUGIN_HANDLED;
		}
	}

	return PLUGIN_CONTINUE;
}

/* Так же планируется над Даблдаком поработать */
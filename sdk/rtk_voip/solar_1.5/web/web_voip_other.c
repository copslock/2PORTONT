#include <stdio.h>
#include "web_voip.h"

static char cid_det[CID_MAX][25] = {
	"FSK_BELLCORE", "FSK_ETSI", "FSK_BT", "FSK_NTT", "DTMF"
};

static char pulse_pps[PULSE_GEN_PPS_MAX][25] = {"10 PPS", "20 PPS"};

int voip_char_replace(const char *src, char old_char, char new_char, char *result)
{
	int i;

	for (i=0; src[i]; i++)
	{
		if (src[i] == old_char)
			result[i] = new_char;
		else
			result[i] = src[i];
	}

	result[i] = 0;
	return 0;
}

#if CONFIG_RTK_VOIP_PACKAGE_865X
int asp_voip_OtherGet(request * wp, int argc, char **argv)
#else
int asp_voip_OtherGet(int ejid, request * wp, int argc, char **argv)
#endif
{
	voipCfgParam_t *pVoIPCfg;
	char key_display[FUNC_KEY_LENGTH];
#ifdef SUPPORT_DSCP
	int i;
#endif

	if (web_flash_get(&pVoIPCfg) != 0)
		return -1;

	if (strcmp(argv[0], "funckey_transfer")==0)
	{
		voip_char_replace(pVoIPCfg->funckey_transfer, '.', '*', key_display);
		boaWrite(wp, "%s", key_display);
	}
	else if (strcmp(argv[0], "funckey_pstn")==0)
	{
		voip_char_replace(pVoIPCfg->funckey_pstn, '.', '*', key_display);
		boaWrite(wp, "%s", key_display);
	}
	else if (strcmp(argv[0], "display_funckey_pstn")==0)
	{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
		boaWrite(wp, "%s", "");
#else
		boaWrite(wp, "%s", "style=\"display:none\"");
#endif
	}
	else if (strcmp(argv[0], "auto_dial_display_title") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* nothing */
#else
		boaWrite(wp, "<p>\n<b>Dial Option</b>\n");
#endif
	}
	else if (strcmp(argv[0], "auto_dial_display") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		boaWrite(wp, "style=\"display:none\"");
#endif
	}
	else if (strcmp(argv[0], "auto_dial")==0)
	{
		boaWrite(wp, "%d", pVoIPCfg->auto_dial & AUTO_DIAL_TIME);
	}
	else if (strcmp(argv[0], "auto_dial_always") == 0)
	{
		boaWrite(wp, "%s", pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS ? "checked" : "");
	}
	else if (strcmp(argv[0], "auto_bypass_relay")==0)
	{
		// PSTN Relay - Auto bypass relay (Switch to PSTN if VoIP failed)
		boaWrite(wp, "%s", pVoIPCfg->auto_bypass_relay & AUTO_BYPASS_RELAY_ENABLE ? "checked" : "");
	}
	else if (strcmp(argv[0], "auto_bypass_warning")==0)
	{
		boaWrite(wp, "%s", pVoIPCfg->auto_bypass_relay & AUTO_BYPASS_WARNING_ENABLE ? "checked" : "");
	}
	else if (strcmp(argv[0], "off_hook_alarm_display_title") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* nothing */
#else
		boaWrite(wp, "<p>\n<b>Off-Hook Alarm</b>\n");
#endif
	}
	else if (strcmp(argv[0], "off_hook_alarm_display") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		boaWrite(wp, "style=\"display:none\"");
#endif
	}
	else if (strcmp(argv[0], "off_hook_alarm")==0)
	{
		boaWrite(wp, "%d", pVoIPCfg->off_hook_alarm);
	}
	// VoIP to PSTN - one stage dialing
	else if (strcmp(argv[0], "one_stage_dial")==0)
		boaWrite(wp, "%s", (pVoIPCfg->one_stage_dial) ? "checked" : "");
	// VoIP to PSTN - two stage dialing
	else if (strcmp(argv[0], "two_stage_dial")==0)
		boaWrite(wp, "%s", (pVoIPCfg->two_stage_dial) ? "checked" : "");
	// PSTN to VoIP - Caller ID Detection
	else if (strcmp(argv[0], "caller_id_auto_det")==0)
	{
		boaWrite(wp, "<input type=\"radio\" name=\"caller_id_auto_det\" value=0 onClick=enable_cid_det_mode() %s>Off",
			pVoIPCfg->cid_auto_det_select == 0 ? "checked" : "");
		boaWrite(wp, "<input type=\"radio\" name=\"caller_id_auto_det\" value=1 onClick=enable_cid_det_mode() %s>On (NTT Support)",
			pVoIPCfg->cid_auto_det_select == 1 ? "checked" : "");
   		boaWrite(wp, "<input type=\"radio\" name=\"caller_id_auto_det\" value=2 onClick=enable_cid_det_mode() %s>On (NTT Not Support)",
			pVoIPCfg->cid_auto_det_select == 2 ? "checked" : "");
	}
	else if (strcmp(argv[0], "caller_id_det")==0)
	{
		boaWrite(wp, "<select name=caller_id_det %s>",
			pVoIPCfg->cid_auto_det_select != 0 ? "disabled" : "");

		for (i=0; i<CID_MAX ;i++)
		{
			if (i == pVoIPCfg->caller_id_det_mode)
				boaWrite(wp, "<option selected>%s</option>", cid_det[i]);
			else
				boaWrite(wp, "<option>%s</option>", cid_det[i]);
		}

		boaWrite(wp, "</select>");
	}
	else if (strcmp(argv[0], "pulse_dial_generation")==0)
	{
		boaWrite(wp, "<input type=\"radio\" name=\"pulse_dial_generation\" value=0 %s>Disable",
    				( pVoIPCfg ->pulse_dial_gen == 0 ? "checked" : "" ) );
		boaWrite(wp, "<input type=\"radio\" name=\"pulse_dial_generation\" value=1 %s>Enable",
    				( pVoIPCfg ->pulse_dial_gen == 1 ? "checked" : "" ) );
	}
	else if (strcmp(argv[0], "pulse_gen_PPS")==0)
	{
		for (i=0; i<PULSE_GEN_PPS_MAX ;i++)
		{
			if (((i+1)*10) == pVoIPCfg->pulse_gen_pps)
				boaWrite(wp, "<option selected>%s</option>", pulse_pps[i]);
			else
				boaWrite(wp, "<option>%s</option>", pulse_pps[i]);
		}
	}
	else if (strcmp(argv[0], "pulse_gen_Make")==0)
	{
		boaWrite(wp, "%d", pVoIPCfg->pulse_gen_make_time);
	}
	else if (strcmp(argv[0], "pulse_gen_Pause")==0)
	{
		boaWrite(wp, "%d", pVoIPCfg->pulse_gen_interdigit_pause);
	}
	else if (strcmp(argv[0], "pulse_dial_detection")==0)
	{
		boaWrite(wp, "<input type=\"radio\" name=\"pulse_dial_detection\" value=0 %s>Disable",
    				( pVoIPCfg ->pulse_dial_det == 0 ? "checked" : "" ) );
		boaWrite(wp, "<input type=\"radio\" name=\"pulse_dial_detection\" value=1 %s>Enable",
    				( pVoIPCfg ->pulse_dial_det == 1 ? "checked" : "" ) );
	}
	else if (strcmp(argv[0], "pulse_det_Pause")==0)
	{
		boaWrite(wp, "%d", pVoIPCfg->pulse_det_pause);
	}
#if defined(CONFIG_RTK_VOIP_DRIVERS_FXO) && !defined(CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)
	else if (strcmp(argv[0], "display_cid_det")==0)
		boaWrite(wp, "%s", "");
	else if (strcmp(argv[0], "display_pulse_dial_gen")==0)
		boaWrite(wp, "%s", "");
#else
	else if (strcmp(argv[0], "display_cid_det")==0)
		boaWrite(wp, "%s", "style=\"display:none\"");
	else if (strcmp(argv[0], "display_pulse_dial_gen")==0)
		boaWrite(wp, "%s", "style=\"display:none\"");
#endif
	else
	{
		return -1;
	}

	return 0;
}

void asp_voip_OtherSet(request * wp, char *path, char *query)
{
	voipCfgParam_t *pVoIPCfg;
	char key_display[FUNC_KEY_LENGTH];
	char *ptr;
	int i;

	if (web_flash_get(&pVoIPCfg) != 0)
		return;

	strcpy(key_display, boaGetVar(wp, "funckey_transfer", "*1"));
	voip_char_replace(key_display, '*', '.', pVoIPCfg->funckey_transfer);

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	strcpy(key_display, boaGetVar(wp, "funckey_pstn", "*0"));
	voip_char_replace(key_display, '*', '.', pVoIPCfg->funckey_pstn);
#endif

	pVoIPCfg->auto_dial = (atoi(boaGetVar(wp, "auto_dial", "5"))) & AUTO_DIAL_TIME;
	if (gstrcmp(boaGetVar(wp, "auto_dial_always", ""), "on") == 0)
		pVoIPCfg->auto_dial |= AUTO_DIAL_ALWAYS;

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	// PSTN Relay - Auto bypass relay (Switch to PSTN if VoIP failed)
	pVoIPCfg->auto_bypass_relay = !gstrcmp(boaGetVar(wp, "auto_bypass_relay", ""), "on");
	if (pVoIPCfg->auto_bypass_relay && !gstrcmp(boaGetVar(wp, "auto_bypass_warning", ""), "on"))
		pVoIPCfg->auto_bypass_relay |= AUTO_BYPASS_WARNING_ENABLE;
#endif

	pVoIPCfg->off_hook_alarm = atoi(boaGetVar(wp, "off_hook_alarm", "30"));

#if defined(CONFIG_RTK_VOIP_DRIVERS_FXO) && !defined(CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)
	// VoIP to PSTN - one stage dialing
	pVoIPCfg->one_stage_dial = !gstrcmp(boaGetVar(wp, "one_stage_dial", ""), "on");

	// VoIP to PSTN - two stage dialing
	pVoIPCfg->two_stage_dial = !gstrcmp(boaGetVar(wp, "two_stage_dial", ""), "on");

	// PSTN to VoIP - Caller ID Detection
	pVoIPCfg->cid_auto_det_select = atoi(boaGetVar(wp, "caller_id_auto_det", ""));
	ptr	 = boaGetVar(wp, "caller_id_det", "");
	for(i=0; i<CID_MAX; i++)
		if (!gstrcmp(ptr, cid_det[i]))
			break;

	pVoIPCfg->caller_id_det_mode = (i == CID_MAX) ? CID_DTMF : i;

	pVoIPCfg->pulse_dial_gen = atoi(boaGetVar(wp, "pulse_dial_generation", ""));

	ptr = boaGetVar(wp, "pulse_gen_PPS", "");
	for(i=0; i<PULSE_GEN_PPS_MAX; i++)
		if (!gstrcmp(ptr, pulse_pps[i]))
			break;
	if (i == PULSE_GEN_PPS_10)
		pVoIPCfg->pulse_gen_pps = 10;
	else if (i == PULSE_GEN_PPS_20)
		pVoIPCfg->pulse_gen_pps = 20;
	else if (i == PULSE_GEN_PPS_MAX)
		pVoIPCfg->pulse_gen_pps = 10;
	pVoIPCfg->pulse_gen_make_time = atoi(boaGetVar(wp, "pulse_gen_Make", "33"));

	pVoIPCfg->pulse_gen_interdigit_pause = atoi(boaGetVar(wp, "pulse_gen_Pause", "700"));

#endif

	pVoIPCfg->pulse_dial_det = atoi(boaGetVar(wp, "pulse_dial_detection", ""));
	pVoIPCfg->pulse_det_pause = atoi(boaGetVar(wp, "pulse_det_Pause", "450"));

	web_flash_set(pVoIPCfg);
#ifdef REBOOT_CHECK
	OK_MSG("/voip_other.asp");
#else
	web_restart_solar();
	boaRedirect(wp, "/voip_other.asp");
#endif
}

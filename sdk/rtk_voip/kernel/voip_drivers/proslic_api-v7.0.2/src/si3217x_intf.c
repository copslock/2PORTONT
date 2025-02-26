/*
** Copyright (c) 2007-2013 by Silicon Laboratories
**
** $Id: si3217x_intf.c 3843 2013-02-28 20:42:39Z cdp $
**
** SI3217X_Intf.c
** SI3217X ProSLIC interface implementation file
**
** Author(s): 
** cdp
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.   
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the implementation file for the main ProSLIC API and is used 
** in the ProSLIC demonstration code. 
**
*/

#ifdef REALTEK_PATCH_FOR_SILAB
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include "rtk_voip.h"
#endif
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#include "si3217x_intf.h"
#include "si3217x.h"
#include "si3217x_common_registers.h"
#include "proslic_api_config.h"

#include "si3217x_revb_intf.h"
#include "si3217x_revc_intf.h"

#define PRAM_ADDR (334 + 0x400)
#define PRAM_DATA (335 + 0x400)

#define WriteReg        pProslic->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg         pProslic->deviceId->ctrlInterface->ReadRegister_fptr
#define pProHW          pProslic->deviceId->ctrlInterface->hCtrl
#define Reset           pProslic->deviceId->ctrlInterface->Reset_fptr
#define Delay           pProslic->deviceId->ctrlInterface->Delay_fptr
#define pProTimer       pProslic->deviceId->ctrlInterface->hTimer
#define WriteRAM        pProslic->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM         pProslic->deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsed     pProslic->deviceId->ctrlInterface->timeElapsed_fptr
#define getTime         pProslic->deviceId->ctrlInterface->getTime_fptr
#define SetSemaphore    pProslic->deviceId->ctrlInterface->Semaphore_fptr

#define WriteRegX       deviceId->ctrlInterface->WriteRegister_fptr
#define ReadRegX        deviceId->ctrlInterface->ReadRegister_fptr
#define pProHWX         deviceId->ctrlInterface->hCtrl
#define DelayX          deviceId->ctrlInterface->Delay_fptr
#define pProTimerX      deviceId->ctrlInterface->hTimer
#define WriteRAMX       deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAMX        deviceId->ctrlInterface->ReadRAM_fptr
#define getTimeX        deviceId->ctrlInterface->getTime_fptr
#define TimeElapsedX    deviceId->ctrlInterface->timeElapsed_fptr

#define BROADCAST 0xff

/*
** Define patch parameters that can be modified by API
*/
#define SI3217X_PRAM_VBATH_NEON			995
#define SI3217X_PRAM_LCRMASK_MWI		987


#ifdef REALTEK_PATCH_FOR_SILAB
#ifndef ENABLE_DEBUG
#define ENABLE_DEBUG
#endif
static const char LOGPRINT_PREFIX[] = "Si3217x: ";
#else
#ifdef ENABLE_DEBUG
static const char LOGPRINT_PREFIX[] = "Si3217x: ";
#endif
#endif

/*
** Externs
*/

/* General Configuration */
extern Si3217x_General_Cfg Si3217x_General_Configuration;
#ifdef SIVOICE_MULTI_BOM_SUPPORT
extern const proslicPatch SI3217X_PATCH_B_FLBK;
extern const proslicPatch SI3217X_PATCH_B_BKBT;
extern const proslicPatch SI3217X_PATCH_B_PBB;
extern Si3217x_General_Cfg Si3217x_General_Configuration_MultiBOM[];
extern int si3217x_genconf_multi_max_preset;
#else
extern const proslicPatch SI3217X_PATCH_B_DEFAULT;
#endif

/* Ringing */
#ifndef DISABLE_RING_SETUP
extern Si3217x_Ring_Cfg Si3217x_Ring_Presets[];
#endif

/* Tone Generation */
#ifndef DISABLE_TONE_SETUP
extern Si3217x_Tone_Cfg Si3217x_Tone_Presets[];
#endif

/* FSK */
#ifndef DISABLE_FSK_SETUP
extern Si3217x_FSK_Cfg Si3217x_FSK_Presets[];
#endif

/* DTMF */
#ifndef DISABLE_DTMF_SETUP
extern Si3217x_DTMFDec_Cfg Si3217x_DTMFDec_Presets[];
#endif

/* Zsynth */
#ifndef DISABLE_ZSYNTH_SETUP
extern Si3217x_Impedance_Cfg Si3217x_Impedance_Presets [];
#endif

/* CI/GCI */
#ifndef DISABLE_CI_SETUP
extern Si3217x_CI_Cfg Si3217x_CI_Presets [];
#endif

/* Audio Gain Scratch */
extern Si3217x_audioGain_Cfg Si3217x_audioGain_Presets[];

/* DC Feed */
#ifndef DISABLE_DCFEED_SETUP
extern Si3217x_DCfeed_Cfg Si3217x_DCfeed_Presets[];
#endif

/* GPIO */
#ifndef DISABLE_GPIO_SETUP
extern Si3217x_GPIO_Cfg Si3217x_GPIO_Configuration ;
#endif

/* Pulse Metering */
#ifndef DISABLE_PULSE_SETUP
extern Si3217x_PulseMeter_Cfg Si3217x_PulseMeter_Presets [];
#endif

/* PCM */
#ifndef DISABLE_PCM_SETUP
extern Si3217x_PCM_Cfg Si3217x_PCM_Presets [];
#endif


/*
** Local functions are defined first
*/


/*
** Function: getChipType
**
** Description: 
** Decode ID register to identify chip type
**
** Input Parameters: 
** ID register value
**
** Return:
** partNumberType
*/
static partNumberType getChipType(uInt8 data){
    data &= 0x38;
    return ((data >> 3) + SI32171);
}

/*
** Function: setUserMode
**
** Description: 
** Puts ProSLIC into user mode or out of user mode
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** on: specifies whether user mode should be turned on (TRUE) or off (FALSE)
**
** Return:
** none
*/
static int setUserMode (proslicChanType *pProslic,BOOLEAN on){
    uInt8 data;
    if (SetSemaphore != NULL){
        while (!(SetSemaphore (pProHW,1)));
        if (on == TRUE){
            if (pProslic->deviceId->usermodeStatus<2)
                pProslic->deviceId->usermodeStatus++;
        } else {
            if (pProslic->deviceId->usermodeStatus>0)
                pProslic->deviceId->usermodeStatus--;
            if (pProslic->deviceId->usermodeStatus != 0)
                return -1;
        }
    }
    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_TEST_CNTL);
    if (((data&1) != 0) == on)
        return 0;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_TEST_CNTL,2);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_TEST_CNTL,8);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_TEST_CNTL,0xe);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_TEST_CNTL,0);
    if (SetSemaphore != NULL)
        SetSemaphore(pProHW,0);
    return 0;
}


/*
** Function: setUserModeBroadcast
**
** Description: 
** Puts ProSLIC into user mode or out of user mode
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** on: specifies whether user mode should be turned on (TRUE) or off (FALSE)
**
** Return:
** none
*/
static int setUserModeBroadcast (proslicChanType *pProslic,BOOLEAN on){
    uInt8 data;
    if (SetSemaphore != NULL){
        while (!(SetSemaphore (pProHW,1)));
        if (on == TRUE){
            if (pProslic->deviceId->usermodeStatus<2)
                pProslic->deviceId->usermodeStatus++;
        } else {
            if (pProslic->deviceId->usermodeStatus>0)
                pProslic->deviceId->usermodeStatus--;
            if (pProslic->deviceId->usermodeStatus != 0)
                return -1;
        }
    }
    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_TEST_CNTL);/*we check first channel. we assume all channels same user mode state*/
    if (((data&1) != 0) == on)
        return 0;
    WriteReg(pProHW,BROADCAST,SI3217X_COM_REG_TEST_CNTL,2);
    WriteReg(pProHW,BROADCAST,SI3217X_COM_REG_TEST_CNTL,8);
    WriteReg(pProHW,BROADCAST,SI3217X_COM_REG_TEST_CNTL,0xe);
    WriteReg(pProHW,BROADCAST,SI3217X_COM_REG_TEST_CNTL,0);
    if (SetSemaphore != NULL)
        SetSemaphore(pProHW,0);
    return 0;
}


/*
** Function: probeDaisyChain
**
** Description: 
** Determine number of devices on chain
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** number of channels detected
*/
static int probeDaisyChain (proslicChanType *pProslic){
    uInt8 i=0;
    WriteReg(pProHW,BROADCAST,SI3217X_COM_REG_RAM_ADDR_HI,0x40);
    while (ReadReg(pProHW,i++,SI3217X_COM_REG_RAM_ADDR_HI) == 0x40 && (i<=32));
    WriteReg(pProHW,BROADCAST,SI3217X_COM_REG_RAM_ADDR_HI,0x0);
    return i-1;
}

/*
** Function: isVerifiedProslic
**
** Description: 
** Determine if DAA or ProSLIC present
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
**
** Return:
** channelType
*/
static int isVerifiedProslic(proslicChanType *pProslic){
uInt8 data;
uInt8 daaEnableSave;
int returnCode;

    /* 
    ** Set bit 0 of DAA_CNTL reg regardless of channel
    ** type.  This will allow us to see the DAA register
    ** space if an Si32178 or Si3050 occupies that channel.
    ** If it is a DAA channel, the register that is actually
    ** written is FXO Control 3, since the address bit 6 is
    ** ignored.  This sets DDL (digital loopback).  This should
    ** be OK provided it is returned to it's entry state once
    ** the channel type has been identified.
	**
    ** 
    */

    /*
    ** Note:   DIAG3 replaced DAA_CNTL on Si3217x RevC.  Setting DIAG3.0
    **         sets PD_GR909, which powers down the HVIC.  Must pre-check
    **         the device revision before doing DAA check.
    */
    data = (ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_ID)&0x02);
    if(data)
    {
        pProslic->channelType = PROSLIC;
        return RC_NONE;
    }

	daaEnableSave = ReadReg(pProHW,pProslic->channel,74);
	WriteReg(pProHW,pProslic->channel,74,daaEnableSave|0x01);
	data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMTXHI);
#ifdef ENABLE_DEBUG
	if(pProslic->debugMode)
	{
		LOGPRINT("%sisVerifiedProslic : REG VAL = %02x\n", LOGPRINT_PREFIX, data);
	}
#endif
    if ( data == 0x40 ){ /* Always 0x40 if lineside device is not enabled yet*/
        pProslic->channelType = DAA;  
        returnCode = RC_CHANNEL_TYPE_ERR;
        WriteReg(pProHW,pProslic->channel,74,daaEnableSave);
    }
    else { /* Not a VDAA - could be ProSLIC or empty channel */
        data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMMODE); /* always 0x05 after reset */
        if ( data == 0x05) {
            pProslic->channelType = PROSLIC;
            returnCode = RC_NONE;
        }
        else {
            pProslic->channelType = UNKNOWN;
            returnCode = RC_CHANNEL_TYPE_ERR;
        }
    }
    return returnCode;
}


/*
** Function: isBroadcastPossible
**
** Description: 
** Determine if device contains both ProSLIC AND DAA,
** in which case broadcast is not supported.
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
**
** Return:
** 0 - broadcast ok
** RC_BROADCAST_FAIL - broadcast cannot be used
*/
static int isBroadcastPossible(proslicChanType *pProslic){
    uInt8 data;
    
    if(pProslic->channelType != PROSLIC)
        return RC_CHANNEL_TYPE_ERR;

    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_ID);
    pProslic->deviceId->chipType = getChipType(data);
    /*check for nonDAA version - otherwise we can't broadcast*/
    if (pProslic->deviceId->chipType == SI32178)
    {
        pProslic->error = RC_BROADCAST_FAIL;
        return RC_BROADCAST_FAIL;
    }

    return RC_NONE;
}


/*
** Function: isReinitRequired
**
** Description: 
** Checks for improper ring exit
**
** Returns:
** RC_NONE                -  Reinit not required
** RC_REINIT_REQUIRED     -  Corrupted state machine - reinit required
**
*/
static int isReinitRequired(proslicChanType *pProslic)
{
	uInt8 lf;
	ramData rkdc_sum;

	if(pProslic->channelType != PROSLIC) 
	{
		return RC_IGNORE;
	}

	/* Check for improper ring exit which may cause dcfeed corruption */
	
	lf = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED);
	setUserMode(pProslic,TRUE);
	rkdc_sum = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RDC_SUM);
#ifdef ENABLE_DEBUG
	if(pProslic->debugMode)
    {
        LOGPRINT("%sisReinitRequired: Linefeed = %d\n", LOGPRINT_PREFIX, lf);
	    LOGPRINT("%sisReinitRequired: RDC_SUM = %d\n", LOGPRINT_PREFIX, (int)rkdc_sum);
	}
#endif
	if((rkdc_sum & 0x400000)&&(!(lf & 0x44)))
	{
		return RC_REINIT_REQUIRED;
	}
	else
	{
		return RC_NONE;
	}
}

/*
** Function: Si3217x_PowerUpConverter
**
** Description: 
** Powers all DC/DC converters sequentially with delay to minimize
** peak power draw on VDC.
**
** Returns:
** int (error)
**
*/
int Si3217x_PowerUpConverter(proslicChanType_ptr pProslic)
{
    errorCodeType error = RC_DCDC_SETUP_ERR;
    int32 vbath,vbat;
    uInt8 reg = 0;
    int timer = 0;


    if(pProslic->channelType != PROSLIC) 
        return RC_CHANNEL_TYPE_ERR;

    /*
    ** - powerup digital dc/dc w/ OV clamping and shutdown
    ** - delay
    ** - verify no short circuits by looking for vbath/2
    ** - clear dcdc status
    ** - switch to analog converter with OV clamping only (no shutdown)
    ** - select analog dcdc and disable pwrsave
    ** - delay
    */

    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED,LF_FWD_OHT);  /* Force out of pwrsave mode if called in error */
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED,LF_OPEN);     /* Ensure open line before powering up converter */
    reg = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_ENHANCE);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_ENHANCE,reg&0x07);  /* Disable powersave mode */

    setUserMode(pProslic,TRUE);
	WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_PD_DCDC,0x700000L); /* In case OV or UV previously occurred */

    /* 
    ** Setup converter drive polarity and charge pump enable 
    ** based on revision, bom, and converter type
    */
    if(pProslic->deviceId->chipRev == SI3217X_REVB)
    {
        error = Si3217x_RevB_ConverterSetup(pProslic);
    }
    else /* Add revC option here */
    {
        error = Si3217x_RevC_ConverterSetup(pProslic);
    }

    if(error != RC_NONE)
    {
        #ifdef ENABLE_DEBUG
        if(pProslic->debugMode)
        {
            LOGPRINT ("%sChannel %d : DCDC initialization failed\n",LOGPRINT_PREFIX, pProslic->channel);
        }
        #endif 
        setUserMode(pProslic,FALSE);
        return error;
    }

	WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_PD_DCDC,0x600000L);
    Delay(pProTimer,50);
    vbath = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VBATH_EXPECT);
    vbat = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_MADC_VBAT);
    if(vbat & 0x10000000L)
        vbat |= 0xF0000000L;
    if(vbat < (vbath / 2)) {
        pProslic->channelEnable = 0;
        error = RC_VBAT_UP_TIMEOUT;
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_PD_DCDC, 0x300000L); /* shutdown converter */
        #ifdef ENABLE_DEBUG
        if(pProslic->debugMode)
        {
            LOGPRINT ("%sChannel %d : DCDC Short Circuit Failure - disabling channel\n",LOGPRINT_PREFIX, pProslic->channel);
            LOGPRINT ("VBAT = %d\n", vbat);
        }
        #endif      
        setUserMode(pProslic,FALSE);
        return error;  
    }
    else { /* Enable analog converter */
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DCDC_STATUS,0L);   
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_PD_DCDC,0x400000L);
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_ENHANCE,reg);   /* Restore ENHANCE */
        Delay(pProTimer,50);
    }

    /*
    ** - monitor vbat vs expected level (VBATH_EXPECT)
    */
    vbath = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VBATH_EXPECT);
    do
    {
        vbat = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_MADC_VBAT);
        if(vbat & 0x10000000L)
            vbat |= 0xF0000000L;
        Delay(pProTimer,10);
    }while((vbat < (vbath - COMP_5V))&&(timer++ < SI3217X_TIMEOUT_DCDC_UP));  /* 2 sec timeout */

#ifdef ENABLE_DEBUG
    if(pProslic->debugMode)
    {
        LOGPRINT ("%sChannel %d : VBAT Up = %d.%d v\n",
            LOGPRINT_PREFIX,
            pProslic->channel,(int)((vbat/SCALE_V_MADC)/1000), 
            (int)(((vbat/SCALE_V_MADC) - (vbat/SCALE_V_MADC)/1000*1000)));
    }
#endif
    if(timer > SI3217X_TIMEOUT_DCDC_UP)
    {
        /* Error handling - shutdown converter, disable channel, set error tag */
        pProslic->channelEnable = 0;
        error = RC_VBAT_UP_TIMEOUT;
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_PD_DCDC, 0x900000L); /* shutdown converter */
#ifdef ENABLE_DEBUG
        if(pProslic->debugMode)
        {
            vbat = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DCDC_STATUS);   
            LOGPRINT ("%sChannel %d : DCDC Power up timeout : Status=0x%08X\n",LOGPRINT_PREFIX, pProslic->channel,vbat);
       }
#endif      
    }

    setUserMode(pProslic,FALSE);
    return error;  
}

/*
** Function: Si3217x_PowerDownConverter
**
** Description: 
** Safely powerdown dcdc converter after ensuring linefeed
** is in the open state.  Test powerdown by setting error
** flag if detected voltage does no fall below 5v.
**
** Returns:
** int (error)
**
*/
int Si3217x_PowerDownConverter(proslicChanType_ptr pProslic)
{
    errorCodeType error = RC_NONE;
    int32 vbat;
    int timer = 0;

    if(pProslic->channelType != PROSLIC) 
        return RC_CHANNEL_TYPE_ERR;
    
    setUserMode(pProslic,TRUE);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED, LF_FWD_OHT); /* Force out of powersave mode */
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED, LF_OPEN);
    Delay(pProTimer,50);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_PD_DCDC,0x900000L);
    Delay(pProTimer,50);

    /*
    ** Verify VBAT falls below 5v
    */
    do
    {
        vbat = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_MADC_VBAT);
        if(vbat & 0x10000000L)
            vbat |= 0xF0000000L;
        Delay(pProTimer,10);
    }while((vbat > COMP_5V)&&(timer++ < SI3217X_TIMEOUT_DCDC_DOWN));  /* 2 sec timeout */
#ifdef ENABLE_DEBUG
    if(pProslic->debugMode)
    {
        LOGPRINT ("%sChannel %d : VBAT Down = %d.%d v\n",
            LOGPRINT_PREFIX,
            pProslic->channel,(int)((vbat/SCALE_V_MADC)/1000), 
            (int)(((vbat/SCALE_V_MADC) - (vbat/SCALE_V_MADC)/1000*1000)));
    }
#endif
    if(timer > SI3217X_TIMEOUT_DCDC_DOWN)
    {
        /* Error handling - shutdown converter, disable channel, set error tag */
        pProslic->channelEnable = 0;
        error = RC_VBAT_DOWN_TIMEOUT;
#ifdef ENABLE_DEBUG
        if(pProslic->debugMode)
        {
            LOGPRINT ("%sChannel %d : DCDC Power Down timeout\n", LOGPRINT_PREFIX, pProslic->channel);
        }
#endif      
    }

    setUserMode(pProslic,FALSE);
    return error;  
}


/*
** Function: Si3217x_Calibrate
**
** Description: 
** Performs calibration based on passed ptr to array of
** desired CALRn settings.
**
** Starts calibration on all channels sequentially (not broadcast)
** and continuously polls for completion.  Return error code if
** CAL_EN does not clear for each enabled channel within the passed
** timeout period.
*/
int Si3217x_Calibrate(proslicChanType_ptr *pProslic, int maxChan, uInt8 *calr, int maxTime)
{
    int i;
    int cal_en = 0;
    int cal_en_chan = 0;
    int timer = 0;

    /*
    ** Launch cals sequentially (not serially)
    */
    for(i=0;i<maxChan;i++)
    {
        if((pProslic[i]->channelEnable)&&(pProslic[i]->channelType == PROSLIC))
        {
            pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3217X_COM_REG_CALR0,calr[0]);
            if(pProslic[i]->deviceId->chipRev == SI3217X_REVB)
            {
                pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3217X_COM_REG_CALR1,calr[1]);
                pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3217X_COM_REG_CALR2,calr[2]);
            }
            else
            {
                pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3217X_COM_REG_CALR1,calr[5]);
                pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3217X_COM_REG_CALR2,calr[6]);
            }
            pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3217X_COM_REG_CALR3,calr[3]);
        }
    }

    /*
    ** Wait for completion or timeout
    */
    do 
    {
        cal_en = 0;
        pProslic[0]->DelayX(pProslic[0]->pProTimerX,10);
        for(i=0;i<maxChan;i++)
        {
            if(pProslic[i]->channelType != PROSLIC) continue;

            if(pProslic[i]->channelEnable)
            {
                cal_en_chan = pProslic[i]->ReadRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3217X_COM_REG_CALR3);
                if((cal_en_chan&0x80)&&(timer == maxTime))
                {
#ifdef ENABLE_DEBUG
                    if(pProslic[i]->debugMode)
                    {
                        LOGPRINT("%sCalibration timout channel %d\n", LOGPRINT_PREFIX, i);
                    }
#endif
                    pProslic[i]->channelEnable = 0;
                    pProslic[i]->error = RC_CAL_TIMEOUT;
                }
                cal_en |= cal_en_chan;
            }
        }         
    }while((timer++ <= maxTime)&&(cal_en&0x80));
    return cal_en;
}



/*
** Function: LoadRegTables
**
** Description: 
** Generic function to load register/RAM with predefined addr/value 
*/
static int LoadRegTables (proslicChanType *pProslic, ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable, int broadcast){
    uInt16 i;
    uInt8 channel;
    if (broadcast){
        channel = BROADCAST;
        setUserModeBroadcast(pProslic,TRUE);
    }
    else if(pProslic->channelType == PROSLIC) {
        channel = pProslic->channel;
        setUserMode(pProslic,TRUE);
    }
    else {
        return RC_CHANNEL_TYPE_ERR;
    }

    i=0; 
    if (pRamTable != 0){
        while (pRamTable[i].address != 0xffff){
            WriteRAM(pProHW,channel,pRamTable[i].address,pRamTable[i].initValue); 
            i++;
        }
    }
    i=0;
    if (pRegTable != 0){
        while (pRegTable[i].address != 0xff){
            WriteReg(pProHW,channel,pRegTable[i].address,pRegTable[i].initValue);
            i++;
        }
    }

    if (broadcast)
        setUserModeBroadcast(pProslic,FALSE);
    else
        setUserMode(pProslic,FALSE);

    return RC_NONE;
}

#if 0
/*
** Function: enableSi3217xLoopman
**
** Description: 
** Enable loopman mode on SI3217x (enabled by default)
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** broadcast:  broadcast flag
**
** Return:
** 0
*/
#ifdef DISABLE_LOOPMAN
#else
int enableSi3217xLoopman (proslicChanType *pProslic, int broadcast){
    uInt8 channel;
    
    if (broadcast){
        setUserModeBroadcast(pProslic,TRUE);
        channel = BROADCAST;
    }
    else if(pProslic->channelType == PROSLIC) {
        setUserMode (pProslic,TRUE); /*make sure we are in user mode */
        channel = pProslic->channel;
    }
    else {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteRAM(pProHW, channel,SI3217X_COM_RAM_MADC_LOOP_MAN, 0);
    WriteRAM(pProHW, channel,SI3217X_COM_RAM_HVIC_CNTL_MAN, 0x200000L);
    WriteRAM(pProHW, channel,SI3217X_COM_RAM_MADC_LOOP_MAN, 0x300000L);
    WriteReg(pProHW, channel,SI3217X_COM_REG_PDN,0x80); /*enable MADC to prevent power alarm. this could also be done before going active*/
   
    if (broadcast){
        setUserModeBroadcast(pProslic,FALSE);
    }
    else {
        setUserMode(pProslic,FALSE); /*turn off user mode*/
    }
    return RC_NONE;
}


/*
** Function: disableSi3217xLoopman
**
** Description: 
** Disable loopman mode on SI3217x (enabled by default)
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** broadcast:  broadcast flag
**
** Return:
** 0
*/
int disableSi3217xLoopman (proslicChanType *pProslic, int broadcast){
    uInt8 channel;
    if (broadcast){
        setUserModeBroadcast(pProslic,TRUE);
        channel = BROADCAST;
    }
    else if(pProslic->channelType == PROSLIC) {
        setUserMode (pProslic,TRUE); /*make sure we are in user mode */
        channel = pProslic->channel;
    }
    else {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteRAM(pProHW, channel,SI3217X_COM_RAM_MADC_LOOP_MAN, 0x0);

    if (broadcast){
        setUserModeBroadcast(pProslic,FALSE);
    }
    else if(pProslic->channelType == PROSLIC){
        setUserMode(pProslic,FALSE); /*turn off user mode*/
    }
    return RC_NONE;
}
#endif

#endif
/*
** Function: LoadSi3217xPatch
**
** Description: 
** Load patch from external file defined as 'RevBPatch'
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** broadcast:  broadcast flag
**
** Return:
** 0
*/
static int LoadSi3217xPatch (proslicChanType *pProslic, const proslicPatch *pPatch,int broadcast){ 
    int32 loop; 
    uInt8 jmp_table=PATCH_JMPTBL_START_ADDR;
    uInt8 channel;
    if (pPatch == NULL)
        return 0;
    if (broadcast){
        setUserModeBroadcast(pProslic,TRUE);
        channel = BROADCAST;
    }
    else{
        setUserMode (pProslic,TRUE); /*make sure we are in user mode to load patch*/
        channel = pProslic->channel;
    }

#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sloading patch: %08lx\n", LOGPRINT_PREFIX, pPatch->patchSerial);
#endif

    WriteReg (pProHW, channel, SI3217X_COM_REG_JMPEN,0); /*disable Patch RAM*/

    for (loop=0;loop<PATCH_NUM_ENTRIES;loop++){
        /*zero out the jump table*/
        WriteReg (pProHW, channel, jmp_table,0);
        WriteReg (pProHW, channel, jmp_table+1,0);
        
        jmp_table+=2;
    }

    WriteRAM(pProHW, channel,SI3217X_COM_RAM_PRAM_ADDR, 0); /*write patch ram address register
                                              If the data is all 0, you have hit the end of the programmed values and can stop loading.*/
    for (loop=0; loop<PATCH_MAX_SIZE; loop++){ 
        if (pPatch->patchData[loop] != 0){
            if ((pProslic->deviceId->chipRev < 3) && broadcast)
                WriteRAM(pProHW, channel,SI3217X_COM_RAM_PRAM_ADDR, loop<<19); /*write patch ram address register (only necessary for broadcast rev c and earlier)*/
            WriteRAM(pProHW, channel,SI3217X_COM_RAM_PRAM_DATA,pPatch->patchData[loop]<<9); /*loading patch, note. data is shifted*/
        }
        else
            loop = 1024;
    }
    /* Delay 1 mSec to ensure last RAM write completed - this should be quicker than doing a SPI access
       to confirm the status register.
     */
#ifdef REALTEK_PATCH_FOR_SILAB
    Delay(pProTimer, 1); //Delay1ms(1);////Delay(pProHW, 1); /*   changed by HenryHuang   */
#else
    Delay(pProHW, 1); 
#endif

    /*zero out RAM_ADDR_HI*/
    WriteReg (pProHW, channel, SI3217X_COM_REG_RAM_ADDR_HI,0);

    jmp_table=PATCH_JMPTBL_START_ADDR;
    for (loop=0;loop<PATCH_NUM_ENTRIES;loop++){
        /* Load the jump table with the new values.*/
        if (pPatch->patchEntries[loop] != 0){
            WriteReg (pProHW, channel, jmp_table,(pPatch->patchEntries[loop])&0xff);
            WriteReg (pProHW, channel, jmp_table+1,pPatch->patchEntries[loop]>>8);
        }
        jmp_table+=2;
    }

    WriteRAM(pProHW,channel,SI3217X_COM_RAM_PATCHID,pPatch->patchSerial); /*write patch identifier*/

    /* Write patch support RAM locations (if any) */
    for (loop=0; loop<PATCH_MAX_SUPPORT_RAM; loop++){
        if(pPatch->psRamAddr[loop] != 0) {
            WriteRAM(pProHW,channel,pPatch->psRamAddr[loop],pPatch->psRamData[loop]);
        }
        else {
            loop = PATCH_MAX_SUPPORT_RAM;
        }
    }

#ifdef DISABLE_VERIFY_PATCH
    WriteReg (pProHW, channel, JMPEN,1); /*enable the patch (do not enable if you want to verify)*/
#endif
    if (broadcast){
        setUserModeBroadcast(pProslic,FALSE);
    }
    else {
        setUserMode(pProslic,FALSE); /*turn off user mode*/
    }
    return 0;
}

/*
** Function: handleError
**
** Description: 
** Called whenever an error is encountered with the proslic
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** fault: error code
**
** Return:
** none
*/
static void handleError (proslicChanType *pProslic, errorCodeType fault){
#ifdef ENABLE_DEBUG
    LOGPRINT ("%sError encountered\n\n", LOGPRINT_PREFIX);
#endif
    /*TODO: add something to recover from power alarm here?*/
    SILABS_UNREFERENCED_PARAMETER(pProslic);
    SILABS_UNREFERENCED_PARAMETER(fault);
}


/*
** Functions below are defined in header file and can be called by external files
*/

/*
**
** PROSLIC INITIALIZATION FUNCTIONS
**
*/

/*
** Function: Si3217x_ShutdownChannel
**
** Description: 
** Safely shutdown channel w/o interruptions to
** other active channels
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
**
** Return:
** 0

*/
int Si3217x_ShutdownChannel (proslicChanType_ptr pProslic){
    /*
    ** set linefeed to open state, powerdown dcdc converter
    */
    if(pProslic->channelType != PROSLIC) 
        return RC_CHANNEL_TYPE_ERR;

    Si3217x_SetLinefeedStatus(pProslic,LF_OPEN);
    Delay(pProTimer,10);
    Si3217x_PowerDownConverter(pProslic);
    
    return RC_NONE;
}

/*
** Function: Si3217x_VerifyControlInterface
**
** Description: 
** Check control interface readback cababilities
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
**
** Return:
** 0
*/
int Si3217x_VerifyControlInterface (proslicChanType_ptr pProslic)
{
    int16 i;
    int16 numOfChan;

    if (isVerifiedProslic(pProslic) == RC_CHANNEL_TYPE_ERR) 
	{
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
                LOGPRINT("%sSkipping Control Interface Verification on Non-ProSLIC Channel %d\n", 
                    LOGPRINT_PREFIX,
                    pProslic->channel);
#endif

        return RC_CHANNEL_TYPE_ERR;
	}

    if (isBroadcastPossible(pProslic) == RC_BROADCAST_FAIL) {
        WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_PCMRXLO,0x5A);
        if (ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMRXLO) != 0x5A){
            pProslic->error = RC_SPI_FAIL;
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
                LOGPRINT("%sProslic %d registers not communicating.\n", LOGPRINT_PREFIX, pProslic->channel);
#endif
            return RC_SPI_FAIL;
        }

        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_UNUSED449,0x12345678L);
        if (ReadRAM(pProHW,pProslic->channel, SI3217X_COM_RAM_UNUSED449) != 0x12345678L){
            pProslic->error = RC_SPI_FAIL;

#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
                LOGPRINT("%sProslic %d RAM not communicating. RAM access fail.\n", 
                    LOGPRINT_PREFIX, pProslic->channel);
#endif
            return RC_SPI_FAIL;
        }
        return 0;
    }

    /* Broadcast OK */
    numOfChan = (int16)probeDaisyChain(pProslic);
    if (numOfChan == 0)
        return RC_SPI_FAIL;
#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT ("%sFound %d channels\n",LOGPRINT_PREFIX, numOfChan);
#endif
    WriteReg(pProHW,BROADCAST,SI3217X_COM_REG_PCMRXLO,0x5a);
    WriteRAM(pProHW,BROADCAST,SI3217X_COM_RAM_UNUSED449,0x12345678L);

    for (i=0;i<numOfChan;i++){
        /*Try to write innocuous register to test SPI is working*/
                
        if (ReadReg(pProHW,(uInt8)i,SI3217X_COM_REG_PCMRXLO) != 0x5A){
            handleError(pProslic,RC_SPI_FAIL);
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
                LOGPRINT("%sProslic %d not communicating. Register access fail.\n", LOGPRINT_PREFIX,i);
#endif
            return RC_SPI_FAIL;
        }       
        if (ReadRAM(pProHW,(uInt8)i,SI3217X_COM_RAM_UNUSED449) != 0x12345678L){
            handleError(pProslic,RC_SPI_FAIL);
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
                LOGPRINT("%sProslic %d not communicating. RAM access fail.\n", LOGPRINT_PREFIX,i);
#endif
            return RC_SPI_FAIL;
        }

    }
    return 0;
}



/*
** Function: Si3217x_Init_MultiBOM
**
** Description: 
** - probe SPI to establish daisy chain length
** - load patch
** - initialize general parameters
** - calibrate madc
** - bring up DC/DC converters
** - calibrate everything except madc & lb
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
**
** Return:
** error code
*/
#ifdef SIVOICE_MULTI_BOM_SUPPORT
int Si3217x_Init_MultiBOM (proslicChanType_ptr *pProslic, int size, int preset){

	if(preset < si3217x_genconf_multi_max_preset)
	{
	    /* Copy selected General Configuration parameters to Std structure */
		Si3217x_General_Configuration = Si3217x_General_Configuration_MultiBOM[preset];
	}
	else
	{
		return RC_INVALID_PRESET;
	}
	return Si3217x_Init(pProslic,size);
}
#endif



/*
** Function: Si3217x_Init_with_Options
**
** Description: 
** - probe SPI to establish daisy chain length
** - load patch
** - initialize general parameters
** - calibrate madc
** - bring up DC/DC converters
** - calibrate everything except madc & lb
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
**
** Return:
** error code
*/

int Si3217x_Init_with_Options (proslicChanType_ptr *pProslic, int size, initOptionsType init_opt){
    /*
    ** This function will initialize the chipRev and chipType members in pProslic
    ** as well as load the initialization structures.
    */
    uInt8 data;
    uInt8 calSetup[] = {0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80};  /* CALR0-CALR3(RevB), CALR0-CALR3(RevC) */ 
    int k;
    const proslicPatch *patch;
    uInt8 status;


	if(init_opt != INIT_REINIT)
	{
		/*
		** Identify channel type (ProSLIC or DAA) before initialization.
		** Channels identified as DAA channels will not be modified during
		** the proslic initialization.
        **
        ** Do not return if non-ProSLIC channel is found, simply skip it
		*/
		for (k=0;k<size;k++)
        {
			isVerifiedProslic(pProslic[k]);     /* Populate channelType parameter in channel struct */
	        #ifdef ENABLE_DEBUG
			if (pProslic[k]->debugMode)
            {
                const char fmt_string[] = "%sChannel %d : Type = %s\n";
			    if(pProslic[k]->channelType == PROSLIC)
                {
#ifndef REALTEK_PATCH_FOR_SILAB
					LOGPRINT(fmt_string, LOGPRINT_PREFIX, pProslic[k]->channel, "PROSLIC");
#else
					printk("%sChannel %d : Type = %s\n", LOGPRINT_PREFIX, pProslic[k]->channel, "PROSLIC");
#endif
                }
				else if(pProslic[k]->channelType == DAA)
                {
#ifndef REALTEK_PATCH_FOR_SILAB
					LOGPRINT(fmt_string, LOGPRINT_PREFIX, pProslic[k]->channel, "DAA");
#else
					printk("%sChannel %d : Type = %s\n", LOGPRINT_PREFIX, pProslic[k]->channel, "PROSLIC");
#endif
                }
				else
                {
#ifndef REALTEK_PATCH_FOR_SILAB
					LOGPRINT(fmt_string, LOGPRINT_PREFIX, pProslic[k]->channel, "UNKNOWN");
#else
					printk("%sChannel %d : Type = %s\n", LOGPRINT_PREFIX, pProslic[k]->channel, "PROSLIC");
#endif
                }
			}
	        #endif
		}


		/*
		** Read channel id to establish chipRev and chipType
		*/
		for (k=0;k<size;k++)
        {
			if(pProslic[k]->channelType == PROSLIC) 
            {
				data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3217X_COM_REG_ID);
				pProslic[k]->deviceId->chipRev = data&0x7;
				pProslic[k]->deviceId->chipType = getChipType(data);
	            #ifdef ENABLE_DEBUG
				if (pProslic[k]->debugMode)
                {
					LOGPRINT("si3217x : Channel %d : Type = %d\n", pProslic[k]->channel,pProslic[k]->deviceId->chipType);
				    LOGPRINT("si3217x : Channel %d : Rev  = %d\n", pProslic[k]->channel,pProslic[k]->deviceId->chipRev);
				}
	            #endif
			}
		}


		/*
		** Probe each channel and enable all channels that respond 
		*/
		for (k=0;k<size;k++)
		{
			if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
			{
				pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3217X_COM_REG_PCMRXLO,0x5a);
				if (pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3217X_COM_REG_PCMRXLO) != 0x5A)
				{
					pProslic[k]->channelEnable = 0;
					pProslic[k]->error = RC_SPI_FAIL;
					return RC_SPI_FAIL;
				}
			}
		}
	} /* init_opt */
     




	if(init_opt != INIT_NO_PATCH_LOAD) /* Must reload patch on single channel devices */
	{
		/*
		** Load patch (load on every channel since single channel device)
		*/
		for (k=0;k<size;k++)
		{
			if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
			{

                /* Select Patch*/
				if (pProslic[k]->deviceId->chipRev == SI3217X_REVB ) 
                {
                    status = (uInt8) Si3217x_RevB_SelectPatch(pProslic[k],&patch);
#ifdef REALTEK_PATCH_FOR_SILAB
					#ifdef ENABLE_DEBUG
					if (pProslic[k]->debugMode)
					{
						LOGPRINT("si3217x : Channel %d : SI3217X_REVB patch \n", pProslic[k]->channel);
					}
					#endif
#endif
                }
				else if (pProslic[k]->deviceId->chipRev == SI3217X_REVC ) 
                {
                    status = (uInt8) Si3217x_RevC_SelectPatch(pProslic[k],&patch);
#ifdef REALTEK_PATCH_FOR_SILAB
					#ifdef ENABLE_DEBUG
					if (pProslic[k]->debugMode)
					{
						LOGPRINT("si3217x : Channel %d : SI3217X_REVC patch \n", pProslic[k]->channel);
					}
					#endif
#endif
                }
                else
                {
	                #ifdef ENABLE_DEBUG
			        if (pProslic[k]->debugMode)
			        {
				        LOGPRINT("%sChannel %d : Unsupported Device Revision (%d)\n",
                        LOGPRINT_PREFIX,
                        pProslic[k]->channel,pProslic[k]->deviceId->chipRev );
			        }
                    #endif
			        pProslic[k]->channelEnable = 0;
			        pProslic[k]->error = RC_UNSUPPORTED_DEVICE_REV;
			        return RC_UNSUPPORTED_DEVICE_REV;
                }

                /* Load Patch */
                if(status == RC_NONE)
                {
				    Si3217x_LoadPatch(pProslic[k],patch);
                }
                else
                {
                    return status;
                }


                /* Optional Patch Verification */
	            #ifdef DISABLE_VERIFY_PATCH
	            #else
				data = (uInt8)Si3217x_VerifyPatch(pProslic[k],patch);
				if (data)
                {
					pProslic[k]->channelEnable=0;
					pProslic[k]->error = RC_PATCH_ERR;  
				} 
                else 
                {
					setUserMode(pProslic[k],TRUE);
					pProslic[k]->WriteRegX (pProslic[k]->pProHWX, pProslic[k]->channel,SI3217X_COM_REG_JMPEN,1);  
					setUserMode(pProslic[k],FALSE);
				}
	            #endif                          
			}
		}
	}/* init_opt */
    


    /*
    ** Load general parameters - includes all BOM dependencies
    */
    for (k=0;k<size;k++){ 
        if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
        {
            setUserMode(pProslic[k],TRUE);      

            if(pProslic[k]->deviceId->chipRev == SI3217X_REVB)
            {
                Si3217x_RevB_GenParamUpdate(pProslic[k],INIT_SEQ_PRE_CAL);
            }
            else
            {
                Si3217x_RevC_GenParamUpdate(pProslic[k],INIT_SEQ_PRE_CAL);
            }
			setUserMode(pProslic[k],FALSE);
        }
    }

	if(init_opt != INIT_NO_CAL) /* Must recal on single channel devices */
	{
		/*
		** Calibrate (madc offset)
		*/
		Si3217x_Calibrate(pProslic,size,calSetup,TIMEOUT_MADC_CAL);
	}/* init_opt */


    /*
    ** Bring up DC/DC converters sequentially to minimize
    ** peak power demand on VDC
    */
    for (k=0;k<size;k++)
    { 
        if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
        {
            setUserMode(pProslic[k],TRUE);      
            pProslic[k]->error = Si3217x_PowerUpConverter(pProslic[k]);
            setUserMode(pProslic[k],FALSE);
        }
    }

	if(init_opt != INIT_NO_CAL)
	{
	    /*
	    ** Calibrate remaining cals (except madc, lb)
	    */
        calSetup[1] = SI3217XB_CAL_STD_CALR1;
        calSetup[2] = SI3217XB_CAL_STD_CALR2;
        calSetup[5] = SI3217XC_CAL_STD_CALR1;
        calSetup[6] = SI3217XC_CAL_STD_CALR2;

	    Si3217x_Calibrate(pProslic,size,calSetup,TIMEOUT_GEN_CAL);
	}

    /*
    ** Apply post calibration general parameters 
    */
    for (k=0;k<size;k++){ 
        if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
        {
            setUserMode(pProslic[k],TRUE);     
            if(pProslic[k]->deviceId->chipRev == SI3217X_REVB)
            {
                Si3217x_RevB_GenParamUpdate(pProslic[k],INIT_SEQ_POST_CAL);
            }
            else
            {
                Si3217x_RevC_GenParamUpdate(pProslic[k],INIT_SEQ_POST_CAL);
            }
            setUserMode(pProslic[k],FALSE);  
        }
    }
    return 0;
}

/*
** Function: Si3217x_Init
**
** Description: 
** - probe SPI to establish daisy chain length
** - load patch
** - initialize general parameters
** - calibrate madc
** - bring up DC/DC converters
** - calibrate remaining items except madc & lb
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
**
** Return:
** error code
*/

int Si3217x_Init (proslicChanType_ptr *pProslic, int size){

	return Si3217x_Init_with_Options(pProslic,size,INIT_NO_OPT);
}



/*
** Function: Si3217x_Reinit
**
** Description: 
** Performs soft reset then calls Si3217x_Init
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
**
** Return:
** error code
*/

int Si3217x_Reinit (proslicChanType_ptr pProslic, int size){
    uInt8 lf;
    int retVal=0;
    int num_reinit_chan = 1;
    SILABS_UNREFERENCED_PARAMETER(size);

	lf = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED);
	Si3217x_PowerDownConverter(pProslic);
	Delay(pProTimer,10);
	/* One channel per device - no need to select channel to reset */
	WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_RESET,0x01);
	Delay(pProTimer,100);
	retVal = Si3217x_Init_with_Options(&pProslic,num_reinit_chan,INIT_REINIT);
	/* 
	** Restore entry linefeed state - if alarm occured, this
	** is likely to be the OPEN state, but this function should
	** be useful for all usage cases.
	*/
	WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED,lf); 
	return (retVal);
}


/*
** Function: Si3217x_PrintDebugReg
**
** Description: 
** Register dump utility
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
**
** Return:
** 0
*/
int Si3217x_PrintDebugReg (proslicChanType *pProslic){
#ifdef ENABLE_DEBUG
    uInt8 i;
    for (i=0;i<99;i++)
	{
        LOGPRINT ("%sRegister %d = %X\n",LOGPRINT_PREFIX, i,ReadReg(pProHW,pProslic->channel,i));
	}
#endif
	return 0;
}

/*
** Function: Si3217x_PrintDebugRAM
**
** Description: 
** Register dump utility
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
**
** Return:
** 0
*/
int Si3217x_PrintDebugRAM (proslicChanType *pProslic){
#ifdef ENABLE_DEBUG
	uInt16 i;
#ifdef REALTEK_PATCH_FOR_SILAB
	setUserMode(pProslic,TRUE);
    for (i=0;i<1624;i++)
	{
        LOGPRINT ("%sRAM %04d = 0x%08X\n",LOGPRINT_PREFIX,i,(unsigned int)(ReadRAM(pProHW,pProslic->channel,i)));
	}
	setUserMode(pProslic,FALSE);
#else
    for (i=0;i<1024;i++)
	{
        LOGPRINT ("%sRAM %d = %X\n",LOGPRINT_PREFIX,i,(unsigned int)(ReadRAM(pProHW,pProslic->channel,i)));
	}
#endif
#endif
    return 0;
}

/*
** Function: Si3217x_PrintDebugData
**
** Description: 
** Register and RAM dump utility
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** broadcast:  broadcast flag
**
** Return:
** 0
*/
int Si3217x_PrintDebugData (proslicChanType *pProslic){
	Si3217x_PrintDebugReg (pProslic);
	Si3217x_PrintDebugRAM (pProslic);
    return 0;
}



/*
** Function: Si3217x_LBCal
**
** Description: 
** Run canned longitudinal balance calibration.  Each channel
** may be calibrated in parallel since there are no shared
** resources between si3217x devices.
**
** Input Parameters: 
** pProslic: pointer to array of PROSLIC channel objects
** size:     number of PROSLIC channel objects   
**
** Return:
** 0
*/
int Si3217x_LBCal(proslicChanType_ptr *pProslic, int size)
{
    int k;
    int i;
    uInt8 data;
    int timeout = 0;

#ifdef DISABLE_MALLOC
    uInt8 lf[64]; 
	uInt8 enhance[64];

    if (size > 64) {
        LOGPRINT("%sToo many channels - wanted %d, max of %d\n",
                LOGPRINT_PREFIX,
                 size, 64);
        return RC_NO_MEM;
    }
#else
    uInt8 *lf;
	uInt8 *enhance;

#ifdef REALTEK_PATCH_FOR_SILAB
    lf = kmalloc(size * sizeof(uInt8), 0x20);	//GFP_ATOMIC
#else
    lf = malloc(size * sizeof(uInt8));
#endif
    if (lf == 0) {
        return RC_NO_MEM;
    }
#ifdef REALTEK_PATCH_FOR_SILAB
    enhance = kmalloc(size * sizeof(uInt8), 0x20);	//GFP_ATOMIC
#else
    enhance = malloc(size * sizeof(uInt8));
#endif
    if (enhance == 0) {
        return RC_NO_MEM;
    }
#endif


    /* Start Cal on each channel first */
    for (k=0;k<size;k++)
    {
        if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
        {
#ifdef ENABLE_DEBUG
            if(pProslic[k]->debugMode)
            {
                LOGPRINT("%sStarting LB Cal on channel %d\n",LOGPRINT_PREFIX,pProslic[k]->channel);
            }
#endif
            enhance[k] = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3217X_COM_REG_ENHANCE); 
			lf[k] = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3217X_COM_REG_LINEFEED); 

			/* Disable powersave mode and set linefeed to fwd active */
			Si3217x_SetPowersaveMode(pProslic[k],PWRSAVE_DISABLE);
			Si3217x_SetLinefeedStatus(pProslic[k],LF_FWD_ACTIVE);
                        
            pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3217X_COM_REG_CALR0,CAL_LB_ALL); /* enable LB cal */
            pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3217X_COM_REG_CALR3,0x80); /* start cal */

        }
    }

    /* Poll for completion sequentially */
    for (k=0; k<size; k++)
    {
        if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
        {
            i=0;
            do {
                data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,
                                             pProslic[k]->channel,
                                             SI3217X_COM_REG_CALR3);
                pProslic[k]->DelayX(pProslic[k]->pProTimerX, 10);

            } while (data&0x80 && ++i<=TIMEOUT_LB_CAL);

            if (i >= TIMEOUT_LB_CAL) 
			{
#ifdef ENABLE_DEBUG
                if (pProslic[k]->debugMode)
                    LOGPRINT("%sCalibration timeout channel %d\n",
                            LOGPRINT_PREFIX,
                             pProslic[k]->channel);
#endif
                pProslic[k]->error = RC_CAL_TIMEOUT;
                pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3217X_COM_REG_LINEFEED,LF_OPEN); 
                timeout = 1;
            } 
			else 
			{
                pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3217X_COM_REG_LINEFEED,lf[k]); 
            }
			pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3217X_COM_REG_ENHANCE,enhance[k]); 
        }
    }

#ifndef DISABLE_MALLOC
#ifdef REALTEK_PATCH_FOR_SILAB
    kfree(lf);
    kfree(enhance);
#else
    free(lf);
	free(enhance);
#endif
#endif
    if (timeout != 0) {
        return RC_CAL_TIMEOUT;
    } else {
        return RC_NONE;
    }
}


/*
** Function: Si3217x_GetLBCalResult
**
** Description: 
** Read applicable calibration coefficients
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** resultx:  pointer to 4 RAM results
**
** Return:
** 0
*/
int Si3217x_GetLBCalResult (proslicChanType *pProslic,int32 *result1,int32 *result2,int32 *result3,int32 *result4){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
    setUserMode(pProslic,TRUE);
    *result1 = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CMDAC_FWD);
    *result2 = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CMDAC_RVS);
    *result3 = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CAL_TRNRD_DACT);
    *result4 = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CAL_TRNRD_DACR);
    setUserMode(pProslic,FALSE);
    return RC_NONE;
}

/*
** Function: Si3217x_GetLBCalResultPacked
**
** Description: 
** Read applicable calibration coefficients
** and pack into single 32bit word
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** result:   pointer to packed result
**
** Return:
** 0
**
** Packed Result Format
**
** Bits 31:24   CMDAC_FWD[
*/
int Si3217x_GetLBCalResultPacked (proslicChanType *pProslic,int32 *result){
    int32 tmpResult;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    setUserMode(pProslic,TRUE);
    tmpResult = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CMDAC_FWD);
    *result = (tmpResult<<6)&0xff000000L;
    tmpResult = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CMDAC_RVS);
    *result |= (tmpResult>>1)&0x00ff0000L;
    tmpResult = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CAL_TRNRD_DACT);
    *result |= (tmpResult>>5)&0x0000ff00L;
    tmpResult = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CAL_TRNRD_DACR);
    *result |= (tmpResult>>13)&0x000000ffL;
    setUserMode(pProslic,FALSE);
    return RC_NONE;
}
/*
** Function: Si3217x_LoadPreviousLBCal
**
** Description: 
** Load applicable calibration coefficients
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** resultx:  pointer to 4 RAM results
**
** Return:
** 0
*/
int Si3217x_LoadPreviousLBCal (proslicChanType *pProslic,int32 result1,int32 result2,int32 result3,int32 result4){
    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
    setUserMode(pProslic,TRUE);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CMDAC_FWD,result1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CMDAC_RVS,result2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CAL_TRNRD_DACT,result3);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CAL_TRNRD_DACR,result4);
    setUserMode(pProslic,FALSE);
    return RC_NONE;
}


/*
** Function: Si3217x_LoadPreviousLBCalPacked
**
** Description: 
** Load applicable calibration coefficients
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** result:   pointer to packed cal results
**
** Return:
** 0
*/
int Si3217x_LoadPreviousLBCalPacked (proslicChanType *pProslic,int32 *result){
    int32 ramVal;
#ifdef API_TEST
    const char fmt_strng = "%sUNPACKED %s = %08x\n";
#endif
    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
    setUserMode(pProslic,TRUE);
    ramVal = (*result&0xff000000L)>>6;
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CMDAC_FWD,ramVal);
    ramVal = (*result&0x00ff0000L)<<1;
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CMDAC_RVS,ramVal);
    ramVal = (*result&0x0000ff00L)<<5;
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CAL_TRNRD_DACT,ramVal);
    ramVal = (*result&0x000000ffL)<<13;
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CAL_TRNRD_DACR,ramVal);
#ifdef API_TEST
    ramVal = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CMDAC_FWD);
    LOGPRINT (fmt_string, LOGPRINT_PREFIX, "CMDAC_FWD",ramVal);
    ramVal = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CMDAC_REV);
    LOGPRINT (fmt_string, LOGPRINT_PREFIX, "CMDAC_REF",ramVal);
    ramVal = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CAL_TRNRD_DACT);
    LOGPRINT (fmt_string, LOGPRINT_PREFIX, "CAL_TRNRD_DACT",ramVal);
    ramVal = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CAL_TRNRD_DACR);
    LOGPRINT (fmt_string, LOGPRINT_PREFIX, "CAL_TRNRD_DACR",ramVal);
#endif
    setUserMode(pProslic,FALSE);
    return RC_NONE;
}

/*
** Function: Si3217x_LoadRegTables
**
** Description: 
** Generic register and ram table loader
**
** Input Parameters:
** pProslic:  pointer to PROSLIC channel object
** pRamTable: pointer to PROSLIC ram table
** pRegTable: pointer to PROSLIC reg table
** size:      number of channels
**
** Return:
** 0
*/
int Si3217x_LoadRegTables (proslicChanType_ptr *pProslic, ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable, int size){
    uInt16 i;
    for (i=0;i<size;i++){
        if (pProslic[i]->channelEnable)
            LoadRegTables(pProslic[i],pRamTable,pRegTable,0);
    }
    return 0;
}


/*
** Function: Si3217x_LoadPatch
**
** Description: 
** Calls patch loading function
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** pPatch:     pointer to PROSLIC patch obj
**
** Returns:
** 0
*/
int Si3217x_LoadPatch (proslicChanType *pProslic, const proslicPatch *pPatch){ 
    LoadSi3217xPatch(pProslic,pPatch,0);
    return 0;
}

/*
** Function: Si3217x_VerifyPatch
**
** Description: 
** Veriy patch load
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** pPatch:     pointer to PROSLIC patch obj
**
** Returns:
** 0
*/
int Si3217x_VerifyPatch (proslicChanType *pProslic, const proslicPatch *pPatch){ 
    int loop;
    uInt8 jmp_table=82;
    uInt8 data; uInt32 ramdata;
    int err = 0;
    if (pPatch == NULL)
        return 0;
    setUserMode (pProslic,TRUE); /*make sure we are in user mode to read patch*/

    WriteReg (pProHW, pProslic->channel, SI3217X_COM_REG_JMPEN,0); /*disable the patch*/

    WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_PRAM_ADDR, 0); /*write patch ram address register*/
        
    /* If the data is all 0, you have hit the end of the programmed values and can stop loading.*/
    for (loop=0; loop<PATCH_MAX_SIZE; loop++){
        if (pPatch->patchData[loop] != 0){
            ramdata = ReadRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_PRAM_DATA); /*note. data is shifted*/
            if (pPatch->patchData[loop]<<9 != ramdata){
                loop = PATCH_MAX_SIZE;                  
                err = 1;
            }
        }
        else
            loop = PATCH_MAX_SIZE;
    }
        
    /*zero out RAM_ADDR_HI*/
    WriteReg (pProHW, pProslic->channel, SI3217X_COM_REG_RAM_ADDR_HI,0);

    
    jmp_table=PATCH_JMPTBL_START_ADDR;
    for (loop=0;loop<PATCH_NUM_ENTRIES;loop++){
        /* check the jump table with the new values.*/
        if (pPatch->patchEntries[loop] != 0){
            data = (uInt8)ReadReg (pProHW, pProslic->channel, jmp_table);
            if (data != ((pPatch->patchEntries[loop])&0xff))
                err = 1;
            data = ReadReg (pProHW, pProslic->channel, jmp_table+1);
            if (data != (pPatch->patchEntries[loop]>>8))
                err = 1;
        }
        jmp_table+=2;
    }
    if (err){
#ifdef ENABLE_DEBUG
        if (pProslic->debugMode)
            LOGPRINT("%sPatch data corrupted: channel %d\n",LOGPRINT_PREFIX,pProslic->channel);
#endif
    }
    else {
        WriteReg (pProHW, pProslic->channel, SI3217X_COM_REG_JMPEN,1); /*enable the patch*/
    }
    setUserMode(pProslic,FALSE); /*turn off user mode*/
    return err;
}


/*
** Function: Si3217x_SetLoopbackMode
**
** Description: 
** Program desired loopback test mode
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** newMode:    desired loopback mode tag
**
** Returns:
** 0
*/
int Si3217x_SetLoopbackMode (proslicChanType_ptr pProslic, ProslicLoopbackModes newMode){
    uInt8 regTemp;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
    regTemp = ReadReg (pProHW,pProslic->channel,SI3217X_COM_REG_LOOPBACK);
    switch (newMode){
    case PROSLIC_LOOPBACK_NONE:
        WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_LOOPBACK,regTemp&~(0x11));
        break;
    case PROSLIC_LOOPBACK_DIG:
        WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_LOOPBACK,regTemp|(0x1));
        break;
    case PROSLIC_LOOPBACK_ANA:
        WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_LOOPBACK,regTemp|(0x10));
        break;
    }
    return RC_NONE;
}

/*
** Function: Si3217x_SetMuteStatus
**
** Description: 
** configure RX and TX path mutes
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** muteEn:     mute configuration tag
**
** Returns:
** 0
*/
int Si3217x_SetMuteStatus (proslicChanType_ptr pProslic, ProslicMuteModes muteEn){
    uInt8 regTemp;
    uInt8 newRegValue;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    regTemp = ReadReg (pProHW,pProslic->channel,SI3217X_COM_REG_DIGCON);
    newRegValue = regTemp &~(0x3);

    WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_DIGCON,regTemp&~(0x3));
       
    if (muteEn & PROSLIC_MUTE_RX){
        newRegValue |= 1;    
    }
    if (muteEn & PROSLIC_MUTE_TX){
        newRegValue |= 2;    
    }

    if(newRegValue != regTemp)
    {
        WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_DIGCON,newRegValue);
    }
    return RC_NONE;
}

/*
** Function: Si3217x_EnableInterrupts
**
** Description: 
** Enables interrupts
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
**
** Returns:
** 0
*/
int Si3217x_EnableInterrupts (proslicChanType_ptr pProslic){

    if (pProslic->deviceId->chipRev == SI3217X_REVA){ /*madc_vbat broken in rev a*/
        WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN1,Si3217x_General_Configuration.irqen1&0x7f);
    } else {
        WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN1,Si3217x_General_Configuration.irqen1);
    }

    WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN2,Si3217x_General_Configuration.irqen2);

    if (pProslic->deviceId->chipRev == SI3217X_REVA){ /*madc_vbat broken in rev a*/
        WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN3,Si3217x_General_Configuration.irqen3&0xC0);
    } else {
        WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN3,Si3217x_General_Configuration.irqen3);
    }

    WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN4,Si3217x_General_Configuration.irqen4);

    return RC_NONE;
}


/*
** Function: Si3217x_DisableInterrupts
**
** Description: 
** Disables/clears interrupts
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
**
** Returns:
** 0
*/
int Si3217x_DisableInterrupts (proslicChanType_ptr pProslic){

    uInt8 data[4];

    /* Disable interrupts */
    WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN1,0);
    WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN2,0);
    WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN3,0);
    WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN4,0);

    /* Now clear them */
    data[0] = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ1);
    data[1] = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ2);
    data[2] = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ3);
    data[3] = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ4);

#ifdef GCI_MODE
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ1,data[0]); /*clear interrupts (gci only)*/
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ2,data[1]);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ3,data[2]);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ4,data[3]);
#endif
    return RC_NONE;
}

/*
** Function: Si3217x_CheckCIDBuffer
**
** Description: 
** configure fsk
**
** Input Parameters:
** pProslic:        pointer to PROSLIC channel obj
** fskBufAvail:     fsk buffer available flag
**
** Returns:
** 0
*/
int Si3217x_CheckCIDBuffer (proslicChanType *pProslic, uInt8 *fskBufAvail){
    uInt8 data;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ1);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ1,data); /*clear (for GCI)*/
    *fskBufAvail = (data&0x40) ? 1 : 0;

    return RC_NONE;
}


/*
**
** PROSLIC CONFIGURATION FUNCTIONS
**
*/

/*
** Function: Si3217x_RingSetup
**
** Description: 
** configure ringing
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** preset:     ring preset
**
** Returns:
** 0
*/
#ifndef DISABLE_RING_SETUP
int Si3217x_RingSetup (proslicChanType *pProslic, int preset){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RTPER,Si3217x_Ring_Presets[preset].rtper);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RINGFR,Si3217x_Ring_Presets[preset].freq);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RINGAMP,Si3217x_Ring_Presets[preset].amp);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RINGPHAS,Si3217x_Ring_Presets[preset].phas);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RINGOF,Si3217x_Ring_Presets[preset].offset);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_SLOPE_RING,Si3217x_Ring_Presets[preset].slope_ring);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_IRING_LIM,Si3217x_Ring_Presets[preset].iring_lim);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RTACTH,Si3217x_Ring_Presets[preset].rtacth);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RTDCTH,Si3217x_Ring_Presets[preset].rtdcth);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RTACDB,Si3217x_Ring_Presets[preset].rtacdb);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RTDCDB,Si3217x_Ring_Presets[preset].rtdcdb);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VOV_RING_BAT,Si3217x_Ring_Presets[preset].vov_ring_bat);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VOV_RING_GND,Si3217x_Ring_Presets[preset].vov_ring_gnd);

#ifndef NOCLAMP_VBATR
	/* Always limit VBATR_EXPECT to the general configuration maximum */
	if(Si3217x_Ring_Presets[preset].vbatr_expect > Si3217x_General_Configuration.vbatr_expect)
	{
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VBATR_EXPECT,Si3217x_General_Configuration.vbatr_expect);
    #ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("%sRingSetup : VBATR_EXPECT : Clamped to Gen Conf Limit\n",LOGPRINT_PREFIX);
		}
    #endif
	}
	else
	{
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VBATR_EXPECT,Si3217x_Ring_Presets[preset].vbatr_expect);
	}

#else
	WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VBATR_EXPECT,Si3217x_Ring_Presets[preset].vbatr_expect);
#endif


    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_RINGTALO,Si3217x_Ring_Presets[preset].talo);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_RINGTAHI,Si3217x_Ring_Presets[preset].tahi);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_RINGTILO,Si3217x_Ring_Presets[preset].tilo);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_RINGTIHI,Si3217x_Ring_Presets[preset].tihi);
  
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VBAT_TRACK_MIN_RNG,Si3217x_Ring_Presets[preset].dcdc_vref_min_rng);

    /* 
    ** LPR Handler for RevB/RevC Differences 
    **
    ** If revC and USERSTAT == 0x01, adjust RINGCON and clear USERSTAT
    */
    if ((pProslic->deviceId->chipRev == SI3217X_REVC )&&(Si3217x_Ring_Presets[preset].userstat == 0x01))
    {
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_RINGCON,0x80);
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_USERSTAT,0x00);
    }
    else
    {
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_RINGCON,Si3217x_Ring_Presets[preset].ringcon);
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_USERSTAT,Si3217x_Ring_Presets[preset].userstat);
    }


    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VCM_RING,Si3217x_Ring_Presets[preset].vcm_ring);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VCM_RING_FIXED,Si3217x_Ring_Presets[preset].vcm_ring_fixed);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DELTA_VCM,Si3217x_Ring_Presets[preset].delta_vcm);

    setUserMode(pProslic,TRUE);

    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DCDC_RNGTYPE,Si3217x_Ring_Presets[preset].dcdc_rngtype);


    /* 
    ** If multi bom supported **AND** a buck boost converter
    ** is being used, force dcdc_rngtype to be fixed.
    */
#ifdef SIVOICE_MULTI_BOM_SUPPORT
#define DCDC_RNGTYPE_BKBT     0L
    /* Automatically adjust DCDC_RNGTYPE */
    if(Si3217x_General_Configuration.bom_option == BO_DCDC_BUCK_BOOST) 
    {
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DCDC_RNGTYPE,DCDC_RNGTYPE_BKBT);
    }
#endif

    setUserMode(pProslic,FALSE);

    return RC_NONE;
}
#endif
/*
** Function: Si3217x_ToneGenSetup
**
** Description: 
** configure tone generators
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** preset:     tone generator preset
**
** Returns:
** 0
*/
#ifndef DISABLE_TONE_SETUP
int Si3217x_ToneGenSetup (proslicChanType *pProslic, int preset){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_OSC1FREQ,Si3217x_Tone_Presets[preset].osc1.freq);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_OSC1AMP,Si3217x_Tone_Presets[preset].osc1.amp);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_OSC1PHAS,Si3217x_Tone_Presets[preset].osc1.phas);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O1TAHI,(Si3217x_Tone_Presets[preset].osc1.tahi));
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O1TALO,(Si3217x_Tone_Presets[preset].osc1.talo));
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O1TIHI,(Si3217x_Tone_Presets[preset].osc1.tihi));
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O1TILO,(Si3217x_Tone_Presets[preset].osc1.tilo));
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_OSC2FREQ,Si3217x_Tone_Presets[preset].osc2.freq);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_OSC2AMP,Si3217x_Tone_Presets[preset].osc2.amp);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_OSC2PHAS,Si3217x_Tone_Presets[preset].osc2.phas);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O2TAHI,(Si3217x_Tone_Presets[preset].osc2.tahi));
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O2TALO,(Si3217x_Tone_Presets[preset].osc2.talo));
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O2TIHI,(Si3217x_Tone_Presets[preset].osc2.tihi));
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O2TILO,(Si3217x_Tone_Presets[preset].osc2.tilo));
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_OMODE,(Si3217x_Tone_Presets[preset].omode));
    return RC_NONE;
}
#endif
/*
** Function: Si3217x_FSKSetup
**
** Description: 
** configure fsk
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** preset:     fsk preset
**
** Returns:
** 0
*/
#ifndef DISABLE_FSK_SETUP
int Si3217x_FSKSetup (proslicChanType *pProslic, int preset){
    uInt8 data; 

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O1TAHI,0);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O1TIHI,0);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O1TILO,0);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_O1TALO,0x13);

    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_OMODE);
    if (Si3217x_FSK_Presets[preset].eightBit)
        data |= 0x80;
    else 
        data &= ~(0x80);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_FSKDEPTH,Si3217x_FSK_Presets[preset].fskdepth);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_OMODE,data);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_FSK01,Si3217x_FSK_Presets[preset].fsk01);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_FSK10,Si3217x_FSK_Presets[preset].fsk10);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_FSKAMP0,Si3217x_FSK_Presets[preset].fskamp0);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_FSKAMP1,Si3217x_FSK_Presets[preset].fskamp1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_FSKFREQ0,Si3217x_FSK_Presets[preset].fskfreq0);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_FSKFREQ1,Si3217x_FSK_Presets[preset].fskfreq1);
    return RC_NONE;
}
#endif

/*
 * Function: Si3217x_ModifyStartBits
 * 
 * Description: To change the FSK start/stop bits field.
 * Returns RC_NONE if OK.
 */
int Si3217x_ModifyCIDStartBits(proslicChanType_ptr pProslic, uInt8 enable_startStop)
{
	uInt8 data;

	if(pProslic->channelType != PROSLIC) 
	{
        return RC_CHANNEL_TYPE_ERR;
    }

	data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_OMODE);
	
	if(enable_startStop == FALSE)
	{
		data &= ~0x80;
	}
	else
	{
		data |= 0x80;
	}

	WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_OMODE,data);

	return RC_NONE;
}

/*
** Function: Si3217x_DTMFDecodeSetup
**
** Description: 
** configure dtmf decode
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** preset:     dtmf preset
**
** Returns:
** 0
*/
#ifndef DISABLE_DTMF_SETUP
int Si3217x_DTMFDecodeSetup (proslicChanType *pProslic, int preset){
     
    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
   
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_B0_1,Si3217x_DTMFDec_Presets[preset].dtmfdtf_b0_1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_B1_1,Si3217x_DTMFDec_Presets[preset].dtmfdtf_b1_1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_B2_1,Si3217x_DTMFDec_Presets[preset].dtmfdtf_b2_1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_A1_1,Si3217x_DTMFDec_Presets[preset].dtmfdtf_a1_1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_A2_1,Si3217x_DTMFDec_Presets[preset].dtmfdtf_a2_1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_B0_2,Si3217x_DTMFDec_Presets[preset].dtmfdtf_b0_2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_B1_2,Si3217x_DTMFDec_Presets[preset].dtmfdtf_b1_2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_B2_2,Si3217x_DTMFDec_Presets[preset].dtmfdtf_b2_2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_A1_2,Si3217x_DTMFDec_Presets[preset].dtmfdtf_a1_2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_A2_2,Si3217x_DTMFDec_Presets[preset].dtmfdtf_a2_2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_B0_3,Si3217x_DTMFDec_Presets[preset].dtmfdtf_b0_3);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_B1_3,Si3217x_DTMFDec_Presets[preset].dtmfdtf_b1_3);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_B2_3,Si3217x_DTMFDec_Presets[preset].dtmfdtf_b2_3);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_A1_3,Si3217x_DTMFDec_Presets[preset].dtmfdtf_a1_3);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_DTMFDTF_A2_3,Si3217x_DTMFDec_Presets[preset].dtmfdtf_a2_3);
    return RC_NONE;
}
#endif


/*
** Function: PROSLIC_SetProfile
**
** Description: 
** set country profile of the proslic
*/
int Si3217x_SetProfile (proslicChanType *pProslic, int preset){
    /*TO DO
      Will be filled in at a later date*/
    SILABS_UNREFERENCED_PARAMETER(pProslic);
    SILABS_UNREFERENCED_PARAMETER(preset);
    return 0;
}

/*
** Function: PROSLIC_ZsynthSetup
**
** Description: 
** configure impedence synthesis
*/
#ifndef DISABLE_ZSYNTH_SETUP
int Si3217x_ZsynthSetup (proslicChanType *pProslic, int preset){
    uInt8 lf;
    uInt8 cal_en = 0;
    uInt16 timer = 500;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    lf = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED,0);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C0,Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c0);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C1,Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C2,Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C3,Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c3);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C0,Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c0);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C1,Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C2,Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C3,Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c3);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECFIR_C2,Si3217x_Impedance_Presets[preset].hybrid.ecfir_c2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECFIR_C3,Si3217x_Impedance_Presets[preset].hybrid.ecfir_c3);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECFIR_C4,Si3217x_Impedance_Presets[preset].hybrid.ecfir_c4);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECFIR_C5,Si3217x_Impedance_Presets[preset].hybrid.ecfir_c5);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECFIR_C6,Si3217x_Impedance_Presets[preset].hybrid.ecfir_c6);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECFIR_C7,Si3217x_Impedance_Presets[preset].hybrid.ecfir_c7);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECFIR_C8,Si3217x_Impedance_Presets[preset].hybrid.ecfir_c8);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECFIR_C9,Si3217x_Impedance_Presets[preset].hybrid.ecfir_c9);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECIIR_B0,Si3217x_Impedance_Presets[preset].hybrid.ecfir_b0);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECIIR_B1,Si3217x_Impedance_Presets[preset].hybrid.ecfir_b1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECIIR_A1,Si3217x_Impedance_Presets[preset].hybrid.ecfir_a1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ECIIR_A2,Si3217x_Impedance_Presets[preset].hybrid.ecfir_a2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ZSYNTH_A1,Si3217x_Impedance_Presets[preset].zsynth.zsynth_a1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ZSYNTH_A2,Si3217x_Impedance_Presets[preset].zsynth.zsynth_a2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ZSYNTH_B1,Si3217x_Impedance_Presets[preset].zsynth.zsynth_b1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ZSYNTH_B0,Si3217x_Impedance_Presets[preset].zsynth.zsynth_b0);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ZSYNTH_B2,Si3217x_Impedance_Presets[preset].zsynth.zsynth_b2);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_RA,Si3217x_Impedance_Presets[preset].zsynth.ra);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACGAIN,Si3217x_Impedance_Presets[preset].txgain);
	WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACGAIN_SAVE,Si3217x_Impedance_Presets[preset].rxgain);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACGAIN,Si3217x_Impedance_Presets[preset].rxgain);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_B0_1,Si3217x_Impedance_Presets[preset].rxachpf_b0_1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_B1_1,Si3217x_Impedance_Presets[preset].rxachpf_b1_1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_A1_1,Si3217x_Impedance_Presets[preset].rxachpf_a1_1);

    /*
    ** Scale based on desired gain plan
    */
    Si3217x_dbgSetTXGain(pProslic,Si3217x_Impedance_Presets[preset].txgain_db,preset,TXACGAIN_SEL);
    Si3217x_dbgSetRXGain(pProslic,Si3217x_Impedance_Presets[preset].rxgain_db,preset,RXACGAIN_SEL);
    Si3217x_TXAudioGainSetup(pProslic,TXACGAIN_SEL);
    Si3217x_RXAudioGainSetup(pProslic,RXACGAIN_SEL);

    /* 
    ** Perform Zcal in case OHT used (eg. no offhook event to trigger auto Zcal) 
    */
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_CALR0,0x00);   
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_CALR1,0x40);   
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_CALR2,0x00); 
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_CALR3,0x80);  /* start cal */

    /* Wait for zcal to finish */
    do {
        cal_en = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_CALR3);
        Delay(pProTimer,1);
        timer--;
    }while((cal_en&0x80)&&(timer>0));  
     
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED,lf);

    if(timer > 0) return RC_NONE;
    else          return RC_CAL_TIMEOUT;
}
#endif



/*
** Function: PROSLIC_GciCISetup
**
** Description: 
** configure CI bits (GCI mode)
*/
#ifndef DISABLE_CI_SETUP
int Si3217x_GciCISetup (proslicChanType *pProslic, int preset){
    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_GCI_CI,Si3217x_CI_Presets[preset].gci_ci);
    return RC_NONE;
}
#endif


/*
** Function: PROSLIC_ModemDetSetup
**
** Description: 
** configure modem detector
*/
int Si3217x_ModemDetSetup (proslicChanType *pProslic, int preset){
    /*TO DO
      Will be filled in at a later date*/
    SILABS_UNREFERENCED_PARAMETER(pProslic);
    SILABS_UNREFERENCED_PARAMETER(preset);
    return 0;
}

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
*/
int Si3217x_TXAudioGainSetup (proslicChanType *pProslic, int preset){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACGAIN,Si3217x_audioGain_Presets[preset].acgain);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C0,Si3217x_audioGain_Presets[preset].aceq_c0);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C1,Si3217x_audioGain_Presets[preset].aceq_c1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C2,Si3217x_audioGain_Presets[preset].aceq_c2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C3,Si3217x_audioGain_Presets[preset].aceq_c3);

    return RC_NONE;
}

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
*/
int Si3217x_RXAudioGainSetup (proslicChanType *pProslic, int preset){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
	WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACGAIN_SAVE,Si3217x_audioGain_Presets[preset].acgain);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACGAIN,Si3217x_audioGain_Presets[preset].acgain);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C0,Si3217x_audioGain_Presets[preset].aceq_c0);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C1,Si3217x_audioGain_Presets[preset].aceq_c1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C2,Si3217x_audioGain_Presets[preset].aceq_c2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C3,Si3217x_audioGain_Presets[preset].aceq_c3);

    return RC_NONE;
}


/*
** Function: PROSLIC_AudioGainScale
**
** Description: 
** Multiply path gain by passed value for PGA and EQ scale (no reference to dB, 
** multiply by a scale factor)
*/
int Si3217x_AudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale,int rx_tx_sel){
 
	if(rx_tx_sel == TXACGAIN_SEL)
	{
		Si3217x_audioGain_Presets[TXACGAIN_SEL].acgain = (Si3217x_Impedance_Presets[preset].txgain/1000)*pga_scale;
		if (Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c0 & 0x10000000L)
			Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c0 |= 0xf0000000L;
		if (Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c1 & 0x10000000L)
			Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c1 |= 0xf0000000L;
		if (Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c2 & 0x10000000L)
			Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c2 |= 0xf0000000L;
		if (Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c3 & 0x10000000L)
			Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c3 |= 0xf0000000L;
		Si3217x_audioGain_Presets[TXACGAIN_SEL].aceq_c0 = ((int32)Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c0/1000)*eq_scale;
		Si3217x_audioGain_Presets[TXACGAIN_SEL].aceq_c1 = ((int32)Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c1/1000)*eq_scale;
		Si3217x_audioGain_Presets[TXACGAIN_SEL].aceq_c2 = ((int32)Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c2/1000)*eq_scale;
		Si3217x_audioGain_Presets[TXACGAIN_SEL].aceq_c3 = ((int32)Si3217x_Impedance_Presets[preset].audioEQ.txaceq_c3/1000)*eq_scale;

		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACGAIN,Si3217x_audioGain_Presets[TXACGAIN_SEL].acgain);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C0,Si3217x_audioGain_Presets[TXACGAIN_SEL].aceq_c0);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C1,Si3217x_audioGain_Presets[TXACGAIN_SEL].aceq_c1);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C2,Si3217x_audioGain_Presets[TXACGAIN_SEL].aceq_c2);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACEQ_C3,Si3217x_audioGain_Presets[TXACGAIN_SEL].aceq_c3);
	}
	else
	{
		Si3217x_audioGain_Presets[RXACGAIN_SEL].acgain = (Si3217x_Impedance_Presets[preset].rxgain/1000)*pga_scale;
		if (Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c0 & 0x10000000L)
			Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c0 |= 0xf0000000L;
		if (Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c1 & 0x10000000L)
			Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c1 |= 0xf0000000L;
		if (Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c2 & 0x10000000L)
			Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c2 |= 0xf0000000L;
		if (Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c3 & 0x10000000L)
			Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c3 |= 0xf0000000L;
		Si3217x_audioGain_Presets[RXACGAIN_SEL].aceq_c0 = ((int32)Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c0/1000)*eq_scale;
		Si3217x_audioGain_Presets[RXACGAIN_SEL].aceq_c1 = ((int32)Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c1/1000)*eq_scale;
		Si3217x_audioGain_Presets[RXACGAIN_SEL].aceq_c2 = ((int32)Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c2/1000)*eq_scale;
		Si3217x_audioGain_Presets[RXACGAIN_SEL].aceq_c3 = ((int32)Si3217x_Impedance_Presets[preset].audioEQ.rxaceq_c3/1000)*eq_scale;
	
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACGAIN_SAVE,Si3217x_audioGain_Presets[RXACGAIN_SEL].acgain);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACGAIN,Si3217x_audioGain_Presets[RXACGAIN_SEL].acgain);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C0,Si3217x_audioGain_Presets[RXACGAIN_SEL].aceq_c0);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C1,Si3217x_audioGain_Presets[RXACGAIN_SEL].aceq_c1);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C2,Si3217x_audioGain_Presets[RXACGAIN_SEL].aceq_c2);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACEQ_C3,Si3217x_audioGain_Presets[RXACGAIN_SEL].aceq_c3);
	}
    return 0;
}
int Si3217x_TXAudioGainScale (proslicChanType *pProslic, int preset,uInt32 pga_scale, uInt32 eq_scale)
{
	return Si3217x_AudioGainScale(pProslic,preset,pga_scale,eq_scale,TXACGAIN_SEL);
}
int Si3217x_RXAudioGainScale (proslicChanType *pProslic, int preset,uInt32 pga_scale, uInt32 eq_scale)
{
	return Si3217x_AudioGainScale(pProslic,preset,pga_scale,eq_scale,RXACGAIN_SEL);
}


/*
** Function: PROSLIC_DCFeedSetup
**
** Description: 
** configure dc feed
*/
#ifndef DISABLE_DCFEED_SETUP
int Si3217x_DCFeedSetupCfg (proslicChanType *pProslic, Si3217x_DCfeed_Cfg *cfg, int preset){
    uInt8 lf;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
    lf = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED,0);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_SLOPE_VLIM,cfg[preset].slope_vlim);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_SLOPE_RFEED,cfg[preset].slope_rfeed);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_SLOPE_ILIM,cfg[preset].slope_ilim);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_SLOPE_DELTA1,cfg[preset].delta1);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_SLOPE_DELTA2,cfg[preset].delta2);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_V_VLIM,cfg[preset].v_vlim);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_V_RFEED,cfg[preset].v_rfeed);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_V_ILIM,cfg[preset].v_ilim);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CONST_RFEED,cfg[preset].const_rfeed);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_CONST_ILIM,cfg[preset].const_ilim);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_I_VLIM,cfg[preset].i_vlim);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_LCRONHK,cfg[preset].lcronhk);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_LCROFFHK,cfg[preset].lcroffhk);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_LCRDBI,cfg[preset].lcrdbi);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_LONGHITH,cfg[preset].longhith);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_LONGLOTH,cfg[preset].longloth);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_LONGDBI,cfg[preset].longdbi);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_LCRMASK,cfg[preset].lcrmask);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_LCRMASK_POLREV,cfg[preset].lcrmask_polrev);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_LCRMASK_STATE,cfg[preset].lcrmask_state);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_LCRMASK_LINECAP,cfg[preset].lcrmask_linecap);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VCM_OH,cfg[preset].vcm_oh);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VOV_BAT,cfg[preset].vov_bat);
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VOV_GND,cfg[preset].vov_gnd);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_LINEFEED,lf);

    return RC_NONE;
}

int Si3217x_DCFeedSetup (proslicChanType *pProslic, int preset){
	Si3217x_DCFeedSetupCfg(pProslic,Si3217x_DCfeed_Presets,preset);
	return RC_NONE;
}

#endif


/*
** Function: PROSLIC_GPIOSetup
**
** Description: 
** configure gpio
*/
#ifndef DISABLE_GPIO_SETUP
int Si3217x_GPIOSetup (proslicChanType *pProslic){
    uInt8 data;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_GPIO);
    data |= Si3217x_GPIO_Configuration.outputEn << 4;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_GPIO,data);
    data = Si3217x_GPIO_Configuration.analog << 4;
    data |= Si3217x_GPIO_Configuration.direction;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_GPIO_CFG1,data);
    data = Si3217x_GPIO_Configuration.manual << 4;
    data |= Si3217x_GPIO_Configuration.polarity;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_GPIO_CFG2,data);
    data |= Si3217x_GPIO_Configuration.openDrain;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_GPIO_CFG3,data);

    return RC_NONE;
}
#endif

/*
** Function: PROSLIC_PulseMeterSetup
**
** Description: 
** configure pulse metering
*/
#ifndef DISABLE_PULSE_SETUP
int Si3217x_PulseMeterSetup (proslicChanType *pProslic, int preset){
    int retval = 0;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    setUserMode(pProslic,1); 
    if(pProslic->deviceId->chipRev == SI3217X_REVB)
    {
        retval = Si3217x_RevB_PulseMeterSetup(pProslic, preset);
    }
    else
    {
        retval = Si3217x_RevC_PulseMeterSetup(pProslic, preset);    
    }
    setUserMode(pProslic,0); 

    return retval;
}
#endif

/*
** Function: PROSLIC_PCMSetup
**
** Description: 
** configure pcm
*/
#ifndef DISABLE_PCM_SETUP
int Si3217x_PCMSetup(proslicChanType *pProslic, int preset){
    uInt8 regTemp;
        
    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    if (Si3217x_PCM_Presets[preset].widebandEn)
	{
#ifdef REALTEK_PATCH_FOR_SILAB
    	PRINT_MSG("Si3217x in Wideband.\n");
#endif
        if ((pProslic->deviceId->chipType == SI32171)||( pProslic->deviceId->chipType == SI32172))
            return RC_UNSUPPORTED_FEATURE;

		/* TXIIR settings */
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B0_1,0x3538E80L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B1_1,0x3538E80L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_A1_1,0x1AA9100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B0_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B1_2,0x2505400L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B2_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_A1_2,0x2CB8100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_A2_2,0x1D7FA500L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B0_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B1_3,0x1276D00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B2_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_A1_3,0x2335300L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_A2_3,0x19D5F700L);
		/* RXIIR settings */
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B0_1,0x6A71D00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B1_1,0x6A71D00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_A1_1,0x1AA9100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B0_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B1_2,0x2505400L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B2_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_A1_2,0x2CB8100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_A2_2,0x1D7FA500L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B0_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B1_3,0x1276D00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B2_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_A1_3,0x2335300L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_A2_3,0x19D5F700L);
		/* 
		** RXHPF  
		** Note:  Calling ProSLIC_ZsynthSetup() will overwrite some
		**        of these values.  ProSLIC_PCMSetup() should always
		**        be called after loading coefficients when using
		**        wideband mode
		*/
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_B0_1,0x7CFF900L);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_B1_1,0x18300700L);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_A1_1,0x79FF201L);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_B0_2,0x7CEDA1DL);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_B1_2,0x106320D4L);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_B2_2,0x7CEDA1DL);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_A1_2,0xF9A910FL);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_A2_2,0x185FFDA8L);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACHPF_GAIN,0x08000000L);
		/* TXHPF */
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACHPF_B0_1,0x0C7FF4CEL);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACHPF_B1_1,0x13800B32L);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACHPF_A1_1,0x079FF201L);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACHPF_B0_2,0x030FDD10L);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACHPF_B1_2,0x19E0996CL);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACHPF_B2_2,0x030FDD10L);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACHPF_A1_2,0x0F9A910FL);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACHPF_A2_2,0x185FFDA8L);
		WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACHPF_GAIN,0x0CD30000L);

        regTemp = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_DIGCON);
#ifndef DISABLE_HPF_WIDEBAND
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_DIGCON,regTemp&~(0xC)); /* Enable HPF */
#else
#ifdef REALTEK_PATCH_FOR_SILAB
		WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_DIGCON,regTemp|(0xC)); /* Disable HPF */
#else
        WriteReg(pProHW,pProslic->channel,DIGCON,regTemp|(0xC)); /* Disable HPF */
#endif
#endif
        regTemp = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_ENHANCE);
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_ENHANCE,regTemp|1);
    } else {
#ifdef REALTEK_PATCH_FOR_SILAB
    	PRINT_MSG("Si3217x in Narrowband.\n");
#endif
        regTemp = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_DIGCON);
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_DIGCON,regTemp&~(0xC));
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B0_1,0x3538E80L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B1_1,0x3538E80L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_A1_1,0x1AA9100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B0_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B1_2,0x2505400L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B2_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_A1_2,0x2CB8100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_A2_2,0x1D7FA500L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B0_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B1_3,0x1276D00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_B2_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_A1_3,0x2335300L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACIIR_A2_3,0x19D5F700L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B0_1,0x6A71D00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B1_1,0x6A71D00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_A1_1,0x1AA9100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B0_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B1_2,0x2505400L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B2_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_A1_2,0x2CB8100L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_A2_2,0x1D7FA500L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B0_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B1_3,0x1276D00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_B2_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_A1_3,0x2335300L);
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_RXACIIR_A2_3,0x19D5F700L);
        regTemp = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_ENHANCE);
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_ENHANCE,regTemp&~(1));
    }
    regTemp = Si3217x_PCM_Presets[preset].pcmFormat;
    regTemp |= Si3217x_PCM_Presets[preset].pcm_tri << 5;
    regTemp |= Si3217x_PCM_Presets[preset].alaw_inv << 2;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMMODE,regTemp);
    regTemp = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMTXHI);
    regTemp &= 3;
    regTemp |= Si3217x_PCM_Presets[preset].tx_edge<<4;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMTXHI,regTemp);

    return RC_NONE;
}
#endif

/*
** Function: PROSLIC_PCMSetup
**
** Description: 
** configure pcm
*/
int Si3217x_PCMTimeSlotSetup (proslicChanType *pProslic, uInt16 rxcount, uInt16 txcount){
    uInt8 data;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    data = txcount & 0xff;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMTXLO,data);
    data = txcount >> 8 ;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMTXHI,data);
    data = rxcount & 0xff;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMRXLO,data);
    data = rxcount >> 8 ;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMRXHI,data);

    return RC_NONE;
}

/*
**
** PROSLIC CONTROL FUNCTIONS
**
*/



/*
** Function: PROSLIC_GetInterrupts
**
** Description: 
** Reads interrupt registers status (IRQ1-4)
**
** Returns:
** array of pending interrupts of type proslicIntType*
**
*/
int Si3217x_GetInterrupts (proslicChanType *pProslic,proslicIntType *pIntData){
    /*Reading the interrupt registers and will clear any bits which are set (SPI mode only)
      Multiple interrupts may occur at once so bear that in mind when
      writing an interrupt handling routine*/
    uInt8 data[4];
    int i,j,k;
	int safetyInt = 0;
    pIntData->number = 0;
        
    if(pProslic->channelType != PROSLIC) {
        return RC_IGNORE;  /* Ignore non-ProSLIC channels - return 0 */
    }

    data[0] = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ1);
    data[1] = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ2);
    data[2] = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ3);
    data[3] = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ4);
#ifdef GCI_MODE
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ1,data[0]); /*clear interrupts (gci only)*/
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ2,data[1]);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ3,data[2]);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQ4,data[3]);
#endif
    for (i=0;i<4;i++){
        for (j=0;j<8;j++){
            if (data[i]&(1<<j)){
                switch (j + (i*8)){
                    /* IRQ 1 */
                case IRQ_OSC1_T1_SI3217X:   /* IRQ1.0 */
                    k=IRQ_OSC1_T1;
                    break;
                case IRQ_OSC1_T2_SI3217X:   /* IRQ1.1 */
                    k=IRQ_OSC1_T2;
                    break;
                case IRQ_OSC2_T1_SI3217X:   /* IRQ1.2 */
                    k=IRQ_OSC2_T1;
                    break;
                case IRQ_OSC2_T2_SI3217X:   /* IRQ1.3 */
                    k=IRQ_OSC2_T2;
                    break;
                case IRQ_RING_T1_SI3217X:   /* IRQ1.4 */
                    k=IRQ_RING_T1;
                    break;
                case IRQ_RING_T2_SI3217X:   /* IRQ1.5 */
                    k=IRQ_RING_T2;
                    break;
                case IRQ_FSKBUF_AVAIL_SI3217X:/* IRQ1.6 */
                    k=IRQ_FSKBUF_AVAIL;
                    break;
                case IRQ_VBAT_SI3217X:      /* IRQ1.7 */
                    k=IRQ_VBAT;
                    break;
                    /* IRQ2 */
                case IRQ_RING_TRIP_SI3217X: /* IRQ2.0 */
                    k=IRQ_RING_TRIP;
                    break;
                case IRQ_LOOP_STAT_SI3217X: /* IRQ2.1 */
                    k=IRQ_LOOP_STATUS;
                    break;
                case IRQ_LONG_STAT_SI3217X: /* IRQ2.2 */
                    k=IRQ_LONG_STAT;
                    break;
                case IRQ_VOC_TRACK_SI3217X: /* IRQ2.3 */
                    k=IRQ_VOC_TRACK;
                    break;
                case IRQ_DTMF_SI3217X:      /* IRQ2.4 */
                    k=IRQ_DTMF;
                    break;
                case IRQ_INDIRECT_SI3217X:  /* IRQ2.5 */
                    k=IRQ_INDIRECT;
                    break;
                case IRQ_TXMDM_SI3217X:     /* IRQ2.6 */
                    k = IRQ_TXMDM;
                    break;
                case IRQ_RXMDM_SI3217X:     /* IRQ2.7 */
                    k=IRQ_RXMDM;
                    break;
                    /* IRQ3 */
                case IRQ_P_HVIC_SI3217X:       /* IRQ3.0 */
                    k=IRQ_P_HVIC;
					safetyInt = 1;
                    break;
                case IRQ_P_THERM_SI3217X:       /* IRQ3.1 */
                    k=IRQ_P_THERM;
					safetyInt = 1;
                    break;
                case IRQ_PQ3_SI3217X:       /* IRQ3.2 */
                    k=IRQ_PQ3;  
                    break;
                case IRQ_PQ4_SI3217X:       /* IRQ3.3 */
                    k=IRQ_PQ4;
                    break;
                case IRQ_PQ5_SI3217X:       /* IRQ3.4 */
                    k=IRQ_PQ5;
                    break;
                case IRQ_PQ6_SI3217X:       /* IRQ3.5 */
                    k=IRQ_PQ6;
                    break;
                case IRQ_DSP_SI3217X:       /* IRQ3.6 */
                    k=IRQ_DSP;
                    break;
                case IRQ_MADC_FS_SI3217X:       /* IRQ3.7 */
                    k=IRQ_MADC_FS;
                    break;
                    /* IRQ4 */
                case IRQ_USER_0_SI3217X: /* IRQ4.0 */
                    k=IRQ_USER_0;
                    break;
                case IRQ_USER_1_SI3217X: /* IRQ4.1 */
                    k=IRQ_USER_1;
                    break;
                case IRQ_USER_2_SI3217X: /* IRQ4.2 */
                    k=IRQ_USER_2;
                    break;
                case IRQ_USER_3_SI3217X: /* IRQ4.3 */
                    k=IRQ_USER_3;
                    break;
                case IRQ_USER_4_SI3217X: /* IRQ4.4 */
                    k=IRQ_USER_4;
                    break;
                case IRQ_USER_5_SI3217X: /* IRQ4.5 */
                    k=IRQ_USER_5;
                    break;
                case IRQ_USER_6_SI3217X: /* IRQ4.6 */
                    k=IRQ_USER_6;
                    break;
                case IRQ_USER_7_SI3217X: /* IRQ4.7 */
                    k=IRQ_USER_7;
                    break;
                default:
                    k=0xff;
                }/* switch */
                pIntData->irqs[pIntData->number] =      k;              
                pIntData->number++;
                        
            }/* if */
        }/* for */      

    }
	/* Check for improper Ring Exit if safety interrupt */
	if(safetyInt)
	{
		if(isReinitRequired(pProslic))
		{
			return RC_REINIT_REQUIRED;
		}
	}

    return pIntData->number;
}


/*
** Function: PROSLIC_ReadHookStatus
**
** Description: 
** Determine hook status
*/
int Si3217x_ReadHookStatus (proslicChanType *pProslic,uInt8 *pHookStat){
    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
    if (ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_LCRRTP) & 2)
        *pHookStat=PROSLIC_OFFHOOK;
    else
        *pHookStat=PROSLIC_ONHOOK;
    return RC_NONE;
}

/*
** Function: PROSLIC_SetLinefeedStatus
**
** Description: 
** Sets linefeed state
*/
int Si3217x_SetLinefeedStatus (proslicChanType *pProslic,uInt8 newLinefeed){
    uInt8 regTemp;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteReg (pProHW, pProslic->channel, SI3217X_COM_REG_LINEFEED,newLinefeed);
    if ((newLinefeed&0xf) == LF_RINGING) {
        /*disable vbat interrupt during ringing*/
        regTemp = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN1);
        WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN1,regTemp&(~0x80));
    }
    else{
        if (pProslic->deviceId->chipRev != 0) {
            regTemp = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN1);
            if (regTemp != 0)
                WriteReg (pProHW,pProslic->channel,SI3217X_COM_REG_IRQEN1,0x80 | regTemp);
        }
    }
    return RC_NONE;
}

/*
** Function: Si324x_SetLinefeedStatusBroadcast
**
** Description: 
** Sets linefeed state
*/
int Si3217x_SetLinefeedStatusBroadcast (proslicChanType *pProslic, uInt8 newLinefeed){

    WriteReg (pProHW, BROADCAST, SI3217X_COM_REG_LINEFEED,newLinefeed);
    return 0;
}


/*
** Function: PROSLIC_PolRev
**
** Description: 
** Sets polarity reversal state
*/
int Si3217x_PolRev (proslicChanType *pProslic,uInt8 abrupt, uInt8 newPolRevState){
    uInt8 data=0;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

	/* Cannont polrev/wink while low power mode is active */
	Si3217x_SetPowersaveMode(pProslic,PWRSAVE_DISABLE);

	if (abrupt)
        data = 1;

    switch (newPolRevState)
	{
		case POLREV_STOP:
			data |= 0;
			break;
		case POLREV_START:
			data |= 2;
			break;
		case WINK_START:
			data = 6; /* No OR - abrupt wink not allowed */
			break;
		case WINK_STOP:
			data = 4;
			break;
    }

    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_POLREV,data);
        
    return RC_NONE;
}

/*
** Function: PROSLIC_GPIOControl
**
** Description: 
** Sets gpio of the proslic
*/
int Si3217x_GPIOControl (proslicChanType *pProslic,uInt8 *pGpioData, uInt8 read){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    if (read)
        *pGpioData = 0xf & ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_GPIO);
    else{
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_GPIO,(*pGpioData)|(ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_GPIO)&0xf0));
    }
    return RC_NONE;
}


/*
** Function: PROSLIC_MWISetup
**
** Description: 
** Modify default vpk or lcrmask_mwi.  Passing 0 will result in the parameter
** not being modified.
**
** Note:  This feature is implemented in patch
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3217x_MWISetup (proslicChanType *pProslic,uInt16 vpk_mag,uInt16 lcrmask_mwi)
{
uInt32 ram_val;

	/* Verify MWI not enabled - cannot make changes while enabled */
	if(ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_USERSTAT) & SIL_MWI_USTAT_SET)
	{
		return RC_MWI_IN_USE;
	}

	/* Voltage mod */
	if(vpk_mag > 0)  /* Skip if 0 passed */
	{
		/* Clamp supplied value to allowable range */
		if(vpk_mag > SIL_MWI_VPK_MAX)		vpk_mag = SIL_MWI_VPK_MAX;
		if(vpk_mag < SIL_MWI_VPK_MIN)		vpk_mag = SIL_MWI_VPK_MIN;
		ram_val = vpk_mag * SCALE_V_MADC * 1000L;
		WriteRAM(pProHW,pProslic->channel,SI3217X_PRAM_VBATH_NEON,ram_val);
	}

	/* LCRMASK mod */
	if(lcrmask_mwi > 0)  /* Skip if 0 passed */
	{
		/* Clamp supplied value to allowable range */
		if(lcrmask_mwi > SIL_MWI_LCRMASK_MAX)		lcrmask_mwi = SIL_MWI_LCRMASK_MAX;
		if(lcrmask_mwi < SIL_MWI_LCRMASK_MIN)		lcrmask_mwi = SIL_MWI_LCRMASK_MIN;
		ram_val = lcrmask_mwi * SIL_MWI_LCRMASK_SCALE;
		WriteRAM(pProHW,pProslic->channel,SI3217X_PRAM_LCRMASK_MWI,ram_val);
	}
	
	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_MWIEnable
**
** Description: 
** Enables MWI feature
**
** Note:  This feature is implemented in patch
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3217x_MWIEnable (proslicChanType *pProslic)
{
uInt8 val;
	
	/* 
	** Check for conditions that would prevent enabling MWI
	*/
	ProSLIC_ReadHookStatus(pProslic,&val);
	if(val != PROSLIC_ONHOOK)	
	{
		return RC_MWI_ENABLE_FAIL;
	}
	else
	{	/* Clear DIAG1 and set USERSTAT enable bit */
		ProSLIC_SetPowersaveMode(pProslic,PWRSAVE_DISABLE);
		WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_DIAG1,SIL_MWI_TOGGLE_LOW);
		val = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_USERSTAT);
		WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_USERSTAT,val|SIL_MWI_USTAT_SET);
	}
	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_MWIDisable
**
** Description: 
** Disables MWI feature
**
** Note:  This feature is implemented in patch
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3217x_MWIDisable (proslicChanType *pProslic)
{
uInt8 val;

	/* Clear DIAG1 and USERSTAT enable bit */
	WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_DIAG1,SIL_MWI_TOGGLE_LOW);
	val = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_USERSTAT);
	WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_USERSTAT,val&SIL_MWI_USTAT_CLEAR);

	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_SetMWIState
**
** Description: 
** Set MWI State.  Must be enabled via ProSLIC_MWIEnable prior
** to calling this function
**
** Note:  This feature is implemented in patch
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3217x_SetMWIState(proslicChanType *pProslic,uInt8 flash_on)
{

	/* Only continue if MWI is enabled */
	if(ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_USERSTAT) & SIL_MWI_USTAT_SET)
	{
		if(flash_on)
		{
			WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_DIAG1,SIL_MWI_TOGGLE_HIGH);
		}
		else
		{
			WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_DIAG1,SIL_MWI_TOGGLE_LOW);
		}
	}
	else
	{
		return RC_MWI_NOT_ENABLED;
	}

	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_GetMWIState
**
** Description: 
** Read MWI State.  Must be enabled via ProSLIC_MWIEnable prior
** to calling this function
**
** Note:  This feature is implemented in patch
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3217x_GetMWIState(proslicChanType *pProslic)
{

	/* Only continue if MWI is enabled */
	if(ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_USERSTAT) & SIL_MWI_USTAT_SET)
	{
		if(ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_DIAG1) & SIL_MWI_TOGGLE_HIGH)
			return SIL_MWI_FLASH_ON;
		else
			return SIL_MWI_FLASH_OFF;
	}
	else
	{
		return RC_MWI_NOT_ENABLED;
	}

	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_MWI
**
** Description: 
** implements message waiting indicator
**
** Message waiting (neon flashing) requires modifications to vbath_expect 
** and slope_vlim. The old values are restored to turn off the lamp. We 
** assume all channels set up the same. During off-hook event lamp must 
** be disabled manually. 
**
** Deprecated.   Use Si3217x_SetMWIState()
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3217x_MWI (proslicChanType *pProslic,uInt8 lampOn){
static int32 vbath_save = 0;
static int32 slope_vlim_save = 0;
uInt8 hkStat; 
int32 slope_vlim_tmp;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    slope_vlim_tmp = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_SLOPE_VLIM);
    Si3217x_ReadHookStatus(pProslic,&hkStat);

    if (lampOn && (hkStat == PROSLIC_OFFHOOK) ) {/*cant neon flash during offhook*/
#ifdef ENABLE_DEBUG
        if (pProslic->debugMode)        
            LOGPRINT ("Si3217x MWI cannot operate offhook\n");
#endif
        return RC_LINE_IN_USE;
    }

    if (lampOn) {
        if (slope_vlim_tmp != 0x8000000L) { /*check we're not already on*/
            vbath_save = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VBATH_EXPECT);
            slope_vlim_save = slope_vlim_tmp;
        }
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VBATH_EXPECT,0x7AE147AL);/*120V*/
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_SLOPE_VLIM,0x8000000L);
    } else {
        if (vbath_save != 0) { /*check we saved some valid value first*/
            WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VBATH_EXPECT,vbath_save);
            WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_SLOPE_VLIM,slope_vlim_save);
        }
    }

    return RC_NONE;
}
#endif

/*
** Function: PROSLIC_StartGenericTone
**
** Description: 
** start tone generators
*/
int Si3217x_ToneGenStart (proslicChanType *pProslic,uInt8 timerEn){
    uInt8 data;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sToneGenStart\n",LOGPRINT_PREFIX);
#endif
    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_OCON);
    data |= 0x11 + (timerEn ? 0x66 : 0);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_OCON,data);

    return RC_NONE;
}


/*
** Function: PROSLIC_StopTone
**
** Description: 
** Stops tone generators
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_ToneGenStop (proslicChanType *pProslic){
    uInt8 data;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sToneGenStop\n",LOGPRINT_PREFIX);
#endif
    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_OCON);
    data &= ~(0x77);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_OCON,data);

    return RC_NONE;
}

/*
** Function: PROSLIC_StartRing
**
** Description: 
** start ring generator
*/
int Si3217x_RingStart (proslicChanType *pProslic){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sRingStart\n",LOGPRINT_PREFIX);
#endif
    Si3217x_SetLinefeedStatus(pProslic,LF_RINGING);

    return RC_NONE;
}


/*
** Function: PROSLIC_StopRing
**
** Description: 
** Stops ring generator
*/
int Si3217x_RingStop (proslicChanType *pProslic){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sRingStop\n",LOGPRINT_PREFIX);
#endif
    Si3217x_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);

    return RC_NONE;
}

/*
** Function: PROSLIC_EnableCID
**
** Description: 
** enable fsk
*/
int Si3217x_EnableCID (proslicChanType *pProslic){
    uInt8 data;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sEnableCID\n", LOGPRINT_PREFIX);
#endif
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_OCON,0);

    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_OMODE);
    data |= 0xA;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_OMODE,data);

    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_OCON,0x5);

    return RC_NONE;
}

/*
** Function: PROSLIC_DisableCID
**
** Description: 
** disable fsk
*/
int Si3217x_DisableCID (proslicChanType *pProslic){
    uInt8 data;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sDisableCID\n",LOGPRINT_PREFIX);
#endif
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_OCON,0);
    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_OMODE);
    data &= ~(0x8);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_OMODE,data);

    return RC_NONE;
}

/*
** Function: PROSLIC_SendCID
**
** Description: 
** send fsk data
*/
int Si3217x_SendCID (proslicChanType *pProslic, uInt8 *buffer, uInt8 numBytes){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sSendCID\n",LOGPRINT_PREFIX);
#endif
    while (numBytes-- > 0){
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_FSKDAT,*(buffer++));
    }
    return RC_NONE;
}

/*
** Function: PROSLIC_StartPCM
**
** Description: 
** Starts PCM
*/
int Si3217x_PCMStart (proslicChanType *pProslic){
    uInt8 data;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sPCMStart\n",LOGPRINT_PREFIX);
#endif
    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMMODE);
    data |= 0x10;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMMODE,data);

    return RC_NONE;
}


/*
** Function: PROSLIC_StopPCM
**
** Description: 
** Disables PCM
*/
int Si3217x_PCMStop (proslicChanType *pProslic){
    uInt8 data;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sPCMStop\n", LOGPRINT_PREFIX);
#endif
    data = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMMODE);
    data &= ~(0x10);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PCMMODE,data);

    return RC_NONE;
}



/*
** Function: PROSLIC_ReadDTMFDigit
**
** Description: 
** Read DTMF digit (would be called after DTMF interrupt to collect digit)
*/
int Si3217x_DTMFReadDigit (proslicChanType *pProslic,uInt8 *pDigit){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

#ifndef PBX_DEMO_ULSB
    if (pProslic->deviceId->chipType >= SI32175 && pProslic->deviceId->chipType <= SI32176)
        return RC_UNSUPPORTED_FEATURE;
#endif
    *pDigit = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_TONDTMF) & 0xf;
#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sDTMFReadDigit %d\n",LOGPRINT_PREFIX,*pDigit);
#endif
        
    return RC_NONE;
}

/*
** Function: PROSLIC_PLLFreeRunStart
**
** Description: 
** initiates pll free run mode
*/
int Si3217x_PLLFreeRunStart (proslicChanType *pProslic){
    uInt8 tmp;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    tmp = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_ENHANCE);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_ENHANCE,tmp|0x4);

    return RC_NONE;
}

/*
** Function: PROSLIC_PLLFreeRunStop
**
** Description: 
** exit pll free run mode
*/
int Si3217x_PLLFreeRunStop (proslicChanType *pProslic){
    uInt8 tmp;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    tmp = ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_ENHANCE);
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_ENHANCE,tmp&~(0x4));

    return RC_NONE;
}


/*
** Function: PROSLIC_PulseMeterEnable
**
** Description: 
** enable pulse meter generation
*/
int Si3217x_PulseMeterEnable (proslicChanType *pProslic){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PMCON,ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_PMCON) | (0x01));

    return RC_NONE;   
}


/*
** Function: PROSLIC_PulseMeterDisable
**
** Description: 
** disable pulse meter generation
*/
int Si3217x_PulseMeterDisable (proslicChanType *pProslic){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PMCON,ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_PMCON) & ~(0x05));

    return RC_NONE;   
}

/*
** Function: PROSLIC_PulseMeterStart
**
** Description: 
** start pulse meter tone
*/
int Si3217x_PulseMeterStart (proslicChanType *pProslic){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PMCON,ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_PMCON) | (0x05));

    return RC_NONE;   
}

/*
** Function: PROSLIC_PulseMeterStop
**
** Description: 
** stop pulse meter tone
*/
int Si3217x_PulseMeterStop (proslicChanType *pProslic){
    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PMCON,ReadReg(pProHW,pProslic->channel,SI3217X_COM_REG_PMCON) & ~(0x04));

    return RC_NONE;
}

/*
** Function: PROSLIC_dbgSetDCFeed
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3217x_dbgSetDCFeed (proslicChanType *pProslic, uInt32 v_vlim_val, uInt32 i_ilim_val, int32 preset){
/* Note:  * needs more descriptive return codes in the event of an out of range arguement */
    uInt16 vslope = 160;
    uInt16 rslope = 720;
    uInt32 vscale1 = 1386; 
    uInt32 vscale2 = 1422;   /* 1386x1422 = 1970892 broken down to minimize trunc err */
    uInt32 iscale1 = 913;
    uInt32 iscale2 = 334;    /* 913x334 = 304942 */
    uInt32 i_rfeed_val, v_rfeed_val, const_rfeed_val, i_vlim_val, const_ilim_val, v_ilim_val;
    int32 signedVal;
    /* Set Linefeed to open state before modifying DC Feed */

    /* Assumptions must be made to minimize computations.  This limits the
    ** range of available settings, but should be more than adequate for
    ** short loop applications.
    **
    ** Assumtions:
    ** 
    ** SLOPE_VLIM      =>  160ohms
    ** SLOPE_RFEED     =>  720ohms
    ** I_RFEED         =>  3*I_ILIM/4
    ** 
    ** With these assumptions, the DC Feed parameters now become 
    **
    ** Inputs:      V_VLIM, I_ILIM
    ** Constants:   SLOPE_VLIM, SLOPE_ILIM, SLOPE_RFEED, SLOPE_DELTA1, SLOPE_DELTA2
    ** Outputs:     V_RFEED, V_ILIM, I_VLIM, CONST_RFEED, CONST_ILIM
    **
    */

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    /* Validate arguements */
    if((i_ilim_val < 15)||(i_ilim_val > 45)) return 1;   /* need error code */
    if((v_vlim_val < 30)||(v_vlim_val > 52)) return 1;   /* need error code */

    /* Calculate voltages in mV and currents in uA */
    v_vlim_val *= 1000;
    i_ilim_val *= 1000;

    /* I_RFEED */
    i_rfeed_val = (3*i_ilim_val)/4;

    /* V_RFEED */
    v_rfeed_val = v_vlim_val - (i_rfeed_val*vslope)/1000;

    /* V_ILIM */ 
    v_ilim_val = v_rfeed_val - (rslope*(i_ilim_val - i_rfeed_val))/1000;

    /* I_VLIM */
    i_vlim_val = (v_vlim_val*1000)/4903;

    /* CONST_RFEED */
    signedVal = v_rfeed_val * (i_ilim_val - i_rfeed_val);
    signedVal /= (v_rfeed_val - v_ilim_val);
    signedVal = i_rfeed_val + signedVal;

    /* signedVal in uA here */
    signedVal *= iscale1;
    signedVal /= 100;
    signedVal *= iscale2;
    signedVal /= 10;

    if(signedVal < 0)
    {
        const_rfeed_val = (signedVal)+ (1L<<29);
    }
    else
    {
        const_rfeed_val = signedVal & 0x1FFFFFFF;
    }

    /* CONST_ILIM */
    const_ilim_val = i_ilim_val;

    /* compute RAM values */
    v_vlim_val *= vscale1;
    v_vlim_val /= 100;
    v_vlim_val *= vscale2;
    v_vlim_val /= 10;

    v_rfeed_val *= vscale1;
    v_rfeed_val /= 100;
    v_rfeed_val *= vscale2;
    v_rfeed_val /= 10;

    v_ilim_val *= vscale1;
    v_ilim_val /= 100;
    v_ilim_val *= vscale2;
    v_ilim_val /= 10;

    const_ilim_val *= iscale1;
    const_ilim_val /= 100;
    const_ilim_val *= iscale2;
    const_ilim_val /= 10;

    i_vlim_val *= iscale1;
    i_vlim_val /= 100;
    i_vlim_val *= iscale2;
    i_vlim_val /= 10;

    Si3217x_DCfeed_Presets[preset].slope_vlim = 0x18842BD7L;
    Si3217x_DCfeed_Presets[preset].slope_rfeed = 0x1E8886DEL;
    Si3217x_DCfeed_Presets[preset].slope_ilim = 0x40A0E0L;
    Si3217x_DCfeed_Presets[preset].delta1 = 0x1EABA1BFL;
    Si3217x_DCfeed_Presets[preset].delta2 = 0x1EF744EAL;
    Si3217x_DCfeed_Presets[preset].v_vlim = v_vlim_val;
    Si3217x_DCfeed_Presets[preset].v_rfeed = v_rfeed_val;
    Si3217x_DCfeed_Presets[preset].v_ilim = v_ilim_val;
    Si3217x_DCfeed_Presets[preset].const_rfeed = const_rfeed_val;
    Si3217x_DCfeed_Presets[preset].const_ilim = const_ilim_val;
    Si3217x_DCfeed_Presets[preset].i_vlim = i_vlim_val;
        
    return RC_NONE;
}

/*
** Function: PROSLIC_dbgSetDCFeedVopen
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage.
** Entry I_ILIM value will be used.
*/
int Si3217x_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val, int32 preset)
{
    uInt32 i_ilim_val;
    uInt32 iscale1 = 913;
    uInt32 iscale2 = 334;    /* 913x334 = 304942 */


    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    /* Read present CONST_ILIM value */
    i_ilim_val = Si3217x_DCfeed_Presets[preset].const_ilim;


    i_ilim_val /= iscale2;
    i_ilim_val /= iscale1;

    return Si3217x_dbgSetDCFeed(pProslic,v_vlim_val,i_ilim_val,preset);
}

/*
** Function: PROSLIC_dbgSetDCFeedIloop
**
** Description: 
** provisionary function for setting up
** dcfeed given desired loop current.
** Entry V_VLIM value will be used.
*/
int Si3217x_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset)
{
    uInt32 v_vlim_val;
    uInt32 vscale1 = 1386; 
    uInt32 vscale2 = 1422;   /* 1386x1422 = 1970892 broken down to minimize trunc err */

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    /* Read present V_VLIM value */
    v_vlim_val = Si3217x_DCfeed_Presets[preset].v_vlim;

    v_vlim_val /= vscale2;
    v_vlim_val /= vscale1;

    return Si3217x_dbgSetDCFeed(pProslic,v_vlim_val,i_ilim_val, preset);
}




typedef struct
{
    uInt8   freq;
    ramData ringfr;      /* trise scale for trap */
    uInt32  ampScale;
} ProSLIC_SineRingFreqLookup;

typedef struct
{
    uInt8    freq;
    ramData  rtacth;
    ramData rtper;
    ramData rtdb;
} ProSLIC_SineRingtripLookup;

typedef struct
{
    uInt8   freq;
    uInt16  cfVal[6];
} ProSLIC_TrapRingFreqLookup;

typedef struct
{
    uInt8   freq;
    ramData rtper;
    ramData rtdb;
    uInt32  rtacth[6];
} ProSLIC_TrapRingtripLookup;




/*
** Function: PROSLIC_dbgRingingSetup
**
** Description: 
** Provisionary function for setting up
** Ring type, frequency, amplitude and dc offset.
** Main use will be by peek/poke applications.
*/
int Si3217x_dbgSetRinging (proslicChanType *pProslic, ProSLIC_dbgRingCfg *ringCfg, int preset){
    int errVal,i=0;
    uInt32 vScale = 1608872L;   /* (2^28/170.25)*((100+4903)/4903) */
    ramData dcdcVminTmp;

    const ProSLIC_SineRingFreqLookup sineRingFreqTable[] =
/*  Freq RINGFR, vScale */
        {{15, 0x7F6E930L, 18968L},
         {16, 0x7F5A8E0L, 20234L},
         {20, 0x7EFD9D5L, 25301L},
         {22, 0x7EC770AL, 27843L},
         {23, 0x7EAA6E2L, 29113L},
         {25, 0x7E6C925L, 31649L},
         {30, 0x7DBB96BL, 38014L},
         {34, 0x7D34155L, 42270L}, /* Actually 33.33Hz */
         {35, 0x7CEAD72L, 44397L},
         {40, 0x7BFA887L, 50802L},
         {45, 0x7AEAE74L, 57233L},
         {50, 0x79BC384L, 63693L},
         {0,0,0}}; /* terminator */

    const ProSLIC_SineRingtripLookup sineRingtripTable[] =
/*  Freq rtacth */
        { {15, 11440000L, 0x6A000L, 0x4000L },
          {16, 10810000L, 0x64000L, 0x4000L },
          {20, 8690000L,  0x50000L, 0x8000L }, 
          {22, 7835000L,  0x48000L, 0x8000L },
          {23, 7622000L,  0x46000L, 0x8000L }, 
          {25, 6980000L,  0x40000L, 0xA000L }, 
          {30, 5900000L,  0x36000L, 0xA000L }, 
          {34, 10490000L, 0x60000L, 0x6000L }, /* Actually 33.33 */
          {35, 10060000L, 0x5C000L, 0x6000L }, 
          {40, 8750000L,  0x50000L, 0x8000L }, 
          {45, 7880000L,  0x48000L, 0x8000L }, 
          {50, 7010000L,  0x40000L, 0xA000L }, 
          {0,0L}}; /* terminator */

    const ProSLIC_TrapRingFreqLookup trapRingFreqTable[] =
/*  Freq multCF11 multCF12 multCF13 multCF14 multCF15 multCF16*/
    {
        {15, {69,122, 163, 196, 222,244}},
        {16, {65,115, 153, 184, 208,229}},
        {20, {52,92, 122, 147, 167,183}},
        {22, {47,83, 111, 134, 152,166}},
        {23, {45,80, 107, 128, 145,159}},
        {25, {42,73, 98, 118, 133,146}},
        {30, {35,61, 82, 98, 111,122}},
        {34, {31,55, 73, 88, 100,110}},
        {35, {30,52, 70, 84, 95,104}},
        {40, {26,46, 61, 73, 83,91}},
        {45, {23,41, 54, 65, 74,81}},
        {50, {21,37, 49, 59, 67,73}},
        {0,{0L,0L,0L,0L}} /* terminator */
    }; 


    const ProSLIC_TrapRingtripLookup trapRingtripTable[] =
/*  Freq rtper rtdb rtacthCR11 rtacthCR12 rtacthCR13 rtacthCR14 rtacthCR15 rtacthCR16*/
    {
        {15, 0x6A000L,  0x4000L, {16214894L, 14369375L, 12933127L, 11793508L, 10874121L, 10121671L}},
        {16, 0x64000L,  0x4000L, {15201463L, 13471289L, 12124806L, 11056414L, 10194489L, 9489067L}},
        {20, 0x50000L,  0x6000L, {12161171L, 10777031L, 9699845L, 8845131L, 8155591L, 7591253L}},
        {22, 0x48000L,  0x6000L, {11055610L, 9797301L, 8818041L, 8041028L, 7414174L, 6901139L}},
        {23, 0x46000L,  0x6000L, {10574931L, 9371331L, 8434648L, 7691418L, 7091818L, 6601090L}},
        {25, 0x40000L,  0x8000L, {9728937L, 8621625L, 7759876L, 7076105L, 6524473L, 6073003L}},
        {30, 0x36000L,  0x8000L, {8107447L, 7184687L, 6466563L, 5896754L, 5437061L, 5060836L}},
        {34, 0x60000L,  0x6000L, {7297432L, 6466865L, 5820489L, 5307609L, 4893844L, 4555208L}},
        {35, 0x5C000L,  0x6000L, {6949240L, 6158303L, 5542769L, 5054361L, 4660338L, 4337859L}},
        {40, 0x50000L,  0x6000L, {6080585L, 5388516L, 4849923L, 4422565L, 4077796L, 3795627L}},
        {45, 0x48000L,  0x6000L, {5404965L, 4789792L, 4311042L, 3931169L, 3624707L, 3373890L}},
        {50, 0x40000L,  0x8000L, {4864468L, 4310812L, 3879938L, 3538052L, 3262236L, 3036501L}},
        {0,0x0L, 0x0L, {0L,0L,0L,0L}} /* terminator */
    }; 

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    errVal = RC_NONE;

    switch(ringCfg->ringtype)
    {
    case ProSLIC_RING_SINE:
        i=0;
        do
        {
            if(sineRingFreqTable[i].freq >= ringCfg->freq) 
            {
                break;
            }
            i++;
        } while (sineRingFreqTable[i].freq);

        /* Set to maximum value if exceeding maximum value from table */
        if(sineRingFreqTable[i].freq == 0)
        {
            i--;
            errVal = RC_RING_V_LIMITED;
        }

        /* Update RINGFR RINGAMP, RINGOFFSET, and RINGCON */
        Si3217x_Ring_Presets[preset].freq = sineRingFreqTable[i].ringfr;
        Si3217x_Ring_Presets[preset].amp = ringCfg->amp * sineRingFreqTable[i].ampScale;
        Si3217x_Ring_Presets[preset].offset = ringCfg->offset * vScale;
        Si3217x_Ring_Presets[preset].phas = 0L;

        /* Don't alter anything in RINGCON other than clearing the TRAP bit */
        Si3217x_Ring_Presets[preset].ringcon &= 0xFE;

        Si3217x_Ring_Presets[preset].rtper = sineRingtripTable[i].rtper;
        Si3217x_Ring_Presets[preset].rtacdb = sineRingtripTable[i].rtdb;
        Si3217x_Ring_Presets[preset].rtdcdb = sineRingtripTable[i].rtdb;
        Si3217x_Ring_Presets[preset].rtdcth = 0xFFFFFFFL;
        Si3217x_Ring_Presets[preset].rtacth = sineRingtripTable[i].rtacth;
        break;

    case ProSLIC_RING_TRAP_CF11:  
    case ProSLIC_RING_TRAP_CF12:     
    case ProSLIC_RING_TRAP_CF13: 
    case ProSLIC_RING_TRAP_CF14: 
    case ProSLIC_RING_TRAP_CF15:  
    case ProSLIC_RING_TRAP_CF16:  
        i=0;
        do
        {
            if(trapRingFreqTable[i].freq >= ringCfg->freq) 
            {
                break;
            }
            i++;
        } while (trapRingFreqTable[i].freq);

        /* Set to maximum value if exceeding maximum value from table */
        if(trapRingFreqTable[i].freq == 0)
        {
            i--;
            errVal = RC_RING_V_LIMITED;
        }

        /* Update RINGFR RINGAMP, RINGOFFSET, and RINGCON */
        Si3217x_Ring_Presets[preset].amp = ringCfg->amp * vScale;
        Si3217x_Ring_Presets[preset].freq = Si3217x_Ring_Presets[preset].amp/trapRingFreqTable[i].cfVal[ringCfg->ringtype];
        Si3217x_Ring_Presets[preset].offset = ringCfg->offset * vScale;
        Si3217x_Ring_Presets[preset].phas = 262144000L/trapRingFreqTable[i].freq;

        /* Don't alter anything in RINGCON other than setting the TRAP bit */
        Si3217x_Ring_Presets[preset].ringcon |= 0x01; 

        /* RTPER and debouce timers  */
        Si3217x_Ring_Presets[preset].rtper = trapRingtripTable[i].rtper;
        Si3217x_Ring_Presets[preset].rtacdb = trapRingtripTable[i].rtdb;
        Si3217x_Ring_Presets[preset].rtdcdb = trapRingtripTable[i].rtdb;  


        Si3217x_Ring_Presets[preset].rtdcth = 0xFFFFFFFL;
        Si3217x_Ring_Presets[preset].rtacth = trapRingtripTable[i].rtacth[ringCfg->ringtype];


        break;
    }

    /* 
    ** DCDC tracking sluggish under light load at higher ring freq.
    ** Reduce tracking depth above 40Hz.  This should have no effect
    ** if using the Buck-Boost architecture.
    */
    if((sineRingFreqTable[i].freq >= 40)||(Si3217x_General_Configuration.bom_option == BO_DCDC_BUCK_BOOST))
    {
        dcdcVminTmp = ringCfg->amp + ringCfg->offset;
        dcdcVminTmp *= 1000;
        dcdcVminTmp *= SCALE_V_MADC;
        Si3217x_Ring_Presets[preset].dcdc_vref_min_rng = dcdcVminTmp;
    }
    else
    {
        Si3217x_Ring_Presets[preset].dcdc_vref_min_rng = 0x1800000L;
    }

    return errVal;
}


typedef struct
{
    int32   gain;
    uInt32 scale;
} ProSLIC_GainScaleLookup;


#define GAIN_MAX 6
#define GAIN_MIN -30
#ifndef ENABLE_HIRES_GAIN
static int Si3217x_dbgSetGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int tx_rx_sel){
    int errVal = 0;
    int32 i;
    int32 gain_pga, gain_eq;
    const ProSLIC_GainScaleLookup gainScaleTable[] =  /*  gain, scale=10^(gain/20) */
    {   
		{-30, 32},
		{-29, 35},
		{-28, 40},
		{-27, 45},
		{-26, 50},
		{-25, 56},
		{-24, 63},
		{-23, 71},
		{-22, 79},
		{-21, 89},
		{-20, 100},
		{-19, 112},
		{-18, 126},
		{-17, 141},
        {-16, 158}, 
        {-15, 178}, 
        {-14, 200}, 
        {-13, 224}, 
        {-12, 251}, 
        {-11, 282}, 
        {-10, 316}, 
        {-9, 355}, 
        {-8, 398}, 
        {-7, 447}, 
        {-6, 501},
        {-5, 562},
        {-4, 631},
        {-3, 708},
        {-2, 794},
        {-1, 891},
        {0, 1000},
        {1, 1122},
        {2, 1259},
        {3, 1413},
        {4, 1585},
        {5, 1778},
        {6, 1995},
        {0xff,0}  /* terminator */
    }; 
 
/* 
** 5.4.0 - Removed relative gain scaling. to support automatic adjustment based on
**         gain plan provided in txgain_db and rxgain_db.  It is presumed that all
**         coefficients were generated for 0dB/0dB gain and the txgain_db and rxgain_db
**         parameters will be used to scale the gain using the existing gain provisioning
**         infrastructure when the zsynth preset is loaded.  This function will ignore 
**         the txgain_db and rxgain_db parameters and scale absolute gain presuming a
**         0dB/0dB coefficient set.
*/

/*
** 6.0.0 - Modifying where gain/attenuation is placed to minimize clipping.
**
**         RX Path:   -30dB < gain <  0dB -   All in RXACGAIN 
**                      0dB < gain <  6dB -   All in RXACEQ
**
**         TX Path:   -30dB < gain <  0dB -   All in TXACEQ
**                      0dB < gain <  6dB -   All in TXACGAIN
*/

    /* Test against max gain */
    if (gain > GAIN_MAX)
	{
		errVal = RC_GAIN_OUT_OF_RANGE;
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("%sdbgSetGain: Gain %d out of range\n", LOGPRINT_PREFIX, (int)gain);
		}
#endif
		gain = GAIN_MAX; /* Clamp to maximum */
	}

	/* Test against min gain */
    if (gain < GAIN_MIN)
	{
		errVal = RC_GAIN_OUT_OF_RANGE;
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("%sdbgSetGain: Gain %d out of range\n", LOGPRINT_PREFIX, (int)gain);
		}
#endif
		gain = GAIN_MIN; /* Clamp to minimum */
	}

	/* Distribute gain */
	if(gain == 0)
	{
		gain_pga = 0;
		gain_eq = 0;
	}
	else if(gain > 0)
	{
		if(tx_rx_sel == TXACGAIN_SEL)
		{
			gain_pga = gain;
			gain_eq  = 0;
		}
		else
		{
			gain_pga = 0;
			gain_eq = gain;
		}
	}
	else
	{
		if(tx_rx_sel == TXACGAIN_SEL)
		{
			gain_pga = 0;
			gain_eq  = gain;
		}
		else
		{
			gain_pga = gain;
			gain_eq = 0;
		}

	}
   

    /* 
	** Lookup PGA Appopriate PGA Gain
	*/
    i=0;
    do
    {
        if(gainScaleTable[i].gain >= gain_pga)   /* was gain_1 */
        {             
            break;
        }
        i++;
    } while (gainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(gainScaleTable[i].gain == 0xff)
    {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

    if(tx_rx_sel == TXACGAIN_SEL)
    {
        Si3217x_audioGain_Presets[0].acgain = (Si3217x_Impedance_Presets[impedance_preset].txgain/1000)*gainScaleTable[i].scale;
    }
    else
    {
        Si3217x_audioGain_Presets[1].acgain = (Si3217x_Impedance_Presets[impedance_preset].rxgain/1000)*gainScaleTable[i].scale;
    }


    /* 
	** Lookup EQ Gain
	*/
    i=0;
    do
    {
        if(gainScaleTable[i].gain >= gain_eq) /* was gain_2 */
        {
            break;
        }
        i++;
    } while (gainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(gainScaleTable[i].gain == 0xff)
    {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

    if(tx_rx_sel == TXACGAIN_SEL)
    {
        /*sign extend negative numbers*/
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 |= 0xf0000000L;

        Si3217x_audioGain_Presets[0].aceq_c0 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0/1000)*gainScaleTable[i].scale;
        Si3217x_audioGain_Presets[0].aceq_c1 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1/1000)*gainScaleTable[i].scale;
        Si3217x_audioGain_Presets[0].aceq_c2 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2/1000)*gainScaleTable[i].scale;
        Si3217x_audioGain_Presets[0].aceq_c3 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3/1000)*gainScaleTable[i].scale;
    }
    else
    {
        /*sign extend negative numbers*/
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 |= 0xf0000000L;

        Si3217x_audioGain_Presets[1].aceq_c0 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0/1000)*gainScaleTable[i].scale;
        Si3217x_audioGain_Presets[1].aceq_c1 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1/1000)*gainScaleTable[i].scale;
        Si3217x_audioGain_Presets[1].aceq_c2 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2/1000)*gainScaleTable[i].scale;
        Si3217x_audioGain_Presets[1].aceq_c3 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3/1000)*gainScaleTable[i].scale;
    }


    return errVal;
}
#else /* ENABLE_HIRES_GAIN */
/*
** Function: Si3217x_dbgSetGainHiRes()
**
** Description: 
** Provisionary function for setting up
** TX and RX gain with 0.1dB resolution instead
** of 1.0dB resolution
*/
static int Si3217x_dbgSetGainHiRes (proslicChanType *pProslic, int32 gain, int impedance_preset, int tx_rx_sel){
    int errVal = 0;
    int32 i;
	int32 coarseGainIndex, fineGainIndex;
    int32 gain_pga, gain_eq;
	int32 coarseGain, fineGain;
	int32 tmp;
    const ProSLIC_GainScaleLookup coarseGainScaleTable[] =  /*  gain, scale=10^(gain/20) */
    { 
		{-30, 32},
		{-29, 35},
		{-28, 40},
		{-27, 45},
		{-26, 50},
		{-25, 56},
		{-24, 63},
		{-23, 71},
		{-22, 79},
		{-21, 89},
		{-20, 100},
		{-19, 112},
		{-18, 126},
		{-17, 141},
        {-16, 158}, 
        {-15, 178}, 
        {-14, 200}, 
        {-13, 224}, 
        {-12, 251}, 
        {-11, 282}, 
        {-10, 316}, 
        {-9, 355}, 
        {-8, 398}, 
        {-7, 447}, 
        {-6, 501},
        {-5, 562},
        {-4, 631},
        {-3, 708},
        {-2, 794},
        {-1, 891},
        {0, 1000},
        {1, 1122},
        {2, 1259},
        {3, 1413},
        {4, 1585},
        {5, 1778},
        {6, 1995},
        {0xff,0}  /* terminator */
    }; 

	const ProSLIC_GainScaleLookup fineGainScaleTable[] =  /*  gain, scale=10^(gain/20) */
    { 
        {-9, 902}, 
        {-8, 912}, 
        {-7, 923}, 
        {-6, 933},
        {-5, 944},
        {-4, 955},
        {-3, 966},
        {-2, 977},
        {-1, 989},
        {0, 1000},
        {1, 1012},
        {2, 1023},
        {3, 1035},
        {4, 1047},
        {5, 1059},
        {6, 1072},
        {7, 1084},
        {8, 1096},
        {9, 1109},
		{0xff,0}  /* terminator */
	};

/*
** 6.0.0 - Modifying where gain/attenuation is placed to minimize clipping.
**
**         RX Path:   -30dB < gain <  0dB -   All in RXACGAIN
**                      0dB < gain <  6dB -   All in RXACEQ
**
**         TX Path:   -30dB < gain <  0dB -   All in TXACEQ
**                      0dB < gain <  6dB -   All in TXACGAIN
**
** 6.2.1 - Added option for fine gain adjust.  All fine adjustment done
**         in RXACGAIN and TXACEQ
*/

    /* Test against max gain */
    if (gain > (GAIN_MAX*10L))
	{
		errVal = RC_GAIN_OUT_OF_RANGE;
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("%sdbgSetGain: Gain %d dB*10 out of range\n", LOGPRINT_PREFIX, gain);
		}
#endif
		gain = (GAIN_MAX*10L); /* Clamp to maximum */
	}

	/* Test against min gain */
    if (gain < (GAIN_MIN*10L))
	{
		errVal = RC_GAIN_OUT_OF_RANGE;
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("%sdbgSetGain: Gain %d dB*10 out of range\n", LOGPRINT_PREFIX, gain);
		}
#endif
		gain = (GAIN_MIN*10); /* Clamp to minimum */
	}

    /* Distribute gain */
	coarseGain = gain/10L;
	fineGain = gain - (coarseGain*10L);

	/* Distribute coarseGain */
	if(coarseGain == 0)
	{
		gain_pga = 0;
		gain_eq = 0;
	}
	else if(coarseGain > 0)
	{
		if(tx_rx_sel == TXACGAIN_SEL)
		{
			gain_pga = coarseGain;
			gain_eq  = 0;
		}
		else
		{
			gain_pga = 0;
			gain_eq = coarseGain;
		}
	}
	else
	{
		if(tx_rx_sel == TXACGAIN_SEL)
		{
			gain_pga = 0;
			gain_eq  = coarseGain;
		}
		else
		{
			gain_pga = coarseGain;
			gain_eq = 0;
		}
	}

    /* 
	** Lookup PGA Appopriate PGA Gain
	*/
    i=0;
    do
    {
        if(coarseGainScaleTable[i].gain >= gain_pga) 
        {       
            break;
        }
        i++;
    } while (coarseGainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(coarseGainScaleTable[i].gain == 0xff)
    {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

	coarseGainIndex = i;  /* Store coarse index */

	/* Find fineGain */
	i = 0;
	do
	{
		if(fineGainScaleTable[i].gain >= fineGain)
		{
			break;
		}
		i++;
	} while (fineGainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(fineGainScaleTable[i].gain == 0xff)
    {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

	fineGainIndex = i;

    if(tx_rx_sel == TXACGAIN_SEL)
    {
        Si3217x_audioGain_Presets[0].acgain = ((Si3217x_Impedance_Presets[impedance_preset].txgain/1000L)*coarseGainScaleTable[coarseGainIndex].scale);
    }
    else
    {
        Si3217x_audioGain_Presets[1].acgain = ((Si3217x_Impedance_Presets[impedance_preset].rxgain/1000L)*coarseGainScaleTable[coarseGainIndex].scale)/1000L * fineGainScaleTable[fineGainIndex].scale;
    }

    /* 
	** Lookup EQ Gain
	*/
    i=0;
    do
    {
        if(coarseGainScaleTable[i].gain >= gain_eq) 
        {
            break;
        }
        i++;
    } while (coarseGainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(coarseGainScaleTable[i].gain == 0xff)
    {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

	coarseGainIndex = i;  /* Store coarse index */

    if(tx_rx_sel == TXACGAIN_SEL)
    {
        /*sign extend negative numbers*/
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 |= 0xf0000000L;

		tmp = (((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0/1000L)*coarseGainScaleTable[coarseGainIndex].scale);
		tmp = tmp / (int32)1000L;
		tmp = tmp * (int32)fineGainScaleTable[fineGainIndex].scale;
		Si3217x_audioGain_Presets[0].aceq_c0 = tmp;

		tmp = (((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1/1000L)*coarseGainScaleTable[coarseGainIndex].scale);
		tmp = tmp / (int32)1000L;
		tmp = tmp * (int32)fineGainScaleTable[fineGainIndex].scale;
		Si3217x_audioGain_Presets[0].aceq_c1 = tmp;

		tmp = (((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2/1000L)*coarseGainScaleTable[coarseGainIndex].scale);
		tmp = tmp / (int32)1000L;
		tmp = tmp * (int32)fineGainScaleTable[fineGainIndex].scale;
		Si3217x_audioGain_Presets[0].aceq_c2 = tmp;

		tmp = (((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3/1000L)*coarseGainScaleTable[coarseGainIndex].scale);
		tmp = tmp / (int32)1000L;
		tmp = tmp * (int32)fineGainScaleTable[fineGainIndex].scale;
		Si3217x_audioGain_Presets[0].aceq_c3 = tmp;
	}
    else
    {
        /*sign extend negative numbers*/
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 |= 0xf0000000L;
        if (Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 & 0x10000000L)
            Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 |= 0xf0000000L;

        Si3217x_audioGain_Presets[1].aceq_c0 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0/1000)*coarseGainScaleTable[i].scale;
        Si3217x_audioGain_Presets[1].aceq_c1 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1/1000)*coarseGainScaleTable[i].scale;
        Si3217x_audioGain_Presets[1].aceq_c2 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2/1000)*coarseGainScaleTable[i].scale;
        Si3217x_audioGain_Presets[1].aceq_c3 = ((int32)Si3217x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3/1000)*coarseGainScaleTable[i].scale;
    }


    return errVal;
}
#endif /* ENABLE_HIRES_GAIN */

/*
** Function: PROSLIC_dbgSetTXGain
**
** Description: 
** Provisionary function for setting up
** TX gain
*/
int Si3217x_dbgSetTXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset){
    SILABS_UNREFERENCED_PARAMETER(audio_gain_preset);
#ifdef ENABLE_HIRES_GAIN
	return Si3217x_dbgSetGainHiRes(pProslic,gain,impedance_preset,TXACGAIN_SEL);
#else
	return Si3217x_dbgSetGain(pProslic,gain,impedance_preset,TXACGAIN_SEL);
#endif
}

/*
** Function: PROSLIC_dbgSetRXGain
**
** Description: 
** Provisionary function for setting up
** RX gain
*/
int Si3217x_dbgSetRXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset){
    SILABS_UNREFERENCED_PARAMETER(audio_gain_preset);
#ifdef ENABLE_HIRES_GAIN
	return Si3217x_dbgSetGainHiRes(pProslic,gain,impedance_preset,RXACGAIN_SEL);
#else
	return Si3217x_dbgSetGain(pProslic,gain,impedance_preset,RXACGAIN_SEL);
#endif
}

/*
** Function: delay_poll
**
** Description: 
** Delay function called within PSTN detection functions
**
** Return Value:
** none
*/
#ifdef PSTN_DET_ENABLE
static void Si3217x_polled_delay(proslicTestStateType *pState, uInt16 delay)
{
uInt16 delayCount;

    if((delay/PSTN_DET_POLL_RATE) < 2)
        delayCount = 0;
    else
        delayCount = (delay/PSTN_DET_POLL_RATE) - 2;

    pState->waitIterations++;
    if((pState->waitIterations == delayCount) || (delayCount == 0))
    {
        pState->waitIterations = 0;
        pState->stage++;
    }
}
#endif

/*
** Function: Si3217x_GetRAMScale
**
** Description: 
** Read scale factor for passed RAM location
**
** Return Value:
** int32 scale
*/
static int32 Si3217x_GetRAMScale(uInt16 addr)
{
int32 scale;

    switch(addr)
    {
        case SI3217X_COM_RAM_MADC_ILOOP:
        case SI3217X_COM_RAM_MADC_ITIP:
        case SI3217X_COM_RAM_MADC_IRING:
        case SI3217X_COM_RAM_MADC_ILONG:
            scale = SCALE_I_MADC;
        break;

        case SI3217X_COM_RAM_MADC_VTIPC:
        case SI3217X_COM_RAM_MADC_VRINGC:
        case SI3217X_COM_RAM_MADC_VBAT:
		case SI3217X_COM_RAM_MADC_VDC:
		case SI3217X_COM_RAM_MADC_VDC_OS:
        case SI3217X_COM_RAM_MADC_VLONG:
        case SI3217X_COM_RAM_VDIFF_SENSE:
        case SI3217X_COM_RAM_VDIFF_FILT:
        case SI3217X_COM_RAM_VDIFF_COARSE:
        case SI3217X_COM_RAM_VTIP:
        case SI3217X_COM_RAM_VRING:
            scale = SCALE_V_MADC;
        break;

        default:
            scale = 1;
        break;
    }

    return scale;
}

/*
** Function: Si3217x_ReadMADCScaled
**
** Description: 
** Read MADC (or other sensed voltages/currents) and
** return scaled value in int32 format.
**
** Return Value:
** int32 voltage in mV or
** int32 current in uA
*/
int32 Si3217x_ReadMADCScaled(proslicChanType_ptr pProslic,uInt16 addr, int32 scale)
{
    int32 data;

    /* 
    ** Read 29-bit RAM and sign extend to 32-bits
    */
    data = ReadRAM(pProHW,pProslic->channel,addr);
    if(data & 0x10000000L)
        data |= 0xF0000000L;

    /*
    ** Scale to provided value, or use defaults if scale = 0
    */
    if(scale == 0)
        scale = Si3217x_GetRAMScale(addr);

    data /= scale;

    return data;
}

/*
** Function: Si3217x_LineMonitor
**
** Description: 
** Monitor line voltages and currents
*/
int Si3217x_LineMonitor(proslicChanType *pProslic, proslicMonitorType *monitor)
{

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    if(pProslic->channelEnable)
    {
        monitor->vtr    = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VDIFF_FILT);
        if(monitor->vtr & 0x10000000L)
            monitor->vtr |= 0xf0000000L;
        monitor->vtr /= SCALE_V_MADC;

        monitor->vtip    = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VTIP);
        if(monitor->vtip & 0x10000000L)
            monitor->vtip |= 0xf0000000L;
        monitor->vtip /= SCALE_V_MADC;

        monitor->vring    = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VRING);
        if(monitor->vring & 0x10000000L)
            monitor->vring |= 0xf0000000L;
        monitor->vring /= SCALE_V_MADC;

		monitor->vlong    = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_MADC_VLONG);
        if(monitor->vlong & 0x10000000L)
            monitor->vlong |= 0xf0000000L;
        monitor->vlong /= SCALE_V_MADC;

        monitor->vbat    = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_MADC_VBAT);
        if(monitor->vbat & 0x10000000L)
            monitor->vbat |= 0xf0000000L;
        monitor->vbat /= SCALE_V_MADC;

		monitor->vdc    = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_MADC_VDC);
        if(monitor->vdc & 0x10000000L)
            monitor->vdc |= 0xf0000000L;
		monitor->vdc *= -1L;  /* Invert since sensor inverted */
        monitor->vdc /= SCALE_V_MADC;

        monitor->itr  = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_MADC_ILOOP);
        if(monitor->itr & 0x10000000L)
            monitor->itr |= 0xf0000000L;
        monitor->itr /= SCALE_I_MADC;

        monitor->itip  = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_MADC_ITIP);
        if(monitor->itip & 0x10000000L)
            monitor->itip |= 0xf0000000L;
        monitor->itip /= SCALE_I_MADC;

        monitor->iring  = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_MADC_IRING);
        if(monitor->iring & 0x10000000L)
            monitor->iring |= 0xf0000000L;
        monitor->iring /= SCALE_I_MADC;

        monitor->ilong  = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_MADC_ILONG);
        if(monitor->ilong & 0x10000000L)
            monitor->ilong |= 0xf0000000L;
        monitor->ilong /= SCALE_I_MADC;

        monitor->p_hvic  = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_P_Q1_D); /* P_HVIC_LPF */
        if(monitor->p_hvic & 0x10000000L)
            monitor->p_hvic |= 0xf0000000L;
        monitor->p_hvic /= SCALE_P_MADC;
    }

    return RC_NONE;
}

/*
** Function: Si3217x_PSTNCheck
**
** Description: 
** Continuous monitoring of longitudinal current.
** If an average of N samples exceed avgThresh or a
** single sample exceeds singleThresh, the linefeed 
** is forced into the open state.
**
** This protects the port from connecting to a live
** pstn line (faster than power alarm).
**
*/
int Si3217x_PSTNCheck (proslicChanType *pProslic,proslicPSTNCheckObjType *pPSTNCheck)
{
    uInt8 i;

    if(pProslic->channelType != PROSLIC) {
        return RC_NONE;    /* Ignore DAA channels */
    }

    /* Adjust buffer index */
    if(pPSTNCheck->count >= pPSTNCheck->samples)
    {
        pPSTNCheck->buffFull = TRUE;
        pPSTNCheck->count = 0;   /* reset buffer ptr */
    }

    /* Read next sample */
    pPSTNCheck->ilong[pPSTNCheck->count]  = ReadRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_MADC_ILONG);
    if(pPSTNCheck->ilong[pPSTNCheck->count] & 0x10000000L)
        pPSTNCheck->ilong[pPSTNCheck->count] |= 0xf0000000L;
    pPSTNCheck->ilong[pPSTNCheck->count] /= SCALE_I_MADC;

    /* Monitor magnitude only */
    if(pPSTNCheck->ilong[pPSTNCheck->count] < 0)
        pPSTNCheck->ilong[pPSTNCheck->count] = -pPSTNCheck->ilong[pPSTNCheck->count];

    /* Quickly test for single measurement violation */
    if(pPSTNCheck->ilong[pPSTNCheck->count] > pPSTNCheck->singleThresh)
        return RC_PSTN_CHECK_SINGLE_FAIL;  /* fail */

    /* Average once buffer is full */
    if(pPSTNCheck->buffFull == TRUE)  
    {
        pPSTNCheck->avgIlong = 0;
        for(i=0;i<pPSTNCheck->samples; i++)
        {
            pPSTNCheck->avgIlong += pPSTNCheck->ilong[i];
        }
        pPSTNCheck->avgIlong /= pPSTNCheck->samples;

        if(pPSTNCheck->avgIlong > pPSTNCheck->avgThresh)    
        {
            /* reinit obj and return fail */
            pPSTNCheck->count = 0;
            pPSTNCheck->buffFull = FALSE;
            return RC_PSTN_CHECK_AVG_FAIL;
        }
        else
        {
            pPSTNCheck->count++;
            return RC_NONE;
        }   
    }
    else
    {
        pPSTNCheck->count++;
        return RC_NONE;
    }
}




#ifdef PSTN_DET_ENABLE
/*
** Function: abs_int32
**
** Description: 
** abs implementation for int32 type
*/
static int32 abs_int32(int32 a)
{
    if(a < 0)
        return -1*a;
    return a;
}

/*
** Function: Si3217x_DiffPSTNCheck
**
** Description: 
** Monitor for excessive longitudinal current, which
** would be present if a live pstn line was connected
** to the port.
**
** Returns:
** RC_NONE             - test in progress
** RC_COMPLETE_NO_ERR  - test complete, no alarms or errors
** RC_POWER_ALARM_HVIC - test interrupted by HVIC power alarm
** RC_
** 
*/

int Si3217x_DiffPSTNCheck (proslicChanType *pProslic, proslicDiffPSTNCheckObjType *pPSTNCheck){
int32 ramData;
uInt8 loop_status;
int i;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;    /* Ignore DAA channels */
    }


    switch(pPSTNCheck->pState.stage) 
    {
        case 0: 
            /* Optional OPEN foreign voltage measurement - only execute if LCS = 0 */
            /* Disable low power mode */
            pPSTNCheck->enhanceRegSave = ReadReg(pProHW,pProslic->channel,ENHANCE);
            WriteReg(pProHW,pProslic->channel, ENHANCE, pPSTNCheck->enhanceRegSave&0x07); /* Disable powersave */
            pPSTNCheck->vdiff1_avg = 0;
            pPSTNCheck->vdiff2_avg = 0;
            pPSTNCheck->iloop1_avg = 0;
            pPSTNCheck->iloop2_avg = 0;
            pPSTNCheck->return_status = RC_COMPLETE_NO_ERR;
            /* Do OPEN state hazardous voltage measurement if enabled and ONHOOK */
            Si3217x_ReadHookStatus(pProslic,&loop_status);
            if((loop_status == ONHOOK)&&(pPSTNCheck->femf_enable == 1)) 
                pPSTNCheck->pState.stage++;
            else
                pPSTNCheck->pState.stage = 10;

            return RC_NONE;

        case 1:
            /* Change linefeed to OPEN state for HAZV measurement, setup coarse sensors */
            pPSTNCheck->lfstate_entry = ReadReg(pProHW,pProslic->channel, LINEFEED);
            ProSLIC_SetLinefeedStatus(pProslic,LF_OPEN);
            pPSTNCheck->pState.stage++;
            return RC_NONE;

        case 2:
            /* Settle */
            Si3217x_polled_delay(&(pPSTNCheck->pState), PSTN_DET_OPEN_FEMF_SETTLE);
            return RC_NONE;

        case 3: 
            /* Measure HAZV */
            pPSTNCheck->vdiff_open = Si3217x_ReadMADCScaled(pProslic,VDIFF_COARSE,0);
            /* Stop PSTN check if differential voltage > max_femf_vopen present */
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
            {
                LOGPRINT("%sDiff PSTN : Vopen = %d mV\n", LOGPRINT_PREFIX, pPSTNCheck->vdiff_open);
            }
#endif
            if(abs_int32(pPSTNCheck->vdiff_open) > pPSTNCheck->max_femf_vopen)
            {
                pPSTNCheck->pState.stage = 70;
                pPSTNCheck->return_status = RC_PSTN_OPEN_FEMF;
            }
            else
            {
                pPSTNCheck->pState.stage = 10;
            }
            return 0;

        case 10:  
            /* Load first DC feed preset */
            ProSLIC_DCFeedSetup(pProslic,pPSTNCheck->dcfPreset1);
            ProSLIC_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
            pPSTNCheck->pState.stage++;
            return RC_NONE; 

        case 11:
            /* Settle */
            Si3217x_polled_delay(&(pPSTNCheck->pState), PSTN_DET_DIFF_IV1_SETTLE);
            return RC_NONE;

        case 12:
            /* Measure VDIFF and ILOOP, switch to 2nd DCFEED setup */
            pPSTNCheck->vdiff1[pPSTNCheck->pState.sampleIterations] = Si3217x_ReadMADCScaled(pProslic,VDIFF_FILT,0);
            pPSTNCheck->iloop1[pPSTNCheck->pState.sampleIterations] = Si3217x_ReadMADCScaled(pProslic,MADC_ILOOP,0);
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
            {
                LOGPRINT("%sDiff PSTN: Vdiff1[%d] = %d mV\n", LOGPRINT_PREFIX,
                    pPSTNCheck->pState.sampleIterations,pPSTNCheck->vdiff1[pPSTNCheck->pState.sampleIterations]);
                LOGPRINT("%sDiff PSTN: Iloop1[%d] = %d uA\n", LOGPRINT_PREFIX, 
                    pPSTNCheck->pState.sampleIterations,pPSTNCheck->iloop1[pPSTNCheck->pState.sampleIterations]);
            }
#endif
            pPSTNCheck->pState.sampleIterations++;
            if(pPSTNCheck->pState.sampleIterations >= pPSTNCheck->samples)
            {
                ProSLIC_DCFeedSetup(pProslic,pPSTNCheck->dcfPreset2);
                pPSTNCheck->pState.stage++;
                pPSTNCheck->pState.sampleIterations = 0;
            }
            return RC_NONE;

        case 13:
            /* Settle feed 500ms */
            Si3217x_polled_delay(&(pPSTNCheck->pState), PSTN_DET_DIFF_IV2_SETTLE);
            return RC_NONE;

        case 14:
            /* Measure VDIFF and ILOOP*/
            pPSTNCheck->vdiff2[pPSTNCheck->pState.sampleIterations] = Si3217x_ReadMADCScaled(pProslic,VDIFF_FILT,0);
            pPSTNCheck->iloop2[pPSTNCheck->pState.sampleIterations] = Si3217x_ReadMADCScaled(pProslic,MADC_ILOOP,0);
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
            {
                LOGPRINT("%sDiff PSTN: Vdiff2[%d] = %d mV\n", LOGPRINT_PREFIX,
                    pPSTNCheck->pState.sampleIterations,pPSTNCheck->vdiff2[pPSTNCheck->pState.sampleIterations]);
                LOGPRINT("%sDiff PSTN: Iloop2[%d] = %d uA\n", LOGPRINT_PREFIX,
                    pPSTNCheck->pState.sampleIterations,pPSTNCheck->iloop2[pPSTNCheck->pState.sampleIterations]);
            }
#endif
            pPSTNCheck->pState.sampleIterations++;
            if(pPSTNCheck->pState.sampleIterations >= pPSTNCheck->samples)
            {
                /* Compute averages */
                for (i=0; i<pPSTNCheck->samples; i++)
                {
                    pPSTNCheck->vdiff1_avg += pPSTNCheck->vdiff1[i];
                    pPSTNCheck->iloop1_avg += pPSTNCheck->iloop1[i];
                    pPSTNCheck->vdiff2_avg += pPSTNCheck->vdiff2[i];
                    pPSTNCheck->iloop2_avg += pPSTNCheck->iloop2[i];
                }
                pPSTNCheck->vdiff1_avg /= pPSTNCheck->samples;
                pPSTNCheck->iloop1_avg /= pPSTNCheck->samples;
                pPSTNCheck->vdiff2_avg /= pPSTNCheck->samples;
                pPSTNCheck->iloop2_avg /= pPSTNCheck->samples;               
                
                /* Force small (probably offset) currents to minimum value */
                if(abs_int32(pPSTNCheck->iloop1_avg) < PSTN_DET_MIN_ILOOP) pPSTNCheck->iloop1_avg = PSTN_DET_MIN_ILOOP;
                if(abs_int32(pPSTNCheck->iloop2_avg) < PSTN_DET_MIN_ILOOP) pPSTNCheck->iloop2_avg = PSTN_DET_MIN_ILOOP;

                /* Calculate measured loop impedance */          
                pPSTNCheck->rl1 = abs_int32((pPSTNCheck->vdiff1_avg*1000L)/pPSTNCheck->iloop1_avg);
                pPSTNCheck->rl2 = abs_int32((pPSTNCheck->vdiff2_avg*1000L)/pPSTNCheck->iloop2_avg);
                
                /* Force non-zero loop resistance */
                if(pPSTNCheck->rl1 == 0) pPSTNCheck->rl1 = 1;
                if(pPSTNCheck->rl2 == 0) pPSTNCheck->rl2 = 1;

                /* Qualify loop impedances */
                pPSTNCheck->rl_ratio = (pPSTNCheck->rl1*1000L)/pPSTNCheck->rl2;
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
            {
                const char fmt_string[] = "%sDiffPSTN: %s = %d %s\n";
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "VDIFF1", pPSTNCheck->vdiff1_avg, "mV");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "ILOOP1",pPSTNCheck->iloop1_avg, "uA");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "VDIFF2",pPSTNCheck->vdiff2_avg, "mV");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "ILOOP2",pPSTNCheck->iloop2_avg, "uA");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "RL1",pPSTNCheck->rl1, "ohm");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "RL2",pPSTNCheck->rl2, "ohm");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "RL_Ratio",pPSTNCheck->rl_ratio, " ");            
            }
#endif
           
                /* Restore */
                pPSTNCheck->pState.sampleIterations = 0; 
                pPSTNCheck->pState.stage = 70;
            }
            return RC_NONE;
    
        case 70:  /* Reset test state, restore entry conditions */
            ProSLIC_DCFeedSetup(pProslic,pPSTNCheck->entryDCFeedPreset);
            ProSLIC_SetLinefeedStatus(pProslic,pPSTNCheck->lfstate_entry);
            WriteReg(pProHW,pProslic->channel,ENHANCE, pPSTNCheck->enhanceRegSave);
            pPSTNCheck->pState.stage = 0;
            pPSTNCheck->pState.waitIterations = 0;
            pPSTNCheck->pState.sampleIterations = 0;
            return pPSTNCheck->return_status;
            
    }
return RC_NONE;
}

#endif


/*
** Function: Si3217x_SetPwrsaveMode
**
** Description: 
** Enable or disable powersave mode
**
** Returns:
** RC_NONE
*/
int Si3217x_SetPowersaveMode (proslicChanType *pProslic, int pwrsave)
{
uInt8 regData;

    if(pProslic->channelType != PROSLIC) {
        return RC_NONE;    /* Ignore DAA channels */
    }

    regData = ReadReg(pProHW,pProslic->channel, SI3217X_COM_REG_ENHANCE);

    if(pwrsave == PWRSAVE_DISABLE)  
    {
        regData &= 0x27; 
    }
    else
    {
        regData |= 0x10; 
    }

    WriteReg(pProHW,pProslic->channel, SI3217X_COM_REG_ENHANCE, regData);

    return RC_NONE;
}


/*
** Function: Si3217x_SetDAAEnable
**
** Description: 
** Enable or disable adjacent FXO channel (Si32178 Only)
**
** Returns:
** RC_NONE
*/
int Si3217x_SetDAAEnable (proslicChanType *pProslic, int enable)
{
	uInt8 regData;

    if(pProslic->channelType != PROSLIC){ /* Ignore DAA channels */
        return RC_CHANNEL_TYPE_ERR;
    }

    if((pProslic->deviceId->chipType != SI32178)||(pProslic->deviceId->chipRev != SI3217X_REVB))  
    {
        return RC_UNSUPPORTED_FEATURE;
    }
    
    regData = ReadReg(pProHW,pProslic->channel, 74);
    if(enable)
    {
        WriteReg(pProHW,pProslic->channel,74,regData|0x01);
    }
    else
    {
        WriteReg(pProHW,pProslic->channel,74,regData&0x7E);
    }

	return(RC_NONE);

}

/*
** Function: Si3217x_ReadReg
**
** Description: 
** Allows direct SPI access at ProSLIC layer
** Channel embeded in channel obj, so it is not passed into this func
**
** Returns:
** uInt8 - register contents
*/
uInt8 Si3217x_ReadReg (proslicChanType *pProslic, uInt8 addr)
{
	return (ReadReg(pProHW,pProslic->channel, addr));
}

/*
** Function: Si3217x_WriteReg
**
** Description: 
** Allows direct SPI access at ProSLIC layer
** Channel embeded in channel obj, so it is not passed into this func
**
** Returns:
** RC_NONE
*/
int Si3217x_WriteReg (proslicChanType *pProslic, uInt8 addr, uInt8 data)
{
	WriteReg(pProHW,pProslic->channel,addr,data);
	return RC_NONE;
}


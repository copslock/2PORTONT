/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Source File for Virtual MAC
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: virtualMac.c,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
*/

#include "rtl_types.h"
#include "rtl_glue.h"
#include "rtl8672_asicregs.h"
#include "virtualMac.h"
#include "icModel.h"
#include "hsModel.h"
#include "modelTrace.h"
#include "icExport.h"
#include "assert.h"
#include "icTest_ringController.h"
#include "rtl8672_tblAsicDrv.h"
#if defined(VSV)||defined(MIILIKE)
#include "vsv_conn.h"
#endif
#ifdef VSV
#include <unistd.h> /* for sleep() */
#endif


/* The current emulated IC type.
 *	IC_TYPE_MODEL:
 *		MacInput(), MacOutput(), HSA, HSB, READ_MEM32(), and WRITE_MEM32() functions are mapped to 
 *		model code, not the phisical IC/address.
 *
 *	IC_TYPE_REAL:
 *		MacInput(), MacOutput(), HSA, HSB, READ_MEM32(), and WRITE_MEM32() functions are mapped to 
 *		real IC/address.
 */
enum IC_TYPE curIcType;


#if defined(RTL865X_MODEL_KERNEL)
static int32 cpu_status_is_set;
static uint32 cpu_status;
#endif


static int8 virtualHsb[HSB_SIZE];
static int8 virtualHsa[HSA_SIZE];
static int8 virtualPPReg[PP_SIZE];
static int8 virtualIfTable[IFTABLE_SIZE];
static int8 virtualAle[ALE_SIZE];
static int8 virtualL2[L2TABLE_SIZE];
static int8 virtualL4[L4TABLE_SIZE];
static int8 virtualDbg1[DBG1_SIZE];
static int8 virtualDbg2[DBG2_SIZE];
static int8 virtualHsbp[HSBP_SIZE];
static int8 virtualXlatorDbg[XLATOR_DBG_SIZE];

int8	*pVirtualHsb=virtualHsb;
int8	*pVirtualHsa=virtualHsa;
int8	*pVirtualPPReg=virtualPPReg;
int8	*pVirtualIfTable=virtualIfTable;
int8	*pVirtualAle=virtualAle;
int8	*pVirtualL2=virtualL2;
int8	*pVirtualL4=virtualL4;
int8 *pVirtualDbg1 = virtualDbg1;
int8 *pVirtualDbg2 = virtualDbg2;
int8 *pVirtualHsbp = virtualHsbp;
int8 *pVirtualXlatorDbg = virtualXlatorDbg;

/*********************************************************************
 *  TARGET MASK
 *
 *  The mask is used for setTestTarget() function.
 *  We can only test REAL even the test bench wants to test both REAL/MODEL.
 *
 *    0 - mode disabled, do not test
 *    1 - mode enabled, may test
 *
 *  In default, we enable ALL target.
 *********************************************************************/
uint32 testTargetMask = 0xffffffff;

int32 model_setTestTargetMask( uint32 mask )
{
	testTargetMask = mask; 
	return SUCCESS;
}


int32 model_getTestTargetMask( uint32* mask )
{
	if ( mask==NULL ) return FAILED;
	*mask = testTargetMask;
	return SUCCESS;
}


/*
 *  This function returns the support status of test target.
 *  Given a IC type, it tells you whether the IC type is supported in this platform.
 *  This function should be synced with model_setTestTarget().
 */
int32 model_tryTestTarget( enum IC_TYPE ic_type )
{
	int32 retval;
	
	/* Check mask, see if enabled in this mode. */
	if ( (testTargetMask&(1<<ic_type))==0 )
		return VMAC_ERROR_NON_AVAILABLE;
	
	switch ( ic_type )
	{
		case IC_TYPE_MODEL:
			retval = SUCCESS;
			break;
			
#if defined(RTL865X_MODEL_USER)

	#if defined(VERA)||defined(VSV)||defined(MIILIKE)
		case IC_TYPE_REAL:
			retval = SUCCESS;
			break;
	#else
		case IC_TYPE_REAL:
			retval = VMAC_ERROR_NON_AVAILABLE;
			break;
	#endif
			
#elif defined(RTL865X_MODEL_KERNEL)
		case IC_TYPE_REAL:
			retval = SUCCESS;
			break;
#endif

		default:
			retval = VMAC_ERROR_NON_AVAILABLE;
			break;
	}
 	
	return retval;
}


int32 _rtl8672_mapToVirtualRegSpace( void )
{
	/* Slower: ICE needs to initialize static virtual space . */



	
	pVirtualHsb = virtualHsb;
	pVirtualHsa = virtualHsa;
	pVirtualPPReg = virtualPPReg;
	pVirtualIfTable = virtualIfTable;
	pVirtualAle = virtualAle;
	pVirtualL2 = virtualL2;
	pVirtualL4 = virtualL4;
	pVirtualDbg1 = virtualDbg1;
	pVirtualDbg2 = virtualDbg2;	
	pVirtualHsbp = virtualHsbp;
	pVirtualXlatorDbg = virtualXlatorDbg;
	return SUCCESS;
}

int32 _rtl8672_mapToRealRegSpace( void )
{
	pVirtualHsb = (int8 *)REAL_HSB_BASE;
	pVirtualHsa = (int8 *)REAL_HSA_BASE;
	pVirtualPPReg = (int8 *)REAL_PP_BASE;
	pVirtualIfTable = (int8 *)REAL_IFTABLE_BASE;
	pVirtualAle = (int8 *)REAL_ALE_BASE;
	pVirtualL2 = (int8 *)REAL_L2TABLE_BASE;
	pVirtualL4 = (int8 *)REAL_L4TABLE_BASE;
	pVirtualDbg1 = (int8 *)REAL_DBG1_BASE;
	pVirtualDbg2 = (int8 *)REAL_DBG2_BASE;
	pVirtualHsbp = (int8 *)REAL_HSBP_BASE;
	pVirtualXlatorDbg = (int8 *)REAL_XLATOR_DBG_BASE;
	return SUCCESS;
}

/*
 *  This function sets the Test Target.
 */
int32 model_setTestTarget( enum IC_TYPE ic_type )
{
	/* Check mask, see if enabled in this mode. */
	if ( (testTargetMask&(1<<ic_type))==0 )
		return VMAC_ERROR_NON_AVAILABLE;
	
	switch ( ic_type )
	{
		case IC_TYPE_MODEL:
			curIcType = ic_type;
			rtlglue_printf("%s --- MODEL\n",__FUNCTION__);
			_rtl8672_mapToVirtualRegSpace();
			return SUCCESS;
		
		case IC_TYPE_REAL:
			curIcType = ic_type;
			rtlglue_printf("%s --- REAL\n",__FUNCTION__);	
			_rtl8672_mapToRealRegSpace();
			{		
				rtl8672_tblAsicDrv_intfParam_t intf;
				init_ring();
				rtl8672_getAsicNetInterface(SAR_INTFS+MAC_TX_INTFS+1, &intf);
				intf.SrcPortFilter=0;
				rtl8672_setAsicNetInterface(SAR_INTFS+MAC_TX_INTFS+1, &intf);				
			}
			_rtl8672_mapToRealRegSpace();
			return SUCCESS;
			
		default:
			return VMAC_ERROR_NON_AVAILABLE;
	}
	
	return SUCCESS;
}


/*
 *  This function returns the Test Target.
 */
int32 model_getTestTarget( enum IC_TYPE *pic_type )
{
	if ( pic_type ) *pic_type = curIcType;

	return SUCCESS;
}


/*===========================================================================
* The Implementation of Virtual Mac Interface 
* 
* This interface provides uniform interface to input/output packets.
* The user of this interface is test bench and 865xB emulation board.
* And this interface will abstract the difference between IC model code, 
*   real IC (865xC), and FPGA.
* 
*==========================================================================*/

int32 miiLikeTx( enum PORT_MASK *toPort, uint8* packet, int32 *len );
int32 miiLikeRx( enum PORT_NUM fromPort, uint8* packet, int32 len );


/*---------------------------------------------------------------------------
 *  virtualMacWaitCycle() is used to wait some cycle for VERA simulation.
 *  INPUT:
 *    cycle | SWCORE-cycle (4ns per cycle)
 *-------------------------------------------------------------------------*/
int32 virtualMacWaitCycle( int32 cycle )
{
	int32 retval;

#if defined(VSV)
	enum IC_TYPE curIcType;

	model_getTestTarget(&curIcType);

	switch ( curIcType )
	{
		case IC_TYPE_MODEL:
			usleep( (cycle*4+999)/1000 );
			retval = SUCCESS;
			break;
		case IC_TYPE_REAL:
			vsv_waitCycle( conn_client, cycle );
			retval = SUCCESS;
			break;
		default:
			retval = VMAC_ERROR_NON_AVAILABLE;
			break;
	}
#elif defined(MIILIKE)
	vsv_waitCycle( conn_client, cycle );
	retval = SUCCESS;
#elif defined(RTL865X_MODEL_KERNEL)
/* According to Ghhuang's suggestion, the for loop estimates about 5 cycles per loop.
 *
 *    802402c8:       24c60001        addiu   a2,a2,1
 *    802402cc:       00c3102b        sltu    v0,a2,v1
 *    802402d0:       1440fffd        bnez    v0,802402c8 <virtualMacWaitCycle+0x50>
 *    802402d4:       00000000        nop
 *
 * SwCore Clock = 250MHz (1 cycle = 4nS), 
 * CPU Clock    = 280MHz (1 cycle = 3.57nS)
 *
 * wait time = (cycle * 4nS) / 3.57nS
 *          := (cycle * 4) / (7 * 2)
 * 
 */
	{
		register uint32 i; 
		for(i=0;i<cycle*4/7*2/5;i++) 
		{
			do {} while(0);  /* about 5ns */
		}
	}
	retval = SUCCESS;
#else
	{
		uint32 i; 
		for(i=0;i<cycle*100;i++) 
		{ 
			do {} while(0);  
		} 
	}
	retval = SUCCESS;
#endif
	return retval;
}


/*---------------------------------------------------------------------------
 *  virtualMacInit() is used to initial corresponding register for different
 *  configurations (example: MIILIKE, and VSV).
 *-------------------------------------------------------------------------*/
int32 virtualMacInit( void )
{
	return SUCCESS;
}


/*---------------------------------------------------------------------------
 *  virtualMacInput() is used to input a complete packet, included L2 CRC,
 *  into FPGA/Model Code.
 *
 *
 *    +-----+--+---+----+-----+                            /-- FPGA/IC
 *    |ether|IP|TCP|data|L2CRC| === virtualMacInput() ==> +
 *    +-----+--+---+----+-----+                            \-- Model Code
 *
 *  SrcPort is needed.
 *-------------------------------------------------------------------------*/
int32 virtualMacInput( enum PORT_NUM fromPort, uint8* packet, int32 len )
{
	int32 retval;
	uint8 szComment[128];

	sprintf( szComment, "Input from port %d with %d bytes", fromPort, len );
//	MT_WATCH( szComment );

	/* The length must be larger than Ethernet minimum packet size. */
	//assert( len >= 64 );

#if defined(CONFIG_RTL865XC)
	retval = rtl865xC_virtualMacInput(fromPort, packet, len);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	retval = 0;
#else
#error please implement your platform here
#endif
	
	return retval;
}


/*
 *  Send the packet to the Mii-like port to simulate a received packet from physical port to SWCORE.
 *    len - included L2 CRC
 */
int32 miiLikeRx( enum PORT_NUM fromPort, uint8* packet, int32 len )
{
	/*************************************************************
	 *  MiiRx (CPU sends packet to ASIC)
	 *************************************************************/
	
#if defined(CONFIG_RTL865XC)
	return rtl865xC_miiLikeRx(fromPort, packet, len);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	return 0;
#else
#error please implement your platform here
#endif
}


/*---------------------------------------------------------------------------
 *  virtualMacOutput() will return a complete packet with a dstPort mask.
 *
 *
 *       FPGA/IC --\                            +-----+--+---+----+
 *                  +=== virtualMacInput() ===> |ether|IP|TCP|data|
 *    Model Code --/                            +-----+--+---+----+
 *
 *  INPUT:
 *    *toPort -- a pointer to the portlist that user candidates to check.
 *               DANGEROUS: If multiple bits are set, only one port will be served. The data of other ports will be DISCARD.
 *                          Therefore, to safely poll every port, please call virtualMacOutput() many times with only one port each time.
 *    *packet -- the buffer to store returned packet content.
 *    *len    -- pointer to store returned packet length.
 *
 *  OUTPUT:
 *    *toPort -- indicate the portlist that packet is exactly forwarded to.
 *    *len    -- packet length is returned in the storage (included L2 CRC)
 *    SUCCESS -- A packet is returned in *packet.
 *    FAILED  -- No packet is returned.
 *--------------------------------------------------------------------------*/
int32 virtualMacOutput( enum PORT_MASK *toPort, uint8* packet, int32 *len )
{
	int32 retval;
	char szComment[128];

	sprintf( szComment, "Wanna get with portmask 0x%03x and buffer %d bytes", *toPort, *len );
//	MT_WATCH( szComment );

	//assert( toPort!=NULL );
	//assert( *toPort );
	//assert( packet!=NULL );
	//assert( len!=NULL );
	//assert( *len>=64 );

#if defined(CONFIG_RTL865XC)
	retval = rtl865xC_virtualMacOutput(toPort, packet, len);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	retval = 0;
#else
#error please implement your platform here
#endif
	
	if ( retval==FAILED )
	{
		*len = 0;
		*toPort = 0;
	}
	
	sprintf( szComment, "[%d] Got a packet with portmask 0x%03x and length %d bytes", retval, *toPort, *len );
//	MT_WATCH( szComment );
	
	return retval;
}


/*
 *  Receive the packet from the Mii-like port to simulate a sending packet from swcore to physical port.
 */
int32 miiLikeTx( enum PORT_MASK *toPort, uint8* packet, int32 *len )
{
	/*************************************************************
	 *  MiiTx (ASIC sends packet to CPU)
	 *************************************************************/

#if defined(CONFIG_RTL865XC)
	return rtl865xC_miiLikeTx(toPort, packet, len);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	return 0;
#else
#error please implement your platform here
#endif
}


/*===========================================================================
 * Before every test, we shall pump all remaining packet in every output 
 *   queue of port.
 *===========================================================================*/
int32 virtualMacPumpPackets( enum PORT_MASK toPort )
{
	enum PORT_MASK askPort;
	static uint8 pktBuf[16*1024+128];
	int32 len;
	int32 retval;

	while( 1 )
	{
		askPort = toPort;
		len = sizeof(pktBuf);
		retval = virtualMacOutput( &askPort, pktBuf, &len );
		if ( retval==FAILED ) break; /* no packet available, break it ! */
	}

	return SUCCESS;
}


/*===========================================================================
 *  Implementation of HSB/HSA Interface
 *
 *  Since the bitmap of ASIC HSB/HSA structure is not friendly for software
 *    (consider that some fields are across two words), we need a wrapper
 *    to provide friendly data strcuture for software development.
 *  Thus, convert* functions are used to convert bit mapping between ASIC and
 *    software data structure.
 *  virtualMac* functions are the API for software. They will translate the
 *    bit mapping between hardware and software.
 *
 *  +------+  virtualMac* +-------+  convert*  +------+
 *  |  SW  | <----------->|convert| <--------> | ASIC |
 *  +------+              +-------+            +------+
 *
 *=========================================================================*/

/*---------------------------------------------------------------------------
 *  +-------+  *this  +-------+         +------+
 *  |  HSB  | ------> |convert| ------> |  SW  |
 *  +-------+         +-------+         +------+
 *-------------------------------------------------------------------------*/
int32 convertHsbToSoftware( hsb_t* rawHsb, hsb_param_t* hsb )
{
#if defined(CONFIG_RTL865XC)
	return rtl865xC_convertHsbToSoftware(rawHsb, hsb);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	return 0;
#else
#error please implement your platform here
#endif
}


/*---------------------------------------------------------------------------
 *  +-------+         +-------+  *this  +------+
 *  |  HSB  | ------> |convert| ------> |  SW  |
 *  +-------+         +-------+         +------+
 *-------------------------------------------------------------------------*/
int32 virtualMacGetHsb( hsb_param_t* hsb )
{
	hsb_t rawHsb;
	int32 ret = SUCCESS;
	
#if defined(CONFIG_RTL865XC)
	#if defined(VSV)&&WAIT_METHOD==1
	vsv_waitCycle( conn_client, RX_ALE_TX_CYCLE ); /* RX-ALE-TX process time. */
	#endif
	ret = rtl865xC_virtualMacGetHsb(&rawHsb);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	ret = 0;
#else
#error please implement your platform here
#endif
	
	convertHsbToSoftware( &rawHsb, hsb );
	return ret;
}


/*---------------------------------------------------------------------------
 *  +------+         +-------+  *this  +-------+
 *  |  SW  | ------> |convert| ------> |  HSB  |
 *  +------+         +-------+         +-------+
 *-------------------------------------------------------------------------*/
int32 convertHsbToAsic( hsb_param_t* hsb, hsb_t* rawHsb )
{
#if defined(CONFIG_RTL865XC)
	return rtl865xC_convertHsbToAsic(hsb, rawHsb);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	return 0;
#else
#error please implement your platform here
#endif
}


/*---------------------------------------------------------------------------
 *  +------+  *this  +-------+         +-------+
 *  |  SW  | ------> |convert| ------> |  HSB  |
 *  +------+         +-------+         +-------+
 *-------------------------------------------------------------------------*/
int32 virtualMacSetHsb( hsb_param_t* hsb )
{
	hsb_t rawHsb;
	int32 ret = SUCCESS;
	
	convertHsbToAsic( hsb, &rawHsb );
	
#if defined(CONFIG_RTL865XC)
	ret = rtl865xC_virtualMacSetHsb(rawHsb);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	ret = 0;
#else
#error please implement your platform here
#endif
	
	return ret;
}


/*---------------------------------------------------------------------------
 *  +-------+  *this  +-------+         +------+
 *  |  HSA  | ------> |convert| ------> |  SW  |
 *  +-------+         +-------+         +------+
 *-------------------------------------------------------------------------*/
int32 convertHsaToSoftware( hsa_t* rawHsa, hsa_param_t* hsa )
{
#if defined(CONFIG_RTL865XC)
	return rtl865xC_convertHsaToSoftware(rawHsa, hsa);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	return 0;
#else
#error please implement your platform here
#endif
}


/*---------------------------------------------------------------------------
 *  +-------+         +-------+  *this  +------+
 *  |  HSA  | ------> |convert| ------> |  SW  |
 *  +-------+         +-------+         +------+
 *-------------------------------------------------------------------------*/
int32 virtualMacGetHsa( hsa_param_t* hsa )
{
	hsa_t rawHsa;
	int32 ret = SUCCESS;

#if defined(CONFIG_RTL865XC)
	#if defined(VSV)&&WAIT_METHOD==1
	vsv_waitCycle( conn_client, RX_ALE_TX_CYCLE ); /* RX-ALE-TX process time. */
	#endif
	ret = rtl865xC_virtualMacGetHsa(&rawHsa);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	ret = 0;
#else
#error please implement your platform here
#endif
	
	convertHsaToSoftware( &rawHsa, hsa );
	return ret;
}


/*---------------------------------------------------------------------------
 *  +------+         +-------+  *this  +-------+
 *  |  SW  | ------> |convert| ------> |  HSA  |
 *  +------+         +-------+         +-------+
 *-------------------------------------------------------------------------*/
int32 convertHsaToAsic( hsa_param_t* hsa, hsa_t* rawHsa )
{
#if defined(CONFIG_RTL865XC)
	return rtl865xC_convertHsaToAsic(hsa, rawHsa);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	return 0;
#else
#error please implement your platform here
#endif
}


/*---------------------------------------------------------------------------
 *  +------+  *this  +-------+         +-------+
 *  |  SW  | ------> |convert| ------> |  HSA  |
 *  +------+         +-------+         +-------+
 *-------------------------------------------------------------------------*/
int32 virtualMacSetHsa( hsa_param_t* hsa )
{
	hsa_t rawHsa;
	int32 ret;
	
	convertHsaToAsic( hsa, &rawHsa );

#if defined(CONFIG_RTL865XC)
	ret = rtl865xC_virtualMacSetHsa(rawHsa);
#elif defined(CONFIG_RTL8672)
	/* [FIXME] for RTL8672 */
	ret = 0;
#else
#error please implement your platform here
#endif
	
	return ret;
}





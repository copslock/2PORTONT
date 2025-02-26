/*
 *   Handling routines for 802.11 SME (Station Management Entity)
 *
 *  $Id: 8192cd_sme.c,v 1.90.2.36 2011/01/10 07:49:07 jerryko Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_SME_C_

#ifdef __KERNEL__
#ifdef __MIPSEB__
#include <asm/addrspace.h>
#include <linux/module.h>
#endif
#include <linux/list.h>
#include <linux/random.h>
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#ifdef CONFIG_RTL_REPORT_LINK_STATUS
#include <cyg/io/eth/rltk/819x/wrapper/if_status.h>
#endif
#endif
#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./wifi.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_rx.h"
#include "./8192cd_debug.h"
#include "./8192cd_psk.h"
#include "./8192cd_security.h"

#ifdef CONFIG_RTK_MESH
#ifdef __ECOS
#include "./mesh_ext/mesh_util.h"
#include "./mesh_ext/mesh_route.h"
#else
#include "../mesh_ext/mesh_util.h"
#include "../mesh_ext/mesh_route.h"
#endif
#ifdef MESH_USE_METRICOP
#include "mesh_ext/mesh_11kv.h"
#endif
#endif

#ifdef CONFIG_RTL_SIMPLE_CONFIG
#include "8192cd_profile.h"
#endif
#ifdef RTK_NL80211
#include "8192cd_cfg80211.h" 
#endif

#ifdef WIFI_SIMPLE_CONFIG
#ifdef P2P_SUPPORT
extern unsigned char WFA_OUI[];
extern unsigned char WFA_OUI_PLUS_TYPE[];
#define MAX_REASSEM_P2P_IE 512
#endif

#ifdef INCLUDE_WPS
#include "./wps/wsc.h"
#endif

#define TAG_REQUEST_TYPE	0x103a
#define TAG_RESPONSE_TYPE	0x103b

/*add for WPS2DOTX*/
#define TAG_VERSION2		0x1067	 
#define TAG_VENDOR_EXT		0x1049
#define VENDOR_VERSION2 	0x00
unsigned char WSC_VENDOR_OUI[3] = {0x00, 0x37, 0x2a};
/*add for WPS2DOTX*/

#define MAX_REQUEST_TYPE_NUM 0x3
UINT8 WSC_IE_OUI[4] = {0x00, 0x50, 0xf2, 0x04};
UINT8 NEC_OUI[3] = {0x00, 0x0D, 0x02};
#endif

#ifdef WIFI_WMM
unsigned char WMM_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
unsigned char WMM_PARA_IE[6] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};/*cfg p2p cfg p2p*/
#endif

#ifdef CONFIG_IEEE80211W_CLI
#include "./sha256.h"
#endif

#define INTEL_OUI_NUM	145
unsigned char INTEL_OUI[INTEL_OUI_NUM][3] =
{{0x00, 0x02, 0xb3}, {0x00, 0x03, 0x47},
{0x00, 0x04, 0x23}, {0x00, 0x07, 0xe9},
{0x00, 0x0c, 0xf1}, {0x00, 0x0e, 0x0c},
{0x00, 0x0e, 0x35}, {0x00, 0x11, 0x11},
{0x00, 0x12, 0xf0}, {0x00, 0x13, 0x02},
{0x00, 0x13, 0x20}, {0x00, 0x13, 0xce},
{0x00, 0x13, 0xe8}, {0x00, 0x15, 0x00},
{0x00, 0x15, 0x17}, {0x00, 0x16, 0x6f},
{0x00, 0x16, 0x76}, {0x00, 0x16, 0xea},
{0x00, 0x16, 0xeb}, {0x00, 0x18, 0xde},
{0x00, 0x19, 0xd1}, {0x00, 0x19, 0xd2},
{0x00, 0x1b, 0x21}, {0x00, 0x1b, 0x77},
{0x00, 0x1c, 0xbf}, {0x00, 0x1c, 0xc0},
{0x00, 0x1d, 0xe0}, {0x00, 0x1d, 0xe1},
{0x00, 0x1e, 0x64}, {0x00, 0x1e, 0x65},
{0x00, 0x1e, 0x67}, {0x00, 0x1f, 0x3b},
{0x00, 0x1f, 0x3c}, {0x00, 0x20, 0x7b},
{0x00, 0x21, 0x5c}, {0x00, 0x21, 0x5d},
{0x00, 0x21, 0x6a}, {0x00, 0x21, 0x6b},
{0x00, 0x22, 0xfa}, {0x00, 0x22, 0xfb},
{0x00, 0x23, 0x14}, {0x00, 0x23, 0x15},
{0x00, 0x24, 0xd6}, {0x00, 0x24, 0xd7},
{0x00, 0x26, 0xc6}, {0x00, 0x26, 0xc7},
{0x00, 0x27, 0x0e}, {0x00, 0x27, 0x10},
{0x00, 0x50, 0xf1}, {0x00, 0x90, 0x27}, 
{0x00, 0xa0, 0xc9}, {0x00, 0xaa, 0x00}, 
{0x00, 0xaa, 0x01}, {0x00, 0xaa, 0x02}, 
{0x00, 0xc2, 0xc6}, {0x00, 0xd0, 0xb7}, 
{0x00, 0xdb, 0xdf}, {0x08, 0x11, 0x96}, 
{0x08, 0xd4, 0x0c}, {0x0c, 0x8b, 0xfd}, 
{0x0c, 0xd2, 0x92}, {0x10, 0x0b, 0xa9}, 
{0x10, 0x4a, 0x7d}, {0x18, 0x3d, 0xa2}, 
{0x18, 0xff, 0x0f}, {0x24, 0x77, 0x03}, 
{0x28, 0xb2, 0xbd}, {0x30, 0x3a, 0x64}, 
{0x34, 0x02, 0x86}, {0x34, 0x13, 0xe8}, 
{0x34, 0xde, 0x1a}, {0x34, 0xe6, 0xad},
{0x3c, 0xa9, 0xf4}, {0x3c, 0xfd, 0xfe},
{0x40, 0x25, 0xc2}, {0x44, 0x85, 0x00}, 
{0x48, 0x51, 0xb7}, {0x4c, 0x79, 0xba}, 
{0x4c, 0x80, 0x93}, {0x4c, 0xeb, 0x42}, 
{0x50, 0x2d, 0xa2}, {0x58, 0x91, 0xcf}, 
{0x58, 0x94, 0x6b}, {0x58, 0xa8, 0x39},
{0x5c, 0x51, 0x4f}, {0x5c, 0xc5, 0xd4},
{0x5c, 0xd2, 0xe4}, {0x5c, 0xe0, 0xc5},
{0x60, 0x36, 0xdd}, {0x60, 0x57, 0x18}, 
{0x60, 0x67, 0x20}, {0x60, 0x6c, 0x66}, 
{0x64, 0x80, 0x99}, {0x64, 0xd4, 0xda}, 
{0x68, 0x05, 0xca}, {0x68, 0x17, 0x29}, 
{0x68, 0x5d, 0x43}, {0x6c, 0x29, 0x95}, 
{0x6c, 0x88, 0x14}, {0x74, 0xe5, 0x0b}, 
{0x78, 0x92, 0x9c}, {0x78, 0xff, 0x57}, 
{0x7c, 0x7a, 0x91}, {0x7c, 0xcc, 0xb8},
{0x80, 0x00, 0x0b}, {0x80, 0x19, 0x34}, 
{0x80, 0x86, 0xf2}, {0x80, 0x9b, 0x20}, 
{0x84, 0x3a, 0x4b}, {0x84, 0xa6, 0xc8}, 
{0x88, 0x53, 0x2e}, {0x8c, 0x70, 0x5a}, 
{0x8c, 0xa9, 0x82}, {0x90, 0x49, 0xfa}, 
{0x90, 0xe2, 0xba}, {0x98, 0x4f, 0xee}, 
{0x9c, 0x4e, 0x36}, {0xa0, 0x36, 0x9f}, 
{0xa0, 0x88, 0x69}, {0xa0, 0x88, 0xb4}, 
{0xa0, 0xa8, 0xcd}, {0xa4, 0x4e, 0x31}, 
{0xac, 0x72, 0x89}, {0xac, 0x7b, 0xa1}, 
{0xac, 0xfd, 0xce}, {0xb4, 0xb6, 0x76}, 
{0xb8, 0x03, 0x05}, {0xb8, 0x08, 0xcf},
{0xb8, 0x8a, 0x60}, {0xb8, 0xbf, 0x83},
{0xbc, 0x77, 0x37}, {0xc4, 0x85, 0x08}, 
{0xc4, 0xd9, 0x87}, {0xc8, 0xf7, 0x33}, 
{0xcc, 0x3d, 0x82}, {0xd0, 0x7e, 0x35},
{0xd8, 0xfc, 0x93}, {0xdc, 0xa9, 0x71}, 
{0xe0, 0x94, 0x67}, {0xe0, 0x9d, 0x31}, 
{0xe8, 0x2a, 0xea}, {0xe8, 0xb1, 0xfc},
{0xf4, 0x06, 0x69}, {0xf8, 0x16, 0x54}, 
{0xfc, 0xf8, 0xae}};

#define HTC_OUI_NUM	17
unsigned char HTC_OUI[][3]= {			
	{0x00,0x09,0x2D},
	{0x00,0x23,0x76},
	{0x18,0x87,0x96},
	{0x1C,0xB0,0x94},
	{0x38,0xE7,0xD8},
	{0x64,0xA7,0x69},
	{0x7C,0x61,0x93},
	{0x84,0x7A,0x88},
	{0x90,0x21,0x55},
	{0x98,0x0D,0x2E},
	{0xA0,0xF4,0x50},
	{0xA8,0x26,0xD9},
	{0xBC,0xCF,0xCC},
	{0xD4,0x20,0x6D},
	{0xD8,0xB3,0x77},
	{0xE8,0x99,0xC4},
	{0xF8,0xDB,0x7F} 
};


#define PSP_OUI_NUM	51
unsigned char PSP_OUI[PSP_OUI_NUM][3] =
{{0x04, 0x76, 0x6E},
{0x00, 0x26, 0x43},
{0x79, 0xC9, 0x74},
{0x8C, 0x7C, 0xB5},
{0x78, 0xDD, 0x08},
{0x50, 0x63, 0x13},
{0x2C, 0x81, 0x58},
{0x66, 0x60, 0xEC},
{0x5C, 0x6D, 0x20},
{0x00, 0x06, 0xF5},
{0xC0, 0x14, 0x3D},
{0x46, 0x8F, 0x25},
{0xD4, 0x4B, 0x5E},
{0x00, 0xD4, 0x4B},
{0x90, 0x34, 0xFC},
{0x4C, 0x0F, 0x6E},
{0xF0, 0xF0, 0x02},
{0x00, 0x07, 0x04},
{0x00, 0x22, 0xCF},
{0x00, 0x06, 0xF7},
{0x60, 0xF4, 0x94},
{0x0C, 0xEE, 0xE6},
{0x00, 0x1D, 0xD9},
{0x00, 0x1C, 0x26},
{0x00, 0x1B, 0xFB},
{0x00, 0x19, 0x7F},
{0x00, 0x19, 0x7E},
{0x00, 0x19, 0x7D},
{0x00, 0x16, 0xCF},
{0x00, 0x16, 0xFE},
{0x00, 0x13, 0xA8},
{0x00, 0x16, 0xCE},
{0x00, 0x13, 0xA9},
{0x00, 0x14, 0xA4},
{0x00, 0x02, 0xC7},
{0x00, 0x19, 0x66},
{0x00, 0x1F, 0x3A},
{0x00, 0x24, 0x33},
{0x00, 0x26, 0x5C},
{0x00, 0x25, 0x56},
{0x00, 0x24, 0x2C},
{0x00, 0x24, 0x2B},
{0x00, 0x23, 0x4E},
{0x00, 0x23, 0x4D},
{0x00, 0x23, 0x06},
{0x00, 0x22, 0x69},
{0x00, 0x22, 0x68},
{0x00, 0x21, 0x4F},
{0x00, 0x1F, 0xE2},
{0x00, 0x1F, 0xE1},
{0x00, 0x01, 0x4A}};


/* for RTL865x suspend mode, the CPU can be suspended initially. */
int gCpuCanSuspend = 1;

#ifdef RTK_NL80211
static int rtl8192cd_query_psd_cfg80211(struct rtl8192cd_priv *priv, int chnl, int bw, int fft_pts);
#endif
static unsigned int OnAssocReq(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnProbeReq(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnProbeRsp(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnBeacon(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnDisassoc(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnAuth(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnDeAuth(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnWmmAction(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int DoReserved(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
#ifdef WDS
static void issue_probereq(struct rtl8192cd_priv * priv, unsigned char * ssid, int ssid_len, unsigned char * da);
#endif
#ifdef CLIENT_MODE
static unsigned int OnAssocRsp(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnBeaconClnt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnATIM(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnDisassocClnt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnAuthClnt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static unsigned int OnDeAuthClnt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
static void start_clnt_assoc(struct rtl8192cd_priv *priv);
static void calculate_rx_beacon(struct rtl8192cd_priv *priv);
static void updateTSF(struct rtl8192cd_priv *priv);
static void issue_PwrMgt_NullData(struct rtl8192cd_priv * priv);
static unsigned int isOurFrameBuffred(unsigned char* tim, unsigned int aid);
#ifdef WIFI_11N_2040_COEXIST
static void issue_coexist_mgt(struct rtl8192cd_priv *priv);
#endif
#endif

/*Site Survey and sorting result by profile related*/
static int compareTpyeByProfile(struct rtl8192cd_priv *priv , const void *entry1, const void *entry2 , int CompareType);
static int get_profile_index(struct rtl8192cd_priv *priv ,char* SSID2Search);
#ifdef MBSSID
/*under multi-repeater case when some STA has connect , the other one don't connect to diff channel AP ; skip this*/ 
static int multiRepeater_startlookup_chk(struct rtl8192cd_priv *priv , int db_idx);
static int multiRepeater_connection_status(struct rtl8192cd_priv *priv );
#endif

int check_dz_mgmt(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct tx_insn* txcfg);

void SelectLowestInitRate(struct rtl8192cd_priv *priv);
void issue_op_mode_notify(struct rtl8192cd_priv *priv, struct stat_info *pstat, char mode);


#ifdef BEAMFORMING_SUPPORT
void DynamicSelect2STA(struct rtl8192cd_priv *priv);
#endif

struct mlme_handler {
	unsigned int   num;
	char* str;
	unsigned int (*func)(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
};

#ifdef CONFIG_RTK_MESH
struct mlme_handler mlme_mp_tbl[]={
	{WIFI_ASSOCREQ,		"OnAssocReq",	OnAssocReq_MP},
	{WIFI_ASSOCRSP,		"OnAssocRsp",	OnAssocRsp_MP},
	{WIFI_REASSOCREQ,	"OnReAssocReq",	OnAssocReq_MP},
	{WIFI_REASSOCRSP,	"OnReAssocRsp",	OnAssocRsp_MP},
	{WIFI_PROBEREQ,		"OnProbeReq",	OnProbeReq},
	{WIFI_PROBERSP,		"OnProbeRsp",	OnProbeRsp},

	/*----------------------------------------------------------
					below 2 are reserved
	-----------------------------------------------------------*/
	{0,					"DoReserved",	DoReserved},
	{0,					"DoReserved",	DoReserved},
	{WIFI_BEACON,		"OnBeacon",		OnBeacon},
	{WIFI_ATIM,			"OnATIM",		DoReserved},
	{WIFI_DISASSOC,		"OnDisassoc",	OnDisassoc_MP},
	{WIFI_AUTH,			"OnAuth",		OnAuth},
	{WIFI_DEAUTH,		"OnDeAuth",		OnDeAuth},
	{WIFI_WMM_ACTION,	"OnWmmAct",		OnWmmAction}
};
#endif	// CONFIG_RTK_MESH

struct mlme_handler mlme_ap_tbl[]={
	{WIFI_ASSOCREQ,		"OnAssocReq",	OnAssocReq},
	{WIFI_ASSOCRSP,		"OnAssocRsp",	DoReserved},
	{WIFI_REASSOCREQ,	"OnReAssocReq",	OnAssocReq},
	{WIFI_REASSOCRSP,	"OnReAssocRsp",	DoReserved},
	{WIFI_PROBEREQ,		"OnProbeReq",	OnProbeReq},
	{WIFI_PROBERSP,		"OnProbeRsp",	OnProbeRsp},

	/*----------------------------------------------------------
					below 2 are reserved
	-----------------------------------------------------------*/
	{0,					"DoReserved",	DoReserved},
	{0,					"DoReserved",	DoReserved},
	{WIFI_BEACON,		"OnBeacon",		OnBeacon},
	{WIFI_ATIM,			"OnATIM",		DoReserved},
	{WIFI_DISASSOC,		"OnDisassoc",	OnDisassoc},
	{WIFI_AUTH,			"OnAuth",		OnAuth},
	{WIFI_DEAUTH,		"OnDeAuth",		OnDeAuth},
	{WIFI_WMM_ACTION,	"OnWmmAct",		OnWmmAction}
};
#ifdef CLIENT_MODE
struct mlme_handler mlme_station_tbl[]={
	{WIFI_ASSOCREQ,		"OnAssocReq",	DoReserved},
	{WIFI_ASSOCRSP,		"OnAssocRsp",	OnAssocRsp},
	{WIFI_REASSOCREQ,	"OnReAssocReq",	DoReserved},
	{WIFI_REASSOCRSP,	"OnReAssocRsp",	OnAssocRsp},
	{WIFI_PROBEREQ,		"OnProbeReq",	OnProbeReq},
	{WIFI_PROBERSP,		"OnProbeRsp",	OnProbeRsp},

	/*----------------------------------------------------------
					below 2 are reserved
	-----------------------------------------------------------*/
	{0,					"DoReserved",	DoReserved},
	{0,					"DoReserved",	DoReserved},
	{WIFI_BEACON,		"OnBeacon",		OnBeaconClnt},
	{WIFI_ATIM,			"OnATIM",		OnATIM},
	{WIFI_DISASSOC,		"OnDisassoc",	OnDisassocClnt},
	{WIFI_AUTH,			"OnAuth",		OnAuthClnt},
	{WIFI_DEAUTH,		"OnDeAuth",		OnDeAuthClnt},
	{WIFI_WMM_ACTION,	"OnWmmAct",		OnWmmAction}
};
#endif

#ifdef CONFIG_RTL_WLAN_DOS_FILTER
#define MAX_BLOCK_MAC		4
unsigned char block_mac[MAX_BLOCK_MAC][6];
unsigned int block_mac_idx = 0;
unsigned int block_sta_time = 0;
unsigned long block_priv;
#endif

static int is_support_wpa_aes(struct rtl8192cd_priv *priv, unsigned char *pucIE, unsigned long ulIELength)
{
	unsigned short version, usSuitCount;
	DOT11_RSN_IE_HEADER *pDot11RSNIEHeader;
	DOT11_RSN_IE_SUITE *pDot11RSNIESuite;
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNIECountSuite;
	unsigned char *ptr;

	if (ulIELength < sizeof(DOT11_RSN_IE_HEADER)) {
		DEBUG_WARN("parseIE err 1!\n");
		return -1;
	}

	pDot11RSNIEHeader = (DOT11_RSN_IE_HEADER *)pucIE;
	ptr = (unsigned char *)&pDot11RSNIEHeader->Version;
	version = (ptr[1] << 8) | ptr[0];
	
	if (version != RSN_VER1) {
		DEBUG_WARN("parseIE err 2!\n");
		return -1;
	}

	if (pDot11RSNIEHeader->ElementID != RSN_ELEMENT_ID ||
			pDot11RSNIEHeader->Length != ulIELength -2 ||
			pDot11RSNIEHeader->OUI[0] != 0x00 || pDot11RSNIEHeader->OUI[1] != 0x50 ||
			pDot11RSNIEHeader->OUI[2] != 0xf2 || pDot11RSNIEHeader->OUI[3] != 0x01 ) {
		DEBUG_WARN("parseIE err 3!\n");
		return -1;
	}

	ulIELength -= sizeof(DOT11_RSN_IE_HEADER);
	pucIE += sizeof(DOT11_RSN_IE_HEADER);

	//----------------------------------------------------------------------------------
 	// Multicast Cipher Suite processing
	//----------------------------------------------------------------------------------
	if (ulIELength < sizeof(DOT11_RSN_IE_SUITE)) {
		DEBUG_WARN("parseIE err 4!\n");
		return -1;
	}

	pDot11RSNIESuite = (DOT11_RSN_IE_SUITE *)pucIE;
	if (pDot11RSNIESuite->OUI[0] != 0x00 ||
		pDot11RSNIESuite->OUI[1] != 0x50 ||
		pDot11RSNIESuite->OUI[2] != 0xF2) {
		DEBUG_WARN("parseIE err 5!\n");
		return -1;
	}

	ulIELength -= sizeof(DOT11_RSN_IE_SUITE);
	pucIE += sizeof(DOT11_RSN_IE_SUITE);

	//----------------------------------------------------------------------------------
	// Pairwise Cipher Suite processing
	//----------------------------------------------------------------------------------
	if (ulIELength < 2 + sizeof(DOT11_RSN_IE_SUITE)) {
		DEBUG_WARN("parseIE err 6!\n");
		return -1;
	}

	pDot11RSNIECountSuite = (PDOT11_RSN_IE_COUNT_SUITE)pucIE;
	pDot11RSNIESuite = pDot11RSNIECountSuite->dot11RSNIESuite;
	ptr = (unsigned char *)&pDot11RSNIECountSuite->SuiteCount;
	usSuitCount = (ptr[1] << 8) | ptr[0];

	if (usSuitCount != 1 ||
			pDot11RSNIESuite->OUI[0] != 0x00 ||
			pDot11RSNIESuite->OUI[1] != 0x50 ||
			pDot11RSNIESuite->OUI[2] != 0xF2) {
		DEBUG_WARN("parseIE err 7!\n");
		return -1;
	}

	if (pDot11RSNIESuite->Type == DOT11_ENC_CCMP)
		return 1;
	else
		return 0;
}


static int is_support_wpa2_aes(struct rtl8192cd_priv *priv, 	unsigned char *pucIE, unsigned long ulIELength)
{
	unsigned short version, usSuitCount;
	DOT11_WPA2_IE_HEADER *pDot11WPA2IEHeader = NULL;
	DOT11_RSN_IE_SUITE  *pDot11RSNIESuite = NULL;
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNIECountSuite = NULL;
	unsigned char *ptr;

	if (ulIELength < sizeof(DOT11_WPA2_IE_HEADER)) {
		DEBUG_WARN("ERROR_INVALID_RSNIE_LEN, err 1\n");
		return -1;
	}

	pDot11WPA2IEHeader = (DOT11_WPA2_IE_HEADER *)pucIE;
	ptr = (unsigned char *)&pDot11WPA2IEHeader->Version;
	version = (ptr[1] << 8) | ptr[0];
	
	if (version != RSN_VER1) {
		DEBUG_WARN("ERROR_UNSUPPORTED_RSNEVERSION, err 2\n");
		return -1;
	}

	if (pDot11WPA2IEHeader->ElementID != WPA2_ELEMENT_ID ||
		pDot11WPA2IEHeader->Length != ulIELength -2 ) {
		DEBUG_WARN("ERROR_INVALID_RSNIE, err 3\n");
		return -1;
	}

	ulIELength -= sizeof(DOT11_WPA2_IE_HEADER);
	pucIE += sizeof(DOT11_WPA2_IE_HEADER);

	//----------------------------------------------------------------------------------
 	// Multicast Cipher Suite processing
	//----------------------------------------------------------------------------------
	if (ulIELength < sizeof(DOT11_RSN_IE_SUITE)) {
		DEBUG_WARN("ERROR_INVALID_RSNIE_LEN, err 4\n");
		return -1;
	}

	pDot11RSNIESuite = (DOT11_RSN_IE_SUITE *)pucIE;
	if (pDot11RSNIESuite->OUI[0] != 0x00 ||
			pDot11RSNIESuite->OUI[1] != 0x0F ||
				pDot11RSNIESuite->OUI[2] != 0xAC) {
		DEBUG_WARN("ERROR_INVALID_RSNIE, err 5\n");
		return -1;
	}

	if (pDot11RSNIESuite->Type > DOT11_ENC_WEP104)	{
		DEBUG_WARN("ERROR_INVALID_MULTICASTCIPHER, err 6\n");
		return -1;
	}

	ulIELength -= sizeof(DOT11_RSN_IE_SUITE);
	pucIE += sizeof(DOT11_RSN_IE_SUITE);

	//----------------------------------------------------------------------------------
	// Pairwise Cipher Suite processing
	//----------------------------------------------------------------------------------
	if (ulIELength < 2 + sizeof(DOT11_RSN_IE_SUITE)) {
		DEBUG_WARN("ERROR_INVALID_RSN_IE_SUITE_LEN, err 7\n");
		return -1;
	}

	pDot11RSNIECountSuite = (PDOT11_RSN_IE_COUNT_SUITE)pucIE;
	pDot11RSNIESuite = pDot11RSNIECountSuite->dot11RSNIESuite;
	ptr = (unsigned char *)&pDot11RSNIECountSuite->SuiteCount;
	usSuitCount = (ptr[1] << 8) | ptr[0];

	if (usSuitCount != 1 ||
		pDot11RSNIESuite->OUI[0] != 0x00 ||
			pDot11RSNIESuite->OUI[1] != 0x0F ||
				pDot11RSNIESuite->OUI[2] != 0xAC) {
		DEBUG_WARN("ERROR_INVALID_RSNIE, err 8\n");
		return -1;
	}

	if (pDot11RSNIESuite->Type > DOT11_ENC_WEP104) {
		DEBUG_WARN("ERROR_INVALID_UNICASTCIPHER, err 9\n");
		return -1;
	}

	if (pDot11RSNIESuite->Type == DOT11_ENC_CCMP)
		return 1;
	else
		return 0;
}

static unsigned char check_probe_sta_rssi_valid(struct rtl8192cd_priv *priv,unsigned char* addr, unsigned char rssi)
{
	int i, idx=-1;
	unsigned char *hwaddr;
	hwaddr = addr;
	for (i=0; i<MAX_PROBE_REQ_STA; i++) {
		if (!memcmp(priv->probe_sta[i].addr, addr, MACADDRLEN)){
			idx = i;
			break;// check if it is already in the list
		}
	}
	if (idx < 0){
		return 1;// if probe req sta isn't in the list, allow it 
	}	
	else{
		if ( priv->probe_sta[idx].rssi && RTL_ABS(priv->probe_sta[idx].rssi,rssi) > 10)
			return 0;
		else
			return 1;	
	}
}
static void add_probe_req_sta(struct rtl8192cd_priv *priv,unsigned char* addr, unsigned char rssi)
{
	int i, idx=-1, idx2 =0;
	unsigned char *hwaddr;
	unsigned char rssi_input;	
	hwaddr = addr;    
	for (i=0; i<MAX_PROBE_REQ_STA; i++) {
		if (priv->probe_sta[i].used == 0) {
			if (idx < 0)
				idx = i; //search for empty entry
			continue;
		}
		if (!memcmp(priv->probe_sta[i].addr, addr, MACADDRLEN)) {
			idx2 = i;
			break;// check if it is already in the list
		}
	}
	if (idx >= 0){
		rssi_input = rssi;
		memcpy(priv->probe_sta[idx].addr, addr, MACADDRLEN);	
		priv->probe_sta[idx].used = 1;		
		priv->probe_sta[idx].Entry = idx;//check which entry is the probe sta recorded
		priv->probe_sta[idx].rssi = rssi_input;
		priv->pshare->ProbeReqEntryOccupied++;
		return;
	}
	else if (idx2){
		rssi_input = ((priv->probe_sta[idx2].rssi * 7)+(rssi * 3)) / 10;			
		priv->probe_sta[idx2].rssi = rssi_input;
		return;
	}
	else if (priv->pshare->ProbeReqEntryOccupied == MAX_PROBE_REQ_STA) {// sta list full, need to replace sta
			idx = priv->pshare->ProbeReqEntryNum;	
			for (i=0; i<MAX_PROBE_REQ_STA; i++) {
				if (!memcmp(priv->probe_sta[i].addr, addr, MACADDRLEN))					
					return;		// check if it is already in the list			
			}
			memcpy(priv->probe_sta[idx].addr, addr, MACADDRLEN);		
			priv->probe_sta[idx].used = 1;
			priv->probe_sta[idx].Entry = idx;		
			priv->probe_sta[idx].rssi = rssi;
			priv->pshare->ProbeReqEntryNum++;			
			if( priv->pshare->ProbeReqEntryNum == MAX_PROBE_REQ_STA)	
				priv->pshare->ProbeReqEntryNum = 0; // Reset entry counter;
			return;
		}	
}

#ifdef WIFI_SIMPLE_CONFIG
/* WPS2DOTX   */
#ifdef __ECOS //search_VendorExt_tag() also defined in wsc/util.c. Static definition here to avoid confilct.
static 
#endif
unsigned char *search_VendorExt_tag(unsigned char *data, unsigned char id, int len, int *out_len)
{
	unsigned char tag, tag_len;
	int size;

	//skip WFA_VENDOR_LEN
	data+=3;
	len-=3;
	
	while (len > 0) {
		memcpy(&tag, data, 1);
		memcpy(&tag_len, data+1, 1);
		if (id == tag) {
			if (len >= (2 + tag_len)) {
				*out_len = (int)tag_len;
				return (&data[2]);
			}
			else {
				_DEBUG_ERR("Found VE tag [0x%x], but invalid length!\n", id);
				break;
			}
		}
		size = 2 + tag_len;
		data += size;
		len -= size;
	}

	return NULL;
}
/* WPS2DOTX   */

unsigned char *search_wsc_tag(unsigned char *data, unsigned short id, int len, int *out_len)
{
	unsigned short tag, tag_len;
	int size;

	while (len > 0) {
		memcpy(&tag, data, 2);
		memcpy(&tag_len, data+2, 2);
		tag = ntohs(tag);
		tag_len = ntohs(tag_len);

		if (id == tag) {
			if (len >= (4 + tag_len)) {
				*out_len = (int)tag_len;
				return (&data[4]);
			}
			else {
				_DEBUG_ERR("Found tag [0x%x], but invalid length!\n", id);
				break;
			}
		}
		size = 4 + tag_len;
		data += size;
		len -= size;
	}

	return NULL;
}


static struct wsc_probe_request_info *search_wsc_probe_sta(struct rtl8192cd_priv *priv, unsigned char *addr)
{
	int i, idx=-1;

	for (i=0; i<MAX_WSC_PROBE_STA; i++) {
		if (priv->wsc_sta[i].used == 0) {
			if (idx < 0)
				idx = i;
			continue;
		}
		if (!memcmp(priv->wsc_sta[i].addr, addr, MACADDRLEN))
			break;
	}

	if ( i != MAX_WSC_PROBE_STA)
		return (&priv->wsc_sta[i]); // return sta info for WSC sta

	if (idx >= 0)
		return (&priv->wsc_sta[idx]); // add sta info for WSC sta
	else {
		// sta list full, need to replace sta
		unsigned long oldest_time_stamp=jiffies;

		for (i=0; i<MAX_WSC_PROBE_STA; i++) {
			if (priv->wsc_sta[i].time_stamp < oldest_time_stamp) {
				oldest_time_stamp = priv->wsc_sta[i].time_stamp;
				idx = i;
			}
		}
		memset(&priv->wsc_sta[idx], 0, sizeof(struct wsc_probe_request_info));

		return (&priv->wsc_sta[idx]);
	}
}


static int search_wsc_pbc_probe_sta(struct rtl8192cd_priv *priv, unsigned char *addr)
{
	int i/*, idx=-1*/;
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	SAVE_INT_AND_CLI(flags);

	for (i=0; i<MAX_WSC_PROBE_STA; i++) {
		if (priv->wsc_sta[i].used==1 && priv->wsc_sta[i].pbcactived==1) {

			if (!memcmp(priv->wsc_sta[i].addr, addr, MACADDRLEN)){

				priv->wsc_sta[i].used=0;
				priv->wsc_sta[i].pbcactived=0;				
				RESTORE_INT(flags);	
				return 1;
			}
		}
	}
	RESTORE_INT(flags);	
	return 0;
	
}

#define TAG_DEVICE_PASSWORD_ID		0x1012
#define PASS_ID_PB					0x4
static void wsc_forward_probe_request(struct rtl8192cd_priv *priv, unsigned char *pframe, unsigned char *IEaddr, unsigned int IElen)
{
	unsigned char *p=IEaddr;
	unsigned int len=IElen;
	unsigned char forwarding=0;
	struct wsc_probe_request_info *wsc_sta=NULL;
	DOT11_PROBE_REQUEST_IND ProbeReq_Ind;
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	unsigned char *p2=IEaddr;
	unsigned int len2=IElen;	
	unsigned short pwid=0;	

	if (IEaddr == NULL || IElen == 0)
		return;
	if (IElen > PROBEIELEN) {
		DEBUG_WARN("[%s] IElen=%d\n", __FUNCTION__, IElen);
		return;
	}
	p = search_wsc_tag(p+2+4, TAG_REQUEST_TYPE, len-4, (int *)&len);
	if (p && (*p <= MAX_REQUEST_TYPE_NUM)) { //forward WPS IE to wsc daemon
		SAVE_INT_AND_CLI(flags);
		wsc_sta = search_wsc_probe_sta(priv, (unsigned char *)GetAddr2Ptr(pframe));
		p2 = search_wsc_tag(p2+2+4, TAG_DEVICE_PASSWORD_ID, len2-4, (int *)&len2);	
		if(p2){
			memcpy(&pwid, p2, len2);		
			pwid = ntohs(pwid);
			if(pwid==PASS_ID_PB){
				wsc_sta->pbcactived=1;
			}
		}
		if (wsc_sta->used) {
			if ((wsc_sta->ProbeIELen != IElen) ||
				(memcmp(wsc_sta->ProbeIE, (void *)(IEaddr), IElen) != 0) ||
				((jiffies - wsc_sta->time_stamp) > RTL_SECONDS_TO_JIFFIES(3))) 
			{
				memcpy(wsc_sta->ProbeIE, (void *)(IEaddr), IElen);
				wsc_sta->ProbeIELen = IElen;
				wsc_sta->time_stamp = jiffies;
				forwarding = 1;
			}
		}
		else {
			memcpy(wsc_sta->addr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
			memcpy(wsc_sta->ProbeIE, (void *)(IEaddr), IElen);
			wsc_sta->ProbeIELen = IElen;
			wsc_sta->time_stamp = jiffies;
			wsc_sta->used = 1;
			forwarding = 1;
		}
		RESTORE_INT(flags);

		if (forwarding) {
			memcpy((void *)ProbeReq_Ind.MACAddr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
			ProbeReq_Ind.EventId = DOT11_EVENT_WSC_PROBE_REQ_IND;
			ProbeReq_Ind.IsMoreEvent = 0;
			ProbeReq_Ind.ProbeIELen = IElen;
			memcpy((void *)ProbeReq_Ind.ProbeIE, (void *)(IEaddr), ProbeReq_Ind.ProbeIELen);
#ifdef INCLUDE_WPS
			//			wps_indicate_evt(priv);
			wps_NonQueue_indicate_evt(priv ,
				(UINT8 *)&ProbeReq_Ind,sizeof(DOT11_PROBE_REQUEST_IND));		
#else
			DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&ProbeReq_Ind, sizeof(DOT11_PROBE_REQUEST_IND));
#ifdef WIFI_HAPD
			event_indicate_hapd(priv, GetAddr2Ptr(pframe), HAPD_WPS_PROBEREQ, (UINT8 *)&ProbeReq_Ind); //_Eric ???? this event is not registered
#ifdef HAPD_DRV_PSK_WPS
			event_indicate(priv, GetAddr2Ptr(pframe), 1);
#endif
#else
			event_indicate(priv, GetAddr2Ptr(pframe), 1);
#endif
#endif
		}
	}
}


static __inline__ void wsc_probe_expire(struct rtl8192cd_priv *priv)
{
	int i;
	//unsigned long flags;

	//SAVE_INT_AND_CLI(flags);
	for (i=0; i<MAX_WSC_PROBE_STA; i++) {
		if (priv->wsc_sta[i].used == 0)
			continue;
		if ((jiffies - priv->wsc_sta[i].time_stamp) > RTL_SECONDS_TO_JIFFIES(180))
			memset(&priv->wsc_sta[i], 0, sizeof(struct wsc_probe_request_info));
	}
	//RESTORE_INT(flags);
}
#endif // WIFI_SIMPLE_CONFIG


static __inline__ UINT8 match_supp_rate(unsigned char *pRate, int len, UINT8 rate)
{
	int idx;
	for (idx=0; idx<len; idx++) {
		if ((pRate[idx] & 0x7f) == rate)
			return 1;
	}

	// TODO: need some more refinement
	if ((rate & 0x80) && ((rate & 0x7f) < 16))
		return 1;

	return 0;
}


// unchainned all the skb chainnned in a given list, like frag_list(type == 0)
void unchainned_all_frag(struct rtl8192cd_priv *priv, struct list_head *phead)
{
	struct rx_frinfo *pfrinfo;
	struct list_head *plist;
	struct sk_buff	 *pskb;

	while (!list_empty(phead)) {
		plist = phead->next;
		list_del(plist);
		
		pfrinfo = list_entry(plist, struct rx_frinfo, mpdu_list);
		pskb = get_pskb(pfrinfo);
		rtl_kfree_skb(priv, pskb, _SKB_RX_);
	}
}


void rtl8192cd_frag_timer(unsigned long task_priv)
{
	unsigned long flags;
	struct list_head	*phead, *plist;
	struct stat_info	*pstat;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	
	struct list_head frag_list;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;
	
	INIT_LIST_HEAD(&frag_list);

	priv->frag_to ^= 0x01;

	phead = &priv->defrag_list;

	DEFRAG_LOCK(flags);
	
	plist = phead->next;
	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, defrag_list);
		plist = plist->next;

		if (pstat->frag_to == priv->frag_to) {
			list_del_init(&pstat->defrag_list);
			list_splice_init(&pstat->frag_list, &frag_list);
			pstat->frag_count = 0;
		}
	}
	
	DEFRAG_UNLOCK(flags);
	
	unchainned_all_frag(priv, &frag_list);

	mod_timer(&priv->frag_to_filter, jiffies + FRAG_TO);
}


#ifdef USB_PKT_RATE_CTRL_SUPPORT
usb_pktCnt_fn get_usb_pkt_cnt_hook = NULL;
register_usb_pkt_cnt_fn register_usb_hook = NULL;

void register_usb_pkt_cnt_f(void *usbPktFunc)
{
	get_usb_pkt_cnt_hook = (usb_pktCnt_fn)(usbPktFunc);
}


void usbPkt_timer_handler(struct rtl8192cd_priv *priv)
{
	unsigned int pkt_cnt, pkt_diff;

	if (!get_usb_pkt_cnt_hook)
		return;

	pkt_cnt = get_usb_pkt_cnt_hook();
	pkt_diff = pkt_cnt - priv->pre_pkt_cnt;

	if (pkt_diff) {
		priv->auto_rate_mask = 0x803fffff;
		priv->change_toggle = ((priv->change_toggle) ? 0 : 1);
	}

	priv->pre_pkt_cnt = pkt_cnt;
	priv->pkt_nsec_diff += pkt_diff;

	if ((++priv->poll_usb_cnt) % 10 == 0) {
		if ((priv->pkt_nsec_diff) < 10 ) {
			priv->auto_rate_mask = 0;
			priv->pkt_nsec_diff = 0;
		}
	}
}
#endif // USB_PKT_RATE_CTRL_SUPPORT

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
static void auth_expire(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	struct list_head	local_head;
#ifdef SMP_SYNC
	unsigned long	flags;
#endif

	INIT_LIST_HEAD(&local_head);

	SMP_LOCK_AUTH_LIST(flags);

	phead = &priv->auth_list;
	plist = phead->next;

	//SAVE_INT_AND_CLI(flags);
	while(plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, auth_list);
		plist = plist->next;

// #if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH) // Skip MP node
#ifdef CONFIG_RTK_MESH // Skip MP node
		if(isPossibleNeighbor(pstat))
			continue;
#endif // CONFIG_RTK_MESH

		pstat->expire_to--;
		if (pstat->expire_to == 0)
		{
			list_del_init(&pstat->auth_list);
			list_add_tail(&pstat->auth_list, &local_head);
		}
	}
	//RESTORE_INT(flags);

	SMP_UNLOCK_AUTH_LIST(flags);

	phead = &local_head;
	plist = phead->next;

	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, auth_list);
		plist = plist->next;

		//below should be take care... since auth fail, just free the stat info...
		DEBUG_INFO("auth expire %02X%02X%02X%02X%02X%02X\n",
			pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);

		free_stainfo(priv, pstat);
	}
}
#else
static void auth_expire(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	//unsigned long	flags;

	phead = &priv->auth_list;
	plist = phead->next;

	//SAVE_INT_AND_CLI(flags);
	while(plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, auth_list);
		plist = plist->next;

// #if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH) // Skip MP node
#ifdef CONFIG_RTK_MESH // Skip MP node
		if(isPossibleNeighbor(pstat))
			continue;
#endif // CONFIG_RTK_MESH

		pstat->expire_to--;
		if (pstat->expire_to == 0)
		{
			list_del_init(&pstat->auth_list);

			//below should be take care... since auth fail, just free the stat info...
			DEBUG_INFO("auth expire %02X%02X%02X%02X%02X%02X\n",
				pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
			free_stainfo(priv, pstat);
		}
	}
	//RESTORE_INT(flags);
}
#endif


#if 0 // def RTL8192SE
void reset_1r_sta_RA(struct rtl8192cd_priv *priv, unsigned int sg_rate){
	struct list_head	*phead, *plist;
	struct stat_info	*pstat;
#ifdef SMP_SYNC
	unsigned long flags=0;
#endif

	phead = &priv->asoc_list;

	SMP_LOCK_ASOC_LIST(flags);

	plist = phead->next;
	while(plist != phead)
	{

		unsigned int sta_band = 0;
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;

		if(pstat && !pstat->ht_cap_len)
			continue;

		if (pstat->tx_ra_bitmap & 0xffff000)
			sta_band |= WIRELESS_11N | WIRELESS_11G | WIRELESS_11B;
		else if (pstat->tx_ra_bitmap & 0xff0)
			sta_band |= WIRELESS_11G |WIRELESS_11B;
		else
			sta_band |= WIRELESS_11B;

		if((pstat->tx_ra_bitmap & 0x0ff00000) == 0 && (pstat->tx_ra_bitmap & BIT(28))!=0 && sg_rate == 0xffff){
			pstat->tx_ra_bitmap &= ~BIT(28); // disable short GI for 1R sta
			set_fw_reg(priv, (0xfd0000a2 | ((REMAP_AID(pstat) & 0x1f)<<4 | (sta_band & 0xf))<<8), pstat->tx_ra_bitmap, 1);
		}
		else if((pstat->tx_ra_bitmap & 0x0ff00000) == 0 && (pstat->tx_ra_bitmap & BIT(28))==0 && sg_rate == 0x7777){
			pstat->tx_ra_bitmap |= BIT(28); // enable short GI for 1R sta
			set_fw_reg(priv, (0xfd0000a2 | ((REMAP_AID(pstat) & 0x1f)<<4 | (sta_band & 0xf))<<8), pstat->tx_ra_bitmap, 1);
		}
	}

	SMP_UNLOCK_ASOC_LIST(flags);

	return;
}
#endif



// for simplify, we consider only two stations. Otherwise we may sorting all the stations and
// hard to maintain the code.
// 0 for path A/B selection(bg only or 1ss rate), 1 for TX Diversity (ex: DIR 655 clone)
#if 0
struct stat_info* switch_ant_enable(struct rtl8192cd_priv *priv, unsigned char flag)
{
	struct stat_info	*pstat, *pstat_chosen = NULL;
	struct list_head	*phead, *plist;
	unsigned int tp_2nd = 0, maxTP = 0;
	unsigned int rssi_2ndTp = 0, rssi_maxTp = 0;
	unsigned int tx_2s_avg = 0;
	unsigned int rx_2s_avg = 0;
	unsigned long total_sum = (priv->pshare->current_tx_bytes+priv->pshare->current_rx_bytes);
	unsigned char th_rssi = 0;
#ifdef SMP_SYNC
	unsigned long flags=0;
#endif

	phead = &priv->asoc_list;

	SMP_LOCK_ASOC_LIST(flags);

	plist = phead->next;
	while (plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;

		if((pstat->tx_avarage + pstat->rx_avarage) > maxTP){
			tp_2nd = maxTP;
			rssi_2ndTp = rssi_maxTp;

			maxTP = pstat->tx_avarage + pstat->rx_avarage;
			rssi_maxTp = pstat->rssi;

			pstat_chosen = pstat;
		}
	}

	SMP_UNLOCK_ASOC_LIST(flags);

	// for debug
//	printk("maxTP: %d, second: %d\n", rssi_maxTp, rssi_2ndTp);

	if(pstat_chosen == NULL){
//		printk("ERROR! NULL pstat_chosen \n");
		return NULL;
	}

	if(total_sum != 0){
		tx_2s_avg = (unsigned int)((pstat_chosen->current_tx_bytes*100) / total_sum);
		rx_2s_avg = (unsigned int)((pstat_chosen->current_rx_bytes*100) / total_sum);
	}

	if( priv->assoc_num > 1 && (tx_2s_avg+rx_2s_avg) < (100/priv->assoc_num)){ // this is not a burst station
		pstat_chosen = NULL;
//		printk("avg is: %d\n", (tx_2s_avg+rx_2s_avg));
		goto out_switch_ant_enable;
	}

	if(flag == 1)
		goto out_switch_ant_enable;

	if(pstat_chosen && (pstat_chosen->sta_in_firmware == 1) && 
        !getSTABitMap(&priv->pshare->has_2r_sta, REMAP_AID(pstat_chosen)) // 1r STA
	  )
		th_rssi = 40;
	else
		th_rssi = 63;

	if((maxTP < tp_2nd*2 && (rssi_maxTp < th_rssi || rssi_2ndTp < th_rssi)))
		pstat_chosen = NULL;
	else if(maxTP >= tp_2nd*2 && rssi_maxTp < th_rssi)
		pstat_chosen = NULL;

out_switch_ant_enable:
	return pstat_chosen;
}
#endif


void dynamic_response_rate(struct rtl8192cd_priv *priv, short rssi)
{
		if(rssi<30){
			if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G){
				if(priv->pshare->current_rsp_rate !=0x10){
					RTL_W16(RRSR,0x10);
					priv->pshare->current_rsp_rate=0x10;
					//SDEBUG("current_rsp_rate=%x\n" ,priv->pshare->current_rsp_rate);
				}
			}else if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G){
				if(priv->pshare->current_rsp_rate!=0x1f){
					RTL_W16(RRSR,0x1f);
					priv->pshare->current_rsp_rate=0x1f;
				}
			}

		}else if(rssi>35){
			if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G){
				if(priv->pshare->current_rsp_rate!=0x150){
					RTL_W16(RRSR,0x150);
					priv->pshare->current_rsp_rate=0x150;					
					//SDEBUG("current_rsp_rate=%x\n" ,priv->pshare->current_rsp_rate);
				}
			}else if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G){
				if(priv->pshare->current_rsp_rate!=0x15f){
					RTL_W16(RRSR,0x15f);
					priv->pshare->current_rsp_rate=0x15f;
				}
			}
		}	
}



static void assoc_expire(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	unsigned int	ok_curr, ok_pre;
	unsigned int	highest_tp = 0;
	struct stat_info	*pstat_highest=NULL;
    int i,j;
	phead = &priv->asoc_list;
	plist = phead;

	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		pstat->link_time++;

#ifdef CLIENT_MODE
		if ((OPMODE & WIFI_STATION_STATE) && (pstat->expire_to > 0)) {
			if ((priv->pshare->rf_ft_var.sta_mode_ps && !priv->ps_state) ||
				(!priv->pshare->rf_ft_var.sta_mode_ps && priv->ps_state)) {
				if (!priv->ps_state)
					priv->ps_state++;
				else
					priv->ps_state = 0;

				issue_PwrMgt_NullData(priv);
			}
#ifdef MULTI_MAC_CLONE
			else if (!memcmp(GET_MIB(priv)->dot11OperationEntry.hwaddr, pstat->sa_addr, MACADDRLEN)) // repeater send null packet
			{
				if (--priv->repeater_to == 0)
				{
					issue_NullData(priv, pstat->hwaddr);
					priv->repeater_to = REPEATER_TO / HZ;
				}
			}
#endif
		}
#endif

		// Check idle using packet transmit....nctu note it
		ok_curr = pstat->tx_pkts - pstat->tx_fail;
		ok_pre = pstat->tx_pkts_pre - pstat->tx_fail_pre;
#ifdef LAZY_WDS
		if ((!(pstat->state & WIFI_WDS) &&
			(ok_curr == ok_pre) &&
			(pstat->rx_pkts == pstat->rx_pkts_pre)) ||
			((pstat->state & WIFI_WDS_LAZY) &&
				((pstat->rx_pkts == pstat->rx_pkts_pre) && !pstat->beacon_num)))
#else
#if defined(__ECOS) && defined(CONFIG_SDIO_HCI) && defined(CONFIG_RTL_88E_SUPPORT)
		if (pstat->rx_pkts == pstat->rx_pkts_pre)
#else
		if ((ok_curr == ok_pre) &&
			(pstat->rx_pkts == pstat->rx_pkts_pre))
#endif
#endif
		{
			pstat->idle_count++;
			if (pstat->expire_to > 0)
			{
				// free queued skb if sta is idle longer than 5 seconds
				if ((priv->expire_to - pstat->expire_to) == 5){			
					pstat->idle_count = 0;
					free_sta_skb(priv, pstat);
				for (i=0; i<8; i++)
					for (j=0; j<TUPLE_WINDOW; j++)
						pstat->tpcache[i][j] = 0xffff;
				pstat->tpcache_mgt = 0xffff;
				}

				if(pstat->idle_count >= 5) {
					pstat->idle_count = 0;
					free_sta_tx_skb(priv, pstat);
				}
				
				// calculate STA number
				if ((pstat->expire_to == 1)
#ifdef WDS
#ifdef LAZY_WDS
					&& (!(pstat->state & WIFI_WDS) || (pstat->state & WIFI_WDS_LAZY))
#else
					&& !(pstat->state & WIFI_WDS)
#endif
#endif
#ifdef A4_STA
                    && !(pstat->state & WIFI_A4_STA)
#endif
				) {

#ifdef LAZY_WDS
					if (!(pstat->state & WIFI_WDS_LAZY)) 
#endif
					{
						cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
						check_sta_characteristic(priv, pstat, DECREASE);
					}

					// CAM entry update
					if (!SWCRYPTO && pstat->dot11KeyMapping.keyInCam) {
						if (CamDeleteOneEntry(priv, pstat->hwaddr, 0, 0)) {
							pstat->dot11KeyMapping.keyInCam = FALSE;
							pstat->tmp_rmv_key = TRUE;
							priv->pshare->CamEntryOccupied--;
						}
						#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
						if (CamDeleteOneEntry(priv, pstat->hwaddr, 0, 0)) {
							pstat->dot11KeyMapping.keyInCam = FALSE;
							pstat->tmp_rmv_key = TRUE;
							priv->pshare->CamEntryOccupied--;
						}
						#endif
					}

					release_remapAid(priv, pstat);

					LOG_MSG("A STA is expired - %02X:%02X:%02X:%02X:%02X:%02X\n",
						pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);

#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
					if ((OPMODE & WIFI_AP_STATE) && priv->sta_status_cb_func)
						priv->sta_status_cb_func("wlan0", pstat->hwaddr, WIFI_STA_STATUS_EXPIRED);
#endif

#ifdef LAZY_WDS
					if (pstat->state & WIFI_WDS_LAZY) {
						asoc_list_unref(priv, pstat);
						delete_wds_entry(priv, pstat);
						return;
					}
#endif
				}

				pstat->expire_to--;

				if (pstat->expire_to == 0) {
#if defined(BR_SHORTCUT) && defined(RTL_CACHED_BR_STA)
					release_brsc_cache(pstat->hwaddr);
#endif

#ifdef P2P_SUPPORT
/*cfg p2p cfg p2p*/
					if ((rtk_p2p_is_enabled(priv)) && (rtk_p2p_chk_role(priv,P2P_TMP_GO))) {
						if (pstat->is_p2p_client) {
							P2P_DEBUG("p2p client leaved excced %d seconds\n", P2P_CLIENT_ASSOC_EXPIRE);
                            if(rtk_p2p_is_enabled(priv)==PROPERTY_P2P)
    							p2p_client_remove(priv, pstat);
						}
					}
#endif
#ifdef BEAMFORMING_SUPPORT
					if ((priv->pmib->dot11RFEntry.txbf == 1) && (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8814A) )
					{
						ODM_RT_TRACE(ODMPTR, BEAMFORMING_DEBUG, ODM_DBG_LOUD, ("%s,\n", __FUNCTION__));
						if(Beamforming_DeInitEntry(priv, pstat->hwaddr))
							Beamforming_Notify(priv);
					}
#endif

#ifdef MULTI_MAC_CLONE
					{
						struct list_head *ptmp = plist->next;
						unsigned char ucChk = 0;
						if (plist == plist->next){
						    ucChk = 1;
						}
						if (OPMODE & WIFI_STATION_STATE) {
							__del_mclone_addr(priv, pstat->sa_addr);
						}else{
							issue_deauth(priv, pstat->hwaddr, _RSON_DEAUTH_STA_LEAVING_);
							free_stainfo(priv, pstat);
						}

						if (ucChk == 1){
							break;  
						}else{
							plist = ptmp;
							continue;
						}
					}
#endif
				}
			}
			else {
				free_sta_tx_skb(priv, pstat);
			}
		}
		else
		{
			pstat->idle_count = 0;
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
		if(!IS_OUTSRC_CHIP(priv))
#endif
		{
			/*
			 * pass rssi info to f/w
			 */
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
			if(CHIP_VER_92X_SERIES(priv))  {
				if (pstat->sta_in_firmware == 1)
					add_update_rssi(priv, pstat);
			}
#endif
		}
#endif

			if (priv->pshare->rf_ft_var.rssi_dump && !(priv->up_time % priv->pshare->rf_ft_var.rssi_dump)) {
					unsigned char mimorssi[4];
#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
					if(IS_OUTSRC_CHIP(priv))
#endif
					{
						mimorssi[0] = pstat->rf_info.mimorssi[0];
						mimorssi[1] = pstat->rf_info.mimorssi[1];
						mimorssi[2] = pstat->rf_info.mimorssi[2];
						mimorssi[3] = pstat->rf_info.mimorssi[3];						
					}
#endif
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
					if(!IS_OUTSRC_CHIP(priv))
#endif
					{
						mimorssi[0] = pstat->rf_info.mimorssi[0];
						mimorssi[1] = pstat->rf_info.mimorssi[1];	
						mimorssi[2] = pstat->rf_info.mimorssi[2];
						mimorssi[3] = pstat->rf_info.mimorssi[3];
					}
#endif
					
			{
				unsigned int FA_total_cnt=0;
				unsigned int CCA_total_cnt=0;

#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
				if(IS_OUTSRC_CHIP(priv))
#endif
				{
					FA_total_cnt = ODMPTR->FalseAlmCnt.Cnt_all;
					CCA_total_cnt = ODMPTR->FalseAlmCnt.Cnt_CCA_all;
				}
#endif

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
				if(!IS_OUTSRC_CHIP(priv))
#endif
				{
					FA_total_cnt = priv->pshare->FA_total_cnt;
					CCA_total_cnt = priv->pshare->CCA_total_cnt;
				}
#endif		
				
#if 1 //txforce	
				if(priv->pshare->rf_ft_var.txforce != 0xff)
				{
					unsigned char txforce = priv->pshare->rf_ft_var.txforce;
					unsigned char dot11_rate_table[]={2,4,11,22,12,18,24,36,48,72,96,108,0};
					unsigned char args[200];
					sprintf(args, "[%d] %d%%  txforce %s%s%s%s%s%d%s rx %s%s%s%s%s%d%s ",
						pstat->aid, pstat->rssi,
					((txforce >= 44)) ? "VHT " : "",
					((txforce >= 12) && (txforce < 44))? "MCS" : "",
					((txforce >= 44) && (txforce < 54)) ? "NSS1 " : "",
					((txforce >= 54) && (txforce < 64)) ? "NSS2 " : "",
					((txforce >= 64) && (txforce < 74)) ? "NSS3 " : "",
					(txforce >= 44) ? ((txforce-44)%10):((txforce >= 12)? txforce-12: dot11_rate_table[txforce]/2),					
					(pstat->ht_current_tx_info&BIT(1))? "s" : " ",
					((pstat->rx_rate) >= VHT_RATE_ID)? "VHT " : "",
					((pstat->rx_rate >=_NSS1_MCS0_RATE_) && (pstat->rx_rate < _NSS2_MCS0_RATE_)) ? "NSS1 " : "",
					((pstat->rx_rate >=_NSS2_MCS0_RATE_) && (pstat->rx_rate < _NSS3_MCS0_RATE_)) ? "NSS2 " : "",					
					((pstat->rx_rate >= _NSS3_MCS0_RATE_) && (pstat->rx_rate < _NSS4_MCS0_RATE_)) ? "NSS3 " : "",
					((pstat->rx_rate >= HT_RATE_ID) && (pstat->rx_rate < VHT_RATE_ID))? "MCS" : "",
					((pstat->rx_rate) >= VHT_RATE_ID) ? ((pstat->rx_rate - VHT_RATE_ID)%10) : ((pstat->rx_rate >= HT_RATE_ID)? (pstat->rx_rate - HT_RATE_ID) : pstat->rx_rate/2),
					pstat->rx_splcp? "s" : " ");
#ifdef CONFIG_WLAN_HAL_8814AE
					sprintf(args, "%s (ss %d %d %d %d)", args,					
								mimorssi[0], 
								mimorssi[1],
								mimorssi[2], 
								mimorssi[3]);	
#else
					sprintf(args, "%s (ss %d %d)", args,
#ifdef CONFIG_RTL_88E_SUPPORT
					(GET_CHIP_VER(priv)==VERSION_8188E)?0:
#endif
					mimorssi[0], 
#ifdef CONFIG_RTL_88E_SUPPORT
					(GET_CHIP_VER(priv)==VERSION_8188E)?0:
#endif
					mimorssi[1]);	
#endif
					sprintf(args, "%s (FA %d)(CCA %d)(DIG 0x%x)(TP %d,%d)", args,
						FA_total_cnt, CCA_total_cnt,			
	   					RTL_R8(0xc50),
						(unsigned int)(pstat->tx_avarage>>17),
						(unsigned int)(pstat->rx_avarage>>17));
#ifdef RX_CRC_EXPTIMER		
					if(priv->pshare->rf_ft_var.crc_enable)
						sprintf(args, "%s (CRC %ld %ld)", args,
							priv->ext_stats.rx_packets_exptimer,
                    		priv->ext_stats.rx_crc_exptimer);
#endif
					panic_printk("%s", args);
				}
				else
#endif
				{
					
					unsigned char args[200];
					sprintf(args, "[%d] %d%%  tx %s%s%s%s%s%d%s rx %s%s%s%s%s%d%s ",
					pstat->aid, pstat->rssi,
					((pstat->current_tx_rate >= VHT_RATE_ID)) ? "VHT " : "",
					((pstat->current_tx_rate >= HT_RATE_ID) && (pstat->current_tx_rate < VHT_RATE_ID))? "MCS" : "",
					((pstat->current_tx_rate >=_NSS1_MCS0_RATE_) && (pstat->current_tx_rate < _NSS2_MCS0_RATE_)) ? "NSS1 " : "",
					((pstat->current_tx_rate >=_NSS2_MCS0_RATE_) && (pstat->current_tx_rate < _NSS3_MCS0_RATE_)) ? "NSS2 " : "",
					((pstat->current_tx_rate >=_NSS3_MCS0_RATE_) && (pstat->current_tx_rate < _NSS4_MCS0_RATE_)) ? "NSS3 " : "",
					(pstat->current_tx_rate >= VHT_RATE_ID) ? ((pstat->current_tx_rate - VHT_RATE_ID)%10):((pstat->current_tx_rate >= HT_RATE_ID)? (pstat->current_tx_rate - HT_RATE_ID) : pstat->current_tx_rate/2),					
					(pstat->ht_current_tx_info&BIT(1))? "s" : " ",
					((pstat->rx_rate) >= VHT_RATE_ID)? "VHT " : "",
					((pstat->rx_rate >=_NSS1_MCS0_RATE_) && (pstat->rx_rate < _NSS2_MCS0_RATE_)) ? "NSS1 " : "",
					((pstat->rx_rate >=_NSS2_MCS0_RATE_) && (pstat->rx_rate < _NSS3_MCS0_RATE_)) ? "NSS2 " : "",					
					((pstat->rx_rate >=_NSS3_MCS0_RATE_) && (pstat->rx_rate < _NSS4_MCS0_RATE_)) ? "NSS3 " : "",
					((pstat->rx_rate >= HT_RATE_ID) && (pstat->rx_rate < VHT_RATE_ID))? "MCS" : "",
					((pstat->rx_rate) >= VHT_RATE_ID) ? ((pstat->rx_rate - VHT_RATE_ID)%10) : ((pstat->rx_rate >= HT_RATE_ID)? (pstat->rx_rate-HT_RATE_ID) : pstat->rx_rate/2),
					pstat->rx_splcp? "s" : " ");
#ifdef CONFIG_WLAN_HAL_8814AE
					sprintf(args, "%s (ss %d %d %d %d)", args,					
								mimorssi[0], 
								mimorssi[1],
								mimorssi[2], 
								mimorssi[3]);	
#else
					sprintf(args, "%s (ss %d %d)", args,
#ifdef CONFIG_RTL_88E_SUPPORT
					(GET_CHIP_VER(priv)==VERSION_8188E)?0:
#endif
					mimorssi[0], 
#ifdef CONFIG_RTL_88E_SUPPORT
					(GET_CHIP_VER(priv)==VERSION_8188E)?0:
#endif
					mimorssi[1]);	
#endif
					sprintf(args, "%s (FA %d)(CCA %d)(DIG 0x%x)(TP %d,%d)", args,
						FA_total_cnt, CCA_total_cnt,			
	   					RTL_R8(0xc50),
						(unsigned int)(pstat->tx_avarage>>17),
						(unsigned int)(pstat->rx_avarage>>17));
#ifdef RX_CRC_EXPTIMER		
					if(priv->pshare->rf_ft_var.crc_enable)
						sprintf(args, "%s (CRC %ld %ld)", args,
							priv->ext_stats.rx_packets_exptimer,
                    		priv->ext_stats.rx_crc_exptimer);
#endif
					panic_printk("%s", args);
				}
			}
#ifdef CONFIG_RTL8672
#ifdef CONFIG_RTL_88E_SUPPORT
				if (GET_CHIP_VER(priv) == VERSION_8188E)
					panic_printk("(FA ");
				else
#endif
					panic_printk("(FA %x,%x ", RTL_R8(0xc50), RTL_R8(0xc58));

				panic_printk("%d, %d)",
#ifdef INTERFERENCE_CONTROL
					priv->pshare->ofdm_FA_total_cnt,
#else
					priv->pshare->ofdm_FA_cnt1+priv->pshare->ofdm_FA_cnt2+priv->pshare->ofdm_FA_cnt3+priv->pshare->ofdm_FA_cnt4,
#endif
					priv->pshare->cck_FA_cnt);
#endif
				panic_printk("\n");

#ifdef RX_CRC_EXPTIMER
                priv->ext_stats.rx_crc_exptimer = 0;
                priv->ext_stats.rx_packets_exptimer = 0;
#endif
			}

#ifdef RX_CRC_EXPTIMER	
			if (priv->pshare->rf_ft_var.crc_dump  && !(priv->up_time % priv->pshare->rf_ft_var.crc_dump)) {
				panic_printk("CRC ");
				for(i=_NSS1_MCS0_RATE_;i<=_NSS1_MCS9_RATE_;i++) {
					if(priv->ext_stats.rx_packets_by_rate[i])
						panic_printk("NSS1MCS%d(%ld/%ld) ", i-_NSS1_MCS0_RATE_,priv->ext_stats.rx_packets_by_rate[i], priv->ext_stats.rx_crc_by_rate[i]);
				}
				for(i=_NSS2_MCS0_RATE_;i<=_NSS2_MCS9_RATE_;i++) {
					if(priv->ext_stats.rx_packets_by_rate[i])
						panic_printk("NSS2MCS%d(%ld/%ld) ", i-_NSS2_MCS0_RATE_,priv->ext_stats.rx_packets_by_rate[i], priv->ext_stats.rx_crc_by_rate[i]);
				}
				for(i=_NSS3_MCS0_RATE_;i<=_NSS3_MCS9_RATE_;i++) {
					if(priv->ext_stats.rx_packets_by_rate[i])
						panic_printk("NSS3MCS%d(%ld/%ld) ", i-_NSS3_MCS0_RATE_,priv->ext_stats.rx_packets_by_rate[i], priv->ext_stats.rx_crc_by_rate[i]);
				}
				for(i=_MCS0_RATE_;i<=_MCS23_RATE_;i++) {
					if(priv->ext_stats.rx_packets_by_rate[i])
						panic_printk("MCS%d(%ld/%ld) ", i-_MCS0_RATE_,priv->ext_stats.rx_packets_by_rate[i], priv->ext_stats.rx_crc_by_rate[i]);
				}
				panic_printk("\n");
				memset(priv->ext_stats.rx_crc_by_rate, 0, 256*sizeof(unsigned long));
				memset(priv->ext_stats.rx_packets_by_rate, 0, 256*sizeof(unsigned long));
			}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
			if ((GET_CHIP_VER(priv) == VERSION_8188E) && priv->pmib->dot11StationConfigEntry.autoRate)
#ifdef RATEADAPTIVE_BY_ODM
				ODMPTR->RAInfo[pstat->aid].RssiStaRA = pstat->rssi;
#else
				priv->pshare->RaInfo[pstat->aid].RssiStaRA = pstat->rssi;
#endif
#endif

			// calculate STA number
			if ((pstat->expire_to == 0)
#ifdef WDS
				&& !(pstat->state & WIFI_WDS)
#endif
#ifdef A4_STA
                && !(pstat->state & WIFI_A4_STA)
#endif

			) {
				cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);
				check_sta_characteristic(priv, pstat, INCREASE);

				// CAM entry update
				if (!SWCRYPTO) {
					if (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm ||
							pstat->tmp_rmv_key == TRUE) {
						unsigned int privacy = pstat->dot11KeyMapping.dot11Privacy;

						if (CamAddOneEntry(priv, pstat->hwaddr, 0, privacy<<2, 0,
								pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKey.skey)) {
							pstat->dot11KeyMapping.keyInCam = TRUE;
							pstat->tmp_rmv_key = FALSE;
							priv->pshare->CamEntryOccupied++;
							assign_aggre_mthod(priv, pstat);
						}
						else {
							if (pstat->aggre_mthd != AGGRE_MTHD_NONE)
								pstat->aggre_mthd = AGGRE_MTHD_NONE;
						}
					}
				}

				// Resume Ratid
#if defined(CONFIG_PCI_HCI)
#ifdef CONFIG_RTL_88E_SUPPORT
    			if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
    				add_RATid(priv, pstat);
#endif
    			} else
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
				if (GET_CHIP_VER(priv) == VERSION_8812E) 
					UpdateHalRAMask8812(priv, pstat, 3);
				else
#endif				
#ifdef CONFIG_WLAN_HAL
				if (IS_HAL_CHIP(priv))
					GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
				else
#endif
				{
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)			
					if(CHIP_VER_92X_SERIES(priv))
						add_update_RATid(priv, pstat);
#endif
				}
    		
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
    				update_STA_RATid(priv, pstat);
#endif

				//pstat->dwngrade_probation_idx = pstat->upgrade_probation_idx = 0;	// unused
				LOG_MSG("A expired STA is resumed - %02X:%02X:%02X:%02X:%02X:%02X\n",
					pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
			}

#ifdef 	P2P_SUPPORT
/*cfg p2p cfg p2p*/
			if( (rtk_p2p_is_enabled(priv)) && (rtk_p2p_chk_role(priv,P2P_TMP_GO)) ){
				if(pstat->is_p2p_client)
					pstat->expire_to = P2P_CLIENT_ASSOC_EXPIRE;

			}else
#endif
			{
				pstat->expire_to = priv->expire_to;
			}

		}

#if defined(RTK_AC_SUPPORT)
			// operating mode notification
			if(priv->pshare->rf_ft_var.opmtest&2) {
				issue_op_mode_notify(priv, pstat, priv->pshare->rf_ft_var.oper_mode_field);
				priv->pshare->rf_ft_var.opmtest &= 1;
			}
#endif

#ifdef WDS
		if (pstat->state & WIFI_WDS) {
			if ((pstat->rx_pkts != pstat->rx_pkts_pre) || pstat->beacon_num)
				pstat->idle_time = 0;
			else
				pstat->idle_time++;

			if ((priv->up_time%2) == 0) {
				if ((pstat->beacon_num == 0) && (pstat->state & WIFI_WDS_RX_BEACON))
					pstat->state &= ~WIFI_WDS_RX_BEACON;
				if (pstat->beacon_num)
					pstat->beacon_num = 0;
			}
		}
#endif
		// update proc bssdesc
		if ((OPMODE & WIFI_STATION_STATE) && !memcmp(priv->pmib->dot11Bss.bssid, pstat->hwaddr, MACADDRLEN)) {
			priv->pmib->dot11Bss.rssi = pstat->rssi; 
			priv->pmib->dot11Bss.sq = pstat->sq; 
		}

#ifdef BEAMFORMING_SUPPORT
			if ((priv->pmib->dot11RFEntry.txbfer == 1) && ((priv->up_time % 3) == 0) &&
				(GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8814A) )	{
				pstat->bf_score = 0;
				if((pstat->ht_cap_len && (cpu_to_le32(pstat->ht_cap_buf.txbf_cap)&_HTCAP_RECEIVED_NDP))       // 
#ifdef RTK_AC_SUPPORT	
				|| (pstat->vht_cap_len && (cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & (BIT(SU_BFEE_S)|BIT(SU_BFER_S))))
#endif	
				) {
					if( ( pstat->rssi > 10) && 
						( pstat->rx_pkts != pstat->rx_pkts_pre) &&
#ifdef DETECT_STA_EXISTANCE
						(!(pstat->leave)) 
#endif
					){
						u1Byte					Idx = 0;
						PRT_BEAMFORMING_ENTRY	pEntry, pBfeeEntry; 							
						pEntry = Beamforming_GetEntryByMacId(priv, pstat->aid, &Idx);
						if(pEntry == NULL) {
							pBfeeEntry = Beamforming_GetFreeBFeeEntry(priv, &Idx);
							if(pBfeeEntry)
								Beamforming_Enter(priv, pstat);
						}
					}
					pstat->bf_score = 100 - pstat->rssi;
					if(pstat->tx_byte_cnt> (1<<16)) {			// 0.5M bps
						pstat->bf_score += 100;
					}
					if((priv->pmib->dot11nConfigEntry.dot11nLDPC) && (pstat) && 
							((pstat->ht_cap_len && cpu_to_le16(pstat->ht_cap_buf.ht_cap_info) & _HTCAP_SUPPORT_RX_LDPC_)
#ifdef RTK_AC_SUPPORT		
							||	(pstat->vht_cap_len && (cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & BIT(RX_LDPC_E)))
#endif		
					))				
						pstat->bf_score -=2;
					if(priv->pmib->dot11nConfigEntry.dot11nSTBC &&  
								((pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_RX_STBC_CAP_)) 
#ifdef RTK_AC_SUPPORT			
									|| (pstat->vht_cap_buf.vht_cap_info & cpu_to_le32(_VHTCAP_RX_STBC_CAP_))
#endif
					))	{			
						pstat->bf_score -=5;			
						if(!(pstat->MIMO_ps & _HT_MIMO_PS_STATIC_)) {
#ifdef RTK_AC_SUPPORT			
							if (pstat->vht_cap_len) {
								if(((le32_to_cpu(pstat->vht_cap_buf.vht_support_mcs[0]) >> 2) & 3) != 3)
									pstat->bf_score -=3;
							} else
#endif
							{
								pstat->tx_ra_bitmap = 0;
								if (pstat->ht_cap_len) {
									for (i=0; i<16; i++) {
										if (pstat->ht_cap_buf.support_mcs[i/8] & BIT(i%8))
											pstat->tx_ra_bitmap |= BIT(i+12);
									}
									if(pstat->tx_ra_bitmap & 0x0ff00000)
										pstat->bf_score -=3;
								}
							}
						}
					}
				}
			}
#endif

		pstat->tx_pkts_pre = pstat->tx_pkts;
		pstat->rx_pkts_pre = pstat->rx_pkts;
		pstat->tx_fail_pre = pstat->tx_fail;

		if ((priv->up_time % 3) == 0) {
#ifndef DRVMAC_LB
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
			if (is_auto_rate(priv, pstat)
				|| (should_restrict_Nrate(priv, pstat) && is_fixedMCSTxRate(priv, pstat)))
				check_RA_by_rssi(priv, pstat);
#endif
#endif

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
			if(CHIP_VER_92X_SERIES(priv))
				check_txrate_by_reg(priv, pstat);
#endif

			/*Now 8812 use txreport for get txreport and inital tx rate*/ 
			#if	0	//defined(CONFIG_RTL_8812_SUPPORT)	
			if(GET_CHIP_VER(priv)==VERSION_8812E){
				check_txrate_by_reg_8812(priv, pstat);
			}
			#endif

			/*
			 *	Check if station is 2T
			 */
		 	if (!pstat->is_2t_mimo_sta && (pstat->highest_rx_rate >= _MCS8_RATE_))
				pstat->is_2t_mimo_sta = TRUE;
#ifdef RTK_AC_SUPPORT
			// Dynamic Enable/Disable LDPC
			if((pstat->IOTPeer == HT_IOT_PEER_REALTEK_8812) && 
				(pstat->WirelessMode & (WIRELESS_MODE_AC_5G|WIRELESS_MODE_AC_24G)) &&
				( (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_ | _HTCAP_SHORTGI_40M_))
				|| ( cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & BIT(SHORT_GI80M_E))	
			)){
				const char thd = 45;
				if (!pstat->disable_ldpc && pstat->rssi> thd+5)
					pstat->disable_ldpc=1;
				else if(pstat->disable_ldpc && pstat->rssi <thd) 
					pstat->disable_ldpc=0;
			}			
#endif
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
if(!IS_OUTSRC_CHIP(priv))
#endif
{

			/*
			 *	Check if station is near by to use lower tx power
			 */
			if((priv->pshare->FA_total_cnt > 1000) || (priv->pshare->FA_total_cnt > 300 && (RTL_R8(0xc50) & 0x7f) >= 0x32)) {
				pstat->hp_level = 0;
			} else {
				if (priv->pshare->rf_ft_var.tx_pwr_ctrl) {
					if ((pstat->hp_level == 0) && (pstat->rssi > HP_LOWER+4)){
						pstat->hp_level = 1;
					}else if ((pstat->hp_level == 1) && (pstat->rssi < (HP_LOWER - 4) )){
						pstat->hp_level = 0;
                    }
				}				
			}
}
#endif			
		}
		{
			const char thd = 30;
			if (!pstat->no_rts && pstat->rssi>thd+5)
				pstat->no_rts=1;
			else if(pstat->no_rts && pstat->rssi<thd) 
				pstat->no_rts=0;
		}
		
		if (pstat->IOTPeer == HT_IOT_PEER_INTEL)
		{
#if 0//def CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8192D) {
				if ((((priv->ext_stats.tx_avarage>>17) + (priv->ext_stats.rx_avarage>>17)) > 80) && (pstat->no_rts == 1))
					pstat->no_rts = 0;
				else if ((((priv->ext_stats.tx_avarage>>17) + (priv->ext_stats.rx_avarage>>17)) < 60) && (pstat->no_rts == 0))
					pstat->no_rts = 1;
			}
#endif
#if 0//def CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E) {
				const char thd = 20;
				if (!pstat->no_rts && pstat->rssi<thd)
					pstat->no_rts=1;
				else if(pstat->no_rts && pstat->rssi>thd+5) 
					pstat->no_rts=0;
			}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8812E) {
				const char thd = 40;
				if (!pstat->no_rts && (pstat->rssi < thd))
				{
					pstat->no_rts=1;
					//printk("Disable rts !!\n");
				}
				else if(pstat->no_rts && (pstat->rssi > (thd+5))) 
				{
					pstat->no_rts=0;
					//printk("Enable rts !!\n");
				}
			}
#endif

			/* Count every Intel clients with complying throughput margin */
			if ((pstat->tx_byte_cnt + pstat->rx_byte_cnt) >= priv->pshare->rf_ft_var.intel_rtylmt_tp_margin)
				priv->pshare->intel_active_sta++;
		}
#if 0
//#ifdef AC2G_256QAM
	{
		if ( (is_ac2g(priv)) && (GET_CHIP_VER(priv)==VERSION_8812E) )
		{
			if (pstat->rssi < priv->pshare->rf_ft_var.ac2g_thd_ldpc)
			{
				priv->pmib->dot11nConfigEntry.dot11nLDPC = 1;
			}
			else if(pstat->rssi > (priv->pshare->rf_ft_var.ac2g_thd_ldpc+5))
			{
				priv->pmib->dot11nConfigEntry.dot11nLDPC = 0;
			}
		}
	}
#endif

#ifdef USE_OUT_SRC	
#ifdef _OUTSRC_COEXIST
		if(IS_OUTSRC_CHIP(priv))
#endif
		ChooseIotMainSTA(priv, pstat);
#endif

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
		if(!IS_OUTSRC_CHIP(priv))
#endif
		choose_IOT_main_sta(priv, pstat);
#endif

#ifdef SW_TX_QUEUE
        {
            SMP_LOCK_XMIT(flags);  
            for (i=BK_QUEUE;i<=VO_QUEUE;i++) 
            {				
                int q_aggnumIncSlow = (priv->assoc_num > 1) ? (2+pstat->swq.q_aggnumIncSlow[i]) : (1+pstat->swq.q_aggnumIncSlow[i]);
                //int q_aggnumIncSlow =  (1+pstat->swq.q_aggnumIncSlow[i]);				
#if 1//def SW_TX_QUEUE_SMALL_PACKET_CHECK
                if ((priv->pshare->swq_en == 0) /*|| (((priv->ext_stats.tx_avarage>>17) + (priv->ext_stats.rx_avarage>>17)) < 20)*/) { // disable check for small udp packet(88B) test with veriwave tool
#else
                if ((priv->pshare->swq_en == 0) || (((priv->ext_stats.tx_avarage>>17) + (priv->ext_stats.rx_avarage>>17)) < 20)) {
#endif
                    //pstat->swq.q_aggnum[i] = priv->pshare->rf_ft_var.swq_aggnum;	

                    pstat->swq.q_aggnumIncSlow[i] = 0;
                }
                else if (((priv->up_time % q_aggnumIncSlow) == 0) && ((priv->pshare->swqen_keeptime != 0) && (priv->up_time > priv->pshare->swqen_keeptime+3)) && (pstat->tx_avarage > 25000) && (pstat->ht_cap_len)) {
                    adjust_swq_setting(priv, pstat, i, CHECK_INC_AGGN);
                }
                /*clear used*/
                pstat->swq.q_used[i] = 0;
                pstat->swq.q_TOCount[i] = 0;
            }
            SMP_UNLOCK_XMIT(flags);    
        }
#endif
		// calculate tx/rx throughput
		pstat->tx_avarage = (pstat->tx_avarage/10)*7 + (pstat->tx_byte_cnt/10)*3;
		pstat->tx_byte_cnt_LowMAW = (pstat->tx_byte_cnt_LowMAW/10)*1 + (pstat->tx_byte_cnt/10)*9;
		pstat->tx_byte_cnt = 0;
		pstat->rx_avarage = (pstat->rx_avarage/10)*7 + (pstat->rx_byte_cnt/10)*3;
		pstat->rx_byte_cnt_LowMAW = (pstat->rx_byte_cnt_LowMAW/10)*1 + (pstat->rx_byte_cnt/10)*9;
		pstat->rx_byte_cnt = 0;

#ifdef PREVENT_BROADCAST_STORM
		// reset rx_pkts_bc in every one second
		pstat->rx_pkts_bc = 0;
#endif
		if ((pstat->tx_avarage + pstat->rx_avarage) > highest_tp) {
			highest_tp = pstat->tx_avarage + pstat->rx_avarage;
			pstat_highest = pstat;
		}
#if defined(HW_ANT_SWITCH)&&( defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
		if(HW_DIV_ENABLE) 
			dm_STA_Ant_Select(priv, pstat);
#endif

#if 0//def SUPPORT_TX_AMSDU
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv) == VERSION_8814A) {			
			if((pstat->aggre_mthd != AGGRE_MTHD_MPDU_AMSDU) && ((pstat->tx_avarage >> 17) > priv->pshare->rf_ft_var.amsdu_th)) {
				//printk("TP:%d Mbps --> amsdu mode\n", (pstat->tx_avarage >> 17));
				pstat->aggre_mthd = AGGRE_MTHD_MPDU_AMSDU;
			} else if((pstat->aggre_mthd == AGGRE_MTHD_MPDU_AMSDU) && ((pstat->tx_avarage >> 17) < priv->pshare->rf_ft_var.amsdu_th2)) {
				//printk("TP:%d Mbps --> ampdu mode\n", (pstat->tx_avarage >> 17));
				pstat->aggre_mthd = AGGRE_MTHD_MPDU;
			}		
		}
#endif	
#else
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv) == VERSION_8814A) 
			assign_aggre_mthod(priv,pstat);
#endif		
#endif
		/*
	         * Broadcom, Intel IOT, dynamic inc or dec retry count
        	 */


			if ((pstat->IOTPeer == HT_IOT_PEER_BROADCOM) || (pstat->IOTPeer == HT_IOT_PEER_INTEL))
        	{
                	int i;
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
                	int j;
#endif
					if (((pstat->tx_avarage + pstat->rx_avarage >= RETRY_TRSHLD_H) 
#ifdef CLIENT_MODE
					|| ((OPMODE & WIFI_STATION_STATE) && (pstat->IOTPeer == HT_IOT_PEER_BROADCOM) &&  (pstat->rssi >= 45))
#endif
						) && (pstat->retry_inc == 0))
        	        {
#ifdef TX_SHORTCUT
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
				for (i = 0; i < NR_NET80211_UP; ++i) {
	                	        for (j = 0; j < TX_SC_ENTRY_NUM; ++j)
	                        	        pstat->tx_sc_ent[i][j].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
				}
#else
                	        for (i=0; i<TX_SC_ENTRY_NUM; i++)
                	        {
#ifdef CONFIG_WLAN_HAL
                                if (IS_HAL_CHIP(priv)) {
                                    GET_HAL_INTERFACE(priv)->SetShortCutTxBuffSizeHandler(priv, pstat->tx_sc_ent[i].hal_hw_desc, 0);
                                } else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif // CONFIG_WLAN_HAL
                                {//not HAL
                        	        pstat->tx_sc_ent[i].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
                                }
                	        }
#endif
#endif
	                        pstat->retry_inc = 1;
        	        }
					else if (((pstat->tx_avarage + pstat->rx_avarage < RETRY_TRSHLD_L) 
#ifdef CLIENT_MODE
						|| ((OPMODE & WIFI_STATION_STATE) && (pstat->IOTPeer == HT_IOT_PEER_BROADCOM) &&  (pstat->rssi <= 30))
#endif
						)&& (pstat->retry_inc == 1))
	                {
#ifdef TX_SHORTCUT
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
				for (i = 0; i < NR_NET80211_UP; ++i) {
	                	        for (j = 0; j< TX_SC_ENTRY_NUM; ++j)
	                        	        pstat->tx_sc_ent[i][j].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
				}
#else
        	                for (i=0; i<TX_SC_ENTRY_NUM; i++)
        	                {
#ifdef CONFIG_WLAN_HAL
                                if (IS_HAL_CHIP(priv)) {
                                    GET_HAL_INTERFACE(priv)->SetShortCutTxBuffSizeHandler(priv, pstat->tx_sc_ent[i].hal_hw_desc, 0);
                                } else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif // CONFIG_WLAN_HAL
                                {//not HAL
                	                pstat->tx_sc_ent[i].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
                                }
        	                }
#endif
#endif
	                        pstat->retry_inc = 0;
        	        }
        	}

		if (/*((GET_ROOT(priv)->up_time % 3) == 1) && */(pstat->rssi < priv->pshare->rssi_min) &&
			(pstat->expire_to > (priv->expire_to - priv->pshare->rf_ft_var.rssi_expire_to))
#ifdef DETECT_STA_EXISTANCE
			&& (!pstat->leave) 
#endif
			)
			priv->pshare->rssi_min = pstat->rssi;

#ifdef MCR_WIRELESS_EXTEND
		if (pstat->IOTPeer == HT_IOT_PEER_CMW)
			priv->pshare->rssi_min = pstat->rssi;
#endif
		/*
		 *	Periodically clear ADDBAreq sent indicator
		 */
		if ((pstat->expire_to > 0) && pstat->ht_cap_len && (pstat->aggre_mthd & AGGRE_MTHD_MPDU))
			memset(pstat->ADDBA_sent, 0, 8);

#if !defined(WIFI_HAPD) && !defined(RTK_NL80211) //update unicast key by hostapd !
		// Update unicast key
		if (priv->pmib->dot1180211AuthEntry.dot11UKRekeyTime)
		{
			if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK) {
				if (pstat->uk_timeout)
					pstat->uk_timeout --;
				else {
					if (pstat->wpa_sta_info->state == PSK_STATE_PTKINITDONE)
						UKRekeyTimeout(priv, pstat);
				}
			}
		}
#endif
		
		if (plist == plist->next)
			break;
	}


	/*dynamic tuning response date rate*/
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef MP_TEST
		if (!((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))
#endif
		{		
			if (priv->up_time % 2){
				if(priv->pshare->rssi_min!=0xff){
#ifdef CONFIG_WLAN_HAL_8814AE
					if(GET_CHIP_VER(priv) != VERSION_8814A)
#endif
						dynamic_response_rate(priv, priv->pshare->rssi_min);
				}
			}			
		}		
	}


#if defined(HW_ANT_SWITCH)&&( defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
	if(HW_DIV_ENABLE)  {
		dm_HW_IdleAntennaSelect(priv);
	}
#endif

	/*
	 * Intel IOT, dynamic enhance beacon tx AGC
	 */
	pstat = pstat_highest;

	if (pstat && pstat->IOTPeer == HT_IOT_PEER_INTEL)
	{
		const char thd = 25;
		if (!priv->bcnTxAGC) {
			if (pstat->rssi < thd)
				priv->bcnTxAGC = 2;
			else if (pstat->rssi < thd+5)
				priv->bcnTxAGC = 1;
		} else if (priv->bcnTxAGC == 1) {
			if (pstat->rssi < thd)
				priv->bcnTxAGC = 2;
			else if (pstat->rssi > thd+10)
				priv->bcnTxAGC = 0;
		} else if (priv->bcnTxAGC == 2) {
			if (pstat->rssi > thd+10)
				priv->bcnTxAGC = 0;
			else if (pstat->rssi > thd+5)
				priv->bcnTxAGC = 1;
		}
 	} else {
		if (priv->bcnTxAGC)
		 	priv->bcnTxAGC = 0;
	}
}


#ifdef WDS
static void wds_probe_expire(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	unsigned int i;

	if ((priv->up_time % 30) != 5)
		return;

	for (i = 0; i < priv->pmib->dot11WdsInfo.wdsNum; i++) {
		pstat = get_stainfo(priv, priv->pmib->dot11WdsInfo.entry[i].macAddr);
		if (pstat) {
			if (pstat->wds_probe_done)
				continue;
			issue_probereq(priv, NULL, 0, pstat->hwaddr);
		}
	}
}
#endif


#ifdef CHECK_HANGUP
#ifdef CHECK_TX_HANGUP
static int check_tx_hangup(struct rtl8192cd_priv *priv, int q_num, int *pTail, int *pIsEmpty)
{
	struct tx_desc	*pdescH, *pdesc;
	volatile int	head, tail;
	struct rtl8192cd_hw	*phw;

	phw	= GET_HW(priv);
	head	= get_txhead(phw, q_num);
	tail	= get_txtail(phw, q_num);
	pdescH	= get_txdesc(phw, q_num);

	*pTail = tail;

	if (CIRC_CNT_RTK(head, tail, CURRENT_NUM_TX_DESC))
	{
		*pIsEmpty = 0;
		pdesc = pdescH + (tail);

#ifdef __MIPSEB__
		pdesc = (struct tx_desc *)KSEG1ADDR(pdesc);
#endif
		if (pdesc && ((get_desc(pdesc->Dword0)) & TX_OWN)) // pending
		{	
			//	In adaptivity test, we need avoid check tx hang
		//	if( check_adaptivity_test(priv) )
		//		return 0;			
		//	else
			return 1;
		}
	}
	else
		*pIsEmpty = 1;

	return 0;
}

int check_adaptivity_test(struct rtl8192cd_priv *priv)
{			
	unsigned int 	value32;

#ifdef USE_OUT_SRC
	if (IS_OUTSRC_CHIP(priv)) 
	{
		if (ODMPTR->SupportICType & ODM_IC_11N_SERIES)
		{
			//ODM_SetBBReg(ODMPTR,ODM_REG_DBG_RPT_11N, bMaskDWord, 0x208);
			value32 = ODM_GetBBReg(ODMPTR, ODM_REG_RPT_11N, bMaskDWord);					
		}
		else if (ODMPTR->SupportICType & ODM_IC_11AC_SERIES)
		{
			//ODM_SetBBReg(ODMPTR,ODM_REG_DBG_RPT_11AC, bMaskDWord, 0x209);
			value32 = ODM_GetBBReg(ODMPTR,ODM_REG_RPT_11AC, bMaskDWord);
		}

		if (value32 & BIT30 && (ODMPTR->SupportICType & (ODM_RTL8723A|ODM_RTL8723B|ODM_RTL8188E)))
			return 1;
		else if(value32 & BIT29)
			return 1;
		else
			return 0;
	}	
	else
#endif
	{
		//PHY_SetBBReg(priv,0x908, bMaskDWord, 0x208);
		value32 = PHY_QueryBBReg(priv, 0xdf4, bMaskDWord);

		if(value32 & BIT29)
			return 1;
		else
			return 0;
	}
}
#endif


#ifdef CHECK_RX_HANGUP
static void check_rx_hangup_send_pkt(struct rtl8192cd_priv *priv)
{
	struct stat_info *pstat;
	struct list_head *phead = &priv->asoc_list;
	struct list_head *plist = phead;
	DECLARE_TXINSN(txinsn);

	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);

		if (pstat->expire_to > 0) {
			txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

			txinsn.q_num = MANAGE_QUE_NUM;
#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
        	if( rtk_p2p_is_enabled(priv) ){
              	txinsn.tx_rate = _6M_RATE_;
        	}else
#endif            
			txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE			
			txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif			
			txinsn.fixed_rate = 1;
			txinsn.fr_type = _PRE_ALLOCHDR_;
			txinsn.phdr = get_wlanhdr_from_poll(priv);
			txinsn.pframe = NULL;

			if (txinsn.phdr == NULL)
				goto send_test_pkt_fail;

			memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

			SetFrameSubType(txinsn.phdr, WIFI_DATA_NULL);

			if (OPMODE & WIFI_AP_STATE) {
				memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
				memcpy((void *)GetAddr2Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
				memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
			}
			else {
				memcpy((void *)GetAddr1Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
				memcpy((void *)GetAddr2Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
				memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
			}

			txinsn.hdr_len = WLAN_HDR_A3_LEN;

			if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS) {
				asoc_list_unref(priv, pstat);
				return;
			}

send_test_pkt_fail:

			if (txinsn.phdr)
				release_wlanhdr_to_poll(priv, txinsn.phdr);
		}
	}
}


static void check_rx_hangup(struct rtl8192cd_priv *priv)
{
	if (priv->rx_start_monitor_running == 0) {
		if (UINT32_DIFF(priv->rx_packets_pre1, priv->rx_packets_pre2) > 20) {
			priv->rx_start_monitor_running = 1;
			//printk("start monitoring = %d\n", priv->rx_start_monitor_running);
		}
	}
	else if (priv->rx_start_monitor_running == 1) {
		if (UINT32_DIFF(priv->net_stats.rx_packets, priv->rx_packets_pre1) == 0)
			priv->rx_start_monitor_running = 2;
		else if (UINT32_DIFF(priv->net_stats.rx_packets, priv->rx_packets_pre1) < 2 &&
			UINT32_DIFF(priv->net_stats.rx_packets, priv->rx_packets_pre1) > 0) {
			priv->rx_start_monitor_running = 0;
			//printk("stop monitoring = %d\n", priv->rx_start_monitor_running);
		}
	}

	if (priv->rx_start_monitor_running >= 2) {
		//printk("\n\n%s %d start monitoring = 2 rx_packets_pre1=%lu; rx_packets_pre2=%lu; net_stats.rx_packets=%lu\n",
			//__FUNCTION__, __LINE__,
			//priv->rx_packets_pre1, priv->rx_packets_pre2,
			//priv->net_stats.rx_packets);
		priv->pshare->rx_hang_checking = 1;
		priv->pshare->selected_priv = priv;;
	}
}


static __inline__ void check_rx_hangup_record_rxpkts(struct rtl8192cd_priv *priv)
{
	priv->rx_packets_pre2 = priv->rx_packets_pre1;
	priv->rx_packets_pre1 = priv->net_stats.rx_packets;
}
#endif // CHECK_RX_HANGUP


#ifdef CONFIG_RTL_92D_DMDP
void reset_dmdp_peer(struct rtl8192cd_priv *from)
{
	extern u32 if_priv[];
	struct rtl8192cd_priv *priv;
#ifdef FAST_RECOVERY
	void *info = NULL;
#ifdef UNIVERSAL_REPEATER
	void *vxd_info = NULL;
#endif
#endif

#ifdef MBSSID
	int i;
	void *vap_info[RTL8192CD_NUM_VWLAN];
	memset(vap_info, 0, sizeof(vap_info));
#endif

	if (from->pshare->wlandev_idx == 0)
		priv = (struct rtl8192cd_priv *)if_priv[1];
	else
		priv = (struct rtl8192cd_priv *)if_priv[0];

	if (priv == NULL)
		return;

	if (!netif_running(priv->dev))
		return;

	priv->reset_hangup = 1;
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		GET_VXD_PRIV(priv)->reset_hangup = 1;
#endif
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (IS_DRV_OPEN(priv->pvap_priv[i]))
				priv->pvap_priv[i]->reset_hangup = 1;
		}
	}
#endif

#ifdef WDS
	if (priv->pmib->dot11WdsInfo.wdsEnabled &&
		(priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_ ||
			priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_) ) {
			int i;
			for (i=0; i<priv->pmib->dot11WdsInfo.wdsNum; i++)
				if (netif_running(priv->wds_dev[i]))
	 				priv->pmib->dot11WdsInfo.wdsMappingKeyLen[i]|=0x80000000;
	}
#endif

#ifdef FAST_RECOVERY
	info = backup_sta(priv);
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (IS_DRV_OPEN(priv->pvap_priv[i]))
				vap_info[i] = backup_sta(priv->pvap_priv[i]);
			else
				vap_info[i] = NULL;
		}
	}
#endif
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		vxd_info = backup_sta(GET_VXD_PRIV(priv));
#endif
#endif // FAST_RECOVERY

	priv->pmib->dot11OperationEntry.keep_rsnie = 1;
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (IS_DRV_OPEN(priv->pvap_priv[i]))
				priv->pvap_priv[i]->pmib->dot11OperationEntry.keep_rsnie = 1;
		}
	}
#endif
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.keep_rsnie = 1;
#endif

    P2P_DEBUG("==>rtl8192cd_close + open\n");

	rtl8192cd_close(priv->dev);
	rtl8192cd_open(priv->dev);

#ifdef FAST_RECOVERY
	if (info)
		restore_backup_sta(priv, info);

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (IS_DRV_OPEN(priv->pvap_priv[i]) && vap_info[i])
				restore_backup_sta(priv->pvap_priv[i], vap_info[i]);
		}
	}
#endif
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && vxd_info)
		restore_backup_sta(GET_VXD_PRIV(priv), vxd_info);
#endif
#endif // FAST_RECOVERY

#ifdef CHECK_AFTER_RESET
	priv->pshare->reset_monitor_cnt_down = 3;
	priv->pshare->reset_monitor_pending = 0;
	priv->pshare->reset_monitor_rx_pkt_cnt = priv->net_stats.rx_packets;
#endif
}
#endif


int check_hangup(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long	flags = 0;
#endif

#ifdef CHECK_TX_HANGUP
	int tail, q_num, is_empty, alive=0;
#endif
	int txhangup, rxhangup, beacon_hangup, reset_fail_hangup;
#ifdef CONFIG_WLAN_HAL
    u4Byte hal_hang_state = 0;
#endif // CONFIG_WLAN_HAL

#ifdef MBSSID
	int i;
#endif
#ifdef FAST_RECOVERY
	void *info = NULL;
#ifdef UNIVERSAL_REPEATER
	void *vxd_info = NULL;
#endif
#endif // FAST_RECOVERY
#ifdef CHECK_RX_HANGUP
	unsigned int rx_cntreg;
#endif

#ifdef MBSSID
	void *vap_info[RTL8192CD_NUM_VWLAN];
	memset(vap_info, 0, sizeof(vap_info));
#endif

/*
#if defined(CHECK_BEACON_HANGUP)
	unsigned int BcnQ_Val = 0;
#endif
*/

// for debug
#if 0
	__DRAM_IN_865X static unsigned char temp_reg_C50, temp_reg_C58,
 		temp_reg_C60, temp_reg_C68, temp_reg_A0A;
  	temp_reg_C50 = 0; temp_reg_C58 = 0; temp_reg_C60 = 0;
	temp_reg_C68 = 0; temp_reg_A0A = 0;
#endif
//---------------------------------------------------------
//	margin = -1;

	txhangup = rxhangup = beacon_hangup = reset_fail_hangup = 0;


#if defined(CONFIG_WLAN_HAL) 
    if (IS_HAL_CHIP(priv)) {

#if defined(CHECK_LX_DMA_ERROR)		
		if(priv->pshare->rf_ft_var.check_hang &  CHECK_LX_DMA_ERROR) {
	        hal_hang_state = GET_HAL_INTERFACE(priv)->CheckHangHandler(priv);

	        switch (hal_hang_state) {
	        case HANG_VAR_TX_STUCK:
	            txhangup = 1;
	            break;
	        case HANG_VAR_RX_STUCK:
	            rxhangup = 1;
	            break;
	        default: // HANG_VAR_NORMAL
	            break;
	        }
		}
#endif
#ifdef CHECK_TX_HANGUP
		if(priv->pshare->rf_ft_var.check_hang &  CHECK_TX_HANGUP) {
			if( check_adaptivity_test(priv)) {
				for(q_num = 0; q_num<13; q_num++)
					if(	priv->pshare->Q_info[q_num].pending_tick){
						priv->pshare->Q_info[q_num].pending_tick++;
						//panic_printk("%d q_num = %d,pending_tick %d\n",__LINE__,q_num,priv->pshare->Q_info[q_num].pending_tick);
					}
			} else {
				for(q_num = 0; q_num<13; q_num++) {
					PHCI_TX_DMA_MANAGER_88XX	ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);	
					PHCI_TX_DMA_QUEUE_STRUCT_88XX	tx_q   = &(ptx_dma->tx_queue[q_num]);

					tail = 	RTL_R32(tx_q->reg_rwptr_idx);
					if( (0xffff & ((tail>>16)^ tail))==0) {
						if(priv->pshare->Q_info[q_num].pending_tick)
							alive++;
						priv->pshare->Q_info[q_num].pending_tick = 0;
					} else {
						tail = (tail>>16) & 0xffff;
						if (priv->pshare->Q_info[q_num].pending_tick &&
							(tail == priv->pshare->Q_info[q_num].pending_tail) &&
							(UINT32_DIFF(priv->up_time, priv->pshare->Q_info[q_num].pending_tick) >= PENDING_PERIOD)) {
							txhangup++;
		//					break;
						}
						
						if ((priv->pshare->Q_info[q_num].pending_tick == 0) ||
							(tail != priv->pshare->Q_info[q_num].pending_tail)) {
							if (tail != priv->pshare->Q_info[q_num].pending_tail)
								alive++;
							priv->pshare->Q_info[q_num].pending_tick = priv->up_time;
							priv->pshare->Q_info[q_num].pending_tail = tail;
						}
						priv->pshare->Q_info[q_num].idle_tick = 0;
		//				break;				
					}			
				}
				if(alive) {
					txhangup =0;
					for(q_num = 0; q_num<13; q_num++)
						priv->pshare->Q_info[q_num].pending_tick = 0;
				} 			
			}
		}
#endif

    } else if(CONFIG_WLAN_NOT_HAL_EXIST)	
#endif // CONFIG_WLAN_HAL
	{
#ifdef CHECK_TX_HANGUP
	// we check Q5, Q0, Q4, Q3, Q2, Q1
	if(priv->pshare->rf_ft_var.check_hang &  CHECK_TX_HANGUP) {
		if( check_adaptivity_test(priv)) {
			for(q_num = 0; q_num<=5; q_num++){
				if(	priv->pshare->Q_info[q_num].pending_tick){
					priv->pshare->Q_info[q_num].pending_tick++;
					//panic_printk("%d q_num = %d,pending_tick %d\n",__LINE__,q_num,priv->pshare->Q_info[q_num].pending_tick);
				}
			}
		} else {
			for(q_num = 0; q_num<=5; q_num++) {

				if (check_tx_hangup(priv, q_num, &tail, &is_empty)) {
					if (priv->pshare->Q_info[q_num].pending_tick &&
						(tail == priv->pshare->Q_info[q_num].pending_tail) &&
						(UINT32_DIFF(priv->up_time, priv->pshare->Q_info[q_num].pending_tick) >= PENDING_PERIOD)) {
						// the stopping is over the period => hangup!
						txhangup++;
		//				break;
					}

					if ((priv->pshare->Q_info[q_num].pending_tick == 0) ||
						(tail != priv->pshare->Q_info[q_num].pending_tail)) {
						if(tail != priv->pshare->Q_info[q_num].pending_tail)
							alive++;
						// the first time stopping or the tail moved
						priv->pshare->Q_info[q_num].pending_tick = priv->up_time;
						priv->pshare->Q_info[q_num].pending_tail = tail;
					}
					priv->pshare->Q_info[q_num].idle_tick = 0;
		//			break;
				}
				else {
					if(priv->pshare->Q_info[q_num].pending_tick)
						alive++;
					else if (tail != priv->pshare->Q_info[q_num].pending_tail)
						alive++;
					priv->pshare->Q_info[q_num].pending_tail = tail;
					// empty or own bit is cleared
					priv->pshare->Q_info[q_num].pending_tick = 0;
					if (!is_empty &&
						priv->pshare->Q_info[q_num].idle_tick &&
						(tail == priv->pshare->Q_info[q_num].pending_tail) &&
						(UINT32_DIFF(priv->up_time, priv->pshare->Q_info[q_num].idle_tick) >= PENDING_PERIOD)) {
						// own bit is cleared, but the tail didn't move and is idle over the period => call DSR
#ifdef SMP_SYNC
						if (!priv->pshare->has_triggered_tx_tasklet) {
#ifdef __ECOS
							priv->pshare->call_dsr = 1;
#else
							tasklet_schedule(&priv->pshare->tx_tasklet);
#endif
							priv->pshare->has_triggered_tx_tasklet = 1;
						}
#else
#ifdef __ECOS
						priv->pshare->has_triggered_tx_tasklet = 1;
						priv->pshare->call_dsr = 1;
#else
						rtl8192cd_tx_dsr((unsigned long)priv);
#endif
#endif
						priv->pshare->Q_info[q_num].idle_tick = 0;
		//				break;
					}
					else {
						if (is_empty)
							priv->pshare->Q_info[q_num].idle_tick = 0;
						else {
							if ((priv->pshare->Q_info[q_num].idle_tick == 0) ||
								(tail != priv->pshare->Q_info[q_num].pending_tail)) {
								// the first time idle, or the own bit is cleared and the tail moved
								priv->pshare->Q_info[q_num].idle_tick = priv->up_time;
								priv->pshare->Q_info[q_num].pending_tail = tail;
		//						break;
							}
						}
					}
				}

			}
			if(alive) {
				txhangup =0;
				for(q_num = 0; q_num<=5; q_num++)
					priv->pshare->Q_info[q_num].pending_tick = 0;
			} 	
		}
	}
#endif
	}

#ifdef CHECK_RX_HANGUP
	// check for rx stop
	if (txhangup == 0) {
		if ((priv->assoc_num > 0
#ifdef WDS
			|| priv->pmib->dot11WdsInfo.wdsEnabled
#endif
			) && !priv->pshare->rx_hang_checking)
			check_rx_hangup(priv);

#ifdef UNIVERSAL_REPEATER
		if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) &&
			GET_VXD_PRIV(priv)->assoc_num > 0 &&
			!priv->pshare->rx_hang_checking)
			check_rx_hangup(GET_VXD_PRIV(priv));
#endif
#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]) &&
					priv->pvap_priv[i]->assoc_num > 0 &&
					!priv->pshare->rx_hang_checking)
					check_rx_hangup(priv->pvap_priv[i]);
			}
		}
#endif

		if (priv->pshare->rx_hang_checking)
		{
			if (priv->pshare->rx_cntreg_log == 0)
				priv->pshare->rx_cntreg_log = RTL_R32(_RXPKTNUM_);
			else {
				rx_cntreg = RTL_R32(_RXPKTNUM_);
				if (priv->pshare->rx_cntreg_log == rx_cntreg) {
					if (priv->pshare->rx_stop_pending_tick) {
						if (UINT32_DIFF(priv->pshare->selected_priv->up_time, priv->pshare->rx_stop_pending_tick) >= (PENDING_PERIOD-1)) {
							rxhangup++;
							//printk("\n\n%s %d rxhangup++ rx_packets_pre1=%lu; rx_packets_pre2=%lu; net_stats.rx_packets=%lu\n",
									//__FUNCTION__, __LINE__,
									//priv->pshare->selected_priv->rx_packets_pre1, priv->pshare->selected_priv->rx_packets_pre2,
									//priv->pshare->selected_priv->net_stats.rx_packets);
						}
					}
					else {
						priv->pshare->rx_stop_pending_tick = priv->pshare->selected_priv->up_time;
						RTL_W32(_RCR_, RTL_R32(_RCR_) | _ACF_);
						check_rx_hangup_send_pkt(priv->pshare->selected_priv);
						//printk("\n\ncheck_rx_hangup_send_pkt!\n");
						//printk("%s %d rx_packets_pre1=%lu; rx_packets_pre2=%lu; net_stats.rx_packets=%lu\n",
								//__FUNCTION__, __LINE__,
								//priv->pshare->selected_priv->rx_packets_pre1, priv->pshare->selected_priv->rx_packets_pre2,
								//priv->pshare->selected_priv->net_stats.rx_packets);
					}
				}
				else {
					//printk("\n\n%s %d Recovered!\n" ,__FUNCTION__, __LINE__);
					priv->pshare->rx_hang_checking = 0;
					priv->pshare->rx_cntreg_log = 0;
					priv->pshare->selected_priv = NULL;
					priv->rx_start_monitor_running = 0;
#ifdef UNIVERSAL_REPEATER
					if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
						GET_VXD_PRIV(priv)->rx_start_monitor_running = 0;
#endif
#ifdef MBSSID
					if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
						for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
							if (IS_DRV_OPEN(priv->pvap_priv[i]))
								priv->pvap_priv[i]->rx_start_monitor_running = 0;
						}
					}
#endif

					if (priv->pshare->rx_stop_pending_tick) {
						priv->pshare->rx_stop_pending_tick = 0;
						RTL_W32(_RCR_, RTL_R32(_RCR_) & (~_ACF_));
					}
				}
			}
		}

		if (rxhangup == 0) {
			if (priv->assoc_num > 0)
				check_rx_hangup_record_rxpkts(priv);
			else if (priv->rx_start_monitor_running) {
				priv->rx_start_monitor_running = 0;
				//printk("stop monitoring = %d\n", priv->rx_start_monitor_running);
			}
#ifdef UNIVERSAL_REPEATER
			if (IS_DRV_OPEN(GET_VXD_PRIV(priv))) {
				if (GET_VXD_PRIV(priv)->assoc_num > 0)
					check_rx_hangup_record_rxpkts(GET_VXD_PRIV(priv));
				else if (GET_VXD_PRIV(priv)->rx_start_monitor_running) {
					GET_VXD_PRIV(priv)->rx_start_monitor_running = 0;
					//printk("stop monitoring = %d\n", GET_VXD_PRIV(priv)->rx_start_monitor_running);
				}
			}
#endif
#ifdef MBSSID
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if (IS_DRV_OPEN(priv->pvap_priv[i])) {
						if (priv->pvap_priv[i]->assoc_num > 0)
							check_rx_hangup_record_rxpkts(priv->pvap_priv[i]);
						else if (priv->pvap_priv[i]->rx_start_monitor_running) {
							priv->pvap_priv[i]->rx_start_monitor_running = 0;
							//printk("stop monitoring = %d\n", priv->pvap_priv[i]->rx_start_monitor_running);
						}
					}
				}
			}
#endif
		}
	}
#endif // CHECK_RX_HANGUP

#ifdef CHECK_RX_DMA_ERROR
	if(priv->pshare->rf_ft_var.check_hang &  CHECK_RX_DMA_ERROR)
	if((GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) 
		|| (GET_CHIP_VER(priv)==VERSION_8192D)|| (GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8192E)) 
	{
		int rxffptr, rxffpkt;

		rxffptr = RTL_R16(RXFF_PTR+2);
		rxffpkt = RTL_R8(RXPKT_NUM+3); 

		if((rxffptr%0x80) && rxffpkt && (RTL_R8(RXPKT_NUM+2)^2)){
			if( (priv->pshare->rx_byte_cnt == priv->net_stats.rx_bytes) &&
				!((rxffptr ^ priv->pshare->rxff_rdptr)|(rxffpkt ^ priv->pshare->rxff_pkt))) {
				if (priv->pshare->rx_dma_err_cnt < 15) { // continue for 15 seconds
					priv->pshare->rx_dma_err_cnt++;
				} else {
					unsigned char tmp;
					tmp = RTL_R8(0x1c);

					// Turn OFF BIT_WLOCK_ALL
					RTL_W8(0x1c, tmp & (~(BIT(0) | BIT(1))));
					// Turn ON BIT_SYSON_DIS_PMCREG_WRMSK
					RTL_W8(0xcc, RTL_R8(0xcc) | BIT(2));
					// Turn ON BIT_STOP_RXQ
					RTL_W8(0x301, RTL_R8(0x301) | BIT(0));
					// Reset flow
					RTL_W8(CR, 0);
					RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) & (~BIT(0)));
					RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) | BIT(0));
					// Turn OFF BIT_STOP_RXQ
					RTL_W8(0x301, RTL_R8(0x301) & (~BIT(0)));
					// Turn OFF BIT_SYSON_DIS_PMCREG_WRMSK
					RTL_W8(0xcc, RTL_R8(0xcc) & (~BIT(2)));
					// Restore BIT_WLOCK_ALL
					RTL_W8(0x1c, tmp);

					rxhangup++;
				}
			} else {
				priv->pshare->rx_dma_err_cnt = 0;
				priv->pshare->rxff_rdptr = rxffptr;
				priv->pshare->rxff_pkt = rxffpkt;
				priv->pshare->rx_byte_cnt = priv->net_stats.rx_bytes;
			}
		}
	}
#endif

#ifdef CHECK_RX_TAG_ERROR
#ifdef CONFIG_WLAN_HAL_8192EE
	if(priv->pshare->rf_ft_var.check_hang &  CHECK_RX_TAG_ERROR)
	if (GET_CHIP_VER(priv) == VERSION_8192E) 	{
		if(((priv->pshare->RxTagPollingCount >20)) && (RTL_R32(0)!=0xffffffff) ) {
			rxhangup++;
//			panic_printk("%s, %d, Rx recover!!", __FUNCTION__, __LINE__);
		}
	}
#endif
#endif

#ifdef CHECK_BEACON_HANGUP
	if (((OPMODE & WIFI_AP_STATE)
			&& !(OPMODE &WIFI_SITE_MONITOR)
			&& priv->pBeaconCapability	// beacon has init
#ifdef WDS
			&& !priv->pmib->dot11WdsInfo.wdsPure
#endif
#if defined(CONFIG_RTK_MESH)
			&& !priv->pmib->dot1180211sInfo.meshSilence
#endif
			&& !priv->pmib->miscEntry.func_off
		   )
#ifdef UNIVERSAL_REPEATER
			|| ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
						(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED))
#endif
		) {
		unsigned long beacon_ok;

#ifdef UNIVERSAL_REPEATER
		if (OPMODE & WIFI_STATION_STATE){
			beacon_ok = GET_VXD_PRIV(priv)->ext_stats.beacon_ok;
/*
			BcnQ_Val = GET_VXD_PRIV(priv)->ext_stats.beaconQ_sts;
			GET_VXD_PRIV(priv)->ext_stats.beaconQ_sts = RTL_R32(0x120);
			if(BcnQ_Val == GET_VXD_PRIV(priv)->ext_stats.beaconQ_sts)
				beacon_hangup = 1;
*/
		}
		else
#endif
		{
			beacon_ok = priv->ext_stats.beacon_ok;
/*
			BcnQ_Val = priv->ext_stats.beaconQ_sts;
			priv->ext_stats.beaconQ_sts = RTL_R32(0x120);// firmware beacon Q stats
			if(BcnQ_Val == priv->ext_stats.beaconQ_sts)
				beacon_hangup = 1;
*/
		}

		if (priv->pshare->beacon_wait_cnt == 0) {
			if (priv->pshare->beacon_ok_cnt == beacon_ok) {
				int threshold=1;
#ifdef MBSSID
				if (priv->pmib->miscEntry.vap_enable)
					threshold=3;
#endif
				if (priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod < 650)
					threshold = 0;
				if (priv->pshare->beacon_pending_cnt++ >= threshold)
					beacon_hangup = 1;
			}
			else {
				priv->pshare->beacon_ok_cnt =beacon_ok;
				if (priv->pshare->beacon_pending_cnt > 0)
					priv->pshare->beacon_pending_cnt = 0;
			}
		}
		else
			priv->pshare->beacon_wait_cnt--;
	}
#endif

#ifdef CHECK_AFTER_RESET
	if (priv->pshare->reset_monitor_cnt_down > 0) {
		priv->pshare->reset_monitor_cnt_down--;
		if (priv->pshare->reset_monitor_rx_pkt_cnt == priv->net_stats.rx_packets)	{
//			if (priv->pshare->reset_monitor_pending++ > 1)
				reset_fail_hangup = 1;
		}
		else {
			priv->pshare->reset_monitor_rx_pkt_cnt = priv->net_stats.rx_packets;
			if (priv->pshare->reset_monitor_pending > 0)
				priv->pshare->reset_monitor_pending = 0;
		}
	}
#endif

	if (txhangup || rxhangup || beacon_hangup || reset_fail_hangup) { // hangup happen
		priv->reset_hangup = 1;
#ifdef UNIVERSAL_REPEATER
		if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
			GET_VXD_PRIV(priv)->reset_hangup = 1;
#endif
#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]))
					priv->pvap_priv[i]->reset_hangup = 1;
			}
		}
#endif

#ifdef CHECK_HANG_DEBUG
		if (txhangup) {
			if(priv->pshare->rf_ft_var.swq_dbg==0)
				GDEBUG("uptime=%ld,txhang\n", priv->up_time);
			priv->check_cnt_tx++;
		}
		else if (rxhangup) {
			if(priv->pshare->rf_ft_var.swq_dbg==0)
				GDEBUG("uptime=%ld,rxhang\n", priv->up_time);
			priv->check_cnt_rx++;
		}
		else if (beacon_hangup) {
			if(priv->pshare->rf_ft_var.swq_dbg==0)
				GDEBUG("uptime=%ld,beacon_hangup\n", priv->up_time);
			priv->check_cnt_bcn++;
		}
		else if (reset_fail_hangup) {
			if(priv->pshare->rf_ft_var.swq_dbg==0)
				GDEBUG("uptime=%ld,reset_fail_hangup\n", priv->up_time);
			priv->check_cnt_rst++;
		}

		return 0;
#else
		if (txhangup)
			priv->check_cnt_tx++;
		else if (rxhangup)
			priv->check_cnt_rx++;
		else if (beacon_hangup)
			priv->check_cnt_bcn++;
		else if (reset_fail_hangup)
			priv->check_cnt_rst++;
#endif

// for debug
#if 0
		if (txhangup)
			printk("do Tx reset, up-time=%lu sec\n", priv->up_time);
		else if (rxhangup)
			printk("do Rx reset, up-time=%lu sec\n", priv->up_time);
		else if (beacon_hangup)
			printk("do Beacon reset, up-time=%lu sec\n", priv->up_time);
		else if (reset_fail_hangup)
			printk("do Reset-fail reset, up-time=%lu sec\n", priv->up_time);
#endif

// Set flag to re-init WDS key in rtl8192cd_open()
#ifdef WDS
	if (priv->pmib->dot11WdsInfo.wdsEnabled &&
		(priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_ ||
			priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_) ) {
			int i;
			for (i=0; i<priv->pmib->dot11WdsInfo.wdsNum; i++)
				if (netif_running(priv->wds_dev[i]))
	 				priv->pmib->dot11WdsInfo.wdsMappingKeyLen[i]|=0x80000000;
	}
#endif
//----------------------------- david+2006-06-30

		PRINT_INFO("Status check! Tx[%d] Rx[%d] Bcnt[%d] Rst[%d] ...\n",
			priv->check_cnt_tx, priv->check_cnt_rx, priv->check_cnt_bcn, priv->check_cnt_rst);

		watchdog_stop(priv);

		SAVE_INT_AND_CLI(flags);
        SMP_LOCK(flags);

#ifdef FAST_RECOVERY
		info = backup_sta(priv);
#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]))
					vap_info[i] = backup_sta(priv->pvap_priv[i]);
				else
					vap_info[i] = NULL;
			}
		}
#endif
#ifdef UNIVERSAL_REPEATER
		if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
			vxd_info = backup_sta(GET_VXD_PRIV(priv));
#endif
#endif // FAST_RECOVERY

		priv->pmib->dot11OperationEntry.keep_rsnie = 1;
#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]))
					priv->pvap_priv[i]->pmib->dot11OperationEntry.keep_rsnie = 1;
			}
		}
#endif
#ifdef UNIVERSAL_REPEATER
		if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
			GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.keep_rsnie = 1;
#endif

		SMP_UNLOCK(flags);
		rtl8192cd_close(priv->dev);

#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			RTL_W8(0x1c, RTL_R8(0x1c) & (~(BIT(0) | BIT(1))));
			RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) & (~ BIT(0)));
			RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) | BIT(0));
			RTL_W8(0x1c, RTL_R8(0x1c) | BIT(0) | BIT(1));
		}
#endif

#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
			reset_dmdp_peer(priv);
#endif

		rtl8192cd_open(priv->dev);
		SMP_LOCK(flags);

#ifdef FAST_RECOVERY
		if (info)
			restore_backup_sta(priv, info);

#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]) && vap_info[i])
					restore_backup_sta(priv->pvap_priv[i], vap_info[i]);
			}
		}
#endif
#ifdef UNIVERSAL_REPEATER
		if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && vxd_info)
			restore_backup_sta(GET_VXD_PRIV(priv), vxd_info);
#endif
#endif // FAST_RECOVERY

#ifdef CHECK_AFTER_RESET
		priv->pshare->reset_monitor_cnt_down = 3;
		priv->pshare->reset_monitor_pending = 0;
		priv->pshare->reset_monitor_rx_pkt_cnt = priv->net_stats.rx_packets;
#endif

		RESTORE_INT(flags);
		SMP_UNLOCK(flags);

		watchdog_resume(priv);

		return 1;
	}
	else
		return 0;
}
#endif // CHECK_HANGUP


#ifdef CONFIG_PCI_HCI
// quick fix of tx stuck especial in client mode
void tx_stuck_fix(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_hw *phw = GET_HW(priv);
	unsigned int val32;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv))
		return;
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{
	SAVE_INT_AND_CLI(flags);
//	RTL_W32(0x350, RTL_R32(0x350) | BIT(26));
	val32 = RTL_R32(0x350);
	if (val32 & BIT(24)) {	// tx stuck
			RTL_W8(0x301, RTL_R8(0x301) | BIT(0));
			delay_us(100);
			rtl8192cd_rx_isr(priv);
			RTL_W8(0x302, RTL_R8(0x302) | BIT(4));
			RTL_W8(0x3, RTL_R8(0x3) & (~BIT(0)));
			RTL_W8(0x3, RTL_R8(0x3) | BIT(0));
#ifdef DELAY_REFILL_RX_BUF
			{
				struct sk_buff	*pskb;

				while (phw->cur_rx_refill != phw->cur_rx) {
					pskb = rtl_dev_alloc_skb(priv, RX_BUF_LEN, _SKB_RX_, 1);
					if (pskb == NULL) {
						printk("[%s] can't allocate skbuff for RX!\n", __FUNCTION__);
					}
					init_rxdesc(pskb, phw->cur_rx_refill, priv);

					phw->cur_rx_refill = (phw->cur_rx_refill + 1) % NUM_RX_DESC;
				}
			}
			phw->cur_rx_refill = 0;
#endif
			phw->cur_rx = 0;
			RTL_W32(RX_DESA, (unsigned int)phw->ring_dma_addr);
			RTL_W8(0x301, RTL_R8(0x301) & (~BIT(0)));
			RTL_W8(PCIE_CTRL_REG, MGQ_POLL | BEQ_POLL);
	}
	RESTORE_INT(flags);
}
}
#endif // CONFIG_PCI_HCI

static struct ac_log_info *aclog_lookfor_entry(struct rtl8192cd_priv *priv, unsigned char *addr)
{
	int i, idx=-1;

	for (i=0; i<MAX_AC_LOG; i++) {
		if (priv->acLog[i].used == 0) {
			if (idx < 0)
				idx = i;
			continue;
		}
		if (!memcmp(priv->acLog[i].addr, addr, MACADDRLEN))
			break;
	}

	if ( i != MAX_AC_LOG)
		return (&priv->acLog[i]);

	if (idx >= 0)
		return (&priv->acLog[idx]);

	return NULL; // table full
}


static void aclog_update_entry(struct ac_log_info *entry, unsigned char *addr)
{
	if (entry->used == 0) {
		memcpy(entry->addr, addr, MACADDRLEN);
		entry->used = 1;
	}
	entry->cur_cnt++;
	entry->last_attack_time = jiffies;
}


static int aclog_check(struct rtl8192cd_priv *priv)
{
	int i, used=0;

	for (i=0; i<MAX_AC_LOG; i++) {
		if (priv->acLog[i].used) {
			used++;
			if (priv->acLog[i].cur_cnt != priv->acLog[i].last_cnt) {
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
				LOG_MSG_DROP("Unauthorized wireless PC try to connect;note:%02X:%02X:%02X:%02X:%02X:%02X;\n",
					priv->acLog[i].addr[0], priv->acLog[i].addr[1], priv->acLog[i].addr[2],
					priv->acLog[i].addr[3], priv->acLog[i].addr[4], priv->acLog[i].addr[5]);
#elif defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
				LOG_MSG_DROP("Unauthorized wireless PC try to connect;note:%02X:%02X:%02X:%02X:%02X:%02X;\n",
					priv->acLog[i].addr[0], priv->acLog[i].addr[1], priv->acLog[i].addr[2],
					priv->acLog[i].addr[3], priv->acLog[i].addr[4], priv->acLog[i].addr[5]);
#elif defined(CONFIG_RTL8196B_TLD)
				LOG_MSG_DEL("[WLAN access denied] from MAC: %02x:%02x:%02x:%02x:%02x:%02x,\n",
					priv->acLog[i].addr[0], priv->acLog[i].addr[1], priv->acLog[i].addr[2],
					priv->acLog[i].addr[3], priv->acLog[i].addr[4], priv->acLog[i].addr[5]);
#else
				LOG_MSG("A wireless client (%02X:%02X:%02X:%02X:%02X:%02X) was rejected due to access control for %d times in 5 minutes\n",
					priv->acLog[i].addr[0], priv->acLog[i].addr[1], priv->acLog[i].addr[2],
					priv->acLog[i].addr[3], priv->acLog[i].addr[4], priv->acLog[i].addr[5],
					priv->acLog[i].cur_cnt - priv->acLog[i].last_cnt);
#endif
				priv->acLog[i].last_cnt = priv->acLog[i].cur_cnt;
			}
			else { // no update, check expired entry
				if ((jiffies - priv->acLog[i].last_attack_time) > AC_LOG_EXPIRE) {
					memset(&priv->acLog[i], '\0', sizeof(struct ac_log_info));
					used--;
				}
			}
		}
	}

	return used;
}


#ifdef WIFI_WMM
static void get_AP_Qos_Info(struct rtl8192cd_priv *priv, unsigned char *temp)
{
	temp[0] = GET_EDCA_PARA_UPDATE;
	temp[0] &= 0x0f;
	if (APSD_ENABLE)
		temp[0] |= BIT(7);
}


static void get_STA_AC_Para_Record(struct rtl8192cd_priv *priv, unsigned char *temp)
{
//BE
	temp[0] = GET_STA_AC_BE_PARA.AIFSN;
	temp[0] &= 0x0f;
	if (GET_STA_AC_BE_PARA.ACM)
		temp[0] |= BIT(4);
	temp[1] = GET_STA_AC_BE_PARA.ECWmax;
	temp[1] <<= 4;
	temp[1] |= GET_STA_AC_BE_PARA.ECWmin;
	temp[2] = GET_STA_AC_BE_PARA.TXOPlimit % 256;
	temp[3] = GET_STA_AC_BE_PARA.TXOPlimit / 256; // 2^8 = 256, for one byte's range

//BK
	temp[4] = GET_STA_AC_BK_PARA.AIFSN;
	temp[4] &= 0x0f;
	if (GET_STA_AC_BK_PARA.ACM)
		temp[4] |= BIT(4);
	temp[4] |= BIT(5);
	temp[5] = GET_STA_AC_BK_PARA.ECWmax;
	temp[5] <<= 4;
	temp[5] |= GET_STA_AC_BK_PARA.ECWmin;
	temp[6] = GET_STA_AC_BK_PARA.TXOPlimit % 256;
	temp[7] = GET_STA_AC_BK_PARA.TXOPlimit / 256;

//VI
	temp[8] = GET_STA_AC_VI_PARA.AIFSN;
	temp[8] &= 0x0f;
	if (GET_STA_AC_VI_PARA.ACM)
		temp[8] |= BIT(4);
	temp[8] |= BIT(6);
	temp[9] = GET_STA_AC_VI_PARA.ECWmax;
	temp[9] <<= 4;
	temp[9] |= GET_STA_AC_VI_PARA.ECWmin;
	temp[10] = GET_STA_AC_VI_PARA.TXOPlimit % 256;
	temp[11] = GET_STA_AC_VI_PARA.TXOPlimit / 256;

//VO
	temp[12] = GET_STA_AC_VO_PARA.AIFSN;
	temp[12] &= 0x0f;
	if (GET_STA_AC_VO_PARA.ACM)
		temp[12] |= BIT(4);
	temp[12] |= BIT(5)|BIT(6);
	temp[13] = GET_STA_AC_VO_PARA.ECWmax;
	temp[13] <<= 4;
	temp[13] |= GET_STA_AC_VO_PARA.ECWmin;
	temp[14] = GET_STA_AC_VO_PARA.TXOPlimit % 256;
	temp[15] = GET_STA_AC_VO_PARA.TXOPlimit /256;
}


void init_WMM_Para_Element(struct rtl8192cd_priv *priv, unsigned char *temp)
{
	if (OPMODE & WIFI_AP_STATE) {
		memcpy(temp, WMM_PARA_IE, 6);
//Qos Info field
		get_AP_Qos_Info(priv, &temp[6]);
//AC Parameters
		get_STA_AC_Para_Record(priv, &temp[8]);
 	}
#ifdef CLIENT_MODE
	else if ((OPMODE & WIFI_STATION_STATE) ||(OPMODE & WIFI_ADHOC_STATE)) {  //  WMM STA
		memcpy(temp, WMM_IE, 6);
		temp[6] = 0x00;  //  set zero to WMM STA Qos Info field
#ifdef WMM_APSD
		if ((OPMODE & WIFI_STATION_STATE) && APSD_ENABLE && priv->uapsd_assoc) {
			if (priv->pmib->dot11QosEntry.UAPSD_AC_BE)
				temp[6] |= BIT(3);
			if (priv->pmib->dot11QosEntry.UAPSD_AC_BK)
				temp[6] |= BIT(2);
			if (priv->pmib->dot11QosEntry.UAPSD_AC_VI)
				temp[6] |= BIT(1);
			if (priv->pmib->dot11QosEntry.UAPSD_AC_VO)
				temp[6] |= BIT(0);
		}
#endif
	}
#endif
}

void default_WMM_para(struct rtl8192cd_priv *priv)
{
#ifdef RTL_MANUAL_EDCA
	if( priv->pmib->dot11QosEntry.ManualEDCA ) {
		GET_STA_AC_BE_PARA.ACM = priv->pmib->dot11QosEntry.STA_manualEDCA[BE].ACM;
		GET_STA_AC_BE_PARA.AIFSN = priv->pmib->dot11QosEntry.STA_manualEDCA[BE].AIFSN;
		GET_STA_AC_BE_PARA.ECWmin = priv->pmib->dot11QosEntry.STA_manualEDCA[BE].ECWmin;
		GET_STA_AC_BE_PARA.ECWmax = priv->pmib->dot11QosEntry.STA_manualEDCA[BE].ECWmax;
		GET_STA_AC_BE_PARA.TXOPlimit = priv->pmib->dot11QosEntry.STA_manualEDCA[BE].TXOPlimit;

		GET_STA_AC_BK_PARA.ACM = priv->pmib->dot11QosEntry.STA_manualEDCA[BK].ACM;
		GET_STA_AC_BK_PARA.AIFSN = priv->pmib->dot11QosEntry.STA_manualEDCA[BK].AIFSN;
		GET_STA_AC_BK_PARA.ECWmin = priv->pmib->dot11QosEntry.STA_manualEDCA[BK].ECWmin;
		GET_STA_AC_BK_PARA.ECWmax = priv->pmib->dot11QosEntry.STA_manualEDCA[BK].ECWmax;
		GET_STA_AC_BK_PARA.TXOPlimit = priv->pmib->dot11QosEntry.STA_manualEDCA[BK].TXOPlimit;

		GET_STA_AC_VI_PARA.ACM = priv->pmib->dot11QosEntry.STA_manualEDCA[VI].ACM;
		GET_STA_AC_VI_PARA.AIFSN = priv->pmib->dot11QosEntry.STA_manualEDCA[VI].AIFSN;
		GET_STA_AC_VI_PARA.ECWmin = priv->pmib->dot11QosEntry.STA_manualEDCA[VI].ECWmin;
		GET_STA_AC_VI_PARA.ECWmax = priv->pmib->dot11QosEntry.STA_manualEDCA[VI].ECWmax;
		GET_STA_AC_VI_PARA.TXOPlimit = priv->pmib->dot11QosEntry.STA_manualEDCA[VI].TXOPlimit; // 6.016ms

		GET_STA_AC_VO_PARA.ACM = priv->pmib->dot11QosEntry.STA_manualEDCA[VO].ACM;
		GET_STA_AC_VO_PARA.AIFSN = priv->pmib->dot11QosEntry.STA_manualEDCA[VO].AIFSN;
		GET_STA_AC_VO_PARA.ECWmin = priv->pmib->dot11QosEntry.STA_manualEDCA[VO].ECWmin;
		GET_STA_AC_VO_PARA.ECWmax = priv->pmib->dot11QosEntry.STA_manualEDCA[VO].ECWmax;
		GET_STA_AC_VO_PARA.TXOPlimit = priv->pmib->dot11QosEntry.STA_manualEDCA[VO].TXOPlimit; // 3.264ms
		} else
#endif
	{
		GET_STA_AC_BE_PARA.ACM = rtl_sta_EDCA[BE].ACM;
		GET_STA_AC_BE_PARA.AIFSN = rtl_sta_EDCA[BE].AIFSN;
		GET_STA_AC_BE_PARA.ECWmin = rtl_sta_EDCA[BE].ECWmin;
		GET_STA_AC_BE_PARA.ECWmax = rtl_sta_EDCA[BE].ECWmax;
		GET_STA_AC_BE_PARA.TXOPlimit = rtl_sta_EDCA[BE].TXOPlimit;

		GET_STA_AC_BK_PARA.ACM = rtl_sta_EDCA[BK].ACM;
		GET_STA_AC_BK_PARA.AIFSN = rtl_sta_EDCA[BK].AIFSN;
		GET_STA_AC_BK_PARA.ECWmin = rtl_sta_EDCA[BK].ECWmin;
		GET_STA_AC_BK_PARA.ECWmax = rtl_sta_EDCA[BK].ECWmax;
		GET_STA_AC_BK_PARA.TXOPlimit = rtl_sta_EDCA[BK].TXOPlimit;

		GET_STA_AC_VI_PARA.ACM = rtl_sta_EDCA[VI].ACM;
		GET_STA_AC_VI_PARA.AIFSN = rtl_sta_EDCA[VI].AIFSN;
		GET_STA_AC_VI_PARA.ECWmin = rtl_sta_EDCA[VI].ECWmin;
		GET_STA_AC_VI_PARA.ECWmax = rtl_sta_EDCA[VI].ECWmax;
		if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G|WIRELESS_11A))
			GET_STA_AC_VI_PARA.TXOPlimit = 94; // 3.008ms							GET_STA_AC_VI_PARA.TXOPlimit = rtl_sta_EDCA[VI_AG].TXOPlimit; // 3.008ms
		else
			GET_STA_AC_VI_PARA.TXOPlimit = 188; // 6.016ms								GET_STA_AC_VI_PARA.TXOPlimit = rtl_sta_EDCA[VI].TXOPlimit; // 6.016ms

		GET_STA_AC_VO_PARA.ACM = rtl_sta_EDCA[VO].ACM;
		GET_STA_AC_VO_PARA.AIFSN = rtl_sta_EDCA[VO].AIFSN;
		GET_STA_AC_VO_PARA.ECWmin = rtl_sta_EDCA[VO].ECWmin;
		GET_STA_AC_VO_PARA.ECWmax = rtl_sta_EDCA[VO].ECWmax;
		if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G|WIRELESS_11A))
			GET_STA_AC_VO_PARA.TXOPlimit = 47; // 1.504ms							GET_STA_AC_VO_PARA.TXOPlimit = rtl_sta_EDCA[VO_AG].TXOPlimit; // 1.504ms
		else
			GET_STA_AC_VO_PARA.TXOPlimit = 102; // 3.264ms								GET_STA_AC_VO_PARA.TXOPlimit = rtl_sta_EDCA[VO].TXOPlimit; // 3.264ms
	}
}



#ifdef CLIENT_MODE
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static void process_WMM_para_ie(struct rtl8192cd_priv *priv, unsigned char *p)
{
	int ACI = (p[0] >> 5) & 0x03;
	/*avoid unaligned load*/
	unsigned short txoplimit;
	memcpy(&txoplimit,&p[2],sizeof(unsigned short));
	if ((ACI >= 0) && (ACI <= 3)) {
		switch(ACI) {
			case 0:
				GET_STA_AC_BE_PARA.ACM = (p[0] >> 4) & 0x01;
				GET_STA_AC_BE_PARA.AIFSN = p[0] & 0x0f;
				GET_STA_AC_BE_PARA.ECWmin = p[1] & 0x0f;
				GET_STA_AC_BE_PARA.ECWmax = p[1] >> 4;
				GET_STA_AC_BE_PARA.TXOPlimit = le16_to_cpu(txoplimit);
				break;
			case 3:
				GET_STA_AC_VO_PARA.ACM = (p[0] >> 4) & 0x01;
				GET_STA_AC_VO_PARA.AIFSN = p[0] & 0x0f;
				GET_STA_AC_VO_PARA.ECWmin = p[1] & 0x0f;
				GET_STA_AC_VO_PARA.ECWmax = p[1] >> 4;
				GET_STA_AC_VO_PARA.TXOPlimit = le16_to_cpu(txoplimit);
				break;
			case 2:
				GET_STA_AC_VI_PARA.ACM = (p[0] >> 4) & 0x01;
				GET_STA_AC_VI_PARA.AIFSN = p[0] & 0x0f;
				GET_STA_AC_VI_PARA.ECWmin = p[1] & 0x0f;
				GET_STA_AC_VI_PARA.ECWmax = p[1] >> 4;
				GET_STA_AC_VI_PARA.TXOPlimit = le16_to_cpu(txoplimit);
				break;
			default:
				GET_STA_AC_BK_PARA.ACM = (p[0] >> 4) & 0x01;
				GET_STA_AC_BK_PARA.AIFSN = p[0] & 0x0f;
				GET_STA_AC_BK_PARA.ECWmin = p[1] & 0x0f;
				GET_STA_AC_BK_PARA.ECWmax = p[1] >> 4;
				GET_STA_AC_BK_PARA.TXOPlimit = le16_to_cpu(txoplimit);
				break;
		}
	}
	else
		printk("WMM AP EDCA Parameter IE error!\n");
}


static void sta_config_EDCA_para(struct rtl8192cd_priv *priv)
{
	unsigned int slot_time = 20, ifs_time = 10;
	unsigned int vo_edca = 0, vi_edca = 0, be_edca = 0, bk_edca = 0;

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N ) ||
		(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G))
		slot_time = 9;

	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
		ifs_time = 16;
#if 1
	if (GET_STA_AC_VO_PARA.AIFSN) {
		vo_edca = (((unsigned short)(GET_STA_AC_VO_PARA.TXOPlimit)) << 16)
			| (((unsigned char)(GET_STA_AC_VO_PARA.ECWmax)) << 12)
			| (((unsigned char)(GET_STA_AC_VO_PARA.ECWmin)) << 8)
			| (ifs_time + GET_STA_AC_VO_PARA.AIFSN * slot_time);

		RTL_W32(EDCA_VO_PARA, vo_edca);
	}

	if (GET_STA_AC_VI_PARA.AIFSN) {
		vi_edca = (((unsigned short)(GET_STA_AC_VI_PARA.TXOPlimit)) << 16)
			| (((unsigned char)(GET_STA_AC_VI_PARA.ECWmax)) << 12)
			| (((unsigned char)(GET_STA_AC_VI_PARA.ECWmin)) << 8)
			| (ifs_time + GET_STA_AC_VI_PARA.AIFSN * slot_time);

		/* WiFi Client mode WMM test IOT refine */
		if (priv->pmib->dot11OperationEntry.wifi_specific && (GET_STA_AC_VI_PARA.AIFSN == 2))
			vi_edca = (vi_edca & ~0xff) | (ifs_time + (GET_STA_AC_VI_PARA.AIFSN + 1) * slot_time);

		RTL_W32(EDCA_VI_PARA, vi_edca);
	}

	if (GET_STA_AC_BE_PARA.AIFSN) {
		be_edca = (((unsigned short)(GET_STA_AC_BE_PARA.TXOPlimit)) << 16)
			| (((unsigned char)(GET_STA_AC_BE_PARA.ECWmax)) << 12)
			| (((unsigned char)(GET_STA_AC_BE_PARA.ECWmin)) << 8)
			| (ifs_time + GET_STA_AC_BE_PARA.AIFSN * slot_time);

		RTL_W32(EDCA_BE_PARA, be_edca);
	}

	if (GET_STA_AC_BK_PARA.AIFSN) {
		bk_edca = (((unsigned short)(GET_STA_AC_BK_PARA.TXOPlimit)) << 16)
			| (((unsigned char)(GET_STA_AC_BK_PARA.ECWmax)) << 12)
			| (((unsigned char)(GET_STA_AC_BK_PARA.ECWmin)) << 8)
			| (ifs_time + GET_STA_AC_BK_PARA.AIFSN * slot_time);
		
		RTL_W32(EDCA_BK_PARA, bk_edca);
	}
#else
	if(GET_STA_AC_VO_PARA.AIFSN > 0) {
		RTL_W32(EDCA_VO_PARA, (((unsigned short)(GET_STA_AC_VO_PARA.TXOPlimit)) << 16)
			| (((unsigned char)(GET_STA_AC_VO_PARA.ECWmax)) << 12)
			| (((unsigned char)(GET_STA_AC_VO_PARA.ECWmin)) << 8)
			| (ifs_time + GET_STA_AC_VO_PARA.AIFSN * slot_time));
		if(GET_STA_AC_VO_PARA.ACM > 0)
			RTL_W8(ACMHWCTRL, RTL_R8(ACMHWCTRL)|BIT(3));
	}

	if(GET_STA_AC_VI_PARA.AIFSN > 0) {
		RTL_W32(EDCA_VI_PARA, (((unsigned short)(GET_STA_AC_VI_PARA.TXOPlimit)) << 16)
			| (((unsigned char)(GET_STA_AC_VI_PARA.ECWmax)) << 12)
			| (((unsigned char)(GET_STA_AC_VI_PARA.ECWmin)) << 8)
			| (ifs_time + GET_STA_AC_VI_PARA.AIFSN * slot_time));
		if(GET_STA_AC_VI_PARA.ACM > 0)
			RTL_W8(ACMHWCTRL, RTL_R8(ACMHWCTRL)|BIT(2));
	}

	if(GET_STA_AC_BE_PARA.AIFSN > 0) {
		RTL_W32(EDCA_BE_PARA, (((unsigned short)(GET_STA_AC_BE_PARA.TXOPlimit)) << 16)
			| (((unsigned char)(GET_STA_AC_BE_PARA.ECWmax)) << 12)
			| (((unsigned char)(GET_STA_AC_BE_PARA.ECWmin)) << 8)
			| (ifs_time + GET_STA_AC_BE_PARA.AIFSN * slot_time));
		if(GET_STA_AC_BE_PARA.ACM > 0)
			RTL_W8(ACMHWCTRL, RTL_R8(ACMHWCTRL)|BIT(1));
	}

	if(GET_STA_AC_BK_PARA.AIFSN > 0) {
		RTL_W32(EDCA_BK_PARA, (((unsigned short)(GET_STA_AC_BK_PARA.TXOPlimit)) << 16)
			| (((unsigned char)(GET_STA_AC_BK_PARA.ECWmax)) << 12)
			| (((unsigned char)(GET_STA_AC_BK_PARA.ECWmin)) << 8)
			| (ifs_time + GET_STA_AC_BK_PARA.AIFSN * slot_time));
	}

	if ((GET_STA_AC_VO_PARA.ACM > 0) || (GET_STA_AC_VI_PARA.ACM > 0) || (GET_STA_AC_BE_PARA.ACM > 0))
		RTL_W8(ACMHWCTRL, RTL_R8(ACMHWCTRL)|BIT(0));
#endif

	priv->pmib->dot11QosEntry.EDCA_STA_config = 1;
	priv->pshare->iot_mode_enable = 0;
	if (priv->pshare->rf_ft_var.wifi_beq_iot)
		priv->pshare->iot_mode_VI_exist = 0;
	priv->pshare->iot_mode_VO_exist = 0;
#ifdef WMM_VIBE_PRI
	priv->pshare->iot_mode_BE_exist = 0;
#endif
#ifdef WMM_BEBK_PRI
	priv->pshare->iot_mode_BK_exist = 0;
#endif
#ifdef LOW_TP_TXOP
	priv->pshare->BE_cwmax_enhance = 0;
#endif
}


static void reset_EDCA_para(struct rtl8192cd_priv *priv)
{
	memset((void *)&GET_STA_AC_VO_PARA, 0, sizeof(struct ParaRecord));
	memset((void *)&GET_STA_AC_VI_PARA, 0, sizeof(struct ParaRecord));
	memset((void *)&GET_STA_AC_BE_PARA, 0, sizeof(struct ParaRecord));
	memset((void *)&GET_STA_AC_BK_PARA, 0, sizeof(struct ParaRecord));

#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
	if(IS_OUTSRC_CHIP(priv))
#endif
	EdcaParaInit(priv);
#endif

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
	if(!IS_OUTSRC_CHIP(priv))
#endif
	init_EDCA_para(priv, priv->pmib->dot11BssType.net_work_type);
#endif

	priv->pmib->dot11QosEntry.EDCA_STA_config = 0;
}
#endif // CLIENT_MODE
#endif // WIFI_WMM


// Realtek proprietary IE
static void process_rtk_ie(struct rtl8192cd_priv *priv)
{
	struct stat_info *pstat;
	int use_long_slottime=0;
	unsigned int threshold;
#if defined(SMP_SYNC) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	unsigned long flags=0;
#endif

	if ((priv->up_time % 3) != 0)
		return;

	if ((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R))
		threshold = 50*1024*1024/8;
	else
		threshold = 100*1024*1024/8;

	if (OPMODE & WIFI_AP_STATE)
	{
		struct list_head *phead = &priv->asoc_list;
		struct list_head *plist;

		SMP_LOCK_ASOC_LIST(flags);
		
		plist = phead->next;
		while(plist != phead)
		{
			pstat = list_entry(plist, struct stat_info, asoc_list);
			plist = plist->next;

			if ((pstat->expire_to > 0) &&
				(/*priv->pshare->is_giga_exist ||*/ !pstat->is_2t_mimo_sta) &&
				((pstat->is_realtek_sta && (pstat->IOTPeer!= HT_IOT_PEER_RTK_APCLIENT) && ((pstat->tx_avarage + pstat->rx_avarage) > threshold))
#ifdef WDS
				  || ((pstat->state & WIFI_WDS) && ((pstat->tx_avarage + pstat->rx_avarage) > (threshold*2/3)))
#endif
				  )) {
				use_long_slottime = 1;
				break;
			}
		}

		SMP_UNLOCK_ASOC_LIST(flags);

		if (priv->pshare->use_long_slottime == 0) {
			if (use_long_slottime) {
				priv->pshare->use_long_slottime = 1;
				set_slot_time(priv, 0);
				priv->pmib->dot11ErpInfo.shortSlot = 0;
				RESET_SHORTSLOT_IN_BEACON_CAP;
				priv->pshare->rtk_ie_buf[5] |= RTK_CAP_IE_USE_LONG_SLOT;
			}
		}
		else {
			if (use_long_slottime == 0) {
				priv->pshare->use_long_slottime = 0;
				check_protection_shortslot(priv);
				priv->pshare->rtk_ie_buf[5] &= (~RTK_CAP_IE_USE_LONG_SLOT);
			}
		}
	}
}

#ifdef RADIUS_ACCOUNTING
void indicate_sta_leaving(struct rtl8192cd_priv *priv,struct stat_info *pstat, unsigned long reason)
{
	DOT11_DISASSOCIATION_IND Disassociation_Ind;

	memcpy((void *)Disassociation_Ind.MACAddr, (void *)(pstat->hwaddr), MACADDRLEN);
	Disassociation_Ind.EventId = DOT11_EVENT_DISASSOCIATION_IND;
	Disassociation_Ind.IsMoreEvent = 0;
	Disassociation_Ind.Reason = reason;
	Disassociation_Ind.tx_packets = pstat->tx_pkts;
	Disassociation_Ind.rx_packets = pstat->rx_pkts;
	Disassociation_Ind.tx_bytes   = pstat->tx_bytes;
	Disassociation_Ind.rx_bytes   = pstat->rx_bytes;
	DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Disassociation_Ind,
				sizeof(DOT11_DISASSOCIATION_IND));
	psk_indicate_evt(priv, DOT11_EVENT_DISASSOCIATION_IND, pstat->hwaddr, NULL, 0);
}

int cal_statistics_acct(struct rtl8192cd_priv *priv)
{
	unsigned long ret=0;
	struct list_head *phead=NULL, *plist=NULL;
	struct stat_info *pstat=NULL;
#ifdef SMP_SYNC
	unsigned long flags=0;
#endif

	phead = &priv->asoc_list;
	if( list_empty(phead) )
		goto acct_cal_out;

	SMP_LOCK_ASOC_LIST(flags);
	
	plist = phead->next;
	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;

		if( pstat->link_time%ACCT_TP_INT == 0 ){
			pstat->rx_bytes_1m = pstat->rx_bytes - pstat->rx_bytes_1m;
			pstat->tx_bytes_1m = pstat->tx_bytes - pstat->tx_bytes_1m;
		}
	}
	
	SMP_UNLOCK_ASOC_LIST(flags);

acct_cal_out:
	return ret;
}

int expire_sta_for_radiusacct(struct rtl8192cd_priv *priv)
{
	int ret=0;
	struct list_head *phead=NULL, *plist=NULL;
	struct stat_info *pstat=NULL;

	if( (ACCT_FUN_TIME == 0) && (ACCT_FUN_TP == 0))
		goto acct_expire_out;

	phead = &priv->asoc_list;
	if(list_empty(phead))
		goto acct_expire_out;

	plist = phead;
	
	while ((plist = asoc_list_get_next(priv, plist)) != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);

		if(pstat->link_time > ACCT_FUN_TIME*60 ){
#if !defined(WITHOUT_ENQUEUE) && (defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)  || defined(RTK_NL80211))
			indicate_sta_leaving(priv,pstat,_RSON_AUTH_NO_LONGER_VALID_);
#endif
			issue_deauth(priv,pstat->hwaddr,_RSON_AUTH_NO_LONGER_VALID_);
			cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
			check_sta_characteristic(priv, pstat, DECREASE);
			LOG_MSG("A STA(%02X:%02X:%02X:%02X:%02X:%02X) is deleted for accounting becoz of time-out\n",
				pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);
		}

		if(pstat->tx_bytes_1m+pstat->rx_bytes_1m < ACCT_FUN_TP*(2^20) ){
#if !defined(WITHOUT_ENQUEUE) && (defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211))
			indicate_sta_leaving(priv,pstat,_RSON_AUTH_NO_LONGER_VALID_);
#endif
			issue_deauth(priv,pstat->hwaddr,_RSON_AUTH_NO_LONGER_VALID_);
			cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
			check_sta_characteristic(priv, pstat, DECREASE);
			LOG_MSG("A STA(%02X:%02X:%02X:%02X:%02X:%02X) is deleted for accounting becoz of low TP\n",
				pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);
		}
	}

acct_expire_out:
	return ret;
}
#endif	//#ifdef RADIUS_ACCOUNTING


#ifdef SMART_REPEATER_MODE
static void switch_chan_to_vxd(struct rtl8192cd_priv *priv)
{
#ifdef MBSSID
	unsigned int i;
#endif

	priv->pmib->dot11RFEntry.dot11channel = priv->pshare->switch_chan_rp;
	priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = priv->pshare->switch_2ndchoff_rp;
	GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11nUse40M = priv->pshare->band_width_rp;
	RTL_W8(TXPAUSE, 0xff);

	// update vxd channel and 2ndChOffset
#ifdef UNIVERSAL_REPEATER	
    //	GET_VXD_PRIV(priv)->pmib->dot11RFEntry.dot11channel = priv->pshare->switch_chan_rp;
    //	GET_VXD_PRIV(priv)->pmib->dot11nConfigEntry.dot11n2ndChOffset = priv->pshare->switch_2ndchoff_rp;
#endif    

	DEBUG_INFO("3. Swiching channel to %d!\n", priv->pmib->dot11RFEntry.dot11channel);
	priv->pmib->dot11OperationEntry.keep_rsnie = 1; 
	
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (IS_DRV_OPEN(priv->pvap_priv[i]))
				priv->pvap_priv[i]->pmib->dot11OperationEntry.keep_rsnie = 1;
		}
	}
#endif

#ifdef UNIVERSAL_REPEATER
	if (GET_VXD_PRIV(priv))
		GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.keep_rsnie = 1;
#endif

	rtl8192cd_close(priv->dev);
	rtl8192cd_open(priv->dev);

	RTL_W8(TXPAUSE, 0x00);			
}
#endif

#if defined(TXREPORT) && defined(CONFIG_WLAN_HAL)
void requestTxReport88XX(struct rtl8192cd_priv *priv)
{
	//unsigned char h2cresult;
	struct stat_info *sta;
	unsigned char H2CCommand[2] = {0xff, 0xff};

	if ( priv->pshare->sta_query_idx == -1)
		return;

#ifdef TXRETRY_CNT
	priv->pshare->sta_query_retry_idx = priv->pshare->sta_query_idx;
#endif

	sta = findNextSTA(priv, &priv->pshare->sta_query_idx);
	if (sta)
	{
		H2CCommand[0] = REMAP_AID(sta);
	}
	else {
		priv->pshare->sta_query_idx = -1;
		return;
	}

	sta = findNextSTA(priv, &priv->pshare->sta_query_idx);
	if (sta)	{
		H2CCommand[1] = REMAP_AID(sta);
	} else {
		priv->pshare->sta_query_idx = -1;
	}

	//WDEBUG("\n");
	//h2cresult = FillH2CCmd8812(priv, H2C_8812_TX_REPORT, 2 , H2CCommand);
	//WDEBUG("h2cresult=%d\n",h2cresult);
	GET_HAL_INTERFACE(priv)->FillH2CCmdHandler(priv, H2C_88XX_AP_REQ_TXREP, 2, H2CCommand);

}

#ifdef TXRETRY_CNT
void requestTxRetry88XX(struct rtl8192cd_priv *priv)
{
	unsigned char counter = 20;
	struct stat_info *sta;
	unsigned char H2CCommand[3] = {0xff, 0xff, 0x2};

	//panic_printk("%s %d \n", __FUNCTION__, __LINE__);

	if ( priv->pshare->sta_query_retry_idx == -1)
		return;

	while (is_h2c_buf_occupy(priv)) {
		delay_ms(2);
		if (--counter == 0)
			break;
	}

	if (!counter)
		return;

	//panic_printk("%s %d \n", __FUNCTION__, __LINE__);

	sta = findNextSTA(priv, &priv->pshare->sta_query_retry_idx);
	if (sta)
	{
		H2CCommand[0] = REMAP_AID(sta);
	}
	else {
		priv->pshare->sta_query_retry_idx = -1;
		return;
	}

	sta = findNextSTA(priv, &priv->pshare->sta_query_retry_idx);
	if (sta)	{
		H2CCommand[1] = REMAP_AID(sta);
	} else {
		priv->pshare->sta_query_retry_idx = -1;
	}

	priv->pshare->sta_query_retry_macid[0] = H2CCommand[0];
	priv->pshare->sta_query_retry_macid[1] = H2CCommand[1];

	GET_HAL_INTERFACE(priv)->FillH2CCmdHandler(priv, H2C_88XX_AP_REQ_TXREP, 3, H2CCommand);

}
#endif

#endif


int is_intel_connected(struct rtl8192cd_priv *priv)
{
	struct list_head *phead, *plist;
	struct stat_info *pstat;
	unsigned char intel_connected=0;
#if defined(SMP_SYNC) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	unsigned long flags = 0;
#endif

	phead = &priv->asoc_list;
	
	if (list_empty(phead)) {
		return 0;
	}

	SMP_LOCK_ASOC_LIST(flags);

	plist = phead->next;
	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;
						
		if(pstat->IOTPeer== HT_IOT_PEER_INTEL)
		{
			intel_connected = 1;
			break;
		}
	}

	SMP_UNLOCK_ASOC_LIST(flags);

	return intel_connected;
}

#ifdef HS2_SUPPORT
#ifdef HS2_CLIENT_TEST
//issue_GASreq for client test used
int issue_GASreq(struct rtl8192cd_priv *priv, DOT11_HS2_GAS_REQ *gas_req, unsigned short qid);
int testflg=0;
#endif
#endif

#ifdef RTK_NL80211//survey_dump
const int MCS_DATA_RATEFloat[2][2][16] =
{
	{{6.5, 13, 19.5, 26, 39, 52, 58.5, 65, 13, 26, 39, 52, 78, 104, 117, 130},						  // Long GI, 20MHz
	 {7.2, 14.4, 21.7, 28.9, 43.3, 57.8, 65, 72.2, 14.4, 28.9, 43.3, 57.8, 86.7, 115.6, 130, 144.5}}, // Short GI, 20MHz
	{{13.5, 27, 40.5, 54, 81, 108, 121.5, 135, 27, 54, 81, 108, 162, 216, 243, 270},                  // Long GI, 40MHz
	 {15, 30, 45, 60, 90, 120, 135, 150, 30, 60, 90, 120, 180, 240, 270, 300}}                        // Short GI, 40MHz
};

void check_sta_throughput(struct rtl8192cd_priv *priv, unsigned int chan_idx)
{
	struct list_head *phead, *plist;
	struct stat_info *pstat;
	int tx_rate=0, rx_rate=0;
	int tx_time=0, rx_time=0;

	//check every sta throughput
	phead = &priv->asoc_list;
	if (list_empty(phead)) {
		return;
	}
	plist = phead->next;

	while (plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		pstat->tx_time_total=0;
		pstat->rx_time_total=0;
		//cal current tx rate
#ifdef RTK_AC_SUPPORT  //vht rate , todo, dump vht rates in Mbps
		if(pstat->current_tx_rate >= 0x90)
		{
			tx_rate = query_vht_rate(pstat);
			rx_rate = query_vht_rate(pstat);
		}
		else
#endif
		if (is_MCS_rate(pstat->current_tx_rate)) 
		{
			tx_rate = MCS_DATA_RATEFloat[(pstat->ht_current_tx_info&BIT(0))?1:0][(pstat->ht_current_tx_info&BIT(1))?1:0][pstat->current_tx_rate&0xf];
			rx_rate = MCS_DATA_RATEFloat[pstat->rx_bw&0x01][pstat->rx_splcp&0x01][pstat->rx_rate&0xf];
		}
		else
		{
			tx_rate = pstat->current_tx_rate/2;
			rx_rate = pstat->rx_rate/2;
		}
		
		//last tx/rx time
		if(tx_rate != 0)
			pstat->tx_time_total += (((pstat->tx_bytes - pstat->tx_byte_last)*8*1000)/(tx_rate*1024));
		if(rx_rate != 0)
			pstat->rx_time_total += (((pstat->rx_bytes - pstat->rx_byte_last)*8*1000)/(rx_rate*1024));
		#if 0
		printk("tx: rate[%d] tx_byte[%d] tx_byte_last[%d]\n",
			tx_rate, pstat->tx_bytes - pstat->tx_byte_last,  pstat->tx_byte_last);
		printk("rx: rate[%d] rx_byte[%d] rx_byte_last[%d]\n",
			rx_rate, pstat->rx_bytes - pstat->rx_byte_last,  pstat->rx_byte_last);
		#endif
		//tx/rx byte last
		pstat->tx_byte_last = pstat->tx_bytes;
		pstat->rx_byte_last = pstat->rx_bytes;
		
		//count all traffic time
		tx_time += pstat->tx_time_total;
		rx_time += pstat->rx_time_total;

		plist = plist->next;
	}
	priv->rtk->survey_info[chan_idx].tx_time = tx_time;
	priv->rtk->survey_info[chan_idx].rx_time = rx_time;

	//printk("tx_time=%d rx_time=%d\n",priv->rtk->survey_info[chan_idx].tx_time, priv->rtk->survey_info[chan_idx].rx_time);
}

int read_snr_report(struct rtl8192cd_priv *priv, unsigned int idx)
{
	int ret=0, snr[RF92CD_PATH_MAX]={0}, snr_max=0, i;

	for (i=RF92CD_PATH_A; i<RF92CD_PATH_MAX; i++) {
		snr[i] = (RTL_R32(0xf88) & ((i==RF92CD_PATH_A)?0xff:0xff00)) >> (i*8);
		//printk("%s %d [Original]snr%s:%02x(HEX), %d(INT)\n",__func__,__LINE__,((i==RF92CD_PATH_A)?"A":"B"),snr[i],snr[i]);
		snr[i] = (~snr[i]+0x1)>>1;
		//printk("%s %d [2's complement]snr%s:%02x(HEX), %d(INT)\n",__func__,__LINE__,((i==RF92CD_PATH_A)?"A":"B"),snr[i],snr[i]);
		if( snr_max == 0 )
			snr_max = snr[i];
		else if (snr[i] > snr_max)
			snr_max = snr[i];
	}

	priv->rtk->survey_info[idx].snr = snr_max;
	//printk("%s %d channel:%d noise floor:%d\n",__func__,__LINE__,priv->rtk->survey_info[idx].channel,priv->rtk->survey_info[idx].snr);

	return ret;
}
#endif

void rtl8192cd_expire_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
#ifndef SMP_SYNC    
	unsigned long	flags=0;
#endif
    int somevapopened=0;
#ifdef MBSSID	
	int i;
#endif
#if defined(CLIENT_MODE) && defined(WIFI_11N_2040_COEXIST)
	static unsigned int to_issue_coexist_mgt = 0;
#endif

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);

#ifdef MULTI_MAC_CLONE
	ACTIVE_ID = 0;
#endif

	// advance driver up timer
	priv->up_time++;

#ifdef TLN_STATS
	if (priv->pshare->rf_ft_var.stats_time_interval) {
		if (priv->stats_time_countdown) {
			priv->stats_time_countdown--;
		} else {
			memset(&priv->wifi_stats, 0, sizeof(struct tln_wifi_stats));
			memset(&priv->ext_wifi_stats, 0, sizeof(struct tln_ext_wifi_stats));

			priv->stats_time_countdown = priv->pshare->rf_ft_var.stats_time_interval;
		}
	}
#endif

#ifdef	INCLUDE_WPS
	// mount wsp wps_1sec_routine
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{

#ifndef CONFIG_MSC		// for verify
		if(priv->pshare->WSC_CONT_S.RdyGetConfig == 0){
			// if not get config from upnp yet ; send request		
			unsigned char tmp12[20];
			sprintf(tmp12 , "wps_get_config=1");
			set_mib(priv , tmp12) ;	
		}
#endif			
		if(priv->pshare->WSC_CONT_S.oneSecTimeStart){
			wps_1sec_routine(priv);
		}
        else{
            printk("%s %d not enter wsc 1sec \n", __FUNCTION__, __LINE__);
        }
	}
#endif


#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1)) {
		SMP_UNLOCK(flags);
		RESTORE_INT(flags);
		return;
	}
#endif


#ifdef CONFIG_RTK_MESH
	if ((GET_MIB(priv)->dot1180211sInfo.mesh_enable == 1 )
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)	// fix: 0000107 2008/10/13
			&& IS_ROOT_INTERFACE(priv)
#endif
	) {
		if( priv->mesh_log )	// advance flow log timer Throughput statistics (sounder)
			priv->log_time++;

		mesh_expire(priv);
	}

#ifdef  _11s_TEST_MODE_
	mesh_debug_sme1(priv);
#endif

#endif	// DOT11_MESH_MODE_

	// check auth_list
	auth_expire(priv);

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		if ((priv->up_time % 2) == 0)
			priv->pshare->highTP_found_pstat = NULL;
		priv->pshare->rssi_min = 0xff;
#ifdef CLIENT_MODE		
		if((OPMODE & WIFI_ADHOC_STATE) && (priv->prev_tsf)) {
			UINT64		tsf, tsf_diff;
			tsf = RTL_R32(TSFTR+4);
			tsf = (tsf<<32) +  RTL_R32(TSFTR);
			tsf_diff = tsf - priv->prev_tsf;
			priv->prev_tsf = tsf;
			if( (tsf > priv->prev_tsf) && (tsf_diff > BIT(24))) {
				tsf = cpu_to_le64(tsf);
				memcpy(priv->rx_timestamp, (void*)(&tsf), 8);				
				updateTSF(priv);
			}			
		}		
#endif		
	}

#ifdef RX_CRC_EXPTIMER
#ifdef CONFIG_WLAN_HAL_8814AE
	if(GET_CHIP_VER(priv) == VERSION_8814A) {
		if(priv->pshare->rf_ft_var.crc_enable || priv->pshare->rf_ft_var.crc_dump)
			RTL_W16(0x608, RTL_R16(0x608) | BIT(8));
		else
			RTL_W16(0x608, RTL_R16(0x608) & ~BIT(8));
	}
#endif
#endif

	// check asoc_list
	assoc_expire(priv);

#if defined(DRVMAC_LB) && defined(WIFI_WMM)
	if (priv->pmib->miscEntry.drvmac_lb && priv->pmib->miscEntry.lb_tps) {
		unsigned int i = 0;
		for (i = 0; i < priv->pmib->miscEntry.lb_tps; i++) {
			if (priv->pmib->miscEntry.lb_mlmp)
				SendLbQosData(priv);
			else
				SendLbQosNullData(priv);
//			if (i > 4)
//				priv->pmib->miscEntry.lb_tps = 0;
		}
	}
#endif

#ifdef WIFI_SIMPLE_CONFIG
	// check wsc probe request list
	if (priv->pmib->wscEntry.wsc_enable & 2) // work as AP (not registrar)
		wsc_probe_expire(priv);
#endif

#ifdef WDS
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11WdsInfo.wdsEnabled &&
		priv->pmib->dot11WdsInfo.wdsNum){
		wds_probe_expire(priv);		
	}
#endif

	// check link status and start/stop net queue
	priv->link_status = chklink_wkstaQ(priv);

#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
#ifdef CLIENT_MODE
	if (OPMODE & WIFI_STATION_STATE) {
		if (priv->link_status != priv->link_status_bak) {
			//WIFI_LINK_STATUS_CONNECTED or WIFI_LINK_STATUS_DISCONNECTED
#ifdef P2P_SUPPORT	
			if ((priv->link_status == WIFI_LINK_STATUS_CONNECTED) &&
			    rtk_p2p_is_enabled(priv) &&  rtk_p2p_chk_role(priv,P2P_CLIENT))
				priv->p2pPtr->clientmode_connected = 1;
#endif
			if (priv->link_status_cb_func)
				priv->link_status_cb_func("wlan0", priv->link_status);
		}
	}
#endif
	priv->link_status_bak = priv->link_status;

#ifdef P2P_SUPPORT
	if (rtk_p2p_is_enabled(priv) &&  rtk_p2p_chk_role(priv,P2P_TMP_GO) &&
	     (priv->assoc_num_bak) && (priv->assoc_num == 0)) {
		if (priv->p2p_event_indicate_cb_func)
			priv->p2p_event_indicate_cb_func("wlan0", WIFI_P2P_EVENT_BACK2DEV);
	}
#endif
	priv->assoc_num_bak = priv->assoc_num;
#endif

#ifdef RTK_AC_SUPPORT //for 11ac logo
	// channel switch
	if(priv->pshare->rf_ft_var.csa) {
		int ch = priv->pshare->rf_ft_var.csa;
		priv->pshare->rf_ft_var.csa = 0;
		
		GET_ROOT(priv)->pmib->dot11DFSEntry.DFS_detected = 1;
		priv->pshare->dfsSwitchChannel = ch;
		priv->pshare->dfsSwitchChCountDown = 6;
		if (priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod >= priv->pshare->dfsSwitchChCountDown)
			priv->pshare->dfsSwitchChCountDown = priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod+1;
	}
#endif

	// for SW LED
	if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX))
	{
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
#ifdef PCIE_POWER_SAVING
		if ((priv->pwr_state != L1) && (priv->pwr_state != L2))
#endif
			calculate_sw_LED_interval(priv);
	}

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)!=VERSION_8188E)
#endif
	{
#ifdef CLIENT_MODE
		if (((OPMODE & WIFI_AP_STATE) ||
			((OPMODE & WIFI_ADHOC_STATE) &&
				((JOIN_RES == STATE_Sta_Ibss_Active) || (JOIN_RES == STATE_Sta_Ibss_Idle)))) &&
			(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G))
#else
		if ((OPMODE & WIFI_AP_STATE) &&
			(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G))
#endif
		{
			if (priv->pmib->dot11ErpInfo.olbcDetected) {
				if (priv->pmib->dot11ErpInfo.olbcExpired > 0)
					priv->pmib->dot11ErpInfo.olbcExpired--;

				if (priv->pmib->dot11ErpInfo.olbcExpired == 0) {
					priv->pmib->dot11ErpInfo.olbcDetected = 0;
					DEBUG_INFO("OLBC expired\n");
					check_protection_shortslot(priv);
				}
			}
		}
	}

#ifdef TX_EARLY_MODE
	priv->pshare->em_tx_byte_cnt  = priv->ext_stats.tx_byte_cnt;
#endif

	// calculate tx/rx throughput
	priv->ext_stats.tx_avarage = (priv->ext_stats.tx_avarage/10)*7 + (priv->ext_stats.tx_byte_cnt/10)*3;
	priv->ext_stats.tx_byte_cnt = 0;
	priv->ext_stats.rx_avarage = (priv->ext_stats.rx_avarage/10)*7 + (priv->ext_stats.rx_byte_cnt/10)*3;
	priv->ext_stats.rx_byte_cnt = 0;

#ifdef RTK_NL80211
#if 0 //brian, collect channel load information during sitesurvey in stead of here to make sure freshness
	//survey_dump
	int val = read_bbp_ch_load(priv);
	if(val != -1)
	{
		priv->rtk->chbusytime = (val/1000)*5;
		start_bbp_ch_load(priv);
	}
	check_sta_throughput(priv);
#endif
	//openwrt_psd
	if(priv->rtk->psd_chnl != 0)
		rtl8192cd_query_psd_cfg80211(priv, priv->rtk->psd_chnl, priv->rtk->psd_bw, priv->rtk->psd_pts);
	priv->rtk->psd_chnl=0;
	priv->rtk->psd_bw=0;
	priv->rtk->psd_pts=0;
#endif

#ifdef CONFIG_RTL8190_THROUGHPUT
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		unsigned long throughput;

		throughput = (priv->ext_stats.tx_avarage + priv->ext_stats.rx_avarage) * 8 / 1024 / 1024; /* unit: Mbps */
		if (gCpuCanSuspend) {
			if (throughput > TP_HIGH_WATER_MARK) {
				gCpuCanSuspend = 0;
			}
		}
		else {
			if (throughput < TP_LOW_WATER_MARK) {
				gCpuCanSuspend = 1;
			}
		}
#ifdef CONFIG_RTL_8812_SUPPORT  
	if(GET_CHIP_VER(priv)== VERSION_8812E) {            
        if( (((priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage) * 8)/1024)/1024 > 50 )	//when TH > 50Mbps
		{
            if(!priv->bIntMigration){    
                //RTL_W32(REG_INT_MIG_8812,0x30300000);
                RTL_W16(REG_PCIE_MULTIFET_CTRL_8812,0xF450);            
                priv->bIntMigration=1;
                //SDEBUG("bIntMigration=1\n");
            }
        }else{
            if(priv->bIntMigration){
                //RTL_W32(REG_INT_MIG_8812,0);
                RTL_W16(REG_PCIE_MULTIFET_CTRL_8812,0);
                priv->bIntMigration=0;
                //SDEBUG("bIntMigration=0\n");
            }            
        }
	}
#endif
	}
#endif

#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
	if (priv->ext_stats.tx_avarage > priv->ext_stats.tx_peak)
		priv->ext_stats.tx_peak = priv->ext_stats.tx_avarage;

	if (priv->ext_stats.rx_avarage > priv->ext_stats.rx_peak)
		priv->ext_stats.rx_peak = priv->ext_stats.rx_avarage;
#endif
//#ifdef CONFIG_RTL865X_AC
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	if (priv->ext_stats.tx_avarage > priv->ext_stats.tx_peak)
		priv->ext_stats.tx_peak = priv->ext_stats.tx_avarage;

	if (priv->ext_stats.rx_avarage > priv->ext_stats.rx_peak)
		priv->ext_stats.rx_peak = priv->ext_stats.rx_avarage;
#endif

#ifdef CLIENT_MODE
	if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE))
	{

		// calculate how many beacons we received and decide if should roaming
#ifdef DFS
		if (!priv->pshare->dfsSwCh_ongoing)
#endif
			calculate_rx_beacon(priv);

#ifdef RTK_BR_EXT
		// expire NAT2.5 entry
		nat25_db_expire(priv);

		if (priv->pppoe_connection_in_progress > 0)
			priv->pppoe_connection_in_progress--;
#endif
	}
#endif

#ifdef A4_STA
    a4_sta_expire(priv);
#endif

#ifdef TV_MODE
    if(OPMODE & WIFI_AP_STATE && (priv->up_time%2 == 0)) {
        if(priv->tv_mode_status & BIT1) { /*TV mode is auto, check if there is STA that support tv auto*/
            tv_mode_auto_support_check(priv);
        }
    }
#endif

#ifdef MP_TEST
#ifdef CONFIG_PCI_HCI
	if ((OPMODE & (WIFI_MP_CTX_BACKGROUND|WIFI_MP_CTX_BACKGROUND_PENDING)) ==
		(WIFI_MP_CTX_BACKGROUND|WIFI_MP_CTX_BACKGROUND_PENDING))
#ifdef SMP_SYNC
			if (!priv->pshare->has_triggered_tx_tasklet) {
				tasklet_schedule(&priv->pshare->tx_tasklet);
				priv->pshare->has_triggered_tx_tasklet = 1;
			}
#else
		rtl8192cd_tx_dsr((unsigned long)priv);
#endif
#endif // CONFIG_PCI_HCI


	if (OPMODE & WIFI_MP_RX) {
		if (priv->pshare->rf_ft_var.rssi_dump) {
#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
			if(IS_OUTSRC_CHIP(priv))
#endif
			{
				panic_printk("%d%%  (ss %d %d )(snr %d %d )(sq %d %d)\n",
				priv->pshare->mp_rssi,
				priv->pshare->mp_rf_info.mimorssi[0], priv->pshare->mp_rf_info.mimorssi[1],
				priv->pshare->mp_rf_info.RxSNRdB[0], priv->pshare->mp_rf_info.RxSNRdB[1],
				priv->pshare->mp_rf_info.mimosq[0], priv->pshare->mp_rf_info.mimosq[1]);
		}
#endif
			
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
			if(!IS_OUTSRC_CHIP(priv))
#endif
			{
				panic_printk("%d%%  (ss %d %d )(snr %d %d )(sq %d %d)\n",
				priv->pshare->mp_rssi,
				priv->pshare->mp_rf_info.mimorssi[0], priv->pshare->mp_rf_info.mimorssi[1],
				priv->pshare->mp_rf_info.RxSNRdB[0], priv->pshare->mp_rf_info.RxSNRdB[1],
				priv->pshare->mp_rf_info.mimosq[0], priv->pshare->mp_rf_info.mimosq[1]);
			}
#endif
		}
	}
#endif

	// Realtek proprietary IE
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)!=VERSION_8188E)
#endif
		process_rtk_ie(priv);

	// check ACL log event
	if ((OPMODE & WIFI_AP_STATE) && priv->acLogCountdown > 0) {
		if (--priv->acLogCountdown == 0)
			if (aclog_check(priv) > 0) // still have active entry
				priv->acLogCountdown = AC_LOG_TIME;
	}

#ifdef CLIENT_MODE
	if (OPMODE & WIFI_AP_STATE)
#endif
	{
		// 11n protection count down
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
			if (priv->ht_legacy_obss_to > 0)
				priv->ht_legacy_obss_to--;
			if (priv->ht_nomember_legacy_sta_to > 0)
				priv->ht_nomember_legacy_sta_to--;
		}
	}
#ifdef WIFI_11N_2040_COEXIST
	if (priv->pmib->dot11nConfigEntry.dot11nCoexist && ((OPMODE & WIFI_AP_STATE) 
#ifdef CLIENT_MODE
		|| ((OPMODE & WIFI_STATION_STATE) && priv->coexist_connection)
#endif
		) && (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N|WIRELESS_11G))) {
		if (priv->bg_ap_timeout) {
			//priv->bg_ap_timeout--;	// don't go back to 40M mode for 6300 wrong channel issue
#ifdef CLIENT_MODE
			if (OPMODE & WIFI_STATION_STATE) {
				unsigned int i;
				for (i = 0; i < 14; i++)
					if (priv->bg_ap_timeout_ch[i])
						priv->bg_ap_timeout_ch[i]--;
			}
#endif
		}
#ifdef CLIENT_MODE
		if ((OPMODE & WIFI_STATION_STATE) && priv->intolerant_timeout)
			priv->intolerant_timeout--;
#endif
	}
#endif

	// Dump Rx FiFo overflow count
	if (priv->pshare->rf_ft_var.rxfifoO) {
		panic_printk("RxFiFo Overflow: %d\n", (unsigned int)(priv->ext_stats.rx_fifoO - priv->pshare->rxFiFoO_pre));
		priv->pshare->rxFiFoO_pre = priv->ext_stats.rx_fifoO;
	}

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef WIFI_WMM
		if (QOS_ENABLE) {
#ifdef CLIENT_MODE
			if((OPMODE & WIFI_STATION_STATE) && (!priv->link_status) && (priv->pmib->dot11QosEntry.EDCA_STA_config)) {
				reset_EDCA_para(priv);
			}
#endif
		}
#endif

		if (fw_was_full(priv) && priv->pshare->fw_free_space > 0) { // there are free space for STA
			// do some algorithms to re-alloc STA into free space
			realloc_RATid(priv);
		}

#ifdef TX_EARLY_MODE
		if (GET_TX_EARLY_MODE) {
			if (!GET_EM_SWQ_ENABLE) {
				if (priv->pshare->em_tx_byte_cnt > EM_TP_UP_BOUND) 
					priv->pshare->reach_tx_limit_cnt++;				
				else					
					priv->pshare->reach_tx_limit_cnt = 0;	

				if (priv->pshare->txop_enlarge && priv->pshare->reach_tx_limit_cnt >= WAIT_TP_TIME) {			
					GET_EM_SWQ_ENABLE = 1;			
					priv->pshare->reach_tx_limit_cnt = 0;				
					enable_em(priv);			
				}
			}
			else {			
				if (priv->pshare->em_tx_byte_cnt < EM_TP_LOW_BOUND)
					priv->pshare->reach_tx_limit_cnt++;				
				else					
					priv->pshare->reach_tx_limit_cnt = 0;	

				if (!priv->pshare->txop_enlarge || priv->pshare->reach_tx_limit_cnt >= WAIT_TP_TIME) {
					GET_EM_SWQ_ENABLE = 0;
					priv->pshare->reach_tx_limit_cnt = 0;
					disable_em(priv);
				}
			}
		}		
#endif

		priv->pshare->phw->LowestInitRate	= _NSS4_MCS9_RATE_;
	}

#ifdef USB_PKT_RATE_CTRL_SUPPORT
	usbPkt_timer_handler(priv);
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	if (CHIP_VER_92X_SERIES(priv))
	{
		if (priv->pshare->rf_ft_var.auto_rts_rate)
			SelectLowestInitRate(priv);
	}
#endif	
#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv) && GET_VXD_PRIV(priv) &&
			netif_running(GET_VXD_PRIV(priv)->dev)) {
		SMP_UNLOCK(flags);
		rtl8192cd_expire_timer((unsigned long)GET_VXD_PRIV(priv));
		SMP_LOCK(flags);
	}
#endif


	if((GET_CHIP_VER(priv) != VERSION_8812E) && (GET_CHIP_VER(priv) != VERSION_8881A))
#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)) 
#endif
	{
		unsigned int tmp_d2c = RTL_R32(0xd2c);
		unsigned char intel_sta_connected = 0;

		if(is_intel_connected(priv))
			intel_sta_connected = 1;

#ifdef MBSSID
//		SMP_UNLOCK(flags);
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i])) {
					if(is_intel_connected(priv->pvap_priv[i])) {
						intel_sta_connected = 1;
						break;
					}
				}	
			}
		}
//		SMP_LOCK(flags);		
#endif

		if(!intel_sta_connected)
		if((tmp_d2c & BIT(11)) == 0)
		{
			tmp_d2c = tmp_d2c | BIT(11);
			RTL_W32(0xd2c, tmp_d2c);
#if 0	//eric-8814 ??
			tmp_d2c = RTL_R32(0xd2c);
			if(tmp_d2c & BIT(11))
				printk("No Intel STA, BIT(11) of 0xd2c = %d, 0xd2c = 0x%x\n", 1, tmp_d2c);
			else
				printk("No Intel STA, BIT(11) of 0xd2c = %d, 0xd2c = 0x%x\n", 0, tmp_d2c);
#endif
		}
	}

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)) {
		SMP_UNLOCK(flags);
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i])) {
					rtl8192cd_expire_timer((unsigned long)priv->pvap_priv[i]);
                    
                    if(priv->pvap_priv[i]->pmib->miscEntry.func_off==0){
                        somevapopened=1;
                    }
                }
			}
		}
		SMP_LOCK(flags);
	}

#endif

#ifdef MBSSID
    if (IS_ROOT_INTERFACE(priv)) 
#endif    
    {  
        if (priv->pmib->miscEntry.func_off) {   // func_off =1        

			if(somevapopened==0){	// not any VAP be opened and func_off not be enabled
			  /*minimum prohibit time[19:8] to 1*/ 
			  RTL_W32(TBTT_PROHIBIT, (RTL_R32(TBTT_PROHIBIT)&0xfff000ff) | 0x100);			  
			} else {
				RTL_W32(TBTT_PROHIBIT, (RTL_R32(TBTT_PROHIBIT)&0xfff000ff) | 0x1df00);	 
			}
			if(!IS_HAL_CHIP(priv))
            if (!priv->func_off_already) {
                RTL_W8(REG_MBSSID_CTRL, RTL_R8(REG_MBSSID_CTRL) & ~(BIT(0)));
                priv->func_off_already = 1;
            }
        }
        else {   // func_off =0
            if (priv->func_off_already) {
                RTL_W8(REG_MBSSID_CTRL, RTL_R8(REG_MBSSID_CTRL) | BIT(0));
                priv->func_off_already = 0;
            }
           
        }

    }
#ifdef MBSSID
    else
    if(IS_VAP_INTERFACE(priv))
    {  
		if (priv->pmib->miscEntry.func_off) {
			if(!IS_HAL_CHIP(priv))
			if (!priv->func_off_already) {
				RTL_W8(REG_MBSSID_CTRL, RTL_R8(REG_MBSSID_CTRL) & ~(1 << priv->vap_init_seq));
				priv->func_off_already = 1;
			}
		}
		else {
			if (priv->func_off_already) {
				RTL_W8(REG_MBSSID_CTRL, RTL_R8(REG_MBSSID_CTRL) | (1 << priv->vap_init_seq));
				priv->func_off_already = 0;
			}
		}
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef MP_TEST
		if (!((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))
#endif
		{
#ifdef INTERFERENCE_CONTROL
			if (priv->pshare->rf_ft_var.rssi_dump && (priv->assoc_num == 0)) {
#ifdef CONFIG_RTL_88E_SUPPORT
				if (GET_CHIP_VER(priv) == VERSION_8188E)
					panic_printk("(FA %x ", RTL_R8(0xc50));
				else
#endif
					panic_printk("(FA %x,%x ", RTL_R8(0xc50), RTL_R8(0xc58));

				panic_printk("%d, %d)\n", priv->pshare->ofdm_FA_total_cnt, priv->pshare->cck_FA_cnt);
			}
#endif

#ifdef BEAMFORMING_SUPPORT
	if (((priv->up_time % 3) == 0) && (priv->pmib->dot11RFEntry.txbfer == 1) &&
		((GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8192E) || GET_CHIP_VER(priv) == VERSION_8814A) 	)
	{
		DynamicSelect2STA(priv);
	}
#endif
#ifdef USE_OUT_SRC
		if(!IS_OUTSRC_CHIP(priv))
#endif
		{
			if ( ((OPMODE & WIFI_AP_STATE)?(!(priv->assoc_num)):(!(OPMODE & WIFI_ASOC_STATE)))
#ifdef UNIVERSAL_REPEATER
				&& !(GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE)
#endif
			)
				priv->pshare->rf_ft_var.bLinked = FALSE;
			else
				priv->pshare->rf_ft_var.bLinked = TRUE;

			rtl8192cd_CheckAdaptivity(priv);
		}

#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
			if(IS_OUTSRC_CHIP(priv))
#endif
			{
				int idx = 0, link=0;
				struct stat_info* pEntry = findNextSTA(priv, &idx);
				while(pEntry) {
					if(pEntry && pEntry->expire_to) {
						link=1;
						break;
					}
					pEntry = findNextSTA(priv, &idx);
				};
				ODM_CmnInfoUpdate(ODMPTR, ODM_CMNINFO_LINK, link );
				ODM_CmnInfoUpdate(ODMPTR, ODM_CMNINFO_RSSI_MIN, priv->pshare->rssi_min);
#ifdef UNIVERSAL_REPEATER
				link = IS_DRV_OPEN(GET_VXD_PRIV(priv))
					&& (GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE);
				ODM_CmnInfoUpdate(ODMPTR, ODM_CMNINFO_VXD_LINK, link);
#endif			
				if(priv->pshare->rf_ft_var.dig_enable && priv->pshare->DIG_on)
					ODMPTR->DM_DigTable.bStopDIG = FALSE;
				else
					ODMPTR->DM_DigTable.bStopDIG = TRUE;
				if(priv->pshare->rf_ft_var.adaptivity_enable)
					ODM_CmnInfoUpdate(ODMPTR, ODM_CMNINFO_ABILITY, ODMPTR->SupportAbility | ODM_BB_ADAPTIVITY);
				else
					ODM_CmnInfoUpdate(ODMPTR, ODM_CMNINFO_ABILITY, ODMPTR->SupportAbility & (~ ODM_BB_ADAPTIVITY));
				
				ODM_DMWatchdog(ODMPTR);
				IotEngine(priv);
			}
#endif

#ifdef CONFIG_WLAN_HAL
            if (IS_HAL_CHIP(priv)) {
                //HAL timer callback entry point
                GET_HAL_INTERFACE(priv)->Timer1SecHandler(priv);
            }
#endif //CONFIG_WLAN_HAL

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
if(!IS_OUTSRC_CHIP(priv))
#endif
{
#if	defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE) 
if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A)||(GET_CHIP_VER(priv)== VERSION_8814A)){
}
else
#endif
{
			if (priv->up_time % 2) {
#ifdef CONFIG_RTL_NEW_AUTOCH
				if( priv->auto_channel ==0 || priv->auto_channel ==2 )
#endif
				FA_statistic(priv);

				if ((priv->up_time > 5) && priv->pshare->rf_ft_var.dig_enable)
					DIG_process(priv);
			}


			if (priv->pshare->rf_ft_var.auto_rts_rate) {
				priv->pshare->phw->RTSInitRate_Candidate = 
					get_rate_index_from_ieee_value(find_rts_rate(priv, priv->pshare->phw->LowestInitRate,
					priv->pmib->dot11ErpInfo.protection));
				
				if (priv->pshare->phw->RTSInitRate_Candidate != priv->pshare->phw->RTSInitRate) {
					priv->pshare->phw->RTSInitRate = priv->pshare->phw->RTSInitRate_Candidate;				
					RTL_W8(INIRTS_RATE_SEL, priv->pshare->phw->RTSInitRate);
				}
			}
			
}

#ifdef SW_ANT_SWITCH
			if ((SW_DIV_ENABLE)  && (priv->up_time % 4==1))
				dm_SW_AntennaSwitch(priv, SWAW_STEP_PEAK);
#endif
}
#endif

#ifdef CONFIG_1RCCA_RF_POWER_SAVING
			if (priv->pshare->rf_ft_var.one_path_cca_ps == 2) {
				if (priv->pshare->rssi_min == 0xff) {	// No Link
					one_path_cca_power_save(priv, 1);
				} else {
					if (priv->pshare->rssi_min > priv->pshare->rf_ft_var.one_path_cca_ps_thd)
						one_path_cca_power_save(priv, 1);
					else
						one_path_cca_power_save(priv, 0);
				}
			}
#endif // CONFIG_1RCCA_RF_POWER_SAVING
		}

		if (priv->pmib->dot11RFEntry.ther && priv->pshare->rf_ft_var.tpt_period)
			TXPowerTracking(priv);

#if defined(WIFI_11N_2040_COEXIST_EXT)
	if((OPMODE & WIFI_AP_STATE))
		checkBandwidth(priv);
#endif	

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
if(!IS_OUTSRC_CHIP(priv))
#endif
{
		
#ifdef HIGH_POWER_EXT_PA
		if(CHIP_VER_92X_SERIES(priv)) {
			if((priv->pshare->rf_ft_var.use_ext_pa) && (priv->pshare->rf_ft_var.tx_pwr_ctrl))
				tx_power_control(priv);
		}
#endif		
		IOT_engine(priv);
		rxBB_dm(priv);

}
#endif

#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
		if(IS_OUTSRC_CHIP(priv))
#endif
		{
			if ((priv->up_time % 3) == 1) {
				if (priv->pshare->rssi_min != 0xff) {
					if (priv->pshare->rf_ft_var.nbi_filter_enable) 
						check_NBI_by_rssi(priv, priv->pshare->rssi_min);
				}
			}					
		}
#endif

#if (CONFIG_WLAN_NOT_HAL_EXIST == 1)
		if (CHIP_VER_92X_SERIES(priv)) {
#ifdef CONFIG_PCI_HCI
			if (priv->pshare->txpause_pstat != NULL) {
				if (TSF_DIFF(jiffies, priv->pshare->txpause_time) > RTL_SECONDS_TO_JIFFIES(1)) {
					RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) & 0xe0);
					priv->pshare->txpause_pstat = NULL;
				}
			}
#endif
		}
#endif // CONFIG_WLAN_NOT_HAL_EXIST

	}

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
	if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E)) {
		struct list_head	*plist, *phead;
		struct stat_info	*pstat;
#if defined(SMP_SYNC) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
		unsigned long local_flags = 0;
#endif
		phead = &priv->sleep_list;

		SMP_LOCK_SLEEP_LIST(local_flags);

		plist = phead->next;
		while(plist != phead)
		{
			pstat = list_entry(plist, struct stat_info, sleep_list);
			plist = plist->next;

			if (pstat->sta_in_firmware == 1) {
				if (pstat->txpause_flag && (TSF_DIFF(jiffies, pstat->txpause_time) > RTL_SECONDS_TO_JIFFIES(1))) {
#ifdef CONFIG_RTL_8812_SUPPORT
					if(GET_CHIP_VER(priv)==VERSION_8812E)
						RTL8812_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
					else
#endif
					{
#ifdef CONFIG_RTL_88E_SUPPORT
						RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
#endif
					}
					pstat->txpause_time = 0;
					pstat->txpause_flag = 0;
				}
			}
		}

		SMP_UNLOCK_SLEEP_LIST(local_flags);
	} else
#endif
#ifdef CONFIG_WLAN_HAL
	if(IS_HAL_CHIP(priv))
	{
		struct list_head	*plist, *phead;
		struct stat_info	*pstat;
#if defined(SMP_SYNC) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
		unsigned long local_flags = 0;
#endif
		phead = &priv->sleep_list;

		SMP_LOCK_SLEEP_LIST(local_flags);

		plist = phead->next;
		while(plist != phead)
		{
			pstat = list_entry(plist, struct stat_info, sleep_list);
			plist = plist->next;

			if (pstat->txpause_flag && (TSF_DIFF(jiffies, pstat->txpause_time) > RTL_SECONDS_TO_JIFFIES(1))) {
				DEBUG_WARN("%s %d expire timeout, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
				GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstat));  
				pstat->txpause_flag = 0;
			}
		}

		SMP_UNLOCK_SLEEP_LIST(local_flags);
	} else
#endif // CONFIG_WLAN_HAL
	{}

		
#if defined(TXREPORT)
	if ( CHIP_VER_92X_SERIES(priv) ||(GET_CHIP_VER(priv)==VERSION_8812E)) {
		

		if (!IS_TEST_CHIP(priv))
		#ifdef MP_TEST
		if (!((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))
		#endif
		#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
		#endif
		{

#if defined(DETECT_STA_EXISTANCE) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
			if(CHIP_VER_92X_SERIES(priv))
			LeavingSTA_RLCheck(priv);
#endif
			if (!(priv->up_time%priv->pmib->staDetectInfo.txRprDetectPeriod) && (priv->pshare->sta_query_idx==-1)) {
				priv->pshare->sta_query_idx = 0;
#if defined(CONFIG_RTL_8812_SUPPORT)
				if((GET_CHIP_VER(priv)==VERSION_8812E)){
					requestTxReport_8812(priv);
				}
#endif
				if(CHIP_VER_92X_SERIES(priv)){
					requestTxReport(priv);
				}
			}
		}
	}
#endif

#ifdef TXREPORT
#ifdef CONFIG_WLAN_HAL
      	if(IS_HAL_CHIP(priv)) {
#ifdef MP_TEST
			if (!((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))
#endif
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (IS_ROOT_INTERFACE(priv))
#endif
			if ( priv->pshare->sta_query_idx == -1) {
				priv->pshare->sta_query_idx = 0;
				requestTxReport88XX(priv);
			}
        }
#endif
#endif

#ifdef CONFIG_IEEE80211W
    {
	struct stat_info *pstat;
	struct list_head *phead, *plist;	

	if(priv->pmib->dot11StationConfigEntry.pmftest == 1) { // for PMF case 4.4, protect broadcast mgmt frame by BIP
		PMFDEBUG("%s(%d) issue broadcast deauth\n", __FUNCTION__, __LINE__);
		memcpy(priv->pmib->dot11StationConfigEntry.deauth_mac,"\xff\xff\xff\xff\xff\xff",6);
		issue_deauth(priv, priv->pmib->dot11StationConfigEntry.deauth_mac, _RSON_UNSPECIFIED_);

		phead = &priv->asoc_list;
		if (netif_running(priv->dev) && !list_empty(phead)) {
			plist = phead->next;
			while (plist != phead) {
				pstat = list_entry(plist, struct stat_info, asoc_list);
				PMFDEBUG("pstat=%02x%02x%02x%02x%02x%02x\n", pstat->hwaddr[0]
																, pstat->hwaddr[1]
																, pstat->hwaddr[2]
																, pstat->hwaddr[3]
																, pstat->hwaddr[4]
																, pstat->hwaddr[5]);
				plist = plist->next;
				del_station(priv,pstat);
				
			}
		}
		priv->pmib->dot11StationConfigEntry.pmftest = 0;
	} else if(priv->pmib->dot11StationConfigEntry.pmftest == 2) {
		PMFDEBUG("%s(%d) issue unicast deauth\n", __FUNCTION__, __LINE__);
		pstat = get_stainfo(priv, priv->pmib->dot11StationConfigEntry.deauth_mac);
		if(!pstat) {
			PMFDEBUG("%s(%d) no associated STA (%02x%02x%02x%02x%02x%02x)\n", __FUNCTION__, __LINE__
										,priv->pmib->dot11StationConfigEntry.deauth_mac[0]
										,priv->pmib->dot11StationConfigEntry.deauth_mac[1]
										,priv->pmib->dot11StationConfigEntry.deauth_mac[2]
										,priv->pmib->dot11StationConfigEntry.deauth_mac[3]
										,priv->pmib->dot11StationConfigEntry.deauth_mac[4]
										,priv->pmib->dot11StationConfigEntry.deauth_mac[5]);
		} else {
			issue_deauth(priv, priv->pmib->dot11StationConfigEntry.deauth_mac, _RSON_UNSPECIFIED_);
			del_station(priv,pstat);
		}
		priv->pmib->dot11StationConfigEntry.pmftest = 0;
	}
#ifdef CONFIG_IEEE80211W_CLI_DEBUG
else if(priv->pmib->dot11StationConfigEntry.pmftest == 3){			// CONFIG_IEEE80211W_CLI
		panic_printk("%s(%d) issue unicast disassoc\n", __FUNCTION__, __LINE__);
		pstat = get_stainfo(priv, priv->pmib->dot11StationConfigEntry.deauth_mac);
		if(!pstat) {
			panic_printk("%s(%d) no associated STA (%02x%02x%02x%02x%02x%02x)\n", __FUNCTION__, __LINE__
										,priv->pmib->dot11StationConfigEntry.deauth_mac[0]
										,priv->pmib->dot11StationConfigEntry.deauth_mac[1]
										,priv->pmib->dot11StationConfigEntry.deauth_mac[2]
										,priv->pmib->dot11StationConfigEntry.deauth_mac[3]
										,priv->pmib->dot11StationConfigEntry.deauth_mac[4]
										,priv->pmib->dot11StationConfigEntry.deauth_mac[5]);
		} else {
			issue_disassoc(priv, priv->pmib->dot11StationConfigEntry.deauth_mac, _RSON_UNSPECIFIED_);
			del_station(priv,pstat);
		}
		priv->pmib->dot11StationConfigEntry.pmftest = 0;
	}else if(priv->pmib->dot11StationConfigEntry.pmftest == 4){			// CONFIG_IEEE80211W_CLI
		panic_printk("%s(%d) issue SA Request\n", __FUNCTION__, __LINE__);
		pstat = get_stainfo(priv, priv->pmib->dot11StationConfigEntry.sa_req_mac);
		if(!pstat) {
			panic_printk("%s(%d) no associated STA (%02x%02x%02x%02x%02x%02x)\n", __FUNCTION__, __LINE__
										,priv->pmib->dot11StationConfigEntry.sa_req_mac[0]
										,priv->pmib->dot11StationConfigEntry.sa_req_mac[1]
										,priv->pmib->dot11StationConfigEntry.sa_req_mac[2]
										,priv->pmib->dot11StationConfigEntry.sa_req_mac[3]
										,priv->pmib->dot11StationConfigEntry.sa_req_mac[4]
										,priv->pmib->dot11StationConfigEntry.sa_req_mac[5]);
		} else {
				if(pstat->sa_query_count == 0) {
					panic_printk("sa_query_end=%lu, sa_query_start=%lu\n", pstat->sa_query_end, pstat->sa_query_start);
					pstat->sa_query_count++;
					issue_SA_Query_Req(priv->dev,priv->pmib->dot11StationConfigEntry.sa_req_mac);
					
					panic_printk("%s(%d), settimer, %x\n", __FUNCTION__, __LINE__, &pstat->SA_timer);
				
					if(timer_pending(&pstat->SA_timer))
						del_timer(&pstat->SA_timer);

					pstat->SA_timer.data = (unsigned long) pstat;
					pstat->SA_timer.function = rtl8192cd_sa_query_timer;
					mod_timer(&pstat->SA_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(SA_QUERY_RETRY_TO));				
					panic_printk("%s(%d), settimer end\n", __FUNCTION__, __LINE__);
				}					
		}
		priv->pmib->dot11StationConfigEntry.pmftest = 0;
	}else if(priv->pmib->dot11StationConfigEntry.pmftest == 5){ // for PMF case 4.4, protect broadcast mgmt frame by BIP
		panic_printk("%s(%d) issue broadcast diassoc\n", __FUNCTION__, __LINE__);			//CONFIG_IEEE80211W_CLI
		memcpy(priv->pmib->dot11StationConfigEntry.deauth_mac,"\xff\xff\xff\xff\xff\xff",6);
		issue_disassoc(priv, priv->pmib->dot11StationConfigEntry.deauth_mac, _RSON_UNSPECIFIED_);

		phead = &priv->asoc_list;
		if (netif_running(priv->dev) && !list_empty(phead)) {
			plist = phead->next;
			while (plist != phead) {
				pstat = list_entry(plist, struct stat_info, asoc_list);
				panic_printk("pstat=%02x%02x%02x%02x%02x%02x\n", pstat->hwaddr[0]
																, pstat->hwaddr[1]
																, pstat->hwaddr[2]
																, pstat->hwaddr[3]
																, pstat->hwaddr[4]
																, pstat->hwaddr[5]);
				plist = plist->next;
				del_station(priv,pstat);
				
			}
		}
		priv->pmib->dot11StationConfigEntry.pmftest = 0;
	}
#endif
    }
#endif //CONFIG_IEEE80211W

#ifdef SW_TX_QUEUE
#ifdef HS2_SUPPORT
	{
		unsigned char zeromac[6]={0x00,0x00,0x00,0x00,0x00,0x00};

		if (memcmp(priv->pmib->hs2Entry.sta_mac, zeromac, 6))
		{
		    DOT11_HS2_TSM_REQ tsmreq;
		    HS2_DEBUG_INFO("send bss tx mgmt req to STA:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n", 
		        priv->pmib->hs2Entry.sta_mac[0], priv->pmib->hs2Entry.sta_mac[1],
		        priv->pmib->hs2Entry.sta_mac[2], priv->pmib->hs2Entry.sta_mac[3],
		        priv->pmib->hs2Entry.sta_mac[4], priv->pmib->hs2Entry.sta_mac[5]);  
		    memcpy(tsmreq.MACAddr, priv->pmib->hs2Entry.sta_mac, 6);
		    tsmreq.Req_mode = priv->pmib->hs2Entry.reqmode;
		    if (tsmreq.Req_mode & 0x08)
		    {
		        tsmreq.term_len = 12;
		        tsmreq.terminal_dur[0] = 4;
		        tsmreq.terminal_dur[1] = 10;
		        tsmreq.terminal_dur[10] = 0x01;
		        tsmreq.terminal_dur[11] = 0;
		    }
		    else
		        tsmreq.term_len = 0;

		    tsmreq.Dialog_token = priv->pmib->hs2Entry.disassoc_timer & 0xff;
		    tsmreq.Disassoc_timer = priv->pmib->hs2Entry.disassoc_timer;
		    tsmreq.Validity_intval = priv->pmib->hs2Entry.validity_intval;  
		    tsmreq.url_len = strlen(priv->pmib->hs2Entry.session_url);
		    memcpy(tsmreq.Session_url, priv->pmib->hs2Entry.session_url, tsmreq.url_len);
		    tsmreq.list_len = 0;    

		    issue_BSS_TSM_req(priv, &tsmreq);
		    memset(priv->pmib->hs2Entry.sta_mac, 0, 6);
		}
	}

#ifdef HS2_CLIENT_TEST
	//printk("swq_dbg=%d\n",priv->pshare->rf_ft_var.swq_dbg);
	if (priv->pshare->rf_ft_var.swq_dbg != 0)
	{
		DOT11_HS2_GAS_REQ gas_req;
		unsigned short query_id;
				
		if ((OPMODE & WIFI_ASOC_STATE) == 0)
		{
		    priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
		    SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		    SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
		    delay_us(250);
		}

		if ((priv->pshare->rf_ft_var.swq_dbg >= 30) && (priv->pshare->rf_ft_var.swq_dbg <= 39))
		{
		    unsigned char tmpda[]={0xff,0xff,0xff,0xff,0xff,0xff};
		    issue_probereq(priv,"",0, tmpda);
		    goto hs_break;
		}
		if (priv->pshare->rf_ft_var.swq_dbg == 20)
		{
		    unsigned char list[]={0x03,0x01,0x30,0x03,0x01,0x50};
		    issue_BSS_TSM_query(priv, list, sizeof(list));
		    priv->pshare->rf_ft_var.swq_dbg = 0;
		    goto hs_break;
		}



		// query_id is identified by the issue_GASreq function
		// refer to Table 8-184
		//capability List  (Test Case 4.2 Proc. (a) in HS2.0 R2 Test Plan v.0.01)
		if (priv->pshare->rf_ft_var.swq_dbg == 3)
		    query_id = 257;
		//Roaming Consortium list (Test Case 4.4 Step2 in HS2.0 R2 Test Plan v.0.01)
		else if (priv->pshare->rf_ft_var.swq_dbg == 4)
		    query_id = 261;
		//venue name  (Test Case 4.4 Step8 in HS2.0 R2 Test Plan v.0.01)    
		else if (priv->pshare->rf_ft_var.swq_dbg == 5)
		    query_id = 258;
		// Network Authentication Type Information  (Test Case 4.4 Step10 in HS2.0 R2 Test Plan v.0.01) 
		else if (priv->pshare->rf_ft_var.swq_dbg == 6)
		    query_id = 260; 
		//IP Address Type Availability Information  (Test Case 4.4 Step12 in HS2.0 R2 Test Plan v.0.01)
		else if (priv->pshare->rf_ft_var.swq_dbg == 7)
		    query_id = 262;
		//combine  nai_list+3gpp+domain_list  (Test Case 4.4 Step4 in HS2.0 R2 Test Plan v.0.01)
		else if (priv->pshare->rf_ft_var.swq_dbg == 8)
		    query_id = 10000;
		//HS Capability list and Operator Friendly Name  (Test Case 4.4 Step6 in HS2.0 R2 Test Plan v.0.01)
		else if (priv->pshare->rf_ft_var.swq_dbg == 9)
		    query_id = 501; 
		//HS WAN Metrics and Connection Lists  (Test Case 4.4 Step14 in HS2.0 R2 Test Plan v.0.01)
		else if (priv->pshare->rf_ft_var.swq_dbg == 10)
		    query_id = 502; 
		//hs wan+hs no support type 
		else if (priv->pshare->rf_ft_var.swq_dbg == 18)
		    query_id = 505;
		//hs home realm query, cnt=0
		else if (priv->pshare->rf_ft_var.swq_dbg == 19)
		    query_id = 506;
		//hs operating class
		else if (priv->pshare->rf_ft_var.swq_dbg == 21)
		    query_id = 507;
		//hs home realm query  (Test Case 4.4 Step16 in HS2.0 R2 Test Plan v.0.01)
		else if (priv->pshare->rf_ft_var.swq_dbg == 11)
		    query_id = 503; 
		//hs  2 home realm query
		else if (priv->pshare->rf_ft_var.swq_dbg == 17)
		    query_id = 504;
		//advt protocol error   
		else if (priv->pshare->rf_ft_var.swq_dbg == 12)
		    query_id = 262; 
		// Query element (qid = roaming list) + HS2 WAN Metrics element  + HS2 Operating Class Indication element
		else if (priv->pshare->rf_ft_var.swq_dbg == 13)
		    query_id = 270; 
		//roaming list    (qid = roaming list) + HS2 element (subtype = reserved) + HS Query List 
		else if (priv->pshare->rf_ft_var.swq_dbg == 14)
		    query_id = 271;     
		//nai list
		else if (priv->pshare->rf_ft_var.swq_dbg == 15)
		    query_id = 263;
		//capa+comback request
		else if (priv->pshare->rf_ft_var.swq_dbg == 16)
		{
		    query_id = 257;
		    testflg = 1;
		}
		else if (priv->pshare->rf_ft_var.swq_dbg == 22) // for Test Case 4.3 Step 4
		    query_id = 272;
		else if (priv->pshare->rf_ft_var.swq_dbg == 23) // for Test Case 4.3 Step 6
		    query_id = 0;
		else if (priv->pshare->rf_ft_var.swq_dbg == 42) { // ICON Request
		    query_id = 508;
		}
		else if (priv->pshare->rf_ft_var.swq_dbg == 43) { // Online Sign-Up Providers list
		    query_id = 509;
		}
		else
			goto hs_break;
		    
		gas_req.Dialog_token = 5;
		issue_GASreq(priv, &gas_req, query_id);
		HS2_DEBUG_INFO("send gas req\n");
		priv->pshare->rf_ft_var.swq_dbg = 0;
	} 
hs_break:
#endif
#endif

#ifdef SW_TX_QUEUE_SMALL_PACKET_CHECK
// add for small udp packet(88B) test with veriwave tool (let sw tx queue start quickly after first round test)
// the interval between every round test is about 10sec
#if 1
	if ((priv->assoc_num > 1) && (AMPDU_ENABLE) && (priv->pshare->swq_txmac_chg >= priv->pshare->rf_ft_var.swq_en_highthd))
		priv->pshare->swq_boost_delay = 0;		
	else
		priv->pshare->swq_boost_delay++;

	if (priv->pshare->swq_boost_delay >= 10)
		priv->pshare->swq_boost_delay = 10;
#else
	priv->pshare->swq_boost_delay = 0;
#endif

#else
	//for debug 
	//if (priv->pshare->rf_ft_var.swq_dbg)
	//	printk("sw cnt:%d:%d,0x%x\n", priv->swq_txmac_chg,priv->swq_en, RTL_R32(EDCA_BE_PARA));

	if (IS_ROOT_INTERFACE(priv))
        priv->pshare->swq_txmac_chg = 0;
#endif		
#endif

#if defined(CLIENT_MODE) && defined(WIFI_11N_2040_COEXIST)
//#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
//	if (IS_ROOT_INTERFACE(priv))
//#endif
	{
		if ((OPMODE & (WIFI_STATION_STATE|WIFI_ASOC_STATE)) 
			&& (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N|WIRELESS_11G))
			&& priv->pmib->dot11nConfigEntry.dot11nCoexist && priv->coexist_connection &&
			(!(priv->up_time % 85) || to_issue_coexist_mgt)) {
			
			int kthroughput = 0;
			kthroughput = ((priv->ext_stats.tx_avarage + priv->ext_stats.rx_avarage) * 8) / 1024;	// k bps		
			//panic_printk("throughput = %d Kbps\n",kthroughput);			
			if(!(priv->up_time % 85) && (kthroughput < 1024)) {
				
				priv->ss_ssidlen = 0;
				DEBUG_INFO("start_clnt_ss, trigger by %s, ss_ssidlen=0\n", (char *)__FUNCTION__);
				priv->ss_req_ongoing = 1;
				start_clnt_ss(priv);
				to_issue_coexist_mgt++;
			}

			if (to_issue_coexist_mgt && !(OPMODE & WIFI_SITE_MONITOR) && 
				(priv->bg_ap_timeout || priv->intolerant_timeout)) {
				issue_coexist_mgt(priv);
				to_issue_coexist_mgt = 0;
			}
		}
	}
#endif

#ifdef RADIUS_ACCOUNTING
	//brian add for accounting
	if(ACCT_FUN)
	{
		cal_statistics_acct(priv);
		expire_sta_for_radiusacct(priv);
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		switch(priv->pshare->intel_active_sta) {
		case 0:
		case 1:
			priv->pshare->intel_rty_lmt =  0x30; /* 48 times */
			break;
		case 2:
			priv->pshare->intel_rty_lmt =  0x18; /* 24 times */
			break;
		default:
			priv->pshare->intel_rty_lmt =  0; /* use system default */
			break;			
		}

		priv->pshare->intel_active_sta = 0;

#ifdef SMART_REPEATER_MODE
		if (priv->pshare->switch_chan_rp && 
				((priv->pmib->dot11RFEntry.dot11channel != priv->pshare->switch_chan_rp) ||
				  (priv->pmib->dot11nConfigEntry.dot11n2ndChOffset != priv->pshare->switch_2ndchoff_rp) ||
				  (priv->pmib->dot11nConfigEntry.dot11nUse40M != priv->pshare->band_width_rp)) ) {
			DEBUG_INFO("swtich chan=%d\n",  priv->pshare->switch_chan_rp);
			switch_chan_to_vxd(priv);			
			priv->pshare->switch_chan_rp = 0;
		}
#endif		
	}

    /*for delay wsc client mode do Scan; special on repeater mode 
	   if  client mode do Scan too frequent and continous
       will cause the client(that connected with repeater's AP interface) disconnect*/
#if defined( WIFI_SIMPLE_CONFIG	) && defined(UNIVERSAL_REPEATER)
    if(IS_VXD_INTERFACE(priv) && (OPMODE & WIFI_STATION_STATE)){
	    if(priv->wsc_ss_delay){
    	    //STADEBUG("wsc_ss_delay remain[%d]\n",priv->wsc_ss_delay);
	        priv->wsc_ss_delay--;
    	}
	}
#endif    

#if defined(CONFIG_RTL_SIMPLE_CONFIG)
	if(priv->pmib->dot11StationConfigEntry.sc_enabled == 1) 
	{
		priv->simple_config_time++;
		if(priv->simple_config_status == 0)
		{
			rtk_sc_start_simple_config(priv);
		}
		else if(priv->simple_config_status == 2)
		{
			if(priv->simple_config_time > priv->pmib->dot11StationConfigEntry.sc_get_sync_time)
			{
				rtk_sc_restart_simple_config(priv);
			}
		}
		else if(priv->simple_config_status>=3 && priv->simple_config_status<=5)
		{
			if (OPMODE & WIFI_ASOC_STATE)
			{
				struct stat_info *pstat;
				pstat = get_stainfo(priv, priv->pmib->dot11StationConfigEntry.dot11DesiredBssid);
				if( pstat)
				{
					if((pstat->link_time >= priv->pmib->dot11StationConfigEntry.sc_check_link_time) || (pstat->wpa_sta_info->clientHndshkDone))
					{
						priv->simple_config_status = 6;
						panic_printk("connect to %s now!\n", priv->pmib->dot11StationConfigEntry.dot11DesiredSSID);
						rtk_sc_stop_simple_config(priv);
					}
				}
			}
			else
			{
				if(priv->simple_config_time > priv->pmib->dot11StationConfigEntry.sc_connect_timeout)
				{
					rtk_sc_restart_simple_config(priv);
				}
			}
		}
		else if(priv->simple_config_status == 6)
		{
			//rtk_sc_stop_simple_config(priv);
		}
#if defined(CONFIG_RTL_SIMPLE_CONFIG_USE_WPS_BUTTON)
		if(priv->simple_config_status >= 2)
		{
			if(priv->pmib->wscEntry.wsc_enable)
				priv->pmib->wscEntry.wsc_enable = 0;
		}
#endif

		if(priv->pmib->dot11StationConfigEntry.sc_duration_time>0)
		{
			priv->pmib->dot11StationConfigEntry.sc_duration_time--;
			if(priv->pmib->dot11StationConfigEntry.sc_duration_time == 0)
			{
				rtk_sc_stop_simple_config(priv);
			}
		}
	}
	else
	{
		if(priv->simple_config_status != 0)
		{
			priv->simple_config_status = 0;
		}
	}
#endif
#ifdef BT_COEXIST
	if(GET_CHIP_VER(priv) == VERSION_8192E		
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		&& IS_ROOT_INTERFACE(priv)
#endif
			){
		/* 
		*	BT coexist dynamic mechanism 
		*/
		if(priv->up_time % 1 == 0){
			bt_coex_dm(priv); 
		}
		/* 
		* Query BT info 
		*/
		if(priv->up_time % 1 == 0){ 
			u1Byte		H2CCommand[1] = {0};
			H2CCommand[0] |= BIT0;
			FillH2CCmd88XX(priv, H2C_88XX_BT_INFO, 1, H2CCommand);
		}
#ifdef BT_COEXIST_DEBUG
		printk("--------------------------------------------------------------------\n");
		printk("\n0x778:%8x\n",PHY_QueryBBReg(priv, 0x778, bMaskDWord));
		printk("0x92c:%8x,",PHY_QueryBBReg(priv, 0x92c, bMaskDWord));
		printk("		0x930:%8x\n",PHY_QueryBBReg(priv, 0x930, bMaskDWord));
		printk("0x40 :%8x,",PHY_QueryBBReg(priv, 0x40,	bMaskDWord));
		printk("		0x4f :%8x\n",PHY_QueryBBReg(priv, 0x4c,  0xff000000));
		printk("0x550:%8x,",PHY_QueryBBReg(priv, 0x550, bMaskDWord));
		printk("		0x522:%8x\n",PHY_QueryBBReg(priv, 0x520, 0x00ff0000));
		printk("0xc50:%8x,",PHY_QueryBBReg(priv, 0xc50, bMaskDWord));
		printk("		0xc58:%8x\n",PHY_QueryBBReg(priv, 0xc58, bMaskDWord));
		printk("0x6c0:%8x,",PHY_QueryBBReg(priv, 0x6c0, bMaskDWord));
		printk("		0x6c4:%8x,",PHY_QueryBBReg(priv, 0x6c4, bMaskDWord));
		printk("		0x6c8:%8x,",PHY_QueryBBReg(priv, 0x6c8, bMaskDWord));
		printk("		0x6cc:%8x\n",PHY_QueryBBReg(priv, 0x6cc, bMaskDWord));
		printk("0x770: hp rx:%d,",PHY_QueryBBReg(priv, 0x770, bMaskHWord));
		printk("		tx:%d\n",PHY_QueryBBReg(priv, 0x770, bMaskLWord));
		printk("0x774: lp rx:%d,",PHY_QueryBBReg(priv, 0x774, bMaskHWord));
		printk("	tx:%d\n",PHY_QueryBBReg(priv, 0x774, bMaskLWord));
		printk("--------------------------------------------------------------------\n");
		PHY_SetBBReg(priv, 0x76c, 0x00ff0000, 0x0c);
#endif
		PHY_SetBBReg(priv, 0x76c, 0x00ff0000, 0x0c); /* reset BT counter*/
	}	
#endif

	RESTORE_INT(flags);
	SMP_UNLOCK(flags);
}


/*
 *	@brief	System 1 sec timer
 *
 *	@param	task_priv: priv
 *
 *	@retval	void
 */
#if defined(CONFIG_PCI_HCI)
// #define CHECK_CRYPTO
void rtl8192cd_1sec_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
#ifdef CHECK_CRYPTO
	unsigned long	flags;
#endif

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1)) {
		goto expire_timer;
	}
#endif

#ifdef CONFIG_PCI_HCI
#ifdef  CONFIG_WLAN_HAL
	if (!IS_HAL_CHIP(priv)) 
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv)!= VERSION_8812E)
#endif
	if (!(priv->up_time % 5))
	tx_stuck_fix(priv);
#endif // CONFIG_PCI_HCI

// 2009.09.08
#ifdef CHECK_HANGUP
#ifdef MP_TEST
	if (!((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))
#endif
		if (check_hangup(priv))
			return;
#endif

#ifdef CHECK_CRYPTO
    // TODO: Filen, Check code below for 8881A & 92E
	if(GET_CHIP_VER(priv) != VERSION_8192D)        
		if((RTL_R32(0x6B8) & 0x3) == 0x3) {
			DEBUG_ERR("Cyrpto checked\n");
			SAVE_INT_AND_CLI(flags);
			RTL_W8(0x522, 0x0F);
			RTL_W8(0x6B8, 0xFF);
			RTL_W8(0x101,0x0);
			RTL_W8(0x21,0x35);
			delay_us(250);
			RTL_W8(0x101,0x02);
			RTL_W8(0x522,0x0);
			RESTORE_INT(flags);
		}
#endif

	// for Rx dynamic tasklet
#if defined(CONFIG_RTL8672) || defined(NOT_RTK_BSP)
	priv->pshare->rxInt_useTsklt = TRUE;
#else
	if (priv->pshare->rxInt_data_delta > priv->pmib->miscEntry.rxInt_thrd)
		priv->pshare->rxInt_useTsklt = TRUE;
	else
		priv->pshare->rxInt_useTsklt = FALSE;
#endif
	priv->pshare->rxInt_data_delta = 0;

#ifdef PCIE_POWER_SAVING
expire_timer:
#endif

#ifdef __KERNEL__
	tasklet_schedule(&priv->pshare->oneSec_tasklet);
#else
	rtl8192cd_expire_timer((unsigned long)priv);
#endif

#ifdef P2P_SUPPORT/*cfg p2p cfg p2p*/
	if (rtk_p2p_is_enabled(priv)==PROPERTY_P2P)
		P2P_1sec_timer(priv);
#endif

#ifdef CONFIG_RTL_WLAN_DOS_FILTER
	if ((block_sta_time > 0) && (block_priv == (unsigned long)priv))
	{
		block_sta_time--;
	}
#endif

	mod_timer(&priv->expire_timer, jiffies + EXPIRE_TO);
}
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
void pre_rtl8192cd_1sec_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if ((priv->pshare->bDriverStopped) || (priv->pshare->bSurpriseRemoved)) {
		printk("[%s] bDriverStopped(%d) OR bSurpriseRemoved(%d)\n",
			__FUNCTION__, priv->pshare->bDriverStopped, priv->pshare->bSurpriseRemoved);
		return;
	}
	
	rtw_enqueue_timer_event(priv, &priv->expire_timer_event, ENQUEUE_TO_TAIL);
}

void rtl8192cd_1sec_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

#ifdef CHECK_HANGUP
#ifdef MP_TEST
	if (!((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))
#endif
		if (check_hangup(priv))
			return;
#endif
	
	rtl8192cd_expire_timer((unsigned long)priv);

#ifdef P2P_SUPPORT
	if (rtk_p2p_is_enabled(priv)==PROPERTY_P2P)
		P2P_1sec_timer(priv);
#endif
	
	mod_timer(&priv->expire_timer, jiffies + EXPIRE_TO);
}
#endif

#if !defined(__LINUX_2_6__) && !defined(__ECOS)
__IRAM_IN_865X
#endif
void pwr_state(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
#ifndef SMP_SYNC
    unsigned long	flags;
#endif
    struct stat_info *pstat;
    unsigned char	*sa, *pframe;

#ifdef HW_DETEC_POWER_STATE    
    unsigned char result,i;
#endif

    pframe = get_pframe(pfrinfo);
    sa = pfrinfo->sa;
    pstat = get_stainfo(priv, sa);

    if (pstat == (struct stat_info *)NULL)
        return;

    if (!(pstat->state & WIFI_ASOC_STATE))
        return;

    #ifdef HW_DETEC_POWER_STATE              

    // If 8814 return success
    if (RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->CheckHWMACIDResultHandler(priv,pfrinfo->macid,&result)) {
        if(HWMACID_RESULT_SUCCESS == result)    {
            // hw detect ps state change return
            return;
        }

    #ifdef HW_DETEC_POWER_STATE_PATCH
        if(HWMACID_RESULT_NOT_READY == result){
        
            printk("%s %d HW_MACID_SEARCH_NOT_READY",__FUNCTION__,__LINE__);
            
            for(i=0;i<128;i++)
            {
                priv->pshare->HWPwrStateUpdate[pstat->aid]--;
            }

            if(priv->pshare->HWPwrStateUpdate[pstat->aid] > 0)   
            {
                // hw already update return
                return;
            }        
        }
    #endif //#ifdef HW_DETEC_POWER_STATE_PATCH    
    }
    #endif //    #ifdef HW_DETEC_POWER_STATE                 

    if (GetPwrMgt(pframe))
    {
        if ((pstat->state & WIFI_SLEEP_STATE) == 0) {
            pstat->state |= WIFI_SLEEP_STATE;
#ifdef CONFIG_PCI_HCI
            if(CHIP_VER_92X_SERIES(priv))
            {
                if (pstat == priv->pshare->highTP_found_pstat) {
                    if (priv->pshare->txpause_pstat == NULL) {
                        RTL_W8(TXPAUSE, RTL_R8(TXPAUSE)|STOP_BE|STOP_BK);


                        priv->pshare->txpause_pstat = pstat;
                        priv->pshare->txpause_time = jiffies;

                    }
                }
            }
#endif // CONFIG_PCI_HCI

            if (pstat->sta_in_firmware == 1)
            {


#ifdef CONFIG_WLAN_HAL
                if(IS_HAL_CHIP(priv))
                {
                    DEBUG_WARN("%s %d client into ps, set MACID sleep AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
                    DEBUG_WARN("Pwr_state jiffies = %lu \n",jiffies);
                    GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 1, REMAP_AID(pstat)); 
                    pstat->txpause_flag = 1;
                    pstat->txpause_time = jiffies;

                } else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
                if (GET_CHIP_VER(priv)==VERSION_8812E) {
                    RTL8812_MACID_PAUSE(priv, 1, REMAP_AID(pstat));
                    pstat->txpause_flag = 1;
                    pstat->txpause_time = jiffies;
                } else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
                if (GET_CHIP_VER(priv)==VERSION_8188E) {
                    RTL8188E_MACID_PAUSE(priv, 1, REMAP_AID(pstat));
                    pstat->txpause_flag = 1;
                    pstat->txpause_time = jiffies;
                } else
#endif
                {
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)				
                    add_update_ps(priv, pstat);
#endif
                }
            }
        }

        SAVE_INT_AND_CLI(flags);
        if (wakeup_list_del(priv, pstat)) {
            DEBUG_INFO("Del fr wakeup_list %02X%02X%02X%02X%02X%02X\n", sa[0],sa[1],sa[2],sa[3],sa[4],sa[5]);
        }
        if (sleep_list_add(priv, pstat)) {
            DEBUG_INFO("Add to sleep_list %02X%02X%02X%02X%02X%02X\n", sa[0],sa[1],sa[2],sa[3],sa[4],sa[5]);
        }
        RESTORE_INT(flags);

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
        stop_sta_xmit(priv, pstat);
#endif
    } else {
        if (pstat->state & WIFI_SLEEP_STATE) {
            pstat->state &= ~(WIFI_SLEEP_STATE);

#ifdef CONFIG_PCI_HCI
            if (pstat == priv->pshare->txpause_pstat) {
#ifdef CONFIG_RTL_8812_SUPPORT
                if (GET_CHIP_VER(priv) == VERSION_8812E && (pstat->IOTPeer == HT_IOT_PEER_BROADCOM))
                RTL_W16(REG_RL_8812, priv->pshare->RL_setting);
                else
#endif
                RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) & 0xe0);
                priv->pshare->txpause_pstat = NULL;
            }			
#endif // CONFIG_PCI_HCI

            if (pstat->sta_in_firmware == 1)
            {

#ifdef CONFIG_WLAN_HAL
                if(IS_HAL_CHIP(priv))
                {
                    if (pstat->txpause_flag) {
                        DEBUG_WARN("%s %d client leave ps, set MACID sleep AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
                        DEBUG_WARN("Pwr_state jiffies = %lu diff = %lu\n",jiffies,jiffies-pstat->txpause_time);
                        GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstat));                
                        pstat->txpause_flag = 0;
                    }
                    pstat->txpause_time = 0;                
                } else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
                if (GET_CHIP_VER(priv)==VERSION_8812E) {
                    if (pstat->txpause_flag) {
                        RTL8812_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
                        pstat->txpause_flag = 0;
                    }
                    pstat->txpause_time = 0;
                } else
#endif               
#ifdef CONFIG_RTL_88E_SUPPORT
                if (GET_CHIP_VER(priv)==VERSION_8188E) {
                    if (pstat->txpause_flag) {
                        RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
                        pstat->txpause_flag = 0;
                    }
                    pstat->txpause_time = 0;
                } else
#endif
                {
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)				
                    add_update_ps(priv, pstat);
#endif
                }
            }
        }
		
        SAVE_INT_AND_CLI(flags);
        if (sleep_list_del(priv, pstat)) {
            DEBUG_INFO("Del fr sleep_list %02X%02X%02X%02X%02X%02X\n", sa[0],sa[1],sa[2],sa[3],sa[4],sa[5]);
        }
        RESTORE_INT(flags);

#if defined(CONFIG_PCI_HCI)
        if ((skb_queue_len(&pstat->dz_queue))
#ifdef WIFI_WMM
#ifdef WMM_APSD
                ||(
#ifdef CLIENT_MODE
                (OPMODE & WIFI_AP_STATE) &&
#endif
                (QOS_ENABLE) && (APSD_ENABLE) && (pstat->QosEnabled) && (pstat->apsd_pkt_buffering) &&
                ((!isFFempty(pstat->VO_dz_queue->head, pstat->VO_dz_queue->tail)) ||
                (!isFFempty(pstat->VI_dz_queue->head, pstat->VI_dz_queue->tail)) ||
                (!isFFempty(pstat->BE_dz_queue->head, pstat->BE_dz_queue->tail)) ||
                (!isFFempty(pstat->BK_dz_queue->head, pstat->BK_dz_queue->tail))))
#endif
                || (!isFFempty(pstat->MGT_dz_queue->head, pstat->MGT_dz_queue->tail))
#ifdef DZ_ADDBA_RSP
                || pstat->dz_addba.used
#endif
#endif
        ) {
            SAVE_INT_AND_CLI(flags);
            if (wakeup_list_add(priv, pstat)) {
                DEBUG_INFO("Add to wakeup_list %02X%02X%02X%02X%02X%02X\n", sa[0],sa[1],sa[2],sa[3],sa[4],sa[5]);
            }
            RESTORE_INT(flags);
        }
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
        wakeup_sta_xmit(priv, pstat);
#endif
    }

#if defined(CONFIG_WLAN_HAL) && defined(CONFIG_PCI_HCI)
	if(IS_HAL_CHIP(priv))
	    check_PS_set_HIQLMT(priv);
#endif
	return;
}

#ifdef HW_DETEC_POWER_STATE
void detect_hw_pwr_state(struct rtl8192cd_priv *priv, unsigned char macIDGroup)
{
    unsigned char i,newPwrBit,oldPwrBit;
    unsigned int  pwr;    

    switch(macIDGroup)
    {
    case 0:
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HW_PS_STATE0, (pu1Byte)&pwr);                 
    break;        
    case 1:
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HW_PS_STATE1, (pu1Byte)&pwr);   
    break;
    case 2:
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HW_PS_STATE2, (pu1Byte)&pwr);   
    break;
    case 3:
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HW_PS_STATE3, (pu1Byte)&pwr);   
    break;
    }

    // check PS change                     
    for(i=0;i<32;i++)
    {
        // getHWPwrStateHander
        //priv->pshare->HWPwrState[i+(macIDGroup<<5)] = (pwr & BIT(i) ? 1:0);

        //if(priv->pshare->HWPwroldState[i+(macIDGroup<<5)]!= priv->pshare->HWPwrState[i+(macIDGroup<<5)])
        newPwrBit= (pwr & BIT(i))>>i;
        oldPwrBit = (priv->pshare->HWPwroldState[macIDGroup] & BIT(i))>>i;
        if(newPwrBit != oldPwrBit)        
        {
#ifndef CONFIG_8814_AP_MAC_VERI     

      //      printk("[%s][%d] MACID%x HW PS0=%x Seq=%x \n",
      //                     __FUNCTION__,__LINE__,i+(macIDGroup<<5),newPwrBit,RTL_R16(0x1152));

            pwr_state_enhaced(priv,i+(macIDGroup<<5),newPwrBit);
            RTL_W8(0x1150,1);   


#endif 

#ifdef CONFIG_8814_AP_MAC_VERI
            priv->pwrStateHWCnt[i+(macIDGroup<<5)]++;
            priv->hw_seq[i+(macIDGroup<<5)] = RTL_R16(0x1152);
            priv->pwrHWState[i+(macIDGroup<<5)] = newPwrBit;
            priv->pwroldHWState[i+(macIDGroup<<5)] = pwr;

            if(priv->testResult == true)
            {
              printk("[%s][%d] MACID%x HW PS0=%x Seq=%x Cnt=%x\n",
                __FUNCTION__,__LINE__,i+(macIDGroup<<5),newPwrBit,priv->hw_seq[i+(macIDGroup<<5)],priv->pwrStateHWCnt[i+(macIDGroup<<5)]);
            }            
#endif //#ifdef CONFIG_8814_AP_MAC_VERI

        } 
    }

    // need for backup new power state
    priv->pshare->HWPwroldState[macIDGroup] = pwr;   
}

__IRAM_IN_865X
void pwr_state_enhaced(struct rtl8192cd_priv *priv, unsigned char macID, unsigned char PwrBit)
{
#ifndef SMP_SYNC
    unsigned long	flags;
#endif
    unsigned char   i;
    struct stat_info *pstat;

    if(macID > 0)
    {
        if((macID == 0x7E))
        {
            for(i=0;i<128;i++)
            {
                priv->pshare->HWPwrStateUpdate[i] = false;
            }
            return;
        }

        if((macID == 0x7F))
        {
            return;
        }        
    }

    priv->pshare->HWPwrStateUpdate[i] = true;    
    pstat = get_HW_mapping_sta(priv,macID);

    // get priv from aidarray
   
    priv = priv->pshare->aidarray[macID-1]->priv;

	if (pstat == (struct stat_info *)NULL)
	{
        printk("%s %d pstat NULL return\n",__FUNCTION__,__LINE__);
		return;
	}

    if (!(pstat->state & WIFI_ASOC_STATE))
    {
        printk("%s %d pstat NULL return\n",__FUNCTION__,__LINE__);        
        return;
    }

    if (PwrBit == 1)
    {
        if ((pstat->state & WIFI_SLEEP_STATE) == 0) {
            pstat->state |= WIFI_SLEEP_STATE;
            if (pstat->sta_in_firmware == 1)
            {
                GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 1, REMAP_AID(pstat)); 
                pstat->txpause_flag = 1;
                pstat->txpause_time = jiffies;			
            }
        }

        SAVE_INT_AND_CLI(flags);
        wakeup_list_del(priv, pstat);
        sleep_list_add(priv, pstat);
        RESTORE_INT(flags);

    } else {
        if (pstat->state & WIFI_SLEEP_STATE) {
            pstat->state &= ~(WIFI_SLEEP_STATE);

            if (pstat->sta_in_firmware == 1)
            {
                DEBUG_WARN("%s %d client leave ps, set MACID sleep AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
                DEBUG_WARN("Pwr_state jiffies = %x diff = %d\n",jiffies,jiffies-pstat->txpause_time);
                GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstat));                
                pstat->txpause_flag = 0;
                pstat->txpause_time = 0;                
            }
        }

        SAVE_INT_AND_CLI(flags);
        sleep_list_del(priv, pstat);
        RESTORE_INT(flags);

        if ((skb_queue_len(&pstat->dz_queue))
#ifdef WIFI_WMM
#ifdef WMM_APSD
                ||(
#ifdef CLIENT_MODE
                (OPMODE & WIFI_AP_STATE) &&
#endif
                (QOS_ENABLE) && (APSD_ENABLE) && (pstat->QosEnabled) && (pstat->apsd_pkt_buffering) &&
                ((!isFFempty(pstat->VO_dz_queue->head, pstat->VO_dz_queue->tail)) ||
                (!isFFempty(pstat->VI_dz_queue->head, pstat->VI_dz_queue->tail)) ||
                (!isFFempty(pstat->BE_dz_queue->head, pstat->BE_dz_queue->tail)) ||
                (!isFFempty(pstat->BK_dz_queue->head, pstat->BK_dz_queue->tail))))
#endif
                || (!isFFempty(pstat->MGT_dz_queue->head, pstat->MGT_dz_queue->tail))
#ifdef DZ_ADDBA_RSP
                || pstat->dz_addba.used
#endif
#endif
        ) {
            SAVE_INT_AND_CLI(flags);
            if (wakeup_list_add(priv, pstat)) {
                //DEBUG_INFO("Add to wakeup_list %02X%02X%02X%02X%02X%02X\n", sa[0],sa[1],sa[2],sa[3],sa[4],sa[5]);
            }
            RESTORE_INT(flags); 
        }
    }

    if(IS_HAL_CHIP(priv))
        check_PS_set_HIQLMT(priv);

    return;
}
#endif //#ifdef HW_DETEC_POWER_STATE


#ifdef CONFIG_WLAN_HAL
void check_PS_set_HIQLMT(struct rtl8192cd_priv *priv)
{
    u1Byte HiQLMTEn;
    u1Byte tmp;

    
#ifdef MBSSID
        if(priv->sleep_list.next != &priv->sleep_list)  // one client into PS mode
        {
           if (IS_ROOT_INTERFACE(priv)) {
           GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn); 
               HiQLMTEn = HiQLMTEn & (~BIT0);
           GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn); 
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&tmp);
           }         
           else {
               if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
               GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn); 
                   HiQLMTEn = HiQLMTEn & ~(BIT(priv->vap_init_seq));
               GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn); 
               GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&tmp);
               }
           }       
        }
        else {  // all client leave PS mode
           if (IS_ROOT_INTERFACE(priv)) {
           GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn); 
               HiQLMTEn = HiQLMTEn | BIT0;
           GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn); 
           GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&tmp);
           }
           else {
               if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
               GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn); 
                    HiQLMTEn = HiQLMTEn | BIT(priv->vap_init_seq);
               GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn); 
                GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&tmp);
               }
           }          
        }       
#else   
        if(priv->sleep_list.next != &priv->sleep_list)  // one client into PS mode
        {
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn); 
            HiQLMTEn = HiQLMTEn & (~BIT0);
           GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn); 
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&tmp);
        }
        else {
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn);
            HiQLMTEn = HiQLMTEn | BIT0;
        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HiQLMTEn);
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&tmp);
        }
    
#endif //#ifdef MBSSID  
    }
#endif //CONFIG_WLAN_HAL
#ifdef CONFIG_IEEE80211W

enum _ROBUST_FRAME_STATE_{
	NOT_ROBUST_MGMT = 0,
	IS_ROBUST_MGMT = 1,	
	MGMT_FRAME_MIC_ERR = 2,	
};

static int isCorrectRobustFrame(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char *pframe = get_pframe(pfrinfo);	
	unsigned char Category_field;
	struct stat_info *pstat = get_stainfo(priv, pfrinfo->sa);
#ifdef CONFIG_IEEE80211W_CLI
	unsigned char *da = GetAddr1Ptr(pframe);
#endif

	if(pframe[0] == WIFI_DEAUTH  || pframe[0] == WIFI_DISASSOC || pframe[0] == WIFI_WMM_ACTION) {
		if (GetPrivacy(pframe) && pstat
#ifdef CONFIG_IEEE80211W_CLI
		&& !IS_MCAST(da)
#endif
		) {
			unsigned char mic[8];
							
			memcpy(mic,pframe + pfrinfo->pktlen - 8, 8);			
			aesccmp_decrypt(priv, pfrinfo, 1);			
			
			if(!aesccmp_checkmic(priv, pfrinfo, mic)) {
				if(pframe[0] == WIFI_DEAUTH)				
					PMFDEBUG("[%s]%s(%d) PMF: DEAUTH MIC_ERR\n", priv->dev->name, __FUNCTION__, __LINE__);
				else if(pframe[0] == WIFI_DISASSOC)				
					PMFDEBUG("[%s]%s(%d) PMF: DISASSOC MIC_ERR\n", priv->dev->name, __FUNCTION__, __LINE__);
				else
					PMFDEBUG("[%s]%s(%d) PMF: MIC_ERR\n", priv->dev->name, __FUNCTION__, __LINE__);
				return MGMT_FRAME_MIC_ERR;
			}
			else {
				memcpy(pframe + pfrinfo->hdr_len, pframe + pfrinfo->hdr_len + 8, pfrinfo->pktlen - pfrinfo->hdr_len - 8 - 8);				
				pfrinfo->pktlen = pfrinfo->pktlen - 16;
			}		
		}
#ifdef CONFIG_IEEE80211W_CLI
		if((priv->support_pmf == TRUE) && pstat){
			pstat->isPMF = TRUE;
		}
#endif
	}
	
	if(pframe[0] == WIFI_DEAUTH  || pframe[0] == WIFI_DISASSOC) 
	{
		return IS_ROBUST_MGMT;
	}
	//pframe += WLAN_HDR_A3_LEN; // Action Field
	
	if(pframe[0] == WIFI_WMM_ACTION) {		
		Category_field = pframe[WLAN_HDR_A3_LEN];
	} else
		return NOT_ROBUST_MGMT;

	return (Category_field != 4) && (Category_field != 7) && (Category_field != 11)
			&& (Category_field != 15) && (Category_field != 127);	
}

#endif

#ifdef CONFIG_IEEE80211W			

#ifdef CONFIG_IEEE80211W_CLI


static int isExistMMIC(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char *pframe = get_pframe(pfrinfo);
	
#ifdef CONFIG_IEEE80211W_CLI_DEBUG
	panic_printk("MMIC IE=");
	int idx;
	for(idx=0;idx<_MMIC_LEN_;idx++)
		panic_printk("%02x", pframe[pfrinfo->pktlen - 18 + idx]);
	panic_printk("\n");
#endif

	if((pframe[pfrinfo->pktlen - 18] == _MMIC_IE_) && (pframe[pfrinfo->pktlen - 17] == _MMIC_LEN_)){
		PMFDEBUG(" MMIC IE Exists!!\n"); 
		return TRUE;
	}
	else{
		PMFDEBUG(" No MMIC IE Exists!!\n"); 
		return FALSE;
	}
}

static int CheckBIPMIC(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char imic[MIC_LEN], pmic[MIC_LEN];
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned short reason = 0;

	memcpy(imic, pframe+pfrinfo->pktlen - MIC_LEN, MIC_LEN);		
	memcpy(&reason, pframe+pfrinfo->hdr_len, 2);
	PMFDEBUG(" reason = %x\n", reason);

	BIP_encrypt(priv, pframe, pframe+pfrinfo->hdr_len + 8,
				&reason, 12, pmic, priv->support_pmf);
	
	if(!memcmp(pmic, imic, MIC_LEN)){
		PMFDEBUG("(%s)line=%d, BIP: MIC Check PASS \n", __FUNCTION__, __LINE__);
		return TRUE;
	}else{
		if(pframe[0] == WIFI_DEAUTH)				
			PMFDEBUG("[%s]%s(%d) BIP: DEAUTH MIC_ERR\n", priv->dev->name, __FUNCTION__, __LINE__);
		else if(pframe[0] == WIFI_DISASSOC)				
			PMFDEBUG("[%s]%s(%d) BIP: DISASSOC MIC_ERR\n", priv->dev->name, __FUNCTION__, __LINE__);
		else
			PMFDEBUG("[%s]%s(%d) BIP: MIC_ERR\n", priv->dev->name, __FUNCTION__, __LINE__);

		return FALSE;
	}
}

#endif

int legal_mgnt_frame(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
		unsigned char *pframe = get_pframe(pfrinfo);
		unsigned char *da = GetAddr1Ptr(pframe);	
		int ret = isCorrectRobustFrame(priv, pfrinfo);
	
		if (ret == MGMT_FRAME_MIC_ERR) { // mgmt frame with mic error
			return 0;
		} else if(ret == IS_ROBUST_MGMT) { // robust mgmt frame
			if(priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == NO_MGMT_FRAME_PROTECTION) {
				if (GetPrivacy(pframe))
					return 0;
			}
			else if(priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_OPTIONAL) {
				struct stat_info *pstat = get_stainfo(priv, pfrinfo->sa);
				if (pstat == NULL) 
					return 0;
							
				if (pstat->isPMF == 0) { // MFPC=0
					if (GetPrivacy(pframe)) 
						return 0;
				} else { // MFPC = 1
					if (!IS_MCAST(da)) {
						if(GET_UNICAST_ENCRYP_KEY == NULL) {
							if((pframe[0] == WIFI_DEAUTH  || pframe[0] == WIFI_DISASSOC)) {
								if(GetPrivacy(pframe))
									return 0;
							} else
								return 0;
						} else {
							if(!GetPrivacy(pframe))
								return 0;
							else {
								ClearPrivacy(pframe);
								return 1;
							}
						}
					}
					// Does AP receive multicast robust frame from client?
					#ifdef CONFIG_IEEE80211W_CLI
					else { // MMPDU has a group RA
						if(!GET_IGROUP_ENCRYP_KEY) {
							if((pframe[0] != WIFI_DEAUTH  && pframe[0] != WIFI_DISASSOC))
								return 0;						
						} else {
							if(!isExistMMIC(priv ,pframe))
								return 0;
							else {
								if(!isExistMMIC(priv, pfrinfo)){
									return 0;
								}else{
									if(!CheckBIPMIC(priv, pfrinfo)) {
										return 0;
									}
								}
							}					
						}
					}
					#endif
				}
			}
			else if(priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_REQUIRED) {
				struct stat_info *pstat = get_stainfo(priv, pfrinfo->sa);
				if (pstat == NULL) 
					return 0;
				
				if (pstat->isPMF == 0) { // MFPC=0
					//if (GetPrivacy(pframe)) 
						return 0;
				} else { // MFPC = 1
					if (!IS_MCAST(da)) {
						if(GET_UNICAST_ENCRYP_KEY == NULL) {
								return 0;
						} else {
							if(!GetPrivacy(pframe))
								return 0;
							else {
								ClearPrivacy(pframe);
								return 1;
							}
						}
					}				
				}
			}
		
		}
		return 1;
}
#endif	// CONFIG_IEEE80211W

void mgt_handler(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	struct mlme_handler *ptable;
	unsigned int index;
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned char *sa = pfrinfo->sa;
	unsigned char *da = pfrinfo->da;
	struct stat_info *pstat = NULL;
	unsigned short frame_type;

#if 0	// already flush cache in rtl8192cd_rx_isr()
#ifdef __MIPSEB__
	pframe = (unsigned char*)KSEG1ADDR(pframe);
#endif
#endif

	if (OPMODE & WIFI_AP_STATE)
		ptable = mlme_ap_tbl;
#ifdef CLIENT_MODE
	else if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE))
		ptable = mlme_station_tbl;
#endif
	else
	{
		DEBUG_ERR("Currently we do not support opmode=%d\n", OPMODE);
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv) || (pfrinfo->is_br_mgnt == 0))
#endif
		rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
		return;
	}

	frame_type = GetFrameSubType(pframe);
	index = frame_type >> 4;
	if (index > 13)
	{
		DEBUG_ERR("Currently we do not support reserved sub-fr-type=%d\n", index);
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv) || (pfrinfo->is_br_mgnt == 0))
#endif
		rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
		return;
	}
	ptable += index;

#ifdef CONFIG_RTK_MESH
	if( is_11s_mgt_frame(ptable->num, priv, pfrinfo))
	{
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		pfrinfo->is_br_mgnt = 0;
#endif
		pfrinfo->is_11s = 1;
		ptable = mlme_mp_tbl;
		ptable += index;
#ifdef MBSSID
		if(!IS_VAP_INTERFACE(priv))
#endif
			// An 11s mgt frame will have Addr3 = 00..00, it might be dispatched to vxd in validate_mpdu
			// Hence, we have to "correct" it here.
			priv = GET_ROOT(priv);
	}
#endif // CONFIG_RTK_MESH

	pstat = get_stainfo(priv, sa);

	if ((pstat != NULL) && (WIFI_PROBEREQ != frame_type))
	{
#ifdef DETECT_STA_EXISTANCE
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			if (pstat->leave!= 0)
				RTL8188E_MACID_NOLINK(priv, 0, REMAP_AID(pstat));
		}
#endif
#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			if (pstat->leave)
				GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat, INCREASE);
		}
#endif
		pstat->leave = 0;
#endif
	}

	if (!IS_MCAST(da))
	{
		//pstat = get_stainfo(priv, sa);

		// only check last cache seq number for management frame, david -------------------------
		if (pstat != NULL) {
			if (GetRetry(pframe)) {
				if (GetTupleCache(pframe) == pstat->tpcache_mgt) {
					priv->ext_stats.rx_decache++;
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
					if (IS_ROOT_INTERFACE(priv) || (pfrinfo->is_br_mgnt == 0))
#endif
					rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
					SNMP_MIB_INC(dot11FrameDuplicateCount, 1);
					return;
				}
				else
				{
					  pstat->tpcache_mgt = GetTupleCache(pframe);
				}
			}
			pstat->tpcache_mgt = GetTupleCache(pframe);
		}
	}

	// log rx statistics...
#ifdef WDS
	if (pstat && (pstat->state & WIFI_WDS) && (ptable->num == WIFI_BEACON)) {
		rx_sum_up(NULL, pstat, pfrinfo->pktlen, 0);
#ifdef RX_CRC_EXPTIMER		
		rx_crc_sum_up(priv, pfrinfo->rx_rate);
#endif
		update_sta_rssi(priv, pstat, pfrinfo);
	}
	else
#endif
#ifdef CONFIG_RTK_MESH
	if (pstat && pfrinfo->is_11s && (ptable->num == WIFI_BEACON)) {
		// count statistics for mesh points -- chris
		rx_sum_up(NULL, pstat, pfrinfo->pktlen, 0);
#ifdef RX_CRC_EXPTIMER		
		rx_crc_sum_up(priv, pfrinfo->rx_rate);
#endif
		update_sta_rssi(priv, pstat, pfrinfo);
	}
	else
#endif
	if (pstat != NULL)
	{
		// If AP mode and rx is a beacon, do not count in statistics. david
#ifdef DONT_COUNT_PROBE_PACKET
		if (!((OPMODE & WIFI_AP_STATE) && ((ptable->num == WIFI_BEACON)||(ptable->num == WIFI_PROBEREQ))))
#else
		if (!((OPMODE & WIFI_AP_STATE) && (ptable->num == WIFI_BEACON)))
#endif
		{
			rx_sum_up(NULL, pstat, pfrinfo->pktlen, 0);
#ifdef RX_CRC_EXPTIMER		
			rx_crc_sum_up(priv, pfrinfo->rx_rate);
#endif			
			update_sta_rssi(priv, pstat, pfrinfo);
		}
	}

	// check power save state
	if ((OPMODE & WIFI_AP_STATE) && (pstat != NULL)) {
		if (IS_BSSID(priv, GetAddr1Ptr(pframe)))
		{
		#ifdef HW_DETEC_POWER_STATE                 
            if (GET_CHIP_VER(priv) == VERSION_8814A) {
                // 8814 power state control only by HW, not by SW.
                // Only if HW detect macid not ready, SW patch this packet
                if(pfrinfo->macid == HW_MACID_SEARCH_NOT_READY)
                {
                    printk("%s %d HW_MACID_SEARCH_NOT_READY",__FUNCTION__,__LINE__);
                    if(priv->pshare->HWPwrStateUpdate[pstat->aid]==false)   
                    {
                        printk("%s %d HW not update By SW Aid = %x \n",__FUNCTION__,__LINE__,pstat->aid);                            
			pwr_state(priv, pfrinfo);
	}
                }
                else if(pfrinfo->macid > HW_MACID_SEARCH_SUPPORT_NUM)
                {
                    pwr_state(priv, pfrinfo);
                }
            } else 
        #endif // #ifdef HW_DETEC_POWER_STATE                 
                     {
                	pwr_state(priv, pfrinfo);
                     }
                }
	}

#ifdef MBSSID
	if (
		GET_ROOT(priv)->pmib->miscEntry.vap_enable &&
		IS_VAP_INTERFACE(priv)) {
		if (IS_MCAST(da) || !memcmp(GET_MY_HWADDR, da, MACADDRLEN))
			ptable->func(priv, pfrinfo);
	}
	else
#endif
	{
#ifdef CONFIG_IEEE80211W
		if(legal_mgnt_frame(priv,pfrinfo))
#endif
		{
			ptable->func(priv, pfrinfo);
		}
	}

#ifdef MBSSID
	if (IS_VAP_INTERFACE(priv)) {
		if (pfrinfo->is_br_mgnt) {
			rx_sum_up(priv, NULL, pfrinfo->pktlen, GetRetry(pframe));
#ifdef RX_CRC_EXPTIMER		
			rx_crc_sum_up(priv, pfrinfo->rx_rate);
#endif			
			return;
		}
	}
	else if (IS_ROOT_INTERFACE(priv) && pfrinfo->is_br_mgnt && (OPMODE & WIFI_AP_STATE)) {
		int i;
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if ((IS_DRV_OPEN(priv->pvap_priv[i])) && ((IS_MCAST(da)) ||
				(!memcmp(priv->pvap_priv[i]->pmib->dot11StationConfigEntry.dot11Bssid, da, MACADDRLEN))))
#ifdef CONFIG_IEEE80211W
				if(legal_mgnt_frame(priv->pvap_priv[i],pfrinfo))
#endif				
				{
					mgt_handler(priv->pvap_priv[i], pfrinfo);
				}
		 }
	}
#endif

#ifdef UNIVERSAL_REPEATER
	if (pfrinfo->is_br_mgnt) {
		pfrinfo->is_br_mgnt = 0;

// fix hang-up issue when root-ap (A+B) + vxd-client ------------
		if (IS_DRV_OPEN(GET_VXD_PRIV(priv))) {
			if ((OPMODE & WIFI_AP_STATE) ||
				((OPMODE & WIFI_STATION_STATE) &&
				GET_VXD_PRIV(priv) && (GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED))) {
//--------------------------------------------david+2006-07-17

				mgt_handler(GET_VXD_PRIV(priv), pfrinfo);
				return;
			}
		}
	}
#endif

	rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
}


/*----------------------------------------------------------------------------
// the purpose of this sub-routine
Any station has changed from sleep to active state, and has data buffer should
be dequeued here!
-----------------------------------------------------------------------------*/
#ifdef CONFIG_PCI_HCI
void process_dzqueue(struct rtl8192cd_priv *priv)
{
	struct stat_info *pstat;
	struct sk_buff *pskb;
	struct list_head *phead = &priv->wakeup_list;
	struct list_head *plist = phead->next;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	while(plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, wakeup_list);
		plist = plist->next;

		while(1)
		{
// 2009.09.08
		SAVE_INT_AND_CLI(flags);
#if defined(WIFI_WMM) && defined(WMM_APSD)
			if (
#ifdef CLIENT_MODE
				(OPMODE & WIFI_AP_STATE) &&
#endif
				(QOS_ENABLE) && (APSD_ENABLE) && pstat && (pstat->QosEnabled) && (pstat->apsd_pkt_buffering)) {
				pskb = (struct sk_buff *)deque(priv, &(pstat->VO_dz_queue->head), &(pstat->VO_dz_queue->tail),
					(unsigned long)(pstat->VO_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE);
				if (pskb == NULL) {
					pskb = (struct sk_buff *)deque(priv, &(pstat->VI_dz_queue->head), &(pstat->VI_dz_queue->tail),
						(unsigned long)(pstat->VI_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE);
					if (pskb == NULL) {
						pskb = (struct sk_buff *)deque(priv, &(pstat->BE_dz_queue->head), &(pstat->BE_dz_queue->tail),
							(unsigned long)(pstat->BE_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE);
						if (pskb == NULL) {
							pskb = (struct sk_buff *)deque(priv, &(pstat->BK_dz_queue->head), &(pstat->BK_dz_queue->tail),
								(unsigned long)(pstat->BK_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE);
							if (pskb == NULL) {
								pstat->apsd_pkt_buffering = 0;
								goto legacy_ps;
							}
							DEBUG_INFO("release BK pkt\n");
						} else {
							DEBUG_INFO("release BE pkt\n");
						}
					} else {
						DEBUG_INFO("release VI pkt\n");
					}
				} else {
					DEBUG_INFO("release VO pkt\n");
				}
			} else
legacy_ps:
#endif
#if defined(WIFI_WMM) 
			if (!isFFempty(pstat->MGT_dz_queue->head, pstat->MGT_dz_queue->tail)){
				struct tx_insn *tx_cfg;
				tx_cfg = (struct tx_insn *)deque(priv, &(pstat->MGT_dz_queue->head), &(pstat->MGT_dz_queue->tail),
					(unsigned long)(pstat->MGT_dz_queue->ptx_insn), NUM_DZ_MGT_QUEUE);
				if ((rtl8192cd_firetx(priv, tx_cfg)) == SUCCESS){
					DEBUG_INFO("release MGT pkt\n");
				}else{
					DEBUG_ERR("release MGT pkt failed!\n");
					if (tx_cfg->phdr)
						release_wlanhdr_to_poll(priv, tx_cfg->phdr);
					if (tx_cfg->pframe)
						release_mgtbuf_to_poll(priv, tx_cfg->pframe);
				}
				kfree(tx_cfg);
				RESTORE_INT(flags);
				continue;
			}
			else
#ifdef DZ_ADDBA_RSP
			if (pstat->dz_addba.used) {
				issue_ADDBArsp(priv, pstat->hwaddr, pstat->dz_addba.dialog_token,
						pstat->dz_addba.TID, pstat->dz_addba.status_code, pstat->dz_addba.timeout);
				pstat->dz_addba.used = 0;
				//printk("issue DZ addba!!!!!!!\n");
				RESTORE_INT(flags);
				continue;
			}
			else
#endif
#endif
			pskb = __skb_dequeue(&pstat->dz_queue);

// 2009.09.08
			RESTORE_INT(flags);

			if (pskb == NULL)
				break;

#ifdef ENABLE_RTL_SKB_STATS
			rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif

			if (rtl8192cd_start_xmit_noM2U(pskb, pskb->dev))
				rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}

		SAVE_INT_AND_CLI(flags);
		if (wakeup_list_del(priv, pstat)) {
			DEBUG_INFO("Del fr wakeup_list %02X%02X%02X%02X%02X%02X\n",
				pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
		}
		RESTORE_INT(flags); 
	}
}


void process_mcast_dzqueue(struct rtl8192cd_priv *priv)
{
	struct sk_buff *pskb;

	priv->release_mcast = 1;
	while(1) {
		pskb = (struct sk_buff *)deque(priv, &(priv->dz_queue.head), &(priv->dz_queue.tail),
			(unsigned long)(priv->dz_queue.pSkb), NUM_TXPKT_QUEUE);

		if (pskb == NULL)
			break;
		
#ifdef ENABLE_RTL_SKB_STATS
		rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif

// stanley: I think using pskb->dev is correct IN THE FUTURE, when mesh0 also applies dzqueue
#ifdef CONFIG_RTK_MESH
		if (rtl8192cd_start_xmit(pskb, pskb->dev))
#else
		if (rtl8192cd_start_xmit(pskb, priv->dev))
#endif
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
	}
	priv->release_mcast = 0;
}
#endif // CONFIG_PCI_HCI


#if 0
 int check_basic_rate(struct rtl8192cd_priv *priv, unsigned char *pRate, int pLen)
{
	int i, match, idx;
	UINT8 rate;

	// david, check if is there is any basic rate existed --
	int any_one_basic_rate_found = 0;

	for (i=0; i<pLen; i++) {
		if (pRate[i] & 0x80) {
			any_one_basic_rate_found = 1;
			break;
		}
	}

	for (i=0; i<AP_BSSRATE_LEN; i++) {
		if (AP_BSSRATE[i] & 0x80) {
			rate = AP_BSSRATE[i] & 0x7f;
			match = 0;
			for (idx=0; idx<pLen; idx++) {
				if ((pRate[idx] & 0x7f) == rate) {
					if (pRate[idx] & 0x80)
						match = 1;
					else {
						if (!any_one_basic_rate_found) {
							pRate[idx] |= 0x80;
							match = 1;
						}
					}
				}
			}
			if (match == 0)
				return FAIL;
		}
	}
	return SUCCESS;
}
#endif

#ifdef BEAMFORMING_SUPPORT
void DynamicSelect2STA(struct rtl8192cd_priv *priv)
{
	int idx = 0, isCandidate, candidateCtr=0, bfeCtr=0;
	struct stat_info* psta, *candidate1=NULL, *candidate2=NULL;
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);	
	PRT_BEAMFORMING_ENTRY	pEntry; 			
	if(pBeamformingInfo->BeamformeeEntry[0].bUsed && pBeamformingInfo->BeamformeeEntry[1].bUsed) {
		candidate1 = psta = findNextSTA(priv, &idx);
		while(psta) {
			if(psta && psta->expire_to) {
				if(	psta->bf_score > candidate1->bf_score)
					candidate1 = psta;
			}
			psta = findNextSTA(priv, &idx);
		};
		if(candidate1 ==NULL)
			return;
		idx = 0;
		candidate2 = psta = findNextSTA(priv, &idx);

		if(candidate2 == candidate1)                 // to avoid 1st STA with highest score case
			candidate2 = psta = findNextSTA(priv, &idx);
			
		while(psta) {
			if(psta && psta->expire_to) {
				if(	(psta->bf_score > candidate2->bf_score) &&  (psta != candidate1))
					candidate2 = psta;
			}
			psta = findNextSTA(priv, &idx);
		};
		if( candidate2  == candidate1)
			candidate2 = NULL;
		if(candidate1)
			++candidateCtr;
		if(candidate2)
			++candidateCtr;
		{
			for(idx = 0; idx < BEAMFORMEE_ENTRY_NUM; idx++) {
				if( pBeamformingInfo->BeamformeeEntry[idx].bUsed) {			
					isCandidate = 0;					
					if(candidate1 && (pBeamformingInfo->BeamformeeEntry[idx].MacId == candidate1->aid))
						++bfeCtr;
					else if(candidate2 && (pBeamformingInfo->BeamformeeEntry[idx].MacId == candidate2->aid))
						++bfeCtr;					
				}
			}
			candidateCtr -=bfeCtr;
			for(idx = 0; idx < BEAMFORMEE_ENTRY_NUM; idx++)
			{
				if(	pBeamformingInfo->BeamformeeEntry[idx].bUsed) {			
					isCandidate = 0;					
					if(candidate1 && (pBeamformingInfo->BeamformeeEntry[idx].MacId == candidate1->aid))
						isCandidate = 1;		 
					if(candidate2 && (pBeamformingInfo->BeamformeeEntry[idx].MacId == candidate2->aid))
						 isCandidate = 2;
					if(isCandidate==0 && candidateCtr) {
						ODM_RT_TRACE(ODMPTR, BEAMFORMING_DEBUG, ODM_DBG_LOUD, ("%s,\n", __FUNCTION__));
						if(Beamforming_DeInitEntry(priv, pBeamformingInfo->BeamformeeEntry[idx].MacAddr))
							Beamforming_Notify(priv);
						
						--candidateCtr;
					}
				}
			}

			if(candidate1) {				
				pEntry = Beamforming_GetEntryByMacId(priv, candidate1->aid, (unsigned char*)&idx);
				if(pEntry == NULL) {
					Beamforming_Enter(priv, candidate1);			
				}
			}
			if(candidate2) 	{				
				pEntry = Beamforming_GetEntryByMacId(priv, candidate2->aid, (unsigned char*)&idx);
				if(pEntry == NULL) {
					Beamforming_Enter(priv, candidate2);			
				}
			}	
		}
	}

}
#endif

// which: 0: set basic rates as mine, 1: set basic rates as peer's
 void get_matched_rate(struct rtl8192cd_priv *priv, unsigned char *pRate, int *pLen, int which)
{
	int i, j, num=0;
	UINT8 rate;
	UINT8 found_rate[32];

	for (i=0; i<AP_BSSRATE_LEN; i++) {
		// see if supported rate existed and matched
		rate = AP_BSSRATE[i] & 0x7f;
		if (match_supp_rate(pRate, *pLen, rate)) {
			if (!which) {
				if (AP_BSSRATE[i] & 0x80)
					rate |= 0x80;
			}
			else {
				for (j=0; j<*pLen; j++) {
					if (rate == (pRate[j] & 0x7f)) {
						if (pRate[j] & 0x80)
							rate |= 0x80;
						break;
					}
				}
			}
			found_rate[num++] = rate;
		}
	}

	if (which) {
		for (i=0; i<num; i++) {
			if (found_rate[i] & 0x80)
				break;
		}
		if (i == num) { // no any basic rates in found_rate
			j = 0;
			while(pRate[j] & 0x80)
				j++;
			memcpy(&(pRate[j]), found_rate, num);
			*pLen = j + num;
			return;
		}
	}

	memcpy(pRate, found_rate, num);
	*pLen = num;
}


 void update_support_rate(struct	stat_info *pstat, unsigned char* buf, int len)
{
	memset(pstat->bssrateset, 0, sizeof(pstat->bssrateset));
	pstat->bssratelen=len;
	memcpy(pstat->bssrateset, buf, len);
}


int isErpSta(struct	stat_info *pstat)
{
	int i, len=pstat->bssratelen;
	UINT8 *buf=pstat->bssrateset;

	for (i=0; i<len; i++) {
		if ( ((buf[i] & 0x7f) != 2) &&
				((buf[i] & 0x7f) != 4) &&
				((buf[i] & 0x7f) != 11) &&
				((buf[i] & 0x7f) != 22) )
			return 1;	// ERP sta existed
	}
	return 0;
}

void SelectLowestInitRate(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	
	if (priv->pmib->dot11StationConfigEntry.autoRate)
	{
		phead = &priv->asoc_list;
		plist = phead;
		
		while ((plist = asoc_list_get_next(priv, plist)) != phead)
		{
			pstat = list_entry(plist, struct stat_info, asoc_list);
			if (pstat->sta_in_firmware == 1 && (pstat->expire_to > 0))
			{

				if(pstat->current_tx_rate < priv->pshare->phw->LowestInitRate)
					 priv->pshare->phw->LowestInitRate = pstat->current_tx_rate;
			}
		}
	}
	else if (priv->pmib->dot11StationConfigEntry.fixedTxRate)
		priv->pshare->phw->LowestInitRate = priv->pmib->dot11StationConfigEntry.fixedTxRate;
	else
		priv->pshare->phw->LowestInitRate = _24M_RATE_;

}

/*----------------------------------------------------------------------------
index: the information element id index, limit is the limit for search
-----------------------------------------------------------------------------*/
/**
 *	@brief	Get Information Element
 *
 *		p (Find ID in limit)		\n
 *	+--- -+------------+-----+---	\n
 *	| ... | element ID | len |...	\n
 *	+--- -+------------+-----+---	\n
 *
 *	@param	pbuf	frame data for search
 *	@param	index	the information element id = index (search target)
 *	@param	limit	limit for search
 *
 *	@retval	p	pointer to element ID
 *	@retval	len	p(IE) len
 */
 unsigned char *get_ie(unsigned char *pbuf, int index, int *len, int limit)
{
	unsigned int tmp,i;
	unsigned char *p;

	if (limit < 1)
		return NULL;

	p = pbuf;
	i = 0;
	*len = 0;
	while(1)
	{
		if (*p == index)
		{
			*len = *(p + 1);
			return (p);
		}
		else
		{
			tmp = *(p + 1);
			p += (tmp + 2);
			i += (tmp + 2);
		}
		if (i >= limit)
			break;
	}
	return NULL;
}


#ifdef RTL_WPA2
/*----------------------------------------------------------------------------
index: the information element id index, limit is the limit for search
-----------------------------------------------------------------------------*/
static unsigned char *get_rsn_ie(struct rtl8192cd_priv *priv, unsigned char *pbuf, int *len, int limit)
{
	unsigned char *p = NULL;

	if (priv->pmib->dot11RsnIE.rsnielen == 0)
		return NULL;

	if ((priv->pmib->dot11RsnIE.rsnie[0] == _RSN_IE_2_) ||
		((priv->pmib->dot11RsnIE.rsnielen > priv->pmib->dot11RsnIE.rsnie[1]) &&
			(priv->pmib->dot11RsnIE.rsnie[priv->pmib->dot11RsnIE.rsnie[1]+2] == _RSN_IE_2_))) {
		p = get_ie(pbuf, _RSN_IE_2_, len, limit);
		if (p != NULL)
			return p;
		else
			return get_ie(pbuf, _RSN_IE_1_, len, limit);
	}
	else {
		p = get_ie(pbuf, _RSN_IE_1_, len, limit);
		if (p != NULL)
			return p;
		else
			return get_ie(pbuf, _RSN_IE_2_, len, limit);
	}
}
#endif


/**
 *	@brief	Set Information Element
 *
 *	Difference between set_fixed_ie, reserve 2 Byte, for Element ID & length \n
 *	\n
 *	+-------+       +------------+--------+----------------+	\n
 *	| pbuf. | <---  | element ID | length |     source     |	\n
 *	+-------+       +------------+--------+----------------+	\n
 *
 *	@param pbuf		buffer(frame) for set
 *	@param index	IE element ID
 *	@param len		IE length content & set length
 *	@param source	IE data for buffer set
 *	@param frlen	total frame length
 *
 *	@retval	pbuf+len+2	pointer of buffer tail.(+2 because element ID and length total 2 bytes)
 */
 unsigned char *set_ie(unsigned char *pbuf, int index, unsigned int len, unsigned char *source,
				unsigned int *frlen)
{
	*pbuf = index;
	*(pbuf + 1) = len;
	if (len > 0)
		memcpy((void *)(pbuf + 2), (void *)source, len);
	*frlen = *frlen + (len + 2);
	return (pbuf + len + 2);
}


static __inline__ int set_virtual_bitmap(unsigned char *pbuf, unsigned int i)
{
	unsigned int r,s,t;

	r = (i >> 3) << 3;
	t = i - r;

	s = BIT(t);

	*(pbuf + (i >> 3)) |= s;

	return	(i >> 3);
}


static __inline__ unsigned char *update_tim(struct rtl8192cd_priv *priv,
				unsigned char *bcn_buf, unsigned int *frlen)
{
#ifdef CONFIG_PCI_HCI
	unsigned int	pre_head;
#endif
	unsigned int	i, set_pvb;
	unsigned char	val8;
#if !defined(SMP_SYNC) || (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))    
	unsigned long	flags;
#endif
	struct list_head	*plist, *phead;
	struct stat_info	*pstat;
	unsigned char	bitmap[(NUM_STAT/8)+1];
	unsigned char	*pbuf= bcn_buf;
	unsigned char	N1, N2, bitmap_offset;

#ifdef CONFIG_PCI_HCI
#if defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8812_SUPPORT)|| defined(CONFIG_RTL_88E_SUPPORT)
    unsigned char Q_info[8];
    unsigned char b_calhwqnum = 0;
#ifdef  CONFIG_WLAN_HAL
    if (IS_HAL_CHIP(priv))
        b_calhwqnum = 8;
#endif		
#ifdef CONFIG_RTL_8812_SUPPORT
    if(GET_CHIP_VER(priv)== VERSION_8812E)
        b_calhwqnum = 8;
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
    if(GET_CHIP_VER(priv)== VERSION_8188E){
        b_calhwqnum = 4;
    }
#endif        

    if(b_calhwqnum == 8) {
        for(i=0; i<4; i++) {
            if(RTL_R8(0x400+i*4+1) & 0x7f) // 14:8     7b
                Q_info[i] = (RTL_R8(0x400+i*4+3)>>1) & 0x7f; //31:25     7b
            else 
                Q_info[i] = 0;
            if(RTL_R8(0x468+i*4+1) & 0x7f) // 14:8     7b
                Q_info[i+4] = (RTL_R8(0x468+i*4+3)>>1) & 0x7f; //31:25     7b
            else
                Q_info[i+4] = 0;
        }
    }
    else if(b_calhwqnum == 4) {
        for(i=0; i<4; i++) {
            if(RTL_R8(0x400+i*4+1)){ // 15:8     7b
                Q_info[i] = (RTL_R8(0x400+i*4+3)>>2) & 0x3f; //31:26     7b
            }
            else 
                Q_info[i] = 0;
        }

    }
#endif
#endif // CONFIG_PCI_HCI

	memset(bitmap, 0, sizeof(bitmap));

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable && IS_VAP_INTERFACE(priv))
		priv->dtimcount = GET_ROOT(priv)->dtimcount;
	else
#endif
	{
		if (priv->dtimcount == 0)
			priv->dtimcount = (priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod - 1);
		else
			priv->dtimcount--;
	}

#ifdef CONFIG_PCI_HCI
#ifdef  CONFIG_WLAN_HAL
	if (!IS_HAL_CHIP(priv)) 
#endif
	{
		//if (priv->pkt_in_dtimQ && (priv->dtimcount == 0))
		SMP_LOCK_XMIT(flags);
		pre_head = get_txhead(priv->pshare->phw, MCAST_QNUM);
		txdesc_rollback(&pre_head);
		SMP_UNLOCK_XMIT(flags);

		if (priv->dtimcount == (priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod - 1)
			&&(*((unsigned char *)priv->beaconbuf + priv->timoffset + 4) & 0x01))  {
			RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) & (~ HIQ_NO_LMT_EN));
		}
		if(priv->dtimcount == (priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod - 1)
			&& !(get_desc((get_txdesc(priv->pshare->phw, MCAST_QNUM) + pre_head)->Dword0) & TX_OWN))
			priv->pkt_in_hiQ = 0;
	}

	if ((priv->dtimcount == 0) &&
		(priv->pkt_in_dtimQ ||
	//		(get_desc((get_txdesc(priv->pshare->phw, MCAST_QNUM) + pre_head)->Dword0) & TX_OWN)))
			priv->pkt_in_hiQ))
		val8 = 0x01;
	else
		val8 = 0x00;
#endif // CONFIG_PCI_HCI

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	if ((priv->dtimcount == 0)
			&& (((MCAST_QNUM == priv->tx_mc_queue.q_num) && (tx_servq_len(&priv->tx_mc_queue) > 0))
#ifdef CONFIG_USB_HCI
			|| (priv->pshare->tx_urb_waiting_queue[MCAST_QNUM].qlen > 0)
#endif
#ifdef CONFIG_SDIO_HCI
			|| (priv->pshare->tx_xmitbuf_waiting_queue[MCAST_QNUM].qlen > 0)
#endif
#if defined(CONFIG_RTL_88E_SUPPORT)
			|| (RTL_R8(HIQ_INFO+1))
#elif defined(CONFIG_WLAN_HAL_8192EE)
			|| (RTL_R8(HIQ_INFO+1) & 0x7F)
#else
			|| (RTL_R8(HIQ_INFO+2))
#endif
			))
		val8 = 0x01;
	else
		val8 = 0x00;
#endif // CONFIG_USB_HCI || CONFIG_SDIO_HCI

#if defined(CONFIG_WLAN_HAL) && defined(CONFIG_PCI_HCI)
    if (IS_HAL_CHIP(priv)) 
    {
        u1Byte	macID_list[NUM_AC_QUEUE];
        u1Byte	macID_index;
      	memset(macID_list, 0, NUM_AC_QUEUE);    
        if(RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->GetMACIDQueueInTXPKTBUFHandler(priv,macID_list))
        {
            for(macID_index=0;macID_index<NUM_AC_QUEUE;macID_index++)
            {
                if(macID_list[macID_index]!=0)
                {
                    set_virtual_bitmap(bitmap, macID_list[macID_index]);
                }
            }        
        }
    }
#endif //#ifdef CONFIG_WLAN_HAL

	*pbuf = _TIM_IE_;
	*(pbuf + 2) = priv->dtimcount;
	*(pbuf + 3) = priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod;

	phead = &priv->sleep_list;

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK_SLEEP_LIST(flags);

	plist = phead->next;
	while(plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, sleep_list);
		plist = plist->next;
		set_pvb = 0;
		
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		for (i = 0; i < MAX_STA_TX_SERV_QUEUE; ++i) {
			if (tx_servq_len(&pstat->tx_queue[i]) > 0) {
				set_pvb++;
				break;
			}
		}
		
#elif defined(CONFIG_PCI_HCI)
#if defined(WIFI_WMM) && defined(WMM_APSD)
		if ((QOS_ENABLE) && (APSD_ENABLE) && (pstat) && ((pstat->apsd_bitmap & 0x0f) == 0x0f) &&
			((!isFFempty(pstat->VO_dz_queue->head, pstat->VO_dz_queue->tail)) ||
			 (!isFFempty(pstat->VI_dz_queue->head, pstat->VI_dz_queue->tail)) ||
			 (!isFFempty(pstat->BE_dz_queue->head, pstat->BE_dz_queue->tail)) ||
			 (!isFFempty(pstat->BK_dz_queue->head, pstat->BK_dz_queue->tail)))) {
			set_pvb++;
		}
		else
#endif

		if (skb_queue_len(&pstat->dz_queue))
			set_pvb++;

#if defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8812_SUPPORT)|| defined(CONFIG_RTL_88E_SUPPORT)
        if(set_pvb == 0 && b_calhwqnum) {
            for(i=0; i<b_calhwqnum; i++) {
                if(pstat->aid == Q_info[i]) {
                    set_pvb++;
                    break;
                }
            }
        }
#endif
#endif

		if (set_pvb) {
			i = pstat->aid;
			i = set_virtual_bitmap(bitmap, i);
		}
	}

	SMP_UNLOCK_SLEEP_LIST(flags);
	RESTORE_INT(flags);

	N1 = 0;
	for(i=0; i<(NUM_STAT/8)+1; i++) {
		if(bitmap[i] != 0) {
			N1 = i;
			break;
		}
	}
	N2 = N1;
	for(i=(NUM_STAT/8); i>N1; i--) {
		if(bitmap[i] != 0) {
			N2 = i;
			break;
		}
	}

	// N1 should be an even number
	N1 = (N1 & 0x01)? (N1-1) : N1;
	bitmap_offset = N1 >> 1;	// == N1/2
	*(pbuf + 1) = N2 - N1 + 4;
	*(frlen) = *frlen + *(pbuf + 1) + 2;

	*(pbuf + 4) = val8 | (bitmap_offset << 1);
	memcpy((void *)(pbuf + 5), &bitmap[N1], (N2-N1+1));

	return (bcn_buf + *(pbuf + 1) + 2);
}

/**
 *	@brief	set fixed information element
 *
 *	set_fixed is haven't Element ID & length, Total length is frlen. \n
 *					 len	\n
 *	+-----------+-----------+	\n
 *	|	pbuf	|   source  |	\n
 *	+-----------+-----------+	\n
 *
 *	@param	pbuf	buffer(frame) for set
 *	@param	len		IE set length
 *	@param	source	IE data for buffer set
 *	@param	frlen	total frame length (Note: frlen have side effect??)
 *
 *	@retval	pbuf+len	pointer of buffer tail. \n
 */
 unsigned char *set_fixed_ie(unsigned char *pbuf, unsigned int len, unsigned char *source,
				unsigned int *frlen)
{
	memcpy((void *)pbuf, (void *)source, len);
	*frlen = *frlen + len;
	return (pbuf + len);
}


void construct_ht_ie(struct rtl8192cd_priv *priv, int use_40m, int offset)
{
	struct ht_cap_elmt	*ht_cap;
	struct ht_info_elmt	*ht_ie;
	int ch_offset;

	if (priv->ht_cap_len == 0) {
		unsigned int sup_mcs = get_supported_mcs(priv);
		// construct HT Capabilities element
		priv->ht_cap_len = sizeof(struct ht_cap_elmt);
		ht_cap = &priv->ht_cap_buf;
		memset(ht_cap, 0, sizeof(struct ht_cap_elmt));
		if (use_40m ==1 || use_40m ==2){
			ht_cap->ht_cap_info |= cpu_to_le16(use_40m ? _HTCAP_SUPPORT_CH_WDTH_ : 0);
		}
		ht_cap->ht_cap_info |= cpu_to_le16(_HTCAP_SMPWR_DISABLE_);
		ht_cap->ht_cap_info |= cpu_to_le16(priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M? _HTCAP_SHORTGI_20M_ : 0);

		if (use_40m == 1 || use_40m ==2 
		|| (priv->pmib->dot11nConfigEntry.dot11nUse40M==1)  
		|| (priv->pmib->dot11nConfigEntry.dot11nUse40M==2)) {
			ht_cap->ht_cap_info |= cpu_to_le16(priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M? _HTCAP_SHORTGI_40M_ : 0);
		}
		if ((get_rf_mimo_mode(priv) == MIMO_1T1R) 
#ifdef CONFIG_RTL_92C_SUPPORT
			|| (GET_CHIP_VER(priv) == VERSION_8192C)
#endif
		)
			ht_cap->ht_cap_info |= cpu_to_le16(priv->pmib->dot11nConfigEntry.dot11nSTBC? _HTCAP_RX_STBC_1S_ : 0);
		else
			ht_cap->ht_cap_info |= cpu_to_le16(priv->pmib->dot11nConfigEntry.dot11nSTBC? (_HTCAP_TX_STBC_ | _HTCAP_RX_STBC_1S_) : 0);
#if defined(CONFIG_WLAN_HAL_8881A)
		if (GET_CHIP_VER(priv) != VERSION_8881A)
#endif		
		ht_cap->ht_cap_info |= cpu_to_le16(priv->pmib->dot11nConfigEntry.dot11nLDPC? (_HTCAP_SUPPORT_RX_LDPC_) : 0);
		ht_cap->ht_cap_info |= cpu_to_le16(priv->pmib->dot11nConfigEntry.dot11nAMSDURecvMax? _HTCAP_AMSDU_LEN_8K_ : 0);
		ht_cap->ht_cap_info |= cpu_to_le16(_HTCAP_CCK_IN_40M_);
// 64k
#if	defined(CONFIG_RTL_8812_SUPPORT)
		if (GET_CHIP_VER(priv)==VERSION_8812E) {
			if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm)
				ht_cap->ampdu_para = ((_HTCAP_AMPDU_SPC_16_US_ << _HTCAP_AMPDU_SPC_SHIFT_) | _HTCAP_AMPDU_FAC_64K_);		
			else
				ht_cap->ampdu_para = ((_HTCAP_AMPDU_SPC_8_US_ << _HTCAP_AMPDU_SPC_SHIFT_) | _HTCAP_AMPDU_FAC_64K_);
		} else 
#endif		
#if	defined(CONFIG_WLAN_HAL)
		if (GET_CHIP_VER(priv)==VERSION_8192E) {
			if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm)
				ht_cap->ampdu_para = ((_HTCAP_AMPDU_SPC_16_US_ << _HTCAP_AMPDU_SPC_SHIFT_) | _HTCAP_AMPDU_FAC_64K_);		
			else
				ht_cap->ampdu_para = ((_HTCAP_AMPDU_SPC_8_US_ << _HTCAP_AMPDU_SPC_SHIFT_) | _HTCAP_AMPDU_FAC_64K_);
		} else
#endif	
		{
			if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm) {
#if defined(CONFIG_WLAN_HAL_8814AE) 			
				if ((GET_CHIP_VER(priv) == VERSION_8814A))
					ht_cap->ampdu_para = ((_HTCAP_AMPDU_SPC_4_US_ << _HTCAP_AMPDU_SPC_SHIFT_) | _HTCAP_AMPDU_FAC_64K_);					
				else
#endif				
				ht_cap->ampdu_para = ((_HTCAP_AMPDU_SPC_16_US_ << _HTCAP_AMPDU_SPC_SHIFT_) | _HTCAP_AMPDU_FAC_32K_);
			}
			else {
#ifdef RTK_AC_SUPPORT //for 11ac logo
				if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A))
				ht_cap->ampdu_para = ((_HTCAP_AMPDU_SPC_NORES_ << _HTCAP_AMPDU_SPC_SHIFT_) | _HTCAP_AMPDU_FAC_64K_);
				else
#endif
				ht_cap->ampdu_para = ((_HTCAP_AMPDU_SPC_8_US_ << _HTCAP_AMPDU_SPC_SHIFT_) | _HTCAP_AMPDU_FAC_32K_);
			}

		}
		ht_cap->support_mcs[0] = (sup_mcs & 0xff);
		ht_cap->support_mcs[1] = (sup_mcs & 0xff00) >> 8;
		ht_cap->support_mcs[2] = (sup_mcs & 0xff0000) >> 16;
		ht_cap->support_mcs[3] = (sup_mcs & 0xff000000) >> 24;
		ht_cap->ht_ext_cap = 0;
#ifdef BEAMFORMING_SUPPORT		
		if (((GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8814A)) &&
			(priv->pmib->dot11RFEntry.txbf == 1)){
			if ((priv->pmib->dot11RFEntry.txbfer == 1) && (priv->pmib->dot11RFEntry.txbfee == 0))
				{
				if(GET_CHIP_VER(priv) == VERSION_8814A)
					ht_cap->txbf_cap = cpu_to_le32(0x18000410);
				else
					ht_cap->txbf_cap = cpu_to_le32(0x08000410);
				}
			else if ((priv->pmib->dot11RFEntry.txbfer == 0) && (priv->pmib->dot11RFEntry.txbfee == 1))
				{
				if(GET_CHIP_VER(priv) == VERSION_8814A)
					ht_cap->txbf_cap = cpu_to_le32(0x01810008);
				else
					ht_cap->txbf_cap = cpu_to_le32(0x00810008);
				}
			else 
				{
				if(GET_CHIP_VER(priv) == VERSION_8814A)
					ht_cap->txbf_cap = cpu_to_le32(0x19810418);
				else
					ht_cap->txbf_cap = cpu_to_le32(0x08810418);
				}
		}
		else
#endif
		ht_cap->txbf_cap = 0;
		ht_cap->asel_cap = 0;

#ifdef CLIENT_MODE
		if ((OPMODE & WIFI_AP_STATE) || (OPMODE & WIFI_ADHOC_STATE))
#endif
		{
			// construct HT Information element
			priv->ht_ie_len = sizeof(struct ht_info_elmt);
			ht_ie = &priv->ht_ie_buf;
			memset(ht_ie, 0, sizeof(struct ht_info_elmt));
			ht_ie->primary_ch = priv->pmib->dot11RFEntry.dot11channel;
			if ((use_40m == 1) || (use_40m == 2)) { 
				if (offset == HT_2NDCH_OFFSET_BELOW)
					ch_offset = _HTIE_2NDCH_OFFSET_BL_ | _HTIE_STA_CH_WDTH_;
				else
					ch_offset = _HTIE_2NDCH_OFFSET_AB_ | _HTIE_STA_CH_WDTH_;
			} else {
				ch_offset = _HTIE_2NDCH_OFFSET_NO_;
			}

			ht_ie->info0 |= ch_offset;
			ht_ie->info1 = 0;
			ht_ie->info2 = 0;
			ht_ie->basic_mcs[0] = (priv->pmib->dot11nConfigEntry.dot11nBasicMCS & 0xff);
			ht_ie->basic_mcs[1] = (priv->pmib->dot11nConfigEntry.dot11nBasicMCS & 0xff00) >> 8;
			ht_ie->basic_mcs[2] = (priv->pmib->dot11nConfigEntry.dot11nBasicMCS & 0xff0000) >> 16;
			ht_ie->basic_mcs[3] = (priv->pmib->dot11nConfigEntry.dot11nBasicMCS & 0xff000000) >> 24;
		}
	}
	else
#ifdef CLIENT_MODE
	if ((OPMODE & WIFI_AP_STATE) || (OPMODE & WIFI_ADHOC_STATE) )
#endif
	{
#ifdef WIFI_11N_2040_COEXIST
		if (priv->pmib->dot11nConfigEntry.dot11nCoexist && (OPMODE & WIFI_AP_STATE)) {
			ht_ie = &priv->ht_ie_buf;
			ht_ie->info0 &= ~(_HTIE_2NDCH_OFFSET_BL_ | _HTIE_STA_CH_WDTH_);
			if (use_40m && (!COEXIST_ENABLE || !(priv->bg_ap_timeout || orForce20_Switch20Map(priv))) ) {
				if (offset == HT_2NDCH_OFFSET_BELOW)
					ch_offset = _HTIE_2NDCH_OFFSET_BL_ | _HTIE_STA_CH_WDTH_;
				else
					ch_offset = _HTIE_2NDCH_OFFSET_AB_ | _HTIE_STA_CH_WDTH_;
			} else {
				ch_offset = _HTIE_2NDCH_OFFSET_NO_;
			}
			ht_ie->info0 |= ch_offset;
		}
#endif
		if (!priv->pmib->dot11StationConfigEntry.protectionDisabled) {
			if (priv->ht_legacy_obss_to || priv->ht_legacy_sta_num)
				priv->ht_protection = 1;
			else
				priv->ht_protection = 0;;
		}

		if (priv->ht_legacy_sta_num) {
			priv->ht_ie_buf.info1 |= cpu_to_le16(_HTIE_OP_MODE3_);
		} else if (priv->ht_legacy_obss_to || priv->ht_nomember_legacy_sta_to) {
			priv->ht_ie_buf.info1 &= cpu_to_le16(~_HTIE_OP_MODE3_);
			priv->ht_ie_buf.info1 |= cpu_to_le16(_HTIE_OP_MODE1_);
		} else {
			priv->ht_ie_buf.info1 &= cpu_to_le16(~_HTIE_OP_MODE3_);
		}

		if (priv->ht_protection)
			priv->ht_ie_buf.info1 |= cpu_to_le16(_HTIE_OBSS_NHT_STA_);
		else
			priv->ht_ie_buf.info1 &= cpu_to_le16(~_HTIE_OBSS_NHT_STA_);
	}
#if defined(UNIVERSAL_REPEATER) 
	if( IS_DRV_OPEN(GET_VXD_PRIV(GET_ROOT(priv)))) {
		ht_cap = &priv->ht_cap_buf;
		ht_cap->ht_cap_info |= cpu_to_le16( _HTCAP_SUPPORT_CH_WDTH_ | _HTCAP_SHORTGI_40M_);
	}
#endif	
}


unsigned char *construct_ht_ie_old_form(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen)
{
	unsigned char old_ht_ie_id[] = {0x00, 0x90, 0x4c};

	*pbuf = _RSN_IE_1_;
	*(pbuf + 1) = 3 + 1 + priv->ht_cap_len;
	memcpy((pbuf + 2), old_ht_ie_id, 3);
	*(pbuf + 5) = 0x33;
	memcpy((pbuf + 6), (unsigned char *)&priv->ht_cap_buf, priv->ht_cap_len);
	*frlen += (*(pbuf + 1) + 2);
	pbuf +=(*(pbuf + 1) + 2);

	*pbuf = _RSN_IE_1_;
	*(pbuf + 1) = 3 + 1 + priv->ht_ie_len;
	memcpy((pbuf + 2), old_ht_ie_id, 3);
	*(pbuf + 5) = 0x34;
	memcpy((pbuf + 6), (unsigned char *)&priv->ht_ie_buf, priv->ht_ie_len);
	*frlen += (*(pbuf + 1) + 2);
	pbuf +=(*(pbuf + 1) + 2);

	return pbuf;
}


#ifdef WIFI_11N_2040_COEXIST
void construct_obss_scan_para_ie(struct rtl8192cd_priv *priv)
{
	struct obss_scan_para_elmt		*obss_scan_para;

	if (priv->obss_scan_para_len == 0) {
		priv->obss_scan_para_len = sizeof(struct obss_scan_para_elmt);
		obss_scan_para = &priv->obss_scan_para_buf;
		memset(obss_scan_para, 0, sizeof(struct obss_scan_para_elmt));

		// except word2, all are default values and meaningless for ap at present
		obss_scan_para->word0 = cpu_to_le16(0x14);
		obss_scan_para->word1 = cpu_to_le16(0x0a);
		obss_scan_para->word2 = cpu_to_le16(180);	// set as 180 second for 11n test plan
		obss_scan_para->word3 = cpu_to_le16(0xc8);
		obss_scan_para->word4 = cpu_to_le16(0x14);
		obss_scan_para->word5 = cpu_to_le16(5);
		obss_scan_para->word6 = cpu_to_le16(0x19);
	}
}

int bg_ap_rssi_chk(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, int channel)
{
	int i;
	int ch_begin = -1;
	int ch_end = -1;
	int chk_l = 0;
	int chk_h = 0;

	for (i = 0; i < priv->available_chnl_num; i++)
	{
		if (priv->available_chnl[i] <= 14)
		{
			if (ch_begin == -1)
				ch_begin = priv->available_chnl[i];

			ch_end = priv->available_chnl[i];
		}
		else
			break;
	}


	if (priv->pmib->dot11RFEntry.dot11channel && (priv->pmib->dot11RFEntry.dot11channel <= 14))
	{
		if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_ABOVE)
		{
			chk_l = priv->pmib->dot11RFEntry.dot11channel + 2 - 5; 
			chk_h = priv->pmib->dot11RFEntry.dot11channel + 2 + 5; 
		}

		if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
		{
			chk_l = priv->pmib->dot11RFEntry.dot11channel - 2 - 5; 
			chk_h = priv->pmib->dot11RFEntry.dot11channel - 2 + 5; 
		}
		
		if (chk_l < ch_begin)
			chk_l = ch_begin;
		if (chk_h > ch_end)
			chk_h = ch_end; 

		if ((channel >= chk_l) && (channel <= chk_h))
		{
			if (pfrinfo->rssi >= priv->pmib->dot11nConfigEntry.dot11nBGAPRssiChkTh)
				return 1;
		}
	}
	
	return 0;
}
#endif

#if 1
//#ifdef PCIE_POWER_SAVING
void fill_bcn_desc(struct rtl8192cd_priv *priv, struct tx_desc *pdesc, void *dat_content, unsigned short txLength, char forceUpdate)
{
#if defined(CONFIG_RTL_8812_SUPPORT)	
if (GET_CHIP_VER(priv)==VERSION_8812E)
{
	memset(pdesc, 0, 40);	
	pdesc->Dword0 |= set_desc(TX_FirstSeg | TX_LastSeg);
	pdesc->Dword0 |= set_desc(TX_BMC| (40<<TX_OffsetSHIFT));
	pdesc->Dword10 = set_desc(get_physical_addr(priv, dat_content, txLength, PCI_DMA_TODEVICE));
	pdesc->Dword0 |= set_desc((unsigned short)(txLength) << TX_PktSizeSHIFT);
	pdesc->Dword1 |= set_desc(0x10 << TX_QSelSHIFT);
	pdesc->Dword9 |= set_desc((GetSequence(dat_content) & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
	pdesc->Dword3 = set_desc(TXdesc_92E_DisDataFB|TXdesc_92E_UseRate);

	if (priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_80) {
		pdesc->Dword5 |= set_desc((priv->pshare->txsc_20 << TXdesc_92E_DataScSHIFT));
	} else if (priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_20_40)   	{
		if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
			pdesc->Dword5 |= set_desc(2 << TXdesc_92E_DataScSHIFT);
		else
			pdesc->Dword5 |= set_desc(1 << TXdesc_92E_DataScSHIFT);
	}
	
    priv->pshare->is_40m_bw_bak = priv->pshare->is_40m_bw;
    
    priv->tx_beacon_len = txLength;

	pdesc->Dword7 |= set_desc((unsigned short)(txLength) & TX_TxBufSizeMask);
	pdesc->Dword9 |= set_desc((GetSequence(dat_content) & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
	//pdesc->Dword9 |= set_desc((priv->timoffset & TXdesc_92E_TX_GroupIEMask) | TXdesc_92E_TX_GroupIEEnable);

	pdesc->Dword4 |= set_desc(0x1f << DATA_RATE_FB_LIMIT);
	pdesc->Dword4 |= set_desc(0xf << RTS_RATE_FB_LIMIT);

	if ((priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)||(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80)||
		((GET_CHIP_VER(priv)==VERSION_8814A) && (priv->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_2G))) //AC2G_256QAM
	{
		pdesc->Dword4 |= set_desc((4 & TXdesc_92E_RtsRateMask) << TXdesc_92E_RtsRateSHIFT);
		pdesc->Dword4 |= set_desc((4 & TXdesc_92E_DataRateMask) << TXdesc_92E_DataRateSHIFT);
	}

/*
		 * Intel IOT, dynamic enhance beacon tx AGC
*/
	pdesc->Dword5 &= set_desc(~((TXdesc_8812_TxPwrOffetMask) << TXdesc_8812_TxPwrOffetSHIFT));
	if (priv->bcnTxAGC ==1)
		pdesc->Dword5 |= set_desc((4 & TXdesc_8812_TxPwrOffetMask) << TXdesc_8812_TxPwrOffetSHIFT);
	else if (priv->bcnTxAGC ==2)
		pdesc->Dword5 |= set_desc((5 & TXdesc_8812_TxPwrOffetMask) << TXdesc_8812_TxPwrOffetSHIFT);

	return; //_eric_8812 ??
}	
#endif  //defined(CONFIG_RTL_8812_SUPPORT)
/*
     * Intel IOT, dynamic enhance beacon tx AGC
    */
	if (priv->bcnTxAGC_bak != priv->bcnTxAGC || forceUpdate)
    {
		memset((void *)&pdesc->Dword6, 0, 4);

#ifdef HIGH_POWER_EXT_PA
	    if (!priv->pshare->rf_ft_var.use_ext_pa)
#endif
        if (priv->bcnTxAGC) 
		{
			pdesc->Dword6 |= set_desc((((priv->bcnTxAGC*6) & 0xfffffffe) & TX_TxAgcAMask) << TX_TxAgcASHIFT);
			pdesc->Dword6 |= set_desc((((priv->bcnTxAGC*6) & 0xfffffffe) & TX_TxAgcBMask) << TX_TxAgcBSHIFT);
		}
        priv->bcnTxAGC_bak = priv->bcnTxAGC;
    }

	if (priv->pshare->is_40m_bw != priv->pshare->is_40m_bw_bak || forceUpdate) {
		memset((void *)&pdesc->Dword4, 0, 4);

		pdesc->Dword4 = set_desc(TX_DisDataFB | TX_UseRate);

		if (priv->pshare->is_40m_bw) {
			if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
				pdesc->Dword4 |= set_desc(2 << TX_DataScSHIFT);
			else
				pdesc->Dword4 |= set_desc(1 << TX_DataScSHIFT);
		}
		priv->pshare->is_40m_bw_bak = priv->pshare->is_40m_bw;

#ifdef CONFIG_RTL_92D_SUPPORT
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
			pdesc->Dword4 |= set_desc((4 & TX_RtsRateMask) << TX_RtsRateSHIFT);
#endif
	}

	if (txLength != priv->tx_beacon_len || forceUpdate)
	{
		memset(pdesc, 0, 24);
		
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		pdesc->Dword0 |= set_desc(TX_OWN|TX_BMC|TX_FirstSeg | TX_LastSeg | ((32)<<TX_OffsetSHIFT));
#elif defined(CONFIG_PCI_HCI)
		memset((void *)&pdesc->Dword7, 0, 8);
		
		pdesc->Dword0 |= set_desc(TX_BMC|TX_FirstSeg | TX_LastSeg | ((32)<<TX_OffsetSHIFT));
#endif
		pdesc->Dword0 |= set_desc((unsigned short)(txLength) << TX_PktSizeSHIFT);
		pdesc->Dword1 |= set_desc(0x10 << TX_QSelSHIFT);

		if (priv->pmib->dot11RFEntry.bcn2path){
			RTL_W32(0x80c,RTL_R32(0x80c)|BIT(31));
			pdesc->Dword2 &= set_desc(0x03ffffff); // clear related bits
			pdesc->Dword2 |= set_desc(3 << TX_TxAntCckSHIFT);	// Set Default CCK rate with 2T
		}
		pdesc->Dword3 |= set_desc((GetSequence(dat_content) & TX_SeqMask) << TX_SeqSHIFT);
		pdesc->Dword4 = set_desc(TX_DisDataFB | TX_UseRate);

		if (priv->pshare->is_40m_bw)
        {
			if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
				pdesc->Dword4 |= set_desc(2 << TX_DataScSHIFT);
			else
				pdesc->Dword4 |= set_desc(1 << TX_DataScSHIFT);
		}
        priv->pshare->is_40m_bw_bak = priv->pshare->is_40m_bw;

		if (priv->pmib->dot11RFEntry.txbf == 1) {
			pdesc->Dword2 &= set_desc(0x03ffffff); // clear related bits

			pdesc->Dword2 |= set_desc(1 << TX_TxAntCckSHIFT);	// Set Default CCK rate with 1T
			pdesc->Dword2 |= set_desc(1 << TX_TxAntlSHIFT); 	// Set Default Legacy rate with 1T
			pdesc->Dword2 |= set_desc(1 << TX_TxAntHtSHIFT);	// Set Default Ht rate	
		}

#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
				pdesc->Dword4 |= set_desc((4 & TX_RtsRateMask) << TX_RtsRateSHIFT);
				pdesc->Dword5 |= set_desc((4 & TX_DataRateMask) << TX_DataRateSHIFT);
			}
		}
#endif

		priv->tx_beacon_len = txLength;

#ifdef CONFIG_PCI_HCI
		pdesc->Dword7 |= set_desc((unsigned short)(txLength) & TX_TxBufSizeMask);
		pdesc->Dword8 = set_desc(get_physical_addr(priv, dat_content, txLength, PCI_DMA_TODEVICE));
#endif // CONFIG_PCI_HCI
	}
	else
	{
		memset((void *)&pdesc->Dword3, 0, 4);
		pdesc->Dword3 |= set_desc((GetSequence(dat_content) & TX_SeqMask) << TX_SeqSHIFT);
	}
    
}
#endif

#ifdef CONFIG_PCI_HCI
void signin_beacon_desc(struct rtl8192cd_priv *priv, unsigned int *beaconbuf, unsigned int frlen)
{
	struct rtl8192cd_hw	*phw=GET_HW(priv);
	struct tx_desc		*pdesc;
//	unsigned int			next_idx = 1;

#ifdef MBSSID
	if (IS_VAP_INTERFACE(priv)) {
		pdesc = phw->tx_descB + priv->vap_init_seq;
//		next_idx =  priv->vap_init_seq + 1;
	}
	else
#endif
		pdesc = phw->tx_descB;

	//memset(pdesc, 0, 32);	// clear all bit

#ifdef DFS
	if (!priv->pmib->dot11DFSEntry.disable_DFS &&
		(timer_pending(&GET_ROOT(priv)->ch_avail_chk_timer)) &&
		(GET_CHIP_VER(priv) == VERSION_8192D)) {
			pdesc->Dword0 &= set_desc(~(TX_OWN));
			RTL_W16(PCIE_CTRL_REG, RTL_R16(PCIE_CTRL_REG)| (BCNQSTOP));
		return;
	}
#endif


	fill_bcn_desc(priv, pdesc, (void*)beaconbuf, frlen, 0);
	
#ifdef CONFIG_RTL_8812_SUPPORT	
	if(GET_CHIP_VER(priv)== VERSION_8812E)
	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(get_desc(pdesc->Dword10)), frlen, PCI_DMA_TODEVICE);
	else
#endif
	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(get_desc(pdesc->Dword8)), frlen, PCI_DMA_TODEVICE);


#ifdef RESERVE_TXDESC_FOR_EACH_IF
	if (GET_ROOT(priv)->pmib->miscEntry.rsv_txdesc)
		priv->use_txdesc_cnt[BEACON_QUEUE]++;
#endif

#ifdef MBSSID
if((GET_CHIP_VER(priv)== VERSION_8188C) || (GET_CHIP_VER(priv)== VERSION_8192C) || (GET_CHIP_VER(priv)== VERSION_8192D))
{
	unsigned int tmp_522 = RTL_R8(0x522); //TXPAUSE register
	if (priv->pmib->miscEntry.func_off && IS_ROOT_INTERFACE(priv))
		RTL_W8(0x522, tmp_522 | BIT(6));
	else if(tmp_522 & BIT(6))
		RTL_W8(0x522, tmp_522 & ~BIT(6));
}
#endif

	pdesc->Dword0 |= set_desc(TX_OWN);
}
#endif // CONFIG_PCI_HCI


int fill_probe_rsp_content(struct rtl8192cd_priv*, UINT8*, UINT8*, UINT8*, int , int , UINT8, UINT8);


/**
 *	@brief	Update beacon content
 *
 *	IBSS parameter set (STA), TIM (AP), ERP & Ext rate not set  \n \n
 *	+----------------------------+-----+--------------------+-----+---------------------+-----+------------+-----+	\n
 *	| DS parameter (init_beacon) | TIM | IBSS parameter set | ERP | EXT supported rates | RSN | Realtek IE | CRC |	\n
 *	+----------------------------+-----+--------------------+-----+---------------------+-----+------------+-----+	\n
 *	\n
 *	set_desc() set data to hardware, 8190n_hw.h define value
 */


#if 1//def RTK_AC_SUPPORT //for 11ac logo
int get_center_channel(struct rtl8192cd_priv *priv, int channel, int offset, int cur) 
{
	int val = channel, bw=0;

	if (cur)
		bw = priv->pshare->CurrentChannelBW;
	else
		bw = priv->pmib->dot11nConfigEntry.dot11nUse40M;

#ifdef RTK_AC_SUPPORT
	if (bw == HT_CHANNEL_WIDTH_80)
	{
#ifdef AC2G_256QAM
		if(is_ac2g(priv))
			val = 7;
		else
#endif
		if(channel <= 48)
			val = 42;
		else if(channel <= 64)
			val = 58;
		else if(channel <= 112)
			val = 106;
		else if(channel <= 128)
			val = 122;
		else if(channel <= 144)
			val = 138;
		else if(channel <= 161)
			val = 155;
		else if(channel <= 177)
			val = 171;	
	} else 
#endif		
	if (bw == HT_CHANNEL_WIDTH_20_40) {
		if (offset == 1)
			val -= 2;
		else
			val += 2;
	}

	//SDEBUG("channel[%d] offset[%d] bw[%d] cent[%d]\n", channel, offset, bw, val);
	return val;
}


#endif


#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
int is_main_AP_interface(struct rtl8192cd_priv *priv)
{
	int i;

	if (IS_ROOT_INTERFACE(priv)) {
		if (priv->pmib->miscEntry.func_off == 0)
			return TRUE;
		else
			return FALSE;
	}
	else {	// vap interfaces
		if (GET_ROOT(priv)->pmib->miscEntry.func_off == 0)
			return FALSE;
#if defined(MBSSID)
		else {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(GET_ROOT(priv)->pvap_priv[i])) {
					if ((GET_ROOT(priv)->pvap_priv[i])->pmib->miscEntry.func_off == 0) {
						if (priv == (GET_ROOT(priv)->pvap_priv[i]))
							return TRUE;
						break;
					}
				}
			}
			return FALSE;
		}
#endif
	}
}
#endif


void update_beacon(struct rtl8192cd_priv *priv)
{
#if !defined(SMP_SYNC) && defined(CONFIG_RTL_WAPI_SUPPORT)
	unsigned long		flags;
#endif

#ifdef TDLS_SUPPORT
    unsigned char tempbuf[6];
#endif


	struct wifi_mib *pmib;
	struct rtl8192cd_hw	*phw;
	unsigned int	frlen;
	unsigned char	*pbuf;
	unsigned char	*pbssrate=NULL;
	int				bssrate_len;
#if defined(TV_MODE) || defined(A4_STA) || defined(USER_ADDIE)   
	unsigned int	i;
#endif

#ifdef	CONFIG_RTK_MESH
	UINT8		meshiearray[32];	// mesh IE buffer (Max byte is mesh_ie_MeshID)
#endif

	pmib = GET_MIB(priv);
	phw = GET_HW(priv);

	if (priv->update_bcn_period)
	{
		unsigned short val16 = 0;
		pbuf = (unsigned char *)priv->beaconbuf;
		frlen = 0;

		pbuf += 24;
		frlen += 24;

        frlen += _TIMESTAMP_;   // for timestamp
	    pbuf += _TIMESTAMP_;

		//setup BeaconPeriod...
        val16 = cpu_to_le16(pmib->dot11StationConfigEntry.dot11BeaconPeriod);
	    pbuf = set_fixed_ie(pbuf, _BEACON_ITERVAL_, (unsigned char *)&val16, &frlen);
		priv->update_bcn_period = 0;
	}
	frlen = priv->timoffset;
	pbuf = (unsigned char *)priv->beaconbuf + priv->timoffset;

	// setting tim field...
	if (OPMODE & WIFI_AP_STATE)
		pbuf = update_tim(priv, pbuf, &frlen);

	if (OPMODE & WIFI_ADHOC_STATE) {
		unsigned short val16 = 0;
		pbuf = set_ie(pbuf, _IBSS_PARA_IE_, 2, (unsigned char *)&val16, &frlen);
	}

#if defined(DOT11D) || defined(DOT11H)    
    if(priv->countryTableIdx) {    
        pbuf = construct_country_ie(priv, pbuf, &frlen);   
    }
#endif

#ifdef DOT11H
    if(priv->pmib->dot11hTPCEntry.tpc_enable) {
        pbuf = set_ie(pbuf, _PWR_CONSTRAINT_IE_, 1, &priv->pshare->rf_ft_var.lpwrc, &frlen);        
        pbuf = construct_TPC_report_ie(priv, pbuf, &frlen);
    }
#endif

		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
			// ERP infomation
			unsigned char val8 = 0;
			if (priv->pmib->dot11ErpInfo.protection)
				val8 |= BIT(1);
			if (priv->pmib->dot11ErpInfo.nonErpStaNum)
				val8 |= BIT(0);

		if (!SHORTPREAMBLE || priv->pmib->dot11ErpInfo.longPreambleStaNum)
			val8 |= BIT(2);

		pbuf = set_ie(pbuf, _ERPINFO_IE_, 1, &val8, &frlen);
	}

		// EXT supported rates
		if (get_bssrate_set(priv, _EXT_SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len))
			pbuf = set_ie(pbuf, _EXT_SUPPORTEDRATES_IE_, bssrate_len, pbssrate, &frlen);


		/*
			2008-12-16, For Buffalo WLI_CB_AG54L 54Mbps NIC interoperability issue.
			This NIC can not connect to our AP when our AP is set to WPA/TKIP encryption.
			This issue can be fixed after move "HT Capability Info" and "Additional HT Info" in front of "WPA" and "WMM".
		 */
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
			construct_ht_ie(priv, priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);
			pbuf = set_ie(pbuf, _HT_CAP_, priv->ht_cap_len, (unsigned char *)&priv->ht_cap_buf, &frlen);
			pbuf = set_ie(pbuf, _HT_IE_, priv->ht_ie_len, (unsigned char *)&priv->ht_ie_buf, &frlen);
		}
// beacon
#ifdef RTK_AC_SUPPORT //for 11ac logo
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
			// 41
			{
				char tmp[8];
				memset(tmp, 0, 8);
				tmp[7] = 0x40;
				pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 8, tmp, &frlen);
			}
			// 60, 61
			construct_vht_ie(priv, priv->pshare->working_channel);
			pbuf = set_ie(pbuf, EID_VHTCapability, priv->vht_cap_len, (unsigned char *)&priv->vht_cap_buf, &frlen);
			pbuf = set_ie(pbuf, EID_VHTOperation, priv->vht_oper_len, (unsigned char *)&priv->vht_oper_buf, &frlen);
			// 62 
			if(priv->pshare->rf_ft_var.lpwrc) {
				char tmp[4];
				pbuf = set_ie(pbuf, _PWR_CONSTRAINT_IE_, 1, &priv->pshare->rf_ft_var.lpwrc, &frlen);
				tmp[1] = tmp[2] = tmp[3] = priv->pshare->rf_ft_var.lpwrc;
				tmp[0] = priv->pshare->CurrentChannelBW;	//	20, 40, 80
				pbuf = set_ie(pbuf, EID_VHTTxPwrEnvelope, tmp[0]+2, tmp, &frlen);	
			}
		}
#endif


#ifdef CONFIG_RTK_MESH		// Mesh IE
	if (GET_MIB(priv)->dot1180211sInfo.mesh_enable && !GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
		// OFDM Parameter Set
        pbuf = set_ie(pbuf, _OFDM_PARAMETER_SET_IE_, mesh_ie_OFDM(priv, meshiearray), meshiearray, &frlen);
		// Mesh ID
		pbuf = set_ie(pbuf, _MESH_ID_IE_, mesh_ie_MeshID(priv, meshiearray, FALSE), meshiearray, &frlen);

		// WLAN Mesh Capability
		pbuf = set_ie(pbuf, _WLAN_MESH_CAP_IE_, mesh_ie_WLANMeshCAP(priv, meshiearray), meshiearray, &frlen);

        if(priv->mesh_swchnl_channel) { /*mesh channel switch procedure is on-going*/
            pbuf = set_ie(pbuf, _MESH_CHANNEL_SWITCH_IE_, mesh_ie_MeshChannelSwitch(priv, meshiearray), meshiearray, &frlen);
            pbuf = set_ie(pbuf, _CSA_IE_, mesh_ie_ChannelSwitchAnnoun(priv, meshiearray), meshiearray, &frlen);            
            pbuf = set_ie(pbuf, _SECONDARY_CHANNEL_OFFSET_IE_, mesh_ie_SecondaryChannelOffset(priv, meshiearray), meshiearray, &frlen);
        }
	}

    /* if pure Mesh Point without mesh enable, don't send beacon*/
	if ( (OPMODE & WIFI_AP_STATE) && pmib->dot1180211sInfo.meshSilence )
		return;   
#endif	// CONFIG_RTK_MESH

// 63
#if defined(DFS) || defined(RTK_AC_SUPPORT) //for 11ac logo
	if (GET_ROOT(priv)->pmib->dot11DFSEntry.DFS_detected && priv->pshare->dfsSwitchChannel) {
		static unsigned int set_stop_bcn = 0;

		if (priv->pshare->dfsSwitchChCountDown) {
			unsigned char tmpStr[3];
			tmpStr[0] = 1;	/* channel switch mode */
			tmpStr[1] = priv->pshare->dfsSwitchChannel;	/* new channel number */
			tmpStr[2] = priv->pshare->dfsSwitchChCountDown;	/* channel switch count */
			pbuf = set_ie(pbuf, _CSA_IE_, 3, tmpStr, &frlen);

#if defined(RTK_AC_SUPPORT) //for 11ac logo
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
				unsigned char tmp2[15];
				int len = 5;
								
				tmp2[0] = EID_WIDEBW_CH_SW;
				tmp2[1] = 3;
				tmp2[2] = (priv->pmib->dot11nConfigEntry.dot11nUse40M==2) ? 1 : 0;
				tmp2[3] = get_center_channel(priv, priv->pshare->dfsSwitchChannel, priv->pshare->offset_2nd_chan, 0);
				tmp2[4] = 0;
				pbuf = set_ie(pbuf, EID_CH_SW_WRAPPER, len, tmp2, &frlen);
			}
#endif

			
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			//if (IS_ROOT_INTERFACE(priv))
			if (is_main_AP_interface(priv))
#endif
			{
				priv->pshare->dfsSwitchChCountDown--;
				if (set_stop_bcn)
					set_stop_bcn = 0;
			}
		}
        else {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			//if (IS_ROOT_INTERFACE(priv))
			if (is_main_AP_interface(priv))
#endif
			{
#if	defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE) 
				if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A)|| (GET_CHIP_VER(priv) == VERSION_8814A)) {
					if (GET_CHIP_VER(priv) == VERSION_8881A){
						PHY_SetBBReg(priv, 0xcb0, 0x000000f0, 4);
					}
					SwitchChannel(GET_ROOT(priv));
					if (GET_CHIP_VER(priv) == VERSION_8881A){
						delay_us(500);
						PHY_SetBBReg(priv, 0xcb0, 0x000000f0, 5);
					}
				}
				else
#endif
				{
#ifdef DFS						
#ifdef __ECOS
					priv->pshare->has_triggered_dfs_switch_channel = 1;
					priv->pshare->call_dsr = 1;
#else
					DFS_SwitchChannel(GET_ROOT(priv));
#endif
#endif
				}
			} 
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			else {
				if (!set_stop_bcn) {
#ifdef  CONFIG_WLAN_HAL
					if (IS_HAL_CHIP(priv)) {
                        u8  RegTXPause;
                        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_TXPAUSE, (pu1Byte)&RegTXPause);
                        RegTXPause |= TXPAUSE_BCN_QUEUE_BIT;
                        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_TXPAUSE, (pu1Byte)&RegTXPause);
					} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
					{//not HAL
						RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) | STOP_BCN);
					}
					set_stop_bcn++;
				}
			}
#endif
			return;
		}
	}
#endif

#ifdef RTK_AC_SUPPORT //for 11ac logo
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
		//66
		if(priv->pshare->rf_ft_var.opmtest&1) {
			pbuf = set_ie(pbuf, EID_VHTOperatingMode, 1, (unsigned char *)&(priv->pshare->rf_ft_var.oper_mode_field), &frlen);
		}	
	}
#endif	

#ifdef WIFI_11N_2040_COEXIST
	if ((OPMODE & WIFI_AP_STATE) && 
		((priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N|WIRELESS_11G))==(WIRELESS_11N|WIRELESS_11G)) &&
		COEXIST_ENABLE && priv->pshare->is_40m_bw) {
		unsigned char temp_buf = _2040_COEXIST_SUPPORT_ ;
		construct_obss_scan_para_ie(priv);
		pbuf = set_ie(pbuf, _OBSS_SCAN_PARA_IE_, priv->obss_scan_para_len,
			(unsigned char *)&priv->obss_scan_para_buf, &frlen);
#ifdef HS2_SUPPORT
        if (priv->pmib->hs2Entry.interworking_ielen)
        {   
			unsigned char capArray[6];
			unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_ | _2040_COEXIST_SUPPORT_ | _BSS_TRANSITION_;
			if (priv->proxy_arp)
			    buf32 |= _PROXY_ARP_;

			if ((priv->pmib->hs2Entry.timezone_ielen!=0) && (priv->pmib->hs2Entry.timeadvt_ielen))
			    buf32 |= _UTC_TSF_OFFSET_;

			temp_buf32 = cpu_to_le32(buf32);
			//capArray[5]=0x40; //WNM notification
			capArray[4]=0x0;
			memcpy(capArray,(void *)(&temp_buf32),4);
			pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 5, capArray, &frlen);
			//pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        }
        else
#endif

		pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 1, &temp_buf, &frlen);
	}
#ifdef HS2_SUPPORT
	else
	{   
	    if (priv->pmib->hs2Entry.interworking_ielen)
	    {
	        unsigned char capArray[6];
	        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_ | _BSS_TRANSITION_;
	        if (priv->proxy_arp)
	            buf32 |= _PROXY_ARP_;
	        
	        if ((priv->pmib->hs2Entry.timezone_ielen!=0) && (priv->pmib->hs2Entry.timeadvt_ielen))
	                    buf32 |= _UTC_TSF_OFFSET_;
	    
	             temp_buf32 = cpu_to_le32(buf32);
	         //capArray[5]=0x40; //WNM notification
	         capArray[4]=0;
	         memcpy(capArray,(void *)(&temp_buf32),4);
	        
	         pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 5, capArray, &frlen);
	         //pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
	    }
	}
#endif
#endif

	if (pmib->dot11RsnIE.rsnielen) {
		memcpy(pbuf, pmib->dot11RsnIE.rsnie, pmib->dot11RsnIE.rsnielen);
		pbuf += pmib->dot11RsnIE.rsnielen;
		frlen += pmib->dot11RsnIE.rsnielen;
	}

#ifdef WIFI_WMM
	//Set WMM Parameter Element
	if (QOS_ENABLE)
		pbuf = set_ie(pbuf, _RSN_IE_1_, _WMM_Para_Element_Length_, GET_WMM_PARA_IE, &frlen);
#endif

#if defined(TV_MODE) || defined(A4_STA)
    i = 0;
#ifdef A4_STA
    if(priv->pshare->rf_ft_var.a4_enable == 2) {
        i |= BIT0;
    }
#endif

#ifdef TV_MODE
    i |= BIT1;
#endif
    if(i)
        pbuf = construct_ecm_tvm_ie(priv, pbuf, &frlen, i);
#endif //defined(TV_MODE) || defined(A4_STA)



#ifdef TDLS_SUPPORT 
				if(priv->pmib->dot11OperationEntry.tdls_prohibited){ 
					tempbuf[4] = 0x40;	// bit 38(tdls_prohibited)
				}
				if(priv->pmib->dot11OperationEntry.tdls_cs_prohibited){ 
					tempbuf[4] = 0x80;	// bit 39(tdls_cs_prohibited)
				}
				pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 5, tempbuf, &frlen); 		 
#endif
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
		/*
			2008-12-16, For Buffalo WLI_CB_AG54L 54Mbps NIC interoperability issue.
			This NIC can not connect to our AP when our AP is set to WPA/TKIP encryption.
			This issue can be fixed after move "HT Capability Info" and "Additional HT Info" in front of "WPA" and "WMM".
		 */
		//construct_ht_ie(priv, priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);
		//pbuf = set_ie(pbuf, _HT_CAP_, priv->ht_cap_len, (unsigned char *)&priv->ht_cap_buf, &frlen);
		//pbuf = set_ie(pbuf, _HT_IE_, priv->ht_ie_len, (unsigned char *)&priv->ht_ie_buf, &frlen);
		pbuf = construct_ht_ie_old_form(priv, pbuf, &frlen);
	}

	// Realtek proprietary IE
	if (priv->pshare->rtk_ie_len)
		pbuf = set_ie(pbuf, _RSN_IE_1_, priv->pshare->rtk_ie_len, priv->pshare->rtk_ie_buf, &frlen);

		// Customer proprietary IE
		if (priv->pmib->miscEntry.private_ie_len) {
			memcpy(pbuf, pmib->miscEntry.private_ie, pmib->miscEntry.private_ie_len);
			pbuf += pmib->miscEntry.private_ie_len;
			frlen += pmib->miscEntry.private_ie_len;
		}

	if(priv->pmib->miscEntry.stage) {
		unsigned char tmp[10] = { 0x00, 0x0d, 0x02, 0x07, 0x01, 0x00, 0x00 };
		if(priv->pmib->miscEntry.stage<=5)
			tmp[6] = 1<<(8-priv->pmib->miscEntry.stage);
		pbuf = set_ie(pbuf, _RSN_IE_1_, 7, tmp, &frlen);
	}
#ifdef USER_ADDIE
{
	int i;
	for (i=0; i<MAX_USER_IE; i++) {
		if (priv->user_ie_list[i].used) {
			memcpy(pbuf, priv->user_ie_list[i].ie, priv->user_ie_list[i].ie_len);
			pbuf += priv->user_ie_list[i].ie_len;
			fr_len += priv->user_ie_list[i].ie_len; 
		}
	}
}
#endif
#if defined(CONFIG_RTL_WAPI_SUPPORT)
		if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
		{
			SAVE_INT_AND_CLI(flags);
			*priv->pBeaconCapability |= cpu_to_le16(BIT(4));	/* set privacy	*/
			priv->wapiCachedBuf = pbuf+2;
			wapiSetIE(priv);
			pbuf[0] = _EID_WAPI_;
			pbuf[1] = priv->wapiCachedLen;
			pbuf += priv->wapiCachedLen+2;
			frlen += priv->wapiCachedLen+2;
			RESTORE_INT(flags);
		}
#endif

#ifdef WIFI_SIMPLE_CONFIG
/*modify for WPS2DOTX SUPPORT*/
	if (pmib->wscEntry.wsc_enable && pmib->wscEntry.beacon_ielen 
		&& priv->pmib->dot11StationConfigEntry.dot11AclMode!=ACL_allow) {
		memcpy(pbuf, pmib->wscEntry.beacon_ie, pmib->wscEntry.beacon_ielen);
		pbuf += pmib->wscEntry.beacon_ielen;
		frlen += pmib->wscEntry.beacon_ielen;
	}
#endif

#ifdef HS2_SUPPORT
	// support hs2 enable, p2p disable
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.hs2_ielen)
	{
		//unsigned char p2ptmpie[]={0x50,0x6f,0x9a,0x09,0x02,0x02,0x00,0x00,0x00};
		unsigned char p2ptmpie[]={0x50,0x6f,0x9a,0x09,0x0a,0x01,0x00,0x05};
		pbuf = set_ie(pbuf, 221, sizeof(p2ptmpie), p2ptmpie, &frlen);
	}	
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.hs2_ielen)
    {
        pbuf = set_ie(pbuf, _HS2_IE_, priv->pmib->hs2Entry.hs2_ielen, priv->pmib->hs2Entry.hs2_ie, &frlen);
		
		priv->pmib->hs2Entry.bssload_ie[0] = priv->assoc_num & 0xff;
		priv->pmib->hs2Entry.bssload_ie[1] = (priv->assoc_num & 0xff00) >> 8;
		priv->pmib->hs2Entry.bssload_ie[2] = priv->channel_utilization;
		priv->pmib->hs2Entry.bssload_ie[3] = 0;
		priv->pmib->hs2Entry.bssload_ie[4] = 0;
		pbuf = set_ie(pbuf, _BSS_LOAD_IE_, 5, priv->pmib->hs2Entry.bssload_ie, &frlen);
    }
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.interworking_ielen)
	{			
		unsigned char magicmac[MACADDRLEN]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
		if (!memcmp(magicmac,priv->pmib->hs2Entry.interworking_ie+3,6))	{ // this case is used for HS2 testing (means MAC address of the AP)
			memcpy(priv->pmib->hs2Entry.interworking_ie+3, priv->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);
			pbuf = set_ie(pbuf, _INTERWORKING_IE_, priv->pmib->hs2Entry.interworking_ielen, priv->pmib->hs2Entry.interworking_ie, &frlen);
			
		}
		else {
			pbuf = set_ie(pbuf, _INTERWORKING_IE_, priv->pmib->hs2Entry.interworking_ielen, priv->pmib->hs2Entry.interworking_ie, &frlen);
		}
	}		
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.advt_proto_ielen)
	{
		pbuf = set_ie(pbuf, _ADVT_PROTO_IE_, priv->pmib->hs2Entry.advt_proto_ielen, priv->pmib->hs2Entry.advt_proto_ie, &frlen);
	}
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.MBSSID_ielen)
	{
		pbuf = set_ie(pbuf, _MUL_BSSID_IE_, priv->pmib->hs2Entry.MBSSID_ielen, priv->pmib->hs2Entry.MBSSID_ie, &frlen);
	}
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.roam_ielen && priv->pmib->hs2Entry.roam_enable)
	{
		pbuf = set_ie(pbuf, _ROAM_IE_, priv->pmib->hs2Entry.roam_ielen, priv->pmib->hs2Entry.roam_ie, &frlen);
	}

	if (priv->dtimcount == 0)
    {
		if (priv->timeadvt_dtimcount == 0)
		{	
			if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.timeadvt_ielen)
			{
	            pbuf = set_ie(pbuf, _TIMEADVT_IE_, priv->pmib->hs2Entry.timeadvt_ielen, priv->pmib->hs2Entry.timeadvt_ie, &frlen);
			}
			if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.timezone_ielen)
			{
	            pbuf = set_ie(pbuf, _TIMEZONE_IE_, priv->pmib->hs2Entry.timezone_ielen, priv->pmib->hs2Entry.timezone_ie, &frlen);
			}	
			priv->timeadvt_dtimcount = priv->pmib->hs2Entry.timeadvt_DTIMIntval-1;
		}
		else
		{
			priv->timeadvt_dtimcount--;
		}
	}
#endif

#ifdef P2P_SUPPORT
	if ((rtk_p2p_is_enabled(priv)) && ((rtk_p2p_chk_role(priv,P2P_PRE_GO))||(rtk_p2p_chk_role(priv,P2P_TMP_GO))) ) 
	{
		if(priv->p2pPtr->p2p_beacon_ie_len){
			memcpy(pbuf, priv->p2pPtr->p2p_beacon_ie, priv->p2pPtr->p2p_beacon_ie_len);
			pbuf += priv->p2pPtr->p2p_beacon_ie_len;
			frlen += priv->p2pPtr->p2p_beacon_ie_len;
		}
	}
#endif

#ifdef USER_ADDIE
	for (i=0; i<MAX_USER_IE; i++) {
		if (priv->user_ie_list[i].used) {
			memcpy(pbuf, priv->user_ie_list[i].ie, priv->user_ie_list[i].ie_len);
			pbuf += priv->user_ie_list[i].ie_len;
			frlen += priv->user_ie_list[i].ie_len; 
		}
	}
#endif

/*
	pdesc->Dword0 = set_desc(TX_FirstSeg| TX_LastSeg|  (32)<<TX_OffsetSHIFT | (frlen) << TX_PktSizeSHIFT);
	pdesc->Dword1 = set_desc(0x10 << TX_QSelSHIFT);
//		pdesc->Dword4 = set_desc((0x7 << TX_RaBRSRIDSHIFT) | TX_UseRate);	// need to confirm
	pdesc->Dword4 = set_desc(TX_UseRate);
	pdesc->Dword4 = set_desc(TX_DisDataFB);
	pdesc->Dword7 = set_desc(frlen & TX_TxBufSizeMask);
*/	// by signin_beacon_desc

#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
	priv->ext_stats.tx_byte_cnt += frlen;
#endif
//#ifdef CONFIG_RTL865X_AC
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	priv->ext_stats.tx_byte_cnt += frlen;
#endif

#ifdef WDS
	// if pure WDS bridge, don't send beacon
	if ((OPMODE & WIFI_AP_STATE) && pmib->dot11WdsInfo.wdsPure)
		return;
#endif

        // if schedule off, don't send beacon
        if (priv->pmib->miscEntry.func_off)
        {
#ifdef MBSSID
            if(!((GET_CHIP_VER(priv)== VERSION_8188C) || (GET_CHIP_VER(priv)== VERSION_8192C) || (GET_CHIP_VER(priv)== VERSION_8192D)))
                return;
            else if(!IS_ROOT_INTERFACE(priv))
                return;
#else
                    return;
#endif
        }

	if (!IS_DRV_OPEN(priv))
		return;

//		pdesc->Dword0 |= set_desc(TX_OWN);	// by signin_beacon_desc
	assign_wlanseq(phw, (unsigned char *)priv->beaconbuf, NULL ,pmib
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
		, 0
#endif
		);
//		pdesc->Dword3 |= set_desc((GetSequence(priv->beaconbuf) & TX_SeqMask)<< TX_SeqSHIFT);	// by signin_beacon_desc
//		rtl_cache_sync_wback(priv, get_desc(pdesc->Dword8), 128*sizeof(unsigned int), PCI_DMA_TODEVICE);	// by signin_beacon_desc

#ifdef PCIE_POWER_SAVING
	if((priv->offload_ctrl&1) && priv->pshare->rf_ft_var.power_save) {
		unsigned char *prsp;
		int len	;
		struct tx_desc tx_desc;
		len = frlen + TX_DESC_SZ;
		if(len%PKT_PAGE_SZ)
			len = (len+PKT_PAGE_SZ-(len%PKT_PAGE_SZ)) ;
		priv->offload_ctrl =  1| (len&0x7f80);

		prsp = (unsigned char *)priv->beaconbuf  + len  ;
		memset(prsp, 0, WLAN_HDR_A3_LEN);
		len = WLAN_HDR_A3_LEN + fill_probe_rsp_content(priv, prsp, prsp+WLAN_HDR_A3_LEN, SSID, SSID_LEN, 1, 0, 0);
		assign_wlanseq(phw, prsp, NULL ,pmib
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
					, 0
#endif
		);
		memset(&tx_desc, 0, TX_DESC_SZ);
		fill_bcn_desc(priv, &tx_desc, (void*)prsp, len, 1);
		tx_desc.Dword5 |= set_desc(TX_RtyLmtEn);
		memcpy(prsp-TX_DESC_SZ, &tx_desc, TX_DESC_SZ );
		len += (priv->offload_ctrl&0x7f80) /*+TX_DESC_SZ*/;
		signin_beacon_desc(priv, priv->beaconbuf, len);
	}
    else {
		signin_beacon_desc(priv, priv->beaconbuf, frlen);
	}

	// Now we use sw beacon, we need to poll it every time.
	RTL_W8(PCIE_CTRL_REG, BCNQ_POLL);

#else


#ifdef CONFIG_OFFLOAD_FUNCTION
#ifdef  CONFIG_WLAN_HAL

	if((priv->offload_function_ctrl&1)) {
    // this section is for AP_OFFLOAD download 
		unsigned char *prsp;
		const unsigned char txDesSize = 40;
        unsigned char pkt_offset = 0;
        unsigned int dummy = 0;      
        unsigned int probePayloadLen = 0;                
        unsigned int TotalPktLen = 0;               
        
		unsigned int len	;
		len = frlen + txDesSize; // add beacon tx desc size , 
		// now len is represent from beacon desc + beacon payload

         dummy = GET_HAL_INTERFACE(priv)->GetRsvdPageLocHandler(priv,len,&pkt_offset);
        // dummy = bcn pkt_len + dummy

        priv->offload_bcn_page = RTL_R8(0x209);
        priv->offload_proc_page = RTL_R8(0x209) + pkt_offset;            

        printk("priv->offload_bcn_page =%x \n",priv->offload_bcn_page);
        printk("priv->offload_proc_page =%x \n",priv->offload_proc_page);  
        printk("dummy =%x \n",dummy);              

		prsp = (unsigned char *)priv->beaconbuf  + dummy ;
		memset(prsp, 0, WLAN_HDR_A3_LEN);
		probePayloadLen = WLAN_HDR_A3_LEN + fill_probe_rsp_content(priv, prsp, prsp+WLAN_HDR_A3_LEN, SSID, SSID_LEN, 1, 0, 0);
        len = probePayloadLen;

		assign_wlanseq(phw, prsp, NULL ,pmib
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
					, 0
#endif
		);
             
        TotalPktLen = probePayloadLen + dummy;

        GET_HAL_INTERFACE(priv)->SetRsvdPageHandler(priv,prsp,priv->beaconbuf,probePayloadLen,TotalPktLen);

        // TODO: currently, we do not implement this function, so mark it temporarily
		// 8814 merge issue
        //GET_HAL_INTERFACE(priv)->DownloadRsvdPageHandler(priv,priv->beaconbuf,frlen);
        priv->offload_function_ctrl = 0;
	}
    else 
#endif // #ifdef  CONFIG_WLAN_HAL		
#endif //#ifdef CONFIG_OFFLOAD_FUNCTION
    {
#if defined(CONFIG_PCI_HCI)
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        GET_HAL_INTERFACE(priv)->SigninBeaconTXBDHandler(priv, priv->beaconbuf, frlen);
        GET_HAL_INTERFACE(priv)->SetBeaconDownloadHandler(priv, HW_VAR_BEACON_ENABLE_DOWNLOAD);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
    {//not HAL
	    signin_beacon_desc(priv, priv->beaconbuf, frlen);
    }
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	signin_beacon_desc(priv, priv->beaconbuf, frlen);
#endif

	// Now we use sw beacon, we need to poll it every time.
	//RTL_W8(PCIE_CTRL_REG, BCNQ_POLL);
#endif		
    }
}


#ifdef RTK_NL80211 // wrt-adhoc
void construct_ibss_beacon(struct rtl8192cd_priv *priv)
{
	unsigned short	val16;
	unsigned char	val8;
	struct wifi_mib *pmib;
	unsigned char	*bssid;

	unsigned int	frlen=0;
	unsigned char	*pbuf=(unsigned char *)priv->beaconbuf_ibss_vxd;
	unsigned char	*pbssrate=NULL;
	int	bssrate_len;
	struct FWtemplate *txfw;
	struct FWtemplate Temptxfw;

	unsigned int rate;


	pmib = GET_MIB(priv);
	bssid = pmib->dot11StationConfigEntry.dot11Bssid;



	{
		unsigned char tmpbssid[MACADDRLEN];

		unsigned char random;
		int i =0;
			
		memset(tmpbssid, 0, MACADDRLEN);
		if (!memcmp(BSSID, tmpbssid, MACADDRLEN)) {
			// generate an unique Ibss ssid
#ifdef __ECOS
			{
				unsigned char random_buf[4];
				get_random_bytes(random_buf, 4);
				random = random_buf[3];
			}
#else
			get_random_bytes(&random, 1);
#endif
			tmpbssid[0] = 0x02;
			for (i=1; i<MACADDRLEN; i++)
				tmpbssid[i] = GET_MY_HWADDR[i-1] ^ GET_MY_HWADDR[i] ^ random;
			while(1) {
				for (i=0; i<priv->site_survey->count_target; i++) {
					if (!memcmp(tmpbssid, priv->site_survey->bss_target[i].bssid, MACADDRLEN)) {
						tmpbssid[5]++;
						break;
					}
				}
				if (i == priv->site_survey->count)
					break;
			}
		}
	}

	memset(pbuf, 0, sizeof(priv->beaconbuf_ibss_vxd));
	
	txfw = &Temptxfw;
	rate = find_rate(priv, NULL, 0, 1);

	if (is_MCS_rate(rate)) {
		// can we use HT rates for beacon?
		txfw->txRate = rate & 0x7f;
		txfw->txHt = 1;
	}
	else {
		txfw->txRate = get_rate_index_from_ieee_value((UINT8)rate);
		if (priv->pshare->is_40m_bw) {
			if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
				txfw->txSC = 2;
			else
				txfw->txSC = 1;
		}
	}

	SetFrameSubType(pbuf, WIFI_BEACON);

	memset((void *)GetAddr1Ptr(pbuf), 0xff, 6);
	memcpy((void *)GetAddr2Ptr(pbuf), GET_MY_HWADDR, 6);
	memcpy((void *)GetAddr3Ptr(pbuf), bssid, 6); // (Indeterminable set null mac (all zero)) (Refer: Draft 1.06, Page 12, 7.2.3, Line 21~30)

	pbuf += 24;
	frlen += 24;

	frlen += _TIMESTAMP_;	// for timestamp
	pbuf += _TIMESTAMP_;

	//setup BeaconPeriod...
	if(priv->beacon_period <= 0)
		priv->beacon_period	= pmib->dot11StationConfigEntry.dot11BeaconPeriod;
	
	if(priv->beacon_period)
		val16 = cpu_to_le16(priv->beacon_period);
	else
		val16 = cpu_to_le16(100);
	
	pbuf = set_fixed_ie(pbuf, _BEACON_ITERVAL_, (unsigned char *)&val16, &frlen);

	val16 = cpu_to_le16(BIT(1)); //IBSS


	if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm)
		val16 |= cpu_to_le16(BIT(4));

	if (SHORTPREAMBLE)
		val16 |= cpu_to_le16(BIT(5));
#ifdef RTK_AC_SUPPORT //for 11ac logo
	if(priv->pshare->rf_ft_var.lpwrc)
		val16 |= cpu_to_le16(BIT(8));	/* set spectrum mgt */
#endif			

	pbuf = set_fixed_ie(pbuf, _CAPABILITY_, (unsigned char *)&val16, &frlen);
	priv->pBeaconCapability = (unsigned short *)(pbuf - _CAPABILITY_);

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && (priv->pmib->dot11ErpInfo.shortSlot))
		SET_SHORTSLOT_IN_BEACON_CAP;
	else
		RESET_SHORTSLOT_IN_BEACON_CAP;

	//set ssid...
	pbuf = set_ie(pbuf, _SSID_IE_, SSID_LEN, SSID, &frlen);

	//supported rates...
	get_bssrate_set(priv, _SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len);
	pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_, bssrate_len, pbssrate, &frlen);

	
	val16 = 0;
	pbuf = set_ie(pbuf, _IBSS_PARA_IE_, 2, (unsigned char *)&val16, &frlen);


#if defined(DOT11D) || defined(DOT11H)    
    if(priv->countryTableIdx) {    
        pbuf = construct_country_ie(priv, pbuf, &frlen);   
    }
#endif

#ifdef DOT11H
    if(priv->pmib->dot11hTPCEntry.tpc_enable) {
        pbuf = set_ie(pbuf, _PWR_CONSTRAINT_IE_, 1, &priv->pshare->rf_ft_var.lpwrc, &frlen);        
        pbuf = construct_TPC_report_ie(priv, pbuf, &frlen);
    }
#endif
	
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
		// ERP infomation
		unsigned char val8 = 0;
		if (priv->pmib->dot11ErpInfo.protection)
			val8 |= BIT(1);
		if (priv->pmib->dot11ErpInfo.nonErpStaNum)
			val8 |= BIT(0);
	
		if (!SHORTPREAMBLE || priv->pmib->dot11ErpInfo.longPreambleStaNum)
			val8 |= BIT(2);
	
		pbuf = set_ie(pbuf, _ERPINFO_IE_, 1, &val8, &frlen);
	}
	
	// EXT supported rates
	if (get_bssrate_set(priv, _EXT_SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len))
			pbuf = set_ie(pbuf, _EXT_SUPPORTEDRATES_IE_, bssrate_len, pbssrate, &frlen);
	

	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
			construct_ht_ie(priv, priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);
			pbuf = set_ie(pbuf, _HT_CAP_, priv->ht_cap_len, (unsigned char *)&priv->ht_cap_buf, &frlen);
			pbuf = set_ie(pbuf, _HT_IE_, priv->ht_ie_len, (unsigned char *)&priv->ht_ie_buf, &frlen);
	}
	// beacon
#ifdef RTK_AC_SUPPORT //for 11ac logo
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
		// 41
		{
			char tmp[8];
			memset(tmp, 0, 8);
			tmp[7] = 0x40;
			pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 8, tmp, &frlen);
		}
		// 60, 61
		construct_vht_ie(priv, priv->pshare->working_channel);
		pbuf = set_ie(pbuf, EID_VHTCapability, priv->vht_cap_len, (unsigned char *)&priv->vht_cap_buf, &frlen);
		pbuf = set_ie(pbuf, EID_VHTOperation, priv->vht_oper_len, (unsigned char *)&priv->vht_oper_buf, &frlen);
		// 62 
		if(priv->pshare->rf_ft_var.lpwrc) {
			char tmp[4];
			pbuf = set_ie(pbuf, _PWR_CONSTRAINT_IE_, 1, &priv->pshare->rf_ft_var.lpwrc, &frlen);
			tmp[1] = tmp[2] = tmp[3] = priv->pshare->rf_ft_var.lpwrc;
			tmp[0] = priv->pshare->CurrentChannelBW;	//	20, 40, 80
			pbuf = set_ie(pbuf, EID_VHTTxPwrEnvelope, tmp[0]+2, tmp, &frlen);	
		}
	}
#endif
	
#ifdef RTK_AC_SUPPORT //for 11ac logo
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
		//66
		if(priv->pshare->rf_ft_var.opmtest&1) {
			pbuf = set_ie(pbuf, EID_VHTOperatingMode, 1, (unsigned char *)&(priv->pshare->rf_ft_var.oper_mode_field), &frlen);
		}	
	}
#endif	
	
	if (pmib->dot11RsnIE.rsnielen) {
		memcpy(pbuf, pmib->dot11RsnIE.rsnie, pmib->dot11RsnIE.rsnielen);
		pbuf += pmib->dot11RsnIE.rsnielen;
		frlen += pmib->dot11RsnIE.rsnielen;
	}
	
#ifdef WIFI_WMM
		//Set WMM Parameter Element
	if (QOS_ENABLE)
		pbuf = set_ie(pbuf, _RSN_IE_1_, _WMM_Para_Element_Length_, GET_WMM_PARA_IE, &frlen);
#endif
	
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
		pbuf = construct_ht_ie_old_form(priv, pbuf, &frlen);
	}
	
	// Realtek proprietary IE
	if (priv->pshare->rtk_ie_len)
		pbuf = set_ie(pbuf, _RSN_IE_1_, priv->pshare->rtk_ie_len, priv->pshare->rtk_ie_buf, &frlen);
	
	// Customer proprietary IE
	if (priv->pmib->miscEntry.private_ie_len) {
		memcpy(pbuf, pmib->miscEntry.private_ie, pmib->miscEntry.private_ie_len);
		pbuf += pmib->miscEntry.private_ie_len;
		frlen += pmib->miscEntry.private_ie_len;
	}

	priv->beaconbuf_ibss_vxd_len = frlen;
	
}


#ifdef __KERNEL__
void issue_beacon_ibss_vxd(unsigned long task_priv)
#elif defined(__ECOS)
void issue_beacon_ibss_vxd(void *task_priv)
#endif
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	unsigned long flags;
	DECLARE_TXINSN(txinsn);
	
	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;
	
	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);

	if(priv->beaconbuf_ibss_vxd_len<=0)
		goto issue_beacon_ibss_done;

	if(!netif_running(priv->dev))
		goto issue_beacon_ibss_done;

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE	
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	txinsn.phdr = get_wlanhdr_from_poll(priv);
	txinsn.pframe = get_mgtbuf_from_poll(priv);

	if (txinsn.phdr == NULL)
		goto issue_beacon_ibss_fail;

	if (txinsn.pframe == NULL)
		goto issue_beacon_ibss_fail;

//fill MAC header
	memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

	SetFrameSubType(txinsn.phdr, WIFI_BEACON);
	memset((void *)GetAddr1Ptr((txinsn.phdr)), 0xff, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	
//fill Beacon frames
	txinsn.fr_len = (priv->beaconbuf_ibss_vxd_len-24);

	if(txinsn.fr_len > 0)
		memcpy((unsigned char *)txinsn.pframe, (((unsigned char *)priv->beaconbuf_ibss_vxd)+24) , txinsn.fr_len);
	else 
		goto issue_beacon_ibss_fail;
	
	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		goto issue_beacon_ibss_done;

issue_beacon_ibss_fail:
	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
	
issue_beacon_ibss_done:
	RESTORE_INT(flags);
	SMP_UNLOCK(flags);	

	mod_timer(&priv->pshare->vxd_ibss_beacon, jiffies + RTL_MILISECONDS_TO_JIFFIES(500));	
	
}


#endif


/**
 *	@brief	Initial beacon
 *
  *	Refer wifi.h and 8190mib.h about MIB define.	\n
 *  Refer 802.11 7,3,13 Beacon interval field	\n
 *	- Timestamp \n - Beacon interval \n - Capability \n - SSID \n - Support rate \n - DS Parameter set \n \n
 *	+-------+-------+-------+	\n
 *	| addr1 | addr2 | addr3 |	\n
 *	+-------+-------+-------+	\n
 *
 *	+-----------+-----------------+------------+------+--------------+------------------+	\n
 *	| Timestamp | Beacon interval | Capability | SSID | Support rate | DS Parameter set |	\n
 *	+-----------+-----------------+------------+------+--------------+------------------+	\n
 *	[Note] \n
 *	abridge FH (unused), CF (AP not support PCF), \n
 *	IBSS parameter set (STA), DTIM (AP), ERP ??Ext rate  IE complete in Update beacon.\n
 *	set_desc is important.
 */

void init_beacon(struct rtl8192cd_priv *priv)
{
	unsigned short	val16;
	unsigned char	val8;
	struct wifi_mib *pmib;
	unsigned char	*bssid;
//		struct tx_desc		*pdesc;
//		struct rtl8192cd_hw	*phw=GET_HW(priv);
//		int next_idx = 1;

	unsigned int	frlen=0;
	unsigned char	*pbuf=(unsigned char *)priv->beaconbuf;
	unsigned char	*pbssrate=NULL;
	int	bssrate_len;
	struct FWtemplate *txfw;
	struct FWtemplate Temptxfw;

	unsigned int rate;

#ifdef RTK_NL80211 //wrt-adhoc
	if(IS_VXD_INTERFACE(priv) && (OPMODE & WIFI_ADHOC_STATE))
		return;

	if(OPMODE&WIFI_AP_STATE) {
		//block beacon until hostapd finish configuration
		priv->pmib->miscEntry.func_off = 1;
	}
#endif

	pmib = GET_MIB(priv);
	bssid = pmib->dot11StationConfigEntry.dot11Bssid;

	//memset(pbuf, 0, 128*4);
	memset(pbuf, 0, sizeof(priv->beaconbuf));
	txfw = &Temptxfw;

#ifdef P2P_SUPPORT
        if(rtk_p2p_is_enabled(priv)){
            rate = _6M_RATE_;
        }else
#endif
    	rate = find_rate(priv, NULL, 0, 1);

#ifdef _11s_TEST_MODE_
	mesh_debug_sme2(priv, &rate);
#endif

	if (is_MCS_rate(rate)) {
		// can we use HT rates for beacon?
		txfw->txRate = rate - HT_RATE_ID;
		txfw->txHt = 1;
	}
	else {
		txfw->txRate = get_rate_index_from_ieee_value((UINT8)rate);
		if (priv->pshare->is_40m_bw) {
			if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
				txfw->txSC = 2;
			else
				txfw->txSC = 1;
		}
	}

	SetFrameSubType(pbuf, WIFI_BEACON);

	memset((void *)GetAddr1Ptr(pbuf), 0xff, 6);
	memcpy((void *)GetAddr2Ptr(pbuf), GET_MY_HWADDR, 6);
	memcpy((void *)GetAddr3Ptr(pbuf), bssid, 6); // (Indeterminable set null mac (all zero)) (Refer: Draft 1.06, Page 12, 7.2.3, Line 21~30)

	pbuf += 24;
	frlen += 24;

	frlen += _TIMESTAMP_;	// for timestamp
	pbuf += _TIMESTAMP_;

	//setup BeaconPeriod...
	val16 = cpu_to_le16(pmib->dot11StationConfigEntry.dot11BeaconPeriod);
#ifdef CLIENT_MODE
	if (OPMODE & WIFI_ADHOC_STATE)
		val16 = cpu_to_le16(priv->beacon_period);
#endif
	pbuf = set_fixed_ie(pbuf, _BEACON_ITERVAL_, (unsigned char *)&val16, &frlen);

#ifdef CONFIG_RTK_MESH
	if ((1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && (0 == GET_MIB(priv)->dot1180211sInfo.mesh_ap_enable))	// non-AP MP (MAP)	only, popen:802.11s Draft 1.0 P17  7.3.1.4 : ESS & IBSS are "0" (PS:val reset here)
		val16= 0;
	else
#endif	// CONFIG_RTK_MESH
	{
		if (OPMODE & WIFI_AP_STATE)
			val16 = cpu_to_le16(BIT(0)); //ESS
		else
			val16 = cpu_to_le16(BIT(1)); //IBSS
	}

	if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm)
		val16 |= cpu_to_le16(BIT(4));

	if (SHORTPREAMBLE)
		val16 |= cpu_to_le16(BIT(5));
    
#ifdef DOT11H
    if(priv->pmib->dot11hTPCEntry.tpc_enable)
        val16 |= cpu_to_le16(BIT(8));	/* set spectrum mgt */
#endif
    
	pbuf = set_fixed_ie(pbuf, _CAPABILITY_, (unsigned char *)&val16, &frlen);
	priv->pBeaconCapability = (unsigned short *)(pbuf - _CAPABILITY_);

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && (priv->pmib->dot11ErpInfo.shortSlot))
		SET_SHORTSLOT_IN_BEACON_CAP;
	else
		RESET_SHORTSLOT_IN_BEACON_CAP;

	//set ssid...
#ifdef WIFI_SIMPLE_CONFIG
	priv->pbeacon_ssid = pbuf;
#endif
#ifdef CONFIG_RTK_MESH
	if ((1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && (0 == GET_MIB(priv)->dot1180211sInfo.mesh_ap_enable))	//	non-AP MP (MAP)  only, popen:802.11s Draft 1.0, Page 11, SSID
		pbuf = set_ie(pbuf, _SSID_IE_, 0, 0, &frlen);	// wildcard SSID (len = 0)
	else
#endif
	{
		if (!HIDDEN_AP && pmib->miscEntry.raku_only == 0)
			pbuf = set_ie(pbuf, _SSID_IE_, SSID_LEN, SSID, &frlen);
		else {
#ifdef CONFIG_RTL8196B_TLD
			pbuf = set_ie(pbuf, _SSID_IE_, 0, NULL, &frlen);
#else
			if (HIDDEN_AP == 2) {
				pbuf = set_ie(pbuf, _SSID_IE_, 0, NULL, &frlen);
			} else {
				unsigned char ssidbuf[32];
				memset(ssidbuf, 0, 32);
				pbuf = set_ie(pbuf, _SSID_IE_, SSID_LEN, ssidbuf, &frlen);
			}
#endif
		}
	}

	//supported rates...
/*cfg p2p cfg p2p*/	
#ifdef P2P_SUPPORT
        if(rtk_p2p_is_enabled(priv))
            get_bssrate_set(priv, _SUPPORTED_RATES_NO_CCK_, &pbssrate, &bssrate_len);   
        else
#endif
	get_bssrate_set(priv, _SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len);
	pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_, bssrate_len, pbssrate, &frlen);

	//ds parameter set...
	val8 = pmib->dot11RFEntry.dot11channel;
#if defined(RTK_5G_SUPPORT) 
	 	if ( priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
#endif			
	pbuf = set_ie(pbuf, _DSSET_IE_, 1, &val8, &frlen);
	priv->timoffset = frlen;
//		pdesc = phw->tx_descB;	// by signin_beacon_desc
	// clear all bit
//		memset(pdesc, 0, 32);	// by signin_beacon_desc

//		pdesc->Dword4 |= set_desc(0x08 << TX_RtsRateSHIFT);	// by signin_beacon_desc
//		pdesc->Dword8 = set_desc(get_physical_addr(priv, priv->beaconbuf, 128*sizeof(unsigned int), PCI_DMA_TODEVICE));	// by signin_beacon_desc

	// next pointer should point to a descriptor, david
	//set NextDescAddress
//		pdesc->Dword10 = set_desc(get_physical_addr(priv, &phw->tx_descB[next_idx], sizeof(struct tx_desc), PCI_DMA_TODEVICE));	// by signin_beacon_desc

#ifdef CONFIG_PCI_HCI
	update_beacon(priv);

	// enable tx bcn
//#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
//		if (IS_ROOT_INTERFACE(priv))
//#endif
//			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION);

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
    	GET_HAL_INTERFACE(priv)->TxPollingHandler(priv, TXPOLL_BEACON_QUEUE);
    } else
#endif
	{		
	// use hw beacon
#ifdef CONFIG_RTL_8812_SUPPORT 
		if(GET_CHIP_VER(priv) != VERSION_8812E)
#endif
		RTL_W8(PCIE_CTRL_REG, BCNQ_POLL);
	}

#ifndef DRVMAC_LB 
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {    
    	GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_ENABLE_BEACON_DMA, NULL);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif	
	{//not HAL
		RTL_W16(PCIE_CTRL_REG, RTL_R16(PCIE_CTRL_REG)& (~ BCNQSTOP) );
	}
#endif
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_USB_HCI
#if defined(CONFIG_RTL_92C_SUPPORT) || (!defined(CONFIG_SUPPORT_USB_INT) || !defined(CONFIG_INTERRUPT_BASED_TXBCN))
	if (!timer_pending(&priv->pshare->beacon_timer))
		mod_timer(&priv->pshare->beacon_timer, jiffies);
#endif
#endif // CONFIG_USB_HCI

#ifdef CONFIG_SDIO_HCI
	if (!timer_pending(&priv->pshare->beacon_timer))
		mod_timer(&priv->pshare->beacon_timer, jiffies + 
			RTL_MILISECONDS_TO_JIFFIES(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod));
#endif // CONFIG_SDIO_HCI
}


#ifndef CONFIG_RTL_NEW_AUTOCH
static void setChannelScore(int number, unsigned int *val, int min, int max)
{
	int i=0, score;

	if (number > max)
		return;

	*(val + number) += 5;

	if (number > min) {
		for (i=number-1, score=4; i>=min && score; i--, score--) {
			*(val + i) += score;
		}
	}
	if (number < max) {
		for (i=number+1, score=4; i<=max && score; i++, score--) {
			*(val +i) += score;
		}
	}
}
#endif


#if defined(CONFIG_RTL_NEW_AUTOCH) && defined(SS_CH_LOAD_PROC)
static void record_SS_report(struct rtl8192cd_priv *priv)
{
	int i, j;
	priv->ch_ss_rpt_cnt = priv->site_survey->count;
	memset(priv->ch_ss_rpt, 0, (sizeof(struct ss_report)*MAX_BSS_NUM));	
	
	for(i=0; i<priv->site_survey->count ;i++){
		priv->ch_ss_rpt[i].channel = priv->site_survey->bss[i].channel;
		priv->ch_ss_rpt[i].is40M = ((priv->site_survey->bss[i].t_stamp[1] & BIT(1)) ? 1 : 0);
		priv->ch_ss_rpt[i].rssi	= priv->site_survey->bss[i].rssi;
		for (j=0; j<priv->available_chnl_num; j++) {
			if (priv->ch_ss_rpt[i].channel == priv->available_chnl[j]) {
				priv->ch_ss_rpt[i].fa_count = priv->chnl_ss_fa_count[j];
				priv->ch_ss_rpt[i].cca_count = priv->chnl_ss_cca_count[j];
				priv->ch_ss_rpt[i].ch_load = priv->chnl_ss_load[j];
			}
		}
	}
}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
struct ap_info {
	unsigned char ch;
	unsigned char bw;
	unsigned char rssi;
};

#define MASK_CH(_ch, _begin, _end) if ((_ch) >= (_begin) && (_ch) < (_end)) score[_ch] = 0xffffffff;
int find_clean_channel(struct rtl8192cd_priv *priv, unsigned int begin, unsigned int end, unsigned int *score)
{
	struct bss_desc *pBss=NULL;
	int i, j, y, found;
	int ap_count[MAX_2G_CHANNEL_NUM];
	struct ap_info ap_rec[MAX_BSS_NUM];
	unsigned int ap_rec_num = 0;

	memset(ap_count, 0, sizeof(ap_count));
	memset(ap_rec, 0, sizeof(ap_rec));
	
	for (y=begin; y<end; y++) {
		score[y] = priv->chnl_ss_fa_count[y];
#ifdef _DEBUG_RTL8192CD_
		printk("ch %d: FA: %d\n", y+1, score[y]);
#endif
	}
	
	for (i=0; i<priv->site_survey->count; i++) {
		pBss = &priv->site_survey->bss[i];

		ap_rec[ap_rec_num].ch = pBss->channel;
		if ((pBss->t_stamp[1] & 0x6) == 0) ap_rec[ap_rec_num].bw = 0;
		else if ((pBss->t_stamp[1] & 0x4) == 0) ap_rec[ap_rec_num].bw = 1;
		else ap_rec[ap_rec_num].bw = 2;
		ap_rec[ap_rec_num++].rssi = pBss->rssi;

		if (pBss->rssi > 15) {
			for (y=begin; y<end; y++) {
				if (pBss->channel == priv->available_chnl[y]) {
					ap_count[y]++;
					if ((pBss->t_stamp[1] & 0x6) == 0) {  // 20M
						for (j=-2; j<=2; j++)
							MASK_CH(y+j, begin, end);
					}	
					else if ((pBss->t_stamp[1] & 0x4) == 0) {  // 40M upper
						for (j=-2; j<=6; j++)
							MASK_CH(y+j, begin, end);
					}	
					else {  // 40M lower
						for (j=-6; j<=2; j++)
							MASK_CH(y+j, begin, end);
					}	
				}
			}
		}
	}

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
		&& priv->pshare->is_40m_bw) {
		for (y=begin; y<end; y++)
			if (priv->available_chnl[y] == 14)
				score[y] = 0xffffffff;		// mask chan14
	}

	if (priv->pmib->dot11RFEntry.disable_ch1213) {
		for (y=begin; y<end; y++) {
			int ch = priv->available_chnl[y];
			if ((ch == 12) || (ch == 13))
				score[y] = 0xffffffff;
		}
	}

	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_GLOBAL) ||
			(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_WORLD_WIDE)) &&
		 (end >= 11) && (end < 14)) {
		score[13] = 0xffffffff;	// mask chan14
		score[12] = 0xffffffff; // mask chan13
		score[11] = 0xffffffff; // mask chan12
	}

#ifdef _DEBUG_RTL8192CD_
	for (y=begin; y<end; y++) {
		if (score[y] == 0xffffffff)
			printk("ch %d: ap_count: %d, score: 0xffffffff ", y+1, ap_count[y]);
		else
			printk("ch %d: ap_count: %d, score: %d ", y+1, ap_count[y], score[y]);

		for (i=0; i<ap_rec_num; i++) {
			if (priv->available_chnl[y] == ap_rec[i].ch) {
				printk("%s:%d ", ap_rec[i].bw==0?"N":(ap_rec[i].bw==1?"U":"L"), ap_rec[i].rssi);
			}
		}
		printk("\n");
	}
#endif
	
	for (y=begin; y<end; y++) {
		found = 1;
		if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20_40)
			i = 4;
		else
			i = 2;

		for (j=-i; j<=i; j++) {
			if (score[y+j] == 0xffffffff) {
				found = 0;
				break;
			}
		}

		if (found) 
			return 1;
	}

	return 0;
}
#endif

int selectClearChannel(struct rtl8192cd_priv *priv)
{
	static unsigned int score2G[MAX_2G_CHANNEL_NUM], score5G[MAX_5G_CHANNEL_NUM];
	unsigned int score[MAX_BSS_NUM];
	unsigned int minScore=0xffffffff;
	unsigned int tmpScore, tmpIdx=0;
	unsigned int traffic_check 			= 0;
	unsigned int fa_count_weighting = 1;
	int i, j, idx=0, idx_2G_end=-1, idx_5G_begin=-1, minChan=0;
	struct bss_desc *pBss=NULL;

#ifdef _DEBUG_RTL8192CD_
	char tmpbuf[400];
	int len=0;
#endif

	memset(score2G, '\0', sizeof(score2G));
	memset(score5G, '\0', sizeof(score5G));

	for (i=0; i<priv->available_chnl_num; i++) {
		if (priv->available_chnl[i] <= 14)
			idx_2G_end = i;
		else
			break;
	}

	for (i=0; i<priv->available_chnl_num; i++) {
		if (priv->available_chnl[i] > 14) {
			idx_5G_begin = i;
			break;
		}
	}

#ifndef CONFIG_RTL_NEW_AUTOCH
	for (i=0; i<priv->site_survey->count; i++) {
		pBss = &priv->site_survey->bss[i];
		for (idx=0; idx<priv->available_chnl_num; idx++) {
			if (pBss->channel == priv->available_chnl[idx]) {
				if (pBss->channel <= 14)
					setChannelScore(idx, score2G, 0, MAX_2G_CHANNEL_NUM-1);
				else
					score5G[idx - idx_5G_begin] += 5;
				break;
			}
		}
	}
#endif

	if (idx_2G_end >= 0)
		for (i=0; i<=idx_2G_end; i++)
			score[i] = score2G[i];
	if (idx_5G_begin >= 0)
		for (i=idx_5G_begin; i<priv->available_chnl_num; i++)
			score[i] = score5G[i - idx_5G_begin];
		
#ifdef CONFIG_RTL_NEW_AUTOCH
	{
		unsigned int y, /*cca_thd=0, ch_num=0,*/ ch_begin=0, ch_end= priv->available_chnl_num;
		/*unsigned int mac_rx_th=0, mac_rx_ch_count=0, fa_ch_count=0;*/
		unsigned int do_ap_check = 1, ap_ratio = 0;
		
		if (idx_2G_end >= 0) 
			ch_end = idx_2G_end+1;
		if (idx_5G_begin >= 0)  
			ch_begin = idx_5G_begin;

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
		if ((GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8192E)
			&& priv->pmib->dot11RFEntry.acs_type)
		{
		    unsigned int use_nhm = 0;
			unsigned int tmp_score[MAX_BSS_NUM];
			memcpy(tmp_score, score, sizeof(score));
			if (find_clean_channel(priv, ch_begin, ch_end, tmp_score)) {
				//memcpy(score, tmp_score, sizeof(score));
#ifdef _DEBUG_RTL8192CD_
				printk("!! Found clean channel, select minimum FA channel\n");
#endif
				goto USE_CLN_CH;
			}
#ifdef _DEBUG_RTL8192CD_
			printk("!! Not found clean channel, use NHM algorithm\n");
#endif
			use_nhm = 1;
USE_CLN_CH:
			for (y=ch_begin; y<ch_end; y++) {
				for (i=0; i<=9; i++) {
					unsigned int val32 = priv->nhm_cnt[y][i];
					for (j=0; j<i; j++)
						val32 *= 3;
					score[y] += val32;
				}

#ifdef _DEBUG_RTL8192CD_				
				printk("nhm_cnt_%d: H<-[ %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d]->L, score: %d\n", 
					y+1, priv->nhm_cnt[y][9], priv->nhm_cnt[y][8], priv->nhm_cnt[y][7], 
					priv->nhm_cnt[y][6], priv->nhm_cnt[y][5], priv->nhm_cnt[y][4],
					priv->nhm_cnt[y][3], priv->nhm_cnt[y][2], priv->nhm_cnt[y][1],
					priv->nhm_cnt[y][0], score[y]);
#endif
			}

			if (!use_nhm)
				memcpy(score, tmp_score, sizeof(score));
			
			goto choose_ch;
		}
#endif

		/*
		 * 	For each channel, weighting behind channels with MAC RX counter
		 * 	For each channel, weighting the channel with FA counter
		 */
		for (y=ch_begin; y<ch_end; y++) {
			score[y] += 8 * priv->chnl_ss_mac_rx_count[y];
			if (priv->chnl_ss_mac_rx_count[y] > 30)
				do_ap_check = 0;
			if( priv->chnl_ss_mac_rx_count[y] > MAC_RX_COUNT_THRESHOLD )
				traffic_check = 1;
			
#ifdef RTK_5G_SUPPORT
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
#endif
			{
				if ((int)(y-4) >= (int)ch_begin)
					score[y-4] += 2 * priv->chnl_ss_mac_rx_count[y];				
				if ((int)(y-3) >= (int)ch_begin)
					score[y-3] += 8 * priv->chnl_ss_mac_rx_count[y];
				if ((int)(y-2) >= (int)ch_begin)
					score[y-2] += 8 * priv->chnl_ss_mac_rx_count[y];
				if ((int)(y-1) >= (int)ch_begin)
					score[y-1] += 10 * priv->chnl_ss_mac_rx_count[y];
				if ((int)(y+1) < (int)ch_end)
					score[y+1] += 10 * priv->chnl_ss_mac_rx_count[y];
				if ((int)(y+2) < (int)ch_end)
					score[y+2] += 8 * priv->chnl_ss_mac_rx_count[y];
				if ((int)(y+3) < (int)ch_end)
					score[y+3] += 8 * priv->chnl_ss_mac_rx_count[y];
				if ((int)(y+4) < (int)ch_end)
					score[y+4] += 2 * priv->chnl_ss_mac_rx_count[y];
			}

			//this is for CH_LOAD caculation
			if( priv->chnl_ss_cca_count[y] > priv->chnl_ss_fa_count[y])
				priv->chnl_ss_cca_count[y]-= priv->chnl_ss_fa_count[y];
			else
				priv->chnl_ss_cca_count[y] = 0;
		}

		for (y=ch_begin; y<ch_end; y++) {
			if (priv->chnl_ss_mac_rx_count_40M[y]) {
				score[y] += 5 * priv->chnl_ss_mac_rx_count_40M[y];
				if (priv->chnl_ss_mac_rx_count_40M[y] > 30)
					do_ap_check = 0;
				if( priv->chnl_ss_mac_rx_count_40M[y] > MAC_RX_COUNT_THRESHOLD )
					traffic_check = 1;
				
#ifdef RTK_5G_SUPPORT
				if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
#endif
				{
					if ((int)(y-6) >= (int)ch_begin)
						score[y-6] += 1 * priv->chnl_ss_mac_rx_count_40M[y];
					if ((int)(y-5) >= (int)ch_begin)
						score[y-5] += 4 * priv->chnl_ss_mac_rx_count_40M[y];
					if ((int)(y-4) >= (int)ch_begin)
						score[y-4] += 4 * priv->chnl_ss_mac_rx_count_40M[y];
					if ((int)(y-3) >= (int)ch_begin)
						score[y-3] += 5 * priv->chnl_ss_mac_rx_count_40M[y];
					if ((int)(y-2) >= (int)ch_begin)
						score[y-2] += (5 * priv->chnl_ss_mac_rx_count_40M[y])/2;
					if ((int)(y-1) >= (int)ch_begin)
						score[y-1] += 5 * priv->chnl_ss_mac_rx_count_40M[y];
					if ((int)(y+1) < (int)ch_end)
						score[y+1] += 5 * priv->chnl_ss_mac_rx_count_40M[y];
					if ((int)(y+2) < (int)ch_end)
						score[y+2] += (5 * priv->chnl_ss_mac_rx_count_40M[y])/2;
					if ((int)(y+3) < (int)ch_end)
						score[y+3] += 5 * priv->chnl_ss_mac_rx_count_40M[y];
					if ((int)(y+4) < (int)ch_end)
						score[y+4] += 4 * priv->chnl_ss_mac_rx_count_40M[y];
					if ((int)(y+5) < (int)ch_end)
						score[y+5] += 4 * priv->chnl_ss_mac_rx_count_40M[y];
					if ((int)(y+6) < (int)ch_end)
						score[y+6] += 1 * priv->chnl_ss_mac_rx_count_40M[y];
				}
			}
		}

		if( traffic_check == 0)
			fa_count_weighting = 5;
		else
			fa_count_weighting = 1;

		for (y=ch_begin; y<ch_end; y++) {
			score[y] += fa_count_weighting * priv->chnl_ss_fa_count[y];
		}

		if (do_ap_check) {
			for (i=0; i<priv->site_survey->count; i++) {				
				pBss = &priv->site_survey->bss[i];
				for (y=ch_begin; y<ch_end; y++) {
					if (pBss->channel == priv->available_chnl[y]) {
						if (pBss->channel <= 14) {
							if (pBss->rssi > 60)
								ap_ratio = 4;
							else if (pBss->rssi > 35)
								ap_ratio = 2;
							else
								ap_ratio = 1;
							
							if ((pBss->t_stamp[1] & 0x6) == 0) {
								score[y] += 50 * ap_ratio;
								if ((int)(y-4) >= (int)ch_begin)
									score[y-4] += 10 * ap_ratio;
								if ((int)(y-3) >= (int)ch_begin)
									score[y-3] += 20 * ap_ratio;
								if ((int)(y-2) >= (int)ch_begin)
									score[y-2] += 30 * ap_ratio;
								if ((int)(y-1) >= (int)ch_begin)
									score[y-1] += 40 * ap_ratio;
								if ((int)(y+1) < (int)ch_end)
									score[y+1] += 40 * ap_ratio;
								if ((int)(y+2) < (int)ch_end)
									score[y+2] += 30 * ap_ratio;
								if ((int)(y+3) < (int)ch_end)
									score[y+3] += 20 * ap_ratio;
								if ((int)(y+4) < (int)ch_end)
									score[y+4] += 10 * ap_ratio;
							}	
							else if ((pBss->t_stamp[1] & 0x4) == 0) {
								score[y] += 50 * ap_ratio;
								if ((int)(y-3) >= (int)ch_begin)
									score[y-3] += 20 * ap_ratio;
								if ((int)(y-2) >= (int)ch_begin)
									score[y-2] += 30 * ap_ratio;
								if ((int)(y-1) >= (int)ch_begin)
									score[y-1] += 40 * ap_ratio;
								if ((int)(y+1) < (int)ch_end)
									score[y+1] += 50 * ap_ratio;
								if ((int)(y+2) < (int)ch_end)
									score[y+2] += 50 * ap_ratio;
								if ((int)(y+3) < (int)ch_end)
									score[y+3] += 50 * ap_ratio;
								if ((int)(y+4) < (int)ch_end)
									score[y+4] += 50 * ap_ratio;
								if ((int)(y+5) < (int)ch_end)
									score[y+5] += 40 * ap_ratio;
								if ((int)(y+6) < (int)ch_end)
									score[y+6] += 30 * ap_ratio;
								if ((int)(y+7) < (int)ch_end)
									score[y+7] += 20 * ap_ratio;	
							}	
							else {
								score[y] += 50 * ap_ratio;
								if ((int)(y-7) >= (int)ch_begin)
									score[y-7] += 20 * ap_ratio;
								if ((int)(y-6) >= (int)ch_begin)
									score[y-6] += 30 * ap_ratio;
								if ((int)(y-5) >= (int)ch_begin)
									score[y-5] += 40 * ap_ratio;
								if ((int)(y-4) >= (int)ch_begin)
									score[y-4] += 50 * ap_ratio;
								if ((int)(y-3) >= (int)ch_begin)
									score[y-3] += 50 * ap_ratio;
								if ((int)(y-2) >= (int)ch_begin)
									score[y-2] += 50 * ap_ratio;
								if ((int)(y-1) >= (int)ch_begin)
									score[y-1] += 50 * ap_ratio;
								if ((int)(y+1) < (int)ch_end)
									score[y+1] += 40 * ap_ratio;
								if ((int)(y+2) < (int)ch_end)
									score[y+2] += 30 * ap_ratio;
								if ((int)(y+3) < (int)ch_end)
									score[y+3] += 20 * ap_ratio;
							}	
						}	
						else {
							if ((pBss->t_stamp[1] & 0x6) == 0) {
								score[y] += 500;
							}
							else if ((pBss->t_stamp[1] & 0x4) == 0) {
								score[y] += 500;
								if ((int)(y+1) < (int)ch_end)
									score[y+1] += 500;
							}
							else {	
								score[y] += 500;
								if ((int)(y-1) >= (int)ch_begin)
									score[y-1] += 500;
							}
						}
						break;
					}
				}
			}
		}

        // display score
		//for (y=ch_begin; y<ch_end; y++) {
		//	panic_printk("ch[%d]:%d,fa[%d],rx_20[%d],rx_40[%d]\n", priv->available_chnl[y],score[y],priv->chnl_ss_fa_count[y],priv->chnl_ss_mac_rx_count[y],priv->chnl_ss_mac_rx_count_40M[y]);
		//}

#ifdef 	SS_CH_LOAD_PROC

		// caculate noise level -- suggested by wilson
		for (y=ch_begin; y<ch_end; y++)  {
			int fa_lv=0, cca_lv=0;
			if (priv->chnl_ss_fa_count[y]>1000) {
				fa_lv = 100;
			} else if (priv->chnl_ss_fa_count[y]>500) {
				fa_lv = 34 * (priv->chnl_ss_fa_count[y]-500) / 500 + 66;
			} else if (priv->chnl_ss_fa_count[y]>200) {
				fa_lv = 33 * (priv->chnl_ss_fa_count[y] - 200) / 300 + 33;
			} else if (priv->chnl_ss_fa_count[y]>100) {
				fa_lv = 18 * (priv->chnl_ss_fa_count[y] - 100) / 100 + 15;
			} else {
				fa_lv = 15 * priv->chnl_ss_fa_count[y] / 100;
			} 
			if (priv->chnl_ss_cca_count[y]>400) {
				cca_lv = 100;
			} else if (priv->chnl_ss_cca_count[y]>200) {
				cca_lv = 34 * (priv->chnl_ss_cca_count[y] - 200) / 200 + 66;
			} else if (priv->chnl_ss_cca_count[y]>80) {
				cca_lv = 33 * (priv->chnl_ss_cca_count[y] - 80) / 120 + 33;
			} else if (priv->chnl_ss_cca_count[y]>40) {
				cca_lv = 18 * (priv->chnl_ss_cca_count[y] - 40) / 40 + 15;
			} else {
				cca_lv = 15 * priv->chnl_ss_cca_count[y] / 40;
			}

			priv->chnl_ss_load[y] = (((fa_lv > cca_lv)? fa_lv : cca_lv)*75+((score[y]>100)?100:score[y])*25)/100;

			DEBUG_INFO("ch:%d f=%d (%d), c=%d (%d), fl=%d, cl=%d, sc=%d, cu=%d\n", 
					priv->available_chnl[y],
					priv->chnl_ss_fa_count[y], fa_thd,
					priv->chnl_ss_cca_count[y], cca_thd,
					fa_lv, 
					cca_lv,
					score[y],					
					priv->chnl_ss_load[y]);
			
		}		
#endif		
	}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
choose_ch:
#endif

#ifdef DFS
	// heavy weighted DFS channel
	if (idx_5G_begin >= 0){
		for (i=idx_5G_begin; i<priv->available_chnl_num; i++) {
			if (!priv->pmib->dot11DFSEntry.disable_DFS && is_DFS_channel(priv->available_chnl[i]) 
			&& (score[i]!= 0xffffffff)){
					score[i] += 1600; 
		}
	}
	}
#endif


//prevent Auto Channel selecting wrong channel in 40M mode-----------------
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
		&& priv->pshare->is_40m_bw) {
#if 0
		if (GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset == 1) {
			//Upper Primary Channel, cannot select the two lowest channels
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
				score[0] = 0xffffffff;
				score[1] = 0xffffffff;
				score[2] = 0xffffffff;
				score[3] = 0xffffffff;
				score[4] = 0xffffffff;

				score[13] = 0xffffffff;
				score[12] = 0xffffffff;
				score[11] = 0xffffffff;
			}

//			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
//				score[idx_5G_begin] = 0xffffffff;
//				score[idx_5G_begin + 1] = 0xffffffff;
//			}
		}
		else if (GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset == 2) {
			//Lower Primary Channel, cannot select the two highest channels
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
				score[0] = 0xffffffff;
				score[1] = 0xffffffff;
				score[2] = 0xffffffff;

				score[13] = 0xffffffff;
				score[12] = 0xffffffff;
				score[11] = 0xffffffff;
				score[10] = 0xffffffff;
				score[9] = 0xffffffff;
			}

//			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
//				score[priv->available_chnl_num - 2] = 0xffffffff;
//				score[priv->available_chnl_num - 1] = 0xffffffff;
//			}
		}
#endif
		for (i=0; i<=idx_2G_end; ++i)
			if (priv->available_chnl[i] == 14)
				score[i] = 0xffffffff;		// mask chan14

#ifdef RTK_5G_SUPPORT
		if (idx_5G_begin >= 0) {
			for (i=idx_5G_begin; i<priv->available_chnl_num; i++) {
				int ch = priv->available_chnl[i];
				if(priv->available_chnl[i] > 144) 
					--ch;
				if((ch%4) || ch==140 || ch == 164 )	//mask ch 140, ch 165, ch 184...
					score[i] = 0xffffffff;
			}
		}
#endif

		
	}

	if (priv->pmib->dot11RFEntry.disable_ch1213) {
		for (i=0; i<=idx_2G_end; ++i) {
			int ch = priv->available_chnl[i];
			if ((ch == 12) || (ch == 13))
				score[i] = 0xffffffff;
		}
	}

	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_GLOBAL) ||
			(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_WORLD_WIDE)) &&
		 (idx_2G_end >= 11) && (idx_2G_end < 14)) {
		score[13] = 0xffffffff;	// mask chan14
		score[12] = 0xffffffff; // mask chan13
		score[11] = 0xffffffff; // mask chan12
	}
	
//------------------------------------------------------------------

#ifdef _DEBUG_RTL8192CD_
	for (i=0; i<priv->available_chnl_num; i++) {
		len += sprintf(tmpbuf+len, "ch%d:%u ", priv->available_chnl[i], score[i]);		
	}
	strcat(tmpbuf, "\n");
	panic_printk("%s", tmpbuf);

#endif

	if ( (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
		&& (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80)) 
	{
		for (i=0; i<priv->available_chnl_num; i++) {
			if (is80MChannel(priv->available_chnl, priv->available_chnl_num, priv->available_chnl[i])) {
				tmpScore = 0;
				for (j=0; j<4; j++) {
					if ((tmpScore != 0xffffffff) && (score[i+j] != 0xffffffff))
						tmpScore += score[i+j];
					else
						tmpScore = 0xffffffff;
				}
				tmpScore = tmpScore / 4;
				if (minScore > tmpScore) {
					minScore = tmpScore;

					tmpScore = 0xffffffff;
					for (j=0; j<4; j++) {
						if (score[i+j] < tmpScore) {
							tmpScore = score[i+j];
							tmpIdx = i+j;
						}
					}

					idx = tmpIdx;
				}
				i += 3;
			}
		}
		if (minScore == 0xffffffff) {
			// there is no 80M channels
			priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_20;
			for (i=0; i<priv->available_chnl_num; i++) {
				if (score[i] < minScore) {
					minScore = score[i];
					idx = i;
				}
			}
		}
	}
	else if( (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
			&& (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20_40))
 	{
		for (i=0; i<priv->available_chnl_num; i++) {
			if(is40MChannel(priv->available_chnl,priv->available_chnl_num,priv->available_chnl[i])) {
				tmpScore = 0;
				for(j=0;j<2;j++) {
					if ((tmpScore != 0xffffffff) && (score[i+j] != 0xffffffff))
						tmpScore += score[i+j];
					else
						tmpScore = 0xffffffff;
				}
				tmpScore = tmpScore / 2;
				if(minScore > tmpScore) {
					minScore = tmpScore;

					tmpScore = 0xffffffff;
					for (j=0; j<2; j++) {
						if (score[i+j] < tmpScore) {
							tmpScore = score[i+j];
							tmpIdx = i+j;
						}
					}

					idx = tmpIdx;
				}
				i += 1;
			}
		}
		if (minScore == 0xffffffff) {
			// there is no 40M channels
			priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_20;
			for (i=0; i<priv->available_chnl_num; i++) {
				if (score[i] < minScore) {
					minScore = score[i];
					idx = i;
				}
			}
		}
	}
	else if( (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
			&& (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20_40)
			&& (priv->available_chnl_num >= 8) )
	{
		unsigned int groupScore[14];

		memset(groupScore, 0xff , sizeof(groupScore));
		for (i=0; i<priv->available_chnl_num-4; i++) {
			if (score[i] != 0xffffffff && score[i+4] != 0xffffffff) {
				groupScore[i] = score[i] + score[i+4];
				DEBUG_INFO("groupScore, ch %d,%d: %d\n", i+1, i+5, groupScore[i]);
				if (groupScore[i] < minScore) {
#ifdef AUTOCH_SS_SPEEDUP
					if(priv->pmib->miscEntry.autoch_1611_enable)
					{
						if(priv->available_chnl[i]==1 || priv->available_chnl[i]==6 || priv->available_chnl[i]==11)
						{
							minScore = groupScore[i];
							idx = i;
						}
					}
					else
#endif
					{					
						minScore = groupScore[i];
						idx = i;
					}
				}
			}
		}

		if (score[idx] < score[idx+4]) {
			GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE;
			priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_ABOVE;			
		} else {
			idx = idx + 4;
			GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW;
			priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_BELOW;			
		}
	}
	else 
	{
		for (i=0; i<priv->available_chnl_num; i++) {
			if (score[i] < minScore) {
#ifdef AUTOCH_SS_SPEEDUP
				if(priv->pmib->miscEntry.autoch_1611_enable)
				{
					if(priv->available_chnl[i]==1 || priv->available_chnl[i]==6 || priv->available_chnl[i]==11)
					{
						minScore = score[i];
						idx = i;
					}
				}
				else
#endif
				{				
					minScore = score[i];
					idx = i;
				}
			}
		}
	}

	if (IS_A_CUT_8881A(priv) &&
		(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80)) {
		if ((priv->available_chnl[idx] == 36) ||
			(priv->available_chnl[idx] == 52) ||
			(priv->available_chnl[idx] == 100) ||
			(priv->available_chnl[idx] == 116) ||
			(priv->available_chnl[idx] == 132) ||
			(priv->available_chnl[idx] == 149) ||
			(priv->available_chnl[idx] == 165))
			idx++;
		else if ((priv->available_chnl[idx] == 48) ||
			(priv->available_chnl[idx] == 64) ||
			(priv->available_chnl[idx] == 112) ||
			(priv->available_chnl[idx] == 128) ||
			(priv->available_chnl[idx] == 144) ||
			(priv->available_chnl[idx] == 161) ||
			(priv->available_chnl[idx] == 177))
			idx--;
	}

	minChan = priv->available_chnl[idx];

	// skip channel 14 if don't support ofdm
	if ((priv->pmib->dot11RFEntry.disable_ch14_ofdm) &&
			(minChan == 14)) {
		score[idx] = 0xffffffff;
		
		minScore = 0xffffffff;
		for (i=0; i<priv->available_chnl_num; i++) {
			if (score[i] < minScore) {
				minScore = score[i];
				idx = i;
			}
		}
		minChan = priv->available_chnl[idx];
	}

#if 0
	//Check if selected channel available for 80M/40M BW or NOT ?
	if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
	{
		if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80)
		{
			if(!is80MChannel(priv->available_chnl,priv->available_chnl_num,minChan))
			{
				//printk("BW=80M, selected channel = %d is unavaliable! reduce to 40M\n", minChan);
				//priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20_40;
				priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_20_40;
			}
		}
			
		if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20_40)
		{
			if(!is40MChannel(priv->available_chnl,priv->available_chnl_num,minChan))
			{
				//printk("BW=40M, selected channel = %d is unavaliable! reduce to 20M\n", minChan);
				//priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20;
				priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_20;
			}
		}
	}
#endif

#ifdef CONFIG_RTL_NEW_AUTOCH
	RTL_W32(RXERR_RPT, RXERR_RPT_RST);
#endif

// auto adjust contro-sideband
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			&& (priv->pshare->is_40m_bw ==1 || priv->pshare->is_40m_bw ==2)) {

#ifdef RTK_5G_SUPPORT
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			if( (minChan>144) ? ((minChan-1)%8) : (minChan%8)) {
				GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE;
				priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_ABOVE;
			} else {
				GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW;
				priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_BELOW;
			}

		} else
#endif		
		{
#if 0
#ifdef CONFIG_RTL_NEW_AUTOCH
			unsigned int ch_max;

			if (priv->available_chnl[idx_2G_end] >= 13)
				ch_max = 13;
			else
				ch_max = priv->available_chnl[idx_2G_end];

			if ((minChan >= 5) && (minChan <= (ch_max-5))) {
				if (score[minChan+4] > score[minChan-4]) { /* what if some channels were cancelled? */
					GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW;
					priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_BELOW;
				} else {
					GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE;
					priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_ABOVE;
				}
			} else
#endif
			{
				if (minChan < 5) {
					GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE;
					priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_ABOVE;
				}
				else if (minChan > 7) {
					GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW;
					priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_BELOW;
				}
			}
#endif
		}
	}
//-----------------------

#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
	panic_printk("Auto channel choose ch:%d\n", minChan);
#else
#ifdef _DEBUG_RTL8192CD_
	panic_printk("Auto channel choose ch:%d\n", minChan);
#endif
#endif

	return minChan;
}


#ifdef CONFIG_RTL_WLAN_DOS_FILTER
int issue_disassoc_from_kernel(void *priv, unsigned char *mac)
{
	memcpy(block_mac[block_mac_idx], mac, 6);
	block_mac_idx++;
	block_mac_idx = block_mac_idx % MAX_BLOCK_MAC;
	
	if (priv != NULL) {
//		issue_disassoc((struct rtl8192cd_priv *)priv, mac, _RSON_UNSPECIFIED_);
		issue_deauth((struct rtl8192cd_priv *)priv, mac, _RSON_UNSPECIFIED_);
		block_sta_time = ((struct rtl8192cd_priv *)priv)->pshare->rf_ft_var.dos_block_time;
		block_priv = (unsigned long)priv;
	}
	return 0;
}
#endif

/**
 *	@brief	issue de-authenticaion
 *
 *	Defragement fail will be de-authentication or STA issue deauthenticaion request
 *
 *	+---------------+-----+----+----+-------+-----+-------------+ \n
 *	| Frame Control | ... | DA | SA | BSSID | ... | Reason Code | \n
 *	+---------------+-----+----+----+-------+-----+-------------+ \n
 */
void issue_deauth(struct rtl8192cd_priv *priv, unsigned char *da, int reason)
{
#ifdef CONFIG_RTK_MESH
	issue_deauth_MP(priv, da, reason, FALSE);
}


void issue_deauth_MP(struct rtl8192cd_priv *priv,	unsigned char *da, int reason, UINT8 is_11s)
{
#endif
	struct wifi_mib *pmib;
	unsigned char	*bssid, *pbuf;
	unsigned short  val;
	struct stat_info *pstat = get_stainfo(priv, da);    
	DECLARE_TXINSN(txinsn);

	if (!memcmp(da, "\x0\x0\x0\x0\x0\x0", 6))
		return;

	// check if da is legal
	if (da[0] & 0x01 
#ifdef CONFIG_IEEE80211W
		&& memcmp(da,"\xff\xff\xff\xff\xff\xff",6)
#endif
		){
		DEBUG_WARN("Send Deauth Req to bad DA %02x%02x%02x%02x%02x%02x", da[0], da[1], da[2], da[3], da[4], da[5]);
		return;
	}

#ifdef TLN_STATS
	stats_conn_rson_counts(priv, reason);
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		pstat = get_stainfo(priv, da);

		if (pstat && pstat->sta_in_firmware == 1) {
			RTL8188E_MACID_NOLINK(priv, 1, REMAP_AID(pstat));
			RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
            pstat->txpause_flag = 0;            
		}
	}
#endif

#ifdef CONFIG_WLAN_HAL
    if(IS_HAL_CHIP(priv))
    {
        pstat = get_stainfo(priv, da);
        
        if(pstat && (REMAP_AID(pstat) < 128))
        {
            DEBUG_WARN("%s %d issue_asocrsp, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
            GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstat));                
            pstat->txpause_flag = 0;
        }
        else
        {
            DEBUG_WARN(" MACID sleep only support 128 STA \n");
        }
    }
#endif

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

	pmib= GET_MIB(priv);

	bssid = pmib->dot11StationConfigEntry.dot11Bssid;

#ifdef CONFIG_RTK_MESH
	txinsn.is_11s = is_11s;
#endif

	txinsn.q_num = MANAGE_QUE_NUM;
#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
        if(rtk_p2p_is_enabled(priv)){
            txinsn.tx_rate = _6M_RATE_;
        }else
#endif
    	txinsn.tx_rate  = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	txinsn.fr_type = _PRE_ALLOCMEM_;
#ifdef CONFIG_IEEE80211W	
	if(!memcmp(da,"\xff\xff\xff\xff\xff\xff",6)) {
		txinsn.isPMF = 1; //?????
	} else {
		pstat = get_stainfo(priv,da);
		PMFDEBUG("txinsn.isPMF,pstat=%x, da=%02x%02x%02x%02x%02x%02x\n",pstat,da[0],da[1],da[2],da[3],da[4],da[5]);
	
		if(pstat)
			txinsn.isPMF = pstat->isPMF;
		else
			txinsn.isPMF = 0;	
	
			//else
				//txinsn.isPMF = 0;
	}	
		//printk("deauth:txinsn.isPMF=%d\n",txinsn.isPMF);
#endif	

	pbuf = txinsn.pframe  = get_mgtbuf_from_poll(priv);

	if (pbuf == NULL)
		goto issue_deauth_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);

	if (txinsn.phdr == NULL)
		goto issue_deauth_fail;

	memset((void *)txinsn.phdr, 0, sizeof(struct  wlan_hdr));

	val = cpu_to_le16(reason);

	pbuf = set_fixed_ie(pbuf, _RSON_CODE_ , (unsigned char *)&val, &txinsn.fr_len);

#ifdef P2P_SUPPORT
/*cfg p2p cfg p2p*/
	if((rtk_p2p_is_enabled(priv)) && rtk_p2p_chk_role(priv,P2P_CLIENT)){
		if(priv->p2pPtr->p2p_disass_ie_len){
			memcpy(pbuf, priv->p2pPtr->p2p_disass_ie, priv->p2pPtr->p2p_disass_ie_len);
			pbuf += priv->p2pPtr->p2p_disass_ie_len;
			txinsn.fr_len += priv->p2pPtr->p2p_disass_ie_len;
		}
	}
#endif


	SetFrameType((txinsn.phdr),WIFI_MGT_TYPE);
	SetFrameSubType((txinsn.phdr),WIFI_DEAUTH);
#ifdef CONFIG_IEEE80211W
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), bssid, MACADDRLEN);

	SNMP_MIB_ASSIGN(dot11DeauthenticateReason, reason);
	SNMP_MIB_COPY(dot11DeauthenticateStation, da, MACADDRLEN);

#ifdef MULTI_MAC_CLONE
	if (OPMODE & WIFI_AP_STATE) { //if priv is AP, disconnect the client in multi-Sta
	    __del_mclone_addr(priv, da);
	}
#endif

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS){
#ifdef CONFIG_RTL_WLAN_STATUS
        priv->wlan_status_flag=1;
#endif       
		return;
    }

issue_deauth_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}


void issue_disassoc(struct rtl8192cd_priv *priv, unsigned char *da, int reason)
{
	struct wifi_mib *pmib;
	unsigned char	*bssid, *pbuf;
	unsigned short  val;
	struct stat_info *pstat;
	DECLARE_TXINSN(txinsn);

	// check if da is legal
	if (da[0] & 0x01) {
		DEBUG_WARN("Send Disassoc Req to bad DA %02x%02x%02x%02x%02x%02x", da[0], da[1], da[2], da[3], da[4], da[5]);
		return;
	}
	pstat = get_stainfo(priv, da);

#ifdef TLN_STATS
	stats_conn_rson_counts(priv, reason);
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		if (pstat && pstat->sta_in_firmware == 1) {
			RTL8188E_MACID_NOLINK(priv, 1, REMAP_AID(pstat));
			RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
            pstat->txpause_flag = 0;            
		}
	}
#endif

#ifdef CONFIG_WLAN_HAL
        if(IS_HAL_CHIP(priv))
        {
            if(pstat && (REMAP_AID(pstat) < 128))
            {
                DEBUG_WARN("%s %d issue disAssoc, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
                GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0 , REMAP_AID(pstat));                
                pstat->txpause_flag = 0;                            
            }
            else
            {
                DEBUG_WARN(" MACID sleep only support 128 STA \n");
            }
        }
#endif


#ifdef P2P_SUPPORT	/*P2P (DUT AS GO) IOT with Phone(M8,nexus7,s4 etc)*/ 
	if(rtk_p2p_is_enabled(priv)){
        if(!(pstat->state & WIFI_ASOC_STATE)){
            /*if peer has sent dis_assoc for DUT, dont' send disassoc to peer*/
    	    P2P_DEBUG("dont' send disassoc to peer\n");            
          	return;
        }
	}
#endif    

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

	pmib= GET_MIB(priv);

	bssid = pmib->dot11StationConfigEntry.dot11Bssid;

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
#ifdef CONFIG_IEEE80211W	
	if(!memcmp(da,"\xff\xff\xff\xff\xff\xff",6)) {
		txinsn.isPMF = 1; //?????
	} else {
		if(pstat)
			txinsn.isPMF = pstat->isPMF;
		else
			txinsn.isPMF = 0;
	}	
		//printk("deauth:txinsn.isPMF=%d\n",txinsn.isPMF);
#endif

#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate  = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;

	pbuf = txinsn.pframe  = get_mgtbuf_from_poll(priv);

	if (pbuf == NULL)
		goto issue_disassoc_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);

	if (txinsn.phdr == NULL)
		goto issue_disassoc_fail;

	memset((void *)txinsn.phdr, 0, sizeof(struct  wlan_hdr));

	val = cpu_to_le16(reason);

	pbuf = set_fixed_ie(pbuf, _RSON_CODE_, (unsigned char *)&val, &txinsn.fr_len);

#ifdef P2P_SUPPORT/*cfg p2p cfg p2p*/
	if((rtk_p2p_is_enabled(priv)) && rtk_p2p_chk_role(priv,P2P_CLIENT)){
		if(priv->p2pPtr->p2p_disass_ie_len){
			memcpy(pbuf, priv->p2pPtr->p2p_disass_ie, priv->p2pPtr->p2p_disass_ie_len);
			pbuf += priv->p2pPtr->p2p_disass_ie_len;
			txinsn.fr_len += priv->p2pPtr->p2p_disass_ie_len;
		}
	}
#endif

	SetFrameType((txinsn.phdr), WIFI_MGT_TYPE);
	SetFrameSubType((txinsn.phdr), WIFI_DISASSOC);

#ifdef CONFIG_IEEE80211W
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif	

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), bssid, MACADDRLEN);

	SNMP_MIB_ASSIGN(dot11DisassociateReason, reason);
	SNMP_MIB_COPY(dot11DisassociateStation, da, MACADDRLEN);

#ifdef MULTI_MAC_CLONE
	if (OPMODE & WIFI_AP_STATE) { //if priv is AP, disconnect the client in multi-Sta
	    __del_mclone_addr(priv, da);
	}
#endif

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS){
#ifdef CONFIG_RTL_WLAN_STATUS
        priv->wlan_status_flag=1;
#endif              
		return;
    }

issue_disassoc_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}


// if pstat == NULL, indiate we are station now...
void issue_auth(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned short status)
{
	struct wifi_mib *pmib;
	unsigned char	*bssid, *pbuf;
	unsigned short  val;
	int use_shared_key=0;

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	UINT8	isMeshMP = FALSE;
#endif	// CONFIG_RTK_MESH

	DECLARE_TXINSN(txinsn);

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

	pmib= GET_MIB(priv);

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	if ((NULL != pstat) && (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && isPossibleNeighbor(pstat))
		isMeshMP = TRUE;

	if ((pstat) || (TRUE == isMeshMP))
#else
	if (pstat)
#endif	//CONFIG_RTK_MESH
		bssid = BSSID;
	else
		bssid = priv->pmib->dot11Bss.bssid;

#if defined(CLIENT_MODE) && defined(INCLUDE_WPA_PSK)
       if (priv->assoc_reject_on && !memcmp(priv->assoc_reject_mac, bssid, MACADDRLEN)) 
	       	       goto issue_auth_fail;
#endif

	txinsn.q_num = MANAGE_QUE_NUM;
#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
    if(rtk_p2p_is_enabled(priv)){	// 2013
        txinsn.tx_rate = _6M_RATE_;
    }else
#endif
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	txinsn.fr_type = _PRE_ALLOCMEM_;

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);

	if (pbuf == NULL)
		goto issue_auth_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);

	if (txinsn.phdr == NULL)
		goto issue_auth_fail;

	memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

	// setting auth algm number
	/* In AP mode,	if auth is set to shared-key, use shared key
	 *		if auth is set to auto, use shared key if client use shared
	 *		otherwise set to open
	 * In client mode, if auth is set to shared-key or auto, and WEP is used,
	 *		use shared key algorithm
	 */
	val = 0;

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)	// skip SIMPLE_CONFIG and Auth check ,Force use open system auth.
	if (FALSE == isMeshMP)
#endif
	{

#ifdef WIFI_SIMPLE_CONFIG
	if (pmib->wscEntry.wsc_enable) {
		if (pstat && (status == _STATS_SUCCESSFUL_) && (pstat->auth_seq == 2) &&
			(pstat->state & WIFI_AUTH_SUCCESS) && (pstat->AuthAlgrthm == 0))
			goto skip_security_check;
		else if ((pstat == NULL) && (AUTH_SEQ == 1))
			goto skip_security_check;
	}
#endif

	if (priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm > 0) {
		if (pstat) {
			if (priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm == 1) // shared key
				val = 1;
			else { // auto mode, check client algorithm
				if (pstat && pstat->AuthAlgrthm)
					val = 1;
			}
		}
		else { // client mode, use shared key if WEP is enabled
			if (priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm == 1) { // shared-key ONLY
				if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
					priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)
					val = 1;
			}
			else { // auto-auth
				if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
					priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_) {
					if (AUTH_SEQ == 1)
						AUTH_MODE_TOGGLE_VAL((AUTH_MODE_TOGGLE ? 0 : 1));

					if (AUTH_MODE_TOGGLE)
						val = 1;
				}
			}
		}

		if (val) {
			val = cpu_to_le16(val);
			use_shared_key = 1;
		}
	}

	if (pstat && (status != _STATS_SUCCESSFUL_))
		val = cpu_to_le16(pstat->AuthAlgrthm);
	}

#ifdef WIFI_SIMPLE_CONFIG
skip_security_check:
#endif

	pbuf = set_fixed_ie(pbuf, _AUTH_ALGM_NUM_, (unsigned char *)&val, &txinsn.fr_len);

	// setting transaction sequence number...
#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	if ((pstat) || (TRUE == isMeshMP))
#else
	if (pstat)
#endif
		val = cpu_to_le16(pstat->auth_seq);	// Mesh only
	else
		val = cpu_to_le16(AUTH_SEQ);

	pbuf = set_fixed_ie(pbuf, _AUTH_SEQ_NUM_, (unsigned char *)&val, &txinsn.fr_len);

	// setting status code...
	val = cpu_to_le16(status);
	pbuf = set_fixed_ie(pbuf, _STATUS_CODE_, (unsigned char *)&val, &txinsn.fr_len);

	// then checking to see if sending challenging text... (Mesh skip this section)
	if (pstat)
	{
		if ((pstat->auth_seq == 2) && (pstat->state & WIFI_AUTH_STATE1) && use_shared_key)
			pbuf = set_ie(pbuf, _CHLGETXT_IE_, 128, pstat->chg_txt, &txinsn.fr_len);
	}
	else
	{
		if ((AUTH_SEQ == 3) && (OPMODE & WIFI_AUTH_STATE1) && use_shared_key)
		{
			pbuf = set_ie(pbuf, _CHLGETXT_IE_, 128, CHG_TXT, &txinsn.fr_len);
			SetPrivacy(txinsn.phdr);
			DEBUG_INFO("sending a privacy pkt with auth_seq=%d\n", AUTH_SEQ);
		}
	}

	SetFrameSubType((txinsn.phdr), WIFI_AUTH);

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	if ((pstat) || (TRUE == isMeshMP))
#else
	if (pstat)	// for AP mode
#endif
	{
		memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
		memcpy((void *)GetAddr2Ptr((txinsn.phdr)), bssid, MACADDRLEN);
	}
	else
	{
		memcpy((void *)GetAddr1Ptr((txinsn.phdr)), bssid, MACADDRLEN);
		memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	}

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	if (TRUE == isMeshMP)	// Though spec define management frames Address 3 is "null mac" (all zero), but avoid filter out by other MP, set "Other MP MAC") (Refer: Draft 1.06, Page 12, 7.2.3, Line 29~30 2007/08/11 by popen)
		memcpy((void *)GetAddr3Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
	else
#endif
		memcpy((void *)GetAddr3Ptr((txinsn.phdr)), bssid, MACADDRLEN);

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return;

issue_auth_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}

void issue_asocrsp(struct rtl8192cd_priv *priv, unsigned short status, struct stat_info *pstat, int pkt_type)
{
	unsigned short	val;
	struct wifi_mib *pmib;
	unsigned char	*bssid,*pbuf;
	DECLARE_TXINSN(txinsn);

#ifdef TLN_STATS
	stats_conn_status_counts(priv, status);
#endif

 
#ifdef CONFIG_IEEE80211W
	unsigned char iebuf[5];
	int timeout;
	unsigned int temp_timeout;
#endif
	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

	pmib= GET_MIB(priv);

	bssid = pmib->dot11StationConfigEntry.dot11Bssid;

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	pbuf = txinsn.pframe  = get_mgtbuf_from_poll(priv);

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		update_remapAid(priv,pstat);
		if (pstat->sta_in_firmware == 1)
		{
			RTL8188E_MACID_NOLINK(priv, (status == _STATS_SUCCESSFUL_)?0:1, REMAP_AID(pstat));
			RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
            pstat->txpause_flag = 0;            
		}
	}
#endif

#ifdef CONFIG_WLAN_HAL
        if(IS_HAL_CHIP(priv))
        {
            if(pstat && (REMAP_AID(pstat) < 128))
            {
                DEBUG_WARN("%s %d issue_asocrsp, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
                GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0 ,REMAP_AID(pstat));                
                pstat->txpause_flag = 0;
            }
            else
            {
                DEBUG_WARN(" MACID sleep only support 128 STA \n");
            }
        }
#endif



	if (pbuf == NULL)
		goto issue_asocrsp_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);

	if (txinsn.phdr == NULL)
		goto issue_asocrsp_fail;

	memset((void *)txinsn.phdr, 0, sizeof(struct  wlan_hdr));

	val = cpu_to_le16(BIT(0));

	if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm)
		val |= cpu_to_le16(BIT(4));

	if (SHORTPREAMBLE)
		val |= cpu_to_le16(BIT(5));

	if (priv->pmib->dot11ErpInfo.shortSlot)
		val |= cpu_to_le16(BIT(10));

#ifdef RTK_AC_SUPPORT //for 11ac logo
	if(priv->pshare->rf_ft_var.lpwrc)
		val |= cpu_to_le16(BIT(8));	/* set spectrum mgt */
#endif		

	pbuf = set_fixed_ie(pbuf, _CAPABILITY_, (unsigned char *)&val, &txinsn.fr_len);

	status = cpu_to_le16(status);
	pbuf = set_fixed_ie(pbuf, _STATUS_CODE_, (unsigned char *)&status, &txinsn.fr_len);

	val = cpu_to_le16(pstat->aid | 0xC000);
	pbuf = set_fixed_ie(pbuf, _ASOC_ID_, (unsigned char *)&val, &txinsn.fr_len);

	if (STAT_OPRATE_LEN <= 8)
		pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_, STAT_OPRATE_LEN, STAT_OPRATE, &txinsn.fr_len);
	else {
		pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_, 8, STAT_OPRATE, &txinsn.fr_len);
		pbuf = set_ie(pbuf, _EXT_SUPPORTEDRATES_IE_, STAT_OPRATE_LEN-8, STAT_OPRATE+8, &txinsn.fr_len);
	}

#ifdef CONFIG_IEEE80211W	
	if(status == cpu_to_le16(_STATS_ASSOC_REJ_TEMP_) && pstat->isPMF) {		
		iebuf[0] = ASSOC_COMEBACK_TIME;

		timeout = (int) RTL_JIFFIES_TO_MILISECONDS(pstat->sa_query_end - jiffies);
		if(timeout < 0)
			timeout = 0;
		if(timeout < SA_QUERY_MAX_TO)
			timeout++;
		PMFDEBUG("ASSOC comeback time, timeout=%d\n", timeout);
		temp_timeout = cpu_to_le32(timeout);
		memcpy(&iebuf[1], &temp_timeout, 4);
		pbuf = set_ie(pbuf, EID_TIMEOUT_INTERVAL, 5, iebuf, &txinsn.fr_len);		
	}
#endif

#ifdef WIFI_WMM
	//Set WMM Parameter Element
	if ((QOS_ENABLE) && (pstat->QosEnabled))
		pbuf = set_ie(pbuf, _RSN_IE_1_, _WMM_Para_Element_Length_, GET_WMM_PARA_IE, &txinsn.fr_len);
#endif

#ifdef WIFI_SIMPLE_CONFIG
/*modify for WPS2DOTX SUPPORT*/
	if (pmib->wscEntry.wsc_enable && pmib->wscEntry.assoc_ielen 
		&& priv->pmib->dot11StationConfigEntry.dot11AclMode!=ACL_allow) {
		memcpy(pbuf, pmib->wscEntry.assoc_ie, pmib->wscEntry.assoc_ielen);
		pbuf += pmib->wscEntry.assoc_ielen;
		txinsn.fr_len += pmib->wscEntry.assoc_ielen;
	}
#endif

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (pstat->ht_cap_len > 0)) {
		if (!should_restrict_Nrate(priv, pstat)) {
			pbuf = set_ie(pbuf, _HT_CAP_, priv->ht_cap_len, (unsigned char *)&priv->ht_cap_buf, &txinsn.fr_len);
			pbuf = set_ie(pbuf, _HT_IE_, priv->ht_ie_len, (unsigned char *)&priv->ht_ie_buf, &txinsn.fr_len);
			pbuf = construct_ht_ie_old_form(priv, pbuf, &txinsn.fr_len);
		}

#ifdef WIFI_11N_2040_COEXIST
		if (priv->pmib->dot11nConfigEntry.dot11nCoexist && priv->pshare->is_40m_bw &&
			(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G)) {
			unsigned char temp_buf = _2040_COEXIST_SUPPORT_ ;
			construct_obss_scan_para_ie(priv);
			pbuf = set_ie(pbuf, _OBSS_SCAN_PARA_IE_, priv->obss_scan_para_len,
				(unsigned char *)&priv->obss_scan_para_buf, &txinsn.fr_len);

			pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 1, &temp_buf, &txinsn.fr_len);
		}
#endif
	}

#ifdef RTK_AC_SUPPORT //for 11ac logo
	if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) && (!should_restrict_Nrate(priv, pstat))) 
	if((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_40_PRIVACY_)
		&& (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_104_PRIVACY_))
	{
// 41
		// operting mode
		{
			char tmp[8];
			memset(tmp, 0, 8);
			tmp[7] = 0x40;
			pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 8, tmp, &txinsn.fr_len);
		}
		
//60, 61
		construct_vht_ie(priv, priv->pshare->working_channel);
		pbuf = set_ie(pbuf, EID_VHTCapability, priv->vht_cap_len, (unsigned char *)&priv->vht_cap_buf, &txinsn.fr_len);
		pbuf = set_ie(pbuf, EID_VHTOperation, priv->vht_oper_len, (unsigned char *)&priv->vht_oper_buf, &txinsn.fr_len);
// 66
		if(priv->pshare->rf_ft_var.opmtest&1)
			pbuf = set_ie(pbuf, EID_VHTOperatingMode, 1, (unsigned char *)&(priv->pshare->rf_ft_var.oper_mode_field), &txinsn.fr_len);		
		
	}
#endif

#if defined(TV_MODE) || defined(A4_STA)
    val = 0;
#ifdef A4_STA
    if(priv->pshare->rf_ft_var.a4_enable == 2) {
        if(pstat->state & WIFI_A4_STA) {
            val |= BIT0;
        }
    }
#endif

#ifdef TV_MODE
    val |= BIT1;
#endif
    if(val)
        pbuf = construct_ecm_tvm_ie(priv, pbuf, &txinsn.fr_len, val);
#endif //defined(TV_MODE) || defined(A4_STA)

	// Realtek proprietary IE
	if (priv->pshare->rtk_ie_len)
		pbuf = set_ie(pbuf, _RSN_IE_1_, priv->pshare->rtk_ie_len, priv->pshare->rtk_ie_buf, &txinsn.fr_len);

#ifdef P2P_SUPPORT/*cfg p2p cfg p2p*/
	if((rtk_p2p_is_enabled(priv)) && rtk_p2p_chk_role(priv,P2P_TMP_GO)){
		if(pstat->is_p2p_client){
			if(priv->p2pPtr->p2p_assoc_RspIe_len){
				memcpy(pbuf, priv->p2pPtr->p2p_assoc_RspIe , priv->p2pPtr->p2p_assoc_RspIe_len);
				pbuf += priv->p2pPtr->p2p_assoc_RspIe_len;
				txinsn.fr_len += priv->p2pPtr->p2p_assoc_RspIe_len;
			}
		}			
	}
#endif

	if ((pkt_type == WIFI_ASSOCRSP) || (pkt_type == WIFI_REASSOCRSP))
		SetFrameSubType((txinsn.phdr), pkt_type);
	else
		goto issue_asocrsp_fail;

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), bssid, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), bssid, MACADDRLEN);


if((GET_CHIP_VER(priv) != VERSION_8812E) && (GET_CHIP_VER(priv) != VERSION_8881A))
{
	unsigned int tmp_d2c = RTL_R32(0xd2c);
						
	if(pstat->IOTPeer== HT_IOT_PEER_INTEL)
	{
		tmp_d2c = (tmp_d2c & (~ BIT(11)));
	}
	else
	{
		if(is_intel_connected(priv))
		tmp_d2c = (tmp_d2c & (~ BIT(11)));
		else
			tmp_d2c = (tmp_d2c | BIT(11));
	}

	RTL_W32(0xd2c, tmp_d2c);

#if 1	
	tmp_d2c = RTL_R32(0xd2c);

	if(tmp_d2c & BIT(11)) {
		DEBUG_TRACE("BIT(11) of 0xd2c = %d, 0xd2c = 0x%x\n", 1, tmp_d2c);
	} else {
		DEBUG_TRACE("BIT(11) of 0xd2c = %d, 0xd2c = 0x%x\n", 0, tmp_d2c);
	}
#endif
	
}

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS) {
//#if !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD)
#if 0
		if(!SWCRYPTO && !IEEE8021X_FUN &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_ ||
			 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)) {
			DOT11_SET_KEY Set_Key;
			memcpy(Set_Key.MACAddr, pstat->hwaddr, 6);
			Set_Key.KeyType = DOT11_KeyType_Pairwise;
			Set_Key.EncType = pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;

			Set_Key.KeyIndex = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
			DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key,
				pmib->dot11DefaultKeysTable.keytype[Set_Key.KeyIndex].skey);
		}
#endif
		return;
	}

issue_asocrsp_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}


int fill_probe_rsp_content(struct rtl8192cd_priv *priv,
				UINT8 *phdr, UINT8 *pbuf,
				UINT8 *ssid, int ssid_len, int set_privacy, UINT8 is_11s, UINT8 is_11b_only)
{
	unsigned short	val;
	struct wifi_mib *pmib;
	unsigned char	*bssid;
	UINT8	val8;
	unsigned char	*pbssrate=NULL;
	int 	bssrate_len, fr_len=0;
#ifdef CONFIG_RTK_MESH
	UINT8 meshiearray[32];	// mesh IE buffer (Max byte is mesh_ie_MeshID)
#endif

#if !defined(SMP_SYNC) && defined(CONFIG_RTL_WAPI_SUPPORT)
	unsigned long		flags;
#endif

#ifdef P2P_SUPPORT
	int need_include_p2pie = 0;
#endif

	pmib= GET_MIB(priv);

	bssid = pmib->dot11StationConfigEntry.dot11Bssid;

	pbuf += _TIMESTAMP_;
	fr_len += _TIMESTAMP_;

    val = cpu_to_le16(pmib->dot11StationConfigEntry.dot11BeaconPeriod);
	pbuf = set_fixed_ie(pbuf,  _BEACON_ITERVAL_ , (unsigned char *)&val, (unsigned int *)&fr_len);

#ifdef P2P_SUPPORT/*cfg p2p cfg p2p*/
	if((rtk_p2p_is_enabled(priv))
		&& (rtk_p2p_chk_role(priv,P2P_DEVICE)) 
		&& (rtk_p2p_chk_state(priv,P2P_S_LISTEN)))
	{
		val |= cpu_to_le16(BIT(0)); //set ESS	 to 1
	}else
#endif
#ifdef CONFIG_RTK_MESH
	if ((1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && (0 == GET_MIB(priv)->dot1180211sInfo.mesh_ap_enable))	// non-AP MP (MAP)	only, popen:802.11s Draft 1.0 P17  7.3.1.4 : ESS & IBSS are "0" (PS:val Reset here.)
		val = 0;
	else
#endif
	{
		if (OPMODE & WIFI_AP_STATE)
			val = cpu_to_le16(BIT(0)); //ESS
		else
			val = cpu_to_le16(BIT(1)); //IBSS
	}

	if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm && set_privacy)
		val |= cpu_to_le16(BIT(4));

#if defined(CONFIG_RTL_WAPI_SUPPORT)
	if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
	{
		val |= cpu_to_le16(BIT(4));	/* set privacy	*/
	}
#endif

	if (SHORTPREAMBLE)
		val |= cpu_to_le16(BIT(5));

	if (priv->pmib->dot11ErpInfo.shortSlot)
		val |= cpu_to_le16(BIT(10));

#ifdef DOT11H
    if(priv->pmib->dot11hTPCEntry.tpc_enable)
        val |= cpu_to_le16(BIT(8));	/* set spectrum mgt */
#endif

	pbuf = set_fixed_ie(pbuf, _CAPABILITY_, (unsigned char *)&val, (unsigned int *)&fr_len);

	pbuf = set_ie(pbuf, _SSID_IE_, ssid_len, ssid, (unsigned int *)&fr_len);

#ifdef P2P_SUPPORT/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv))
		get_bssrate_set(priv, _SUPPORTED_RATES_NO_CCK_, &pbssrate, &bssrate_len);	
	else
#endif
    	get_bssrate_set(priv, _SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len);
	pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_, bssrate_len, pbssrate, (unsigned int *)&fr_len);


#ifdef P2P_SUPPORT		// fill DSSET
/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv) && (rtk_p2p_chk_role(priv,P2P_DEVICE))&& (rtk_p2p_chk_state(priv,P2P_S_LISTEN) )){
		val8 = priv->pmib->p2p_mib.p2p_listen_channel;
	}else
#endif
	{
		val8 = pmib->dot11RFEntry.dot11channel;
	}

#if defined(RTK_5G_SUPPORT) 
  	 if ( priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
#endif
	pbuf = set_ie(pbuf, _DSSET_IE_, 1, &val8 , (unsigned int *)&fr_len);

	if (OPMODE & WIFI_ADHOC_STATE) {
		unsigned short val16 = 0;
		pbuf = set_ie(pbuf, _IBSS_PARA_IE_, 2, (unsigned char *)&val16, (unsigned int *)&fr_len);
	}

#if defined(DOT11D) || defined(DOT11H)    
     if(priv->countryTableIdx) {    
         pbuf = construct_country_ie(priv, pbuf, &fr_len);   
     }
#endif


#ifdef DOT11H
    if(priv->pmib->dot11hTPCEntry.tpc_enable) {        
        pbuf = set_ie(pbuf, _PWR_CONSTRAINT_IE_, 1, &priv->pshare->rf_ft_var.lpwrc, &fr_len);
        pbuf = construct_TPC_report_ie(priv, pbuf, &fr_len);
    }
#endif


	if (OPMODE & WIFI_AP_STATE) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
		 	//ERP infomation.
			val8=0;
			if (priv->pmib->dot11ErpInfo.protection)
				val8 |= BIT(1);
			if (priv->pmib->dot11ErpInfo.nonErpStaNum)
				val8 |= BIT(0);
			pbuf = set_ie(pbuf, _ERPINFO_IE_ , 1 , &val8, (unsigned int *)&fr_len);
		}
	}

	//EXT supported rates.
	if (get_bssrate_set(priv, _EXT_SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len))
		pbuf = set_ie(pbuf, _EXT_SUPPORTEDRATES_IE_ , bssrate_len , pbssrate, (unsigned int *)&fr_len);

	/*
		2008-12-16, For Buffalo WLI_CB_AG54L 54Mbps NIC interoperability issue.
		This NIC can not connect to our AP when our AP is set to WPA/TKIP encryption.
		This issue can be fixed after move "HT Capability Info" and "Additional HT Info" in front of "WPA" and "WMM".
	 */
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (!is_11b_only)) {
		pbuf = set_ie(pbuf, _HT_CAP_, priv->ht_cap_len, (unsigned char *)&priv->ht_cap_buf, (unsigned int *)&fr_len);
		pbuf = set_ie(pbuf, _HT_IE_, priv->ht_ie_len, (unsigned char *)&priv->ht_ie_buf, (unsigned int *)&fr_len);
	}
// probe

#ifdef RTK_AC_SUPPORT //for 11ac logo
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
		// 41
		{
			char tmp[8];
			memset(tmp, 0, 8);
			tmp[7] = 0x40;
			pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 8, tmp, &fr_len);
		}
			
		// 60, 61
		construct_vht_ie(priv, priv->pshare->working_channel);
		pbuf = set_ie(pbuf, EID_VHTCapability, priv->vht_cap_len, (unsigned char *)&priv->vht_cap_buf, &fr_len);
		pbuf = set_ie(pbuf, EID_VHTOperation, priv->vht_oper_len, (unsigned char *)&priv->vht_oper_buf, &fr_len);
	
		// 62 
		if(priv->pshare->rf_ft_var.lpwrc) {
			char tmp[4];
			pbuf = set_ie(pbuf, _PWR_CONSTRAINT_IE_, 1, &priv->pshare->rf_ft_var.lpwrc, &fr_len);
			tmp[1] = tmp[2] = tmp[3] = priv->pshare->rf_ft_var.lpwrc;
			tmp[0] = priv->pshare->CurrentChannelBW;	//	20, 40, 80
			pbuf = set_ie(pbuf, EID_VHTTxPwrEnvelope, tmp[0]+2, tmp, &fr_len);	
		}

		//66
		if(priv->pshare->rf_ft_var.opmtest&1)
		pbuf = set_ie(pbuf, EID_VHTOperatingMode, 1, (unsigned char *)&(priv->pshare->rf_ft_var.oper_mode_field), &fr_len); 	
			
	}
#endif

#ifdef WIFI_11N_2040_COEXIST
	if ((OPMODE & WIFI_AP_STATE) && 
		(priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N|WIRELESS_11G)) &&
		COEXIST_ENABLE && priv->pshare->is_40m_bw) {
		unsigned char temp_buf = _2040_COEXIST_SUPPORT_ ;
		construct_obss_scan_para_ie(priv);
		pbuf = set_ie(pbuf, _OBSS_SCAN_PARA_IE_, priv->obss_scan_para_len,
			(unsigned char *)&priv->obss_scan_para_buf, (unsigned int *)&fr_len);
#ifdef HS2_SUPPORT
        if (priv->pmib->hs2Entry.interworking_ielen)
        {
            unsigned char capArray[6];
            unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_ | _BSS_TRANSITION_;
            if (priv->proxy_arp)
                buf32 |= _PROXY_ARP_;

            if ((priv->pmib->hs2Entry.timezone_ielen!=0) && (priv->pmib->hs2Entry.timeadvt_ielen))
                buf32 |= _UTC_TSF_OFFSET_;

            temp_buf32 = cpu_to_le32(buf32);
            //capArray[5]=0x40; //WNM notification
                capArray[4]=0x0;
                memcpy(capArray,(void *)(&temp_buf32),4);
                pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 5, capArray, &fr_len);
            //pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, (unsigned int *)&fr_len);
        }
        else
#endif
		pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 1, &temp_buf, (unsigned int *)&fr_len);
	}
#ifdef HS2_SUPPORT
    else
    {   
        if (priv->pmib->hs2Entry.interworking_ielen)
        {
            unsigned char capArray[6];
            unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_ | _BSS_TRANSITION_;

            if (priv->proxy_arp)
                buf32 |= _PROXY_ARP_;

            if ((priv->pmib->hs2Entry.timezone_ielen!=0) && (priv->pmib->hs2Entry.timeadvt_ielen))
                buf32 |= _UTC_TSF_OFFSET_;

            temp_buf32 = cpu_to_le32(buf32);
            //capArray[5]=0x40; //WNM notification
                capArray[4]=0x0;
                memcpy(capArray,(void *)(&temp_buf32),4);
                pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 5, capArray, &fr_len);

            //pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, (unsigned int *)&fr_len);
        }
    }
#endif
#endif

	if (pmib->dot11RsnIE.rsnielen && set_privacy
	#if defined(CONFIG_RTL_WAPI_SUPPORT)
		&&(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WAPI_SMS4_)
	#endif
		)
	{
		memcpy(pbuf, pmib->dot11RsnIE.rsnie, pmib->dot11RsnIE.rsnielen);
		pbuf += pmib->dot11RsnIE.rsnielen;
		fr_len += pmib->dot11RsnIE.rsnielen;
	}

#ifdef WIFI_WMM
	//Set WMM Parameter Element
	if (QOS_ENABLE && (is_11b_only != 0xf))
		pbuf = set_ie(pbuf, _RSN_IE_1_, _WMM_Para_Element_Length_, GET_WMM_PARA_IE, (unsigned int *)&fr_len);
#endif


#if defined(TV_MODE) || defined(A4_STA)
    val8 = 0;
#ifdef A4_STA
    if(priv->pshare->rf_ft_var.a4_enable == 2) {
        val8 |= BIT0;
    }
#endif

#ifdef TV_MODE
    val8 |= BIT1;
#endif
    if(val8)
        pbuf = construct_ecm_tvm_ie(priv, pbuf, &fr_len, val8);
#endif //defined(TV_MODE) || defined(A4_STA)



	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (!is_11b_only)) {
		/*
			2008-12-16, For Buffalo WLI_CB_AG54L 54Mbps NIC interoperability issue.
			This NIC can not connect to our AP when our AP is set to WPA/TKIP encryption.
			This issue can be fixed after move "HT Capability Info" and "Additional HT Info" in front of "WPA" and "WMM".
		 */
		//pbuf = set_ie(pbuf, _HT_CAP_, priv->ht_cap_len, (unsigned char *)&priv->ht_cap_buf, &txinsn.fr_len);
		//pbuf = set_ie(pbuf, _HT_IE_, priv->ht_ie_len, (unsigned char *)&priv->ht_ie_buf, &txinsn.fr_len);
		pbuf = construct_ht_ie_old_form(priv, pbuf, (unsigned int *)&fr_len);
	}

#ifdef CONFIG_RTK_MESH
	if((TRUE == is_11s))
	{
        pbuf = set_ie(pbuf, _OFDM_PARAMETER_SET_IE_, mesh_ie_OFDM(priv, meshiearray), meshiearray, &fr_len);
		pbuf = set_ie(pbuf, _MESH_ID_IE_, mesh_ie_MeshID(priv, meshiearray, FALSE), meshiearray, &fr_len);
		pbuf = set_ie(pbuf, _WLAN_MESH_CAP_IE_, mesh_ie_WLANMeshCAP(priv, meshiearray), meshiearray, &fr_len);

        if(priv->mesh_swchnl_channel) { /*mesh channel switch procedure is on-going*/
            pbuf = set_ie(pbuf, _MESH_CHANNEL_SWITCH_IE_, mesh_ie_MeshChannelSwitch(priv, meshiearray), meshiearray, &fr_len);
            pbuf = set_ie(pbuf, _CSA_IE_, mesh_ie_ChannelSwitchAnnoun(priv, meshiearray), meshiearray, &fr_len);            
            pbuf = set_ie(pbuf, _SECONDARY_CHANNEL_OFFSET_IE_, mesh_ie_SecondaryChannelOffset(priv, meshiearray), meshiearray, &fr_len);
        }
	}
#endif

	// Realtek proprietary IE
	if (priv->pshare->rtk_ie_len)
		pbuf = set_ie(pbuf, _RSN_IE_1_, priv->pshare->rtk_ie_len, priv->pshare->rtk_ie_buf, (unsigned int *)&fr_len);

	// Customer proprietary IE
	if (priv->pmib->miscEntry.private_ie_len) {
		memcpy(pbuf, pmib->miscEntry.private_ie, pmib->miscEntry.private_ie_len);
		pbuf += pmib->miscEntry.private_ie_len;
		fr_len += pmib->miscEntry.private_ie_len;
	}

	if(priv->pmib->miscEntry.stage) {
		unsigned char tmp[10] = { 0x00, 0x0d, 0x02, 0x07, 0x01, 0x00, 0x00 };
		if(priv->pmib->miscEntry.stage<=5)
			tmp[6] = 1<<(8-priv->pmib->miscEntry.stage);
		pbuf = set_ie(pbuf, _RSN_IE_1_, 7, tmp, &fr_len);
	}
#ifdef HS2_SUPPORT
	//if support hs2 enable, p2p disable
    if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.hs2_ielen)
    {
        //unsigned char p2ptmpie[]={0x50,0x6f,0x9a,0x09,0x02,0x02,0x00,0x00,0x00};
        unsigned char p2ptmpie[]={0x50,0x6f,0x9a,0x09,0x0a,0x01,0x00,0x01};
        pbuf = set_ie(pbuf, 221, sizeof(p2ptmpie), p2ptmpie, &fr_len);
    }
	//if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.hs2_ielen)
	//{
    //    pbuf = set_ie(pbuf, _HS2_IE_, priv->pmib->hs2Entry.hs2_ielen, priv->pmib->hs2Entry.hs2_ie, &fr_len);
    //}
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.hs2_ielen)
    {
        pbuf = set_ie(pbuf, _HS2_IE_, priv->pmib->hs2Entry.hs2_ielen, priv->pmib->hs2Entry.hs2_ie, &fr_len);
		priv->pmib->hs2Entry.bssload_ie[0] = priv->assoc_num & 0xff;
		priv->pmib->hs2Entry.bssload_ie[1] = (priv->assoc_num & 0xff00) >> 8;
		priv->pmib->hs2Entry.bssload_ie[2] = priv->channel_utilization;
		priv->pmib->hs2Entry.bssload_ie[3] = 0;
		priv->pmib->hs2Entry.bssload_ie[4] = 0;
		pbuf = set_ie(pbuf, _BSS_LOAD_IE_, 5, priv->pmib->hs2Entry.bssload_ie, &fr_len);
    }
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.interworking_ielen)
	{
		pbuf = set_ie(pbuf, _INTERWORKING_IE_, priv->pmib->hs2Entry.interworking_ielen, priv->pmib->hs2Entry.interworking_ie, &fr_len);
	}
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.advt_proto_ielen)
	{
		pbuf = set_ie(pbuf, _ADVT_PROTO_IE_, priv->pmib->hs2Entry.advt_proto_ielen, priv->pmib->hs2Entry.advt_proto_ie, &fr_len);
	}
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.roam_ielen && priv->pmib->hs2Entry.roam_enable)
	{
		pbuf = set_ie(pbuf, _ROAM_IE_, priv->pmib->hs2Entry.roam_ielen, priv->pmib->hs2Entry.roam_ie, &fr_len);
	}
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.timeadvt_ielen)
    {
        pbuf = set_ie(pbuf, _TIMEADVT_IE_, priv->pmib->hs2Entry.timeadvt_ielen, priv->pmib->hs2Entry.timeadvt_ie, &fr_len);
    }
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.MBSSID_ielen)
    {
      pbuf = set_ie(pbuf, _MUL_BSSID_IE_, priv->pmib->hs2Entry.MBSSID_ielen, priv->pmib->hs2Entry.MBSSID_ie, &fr_len);
    }
    if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.timezone_ielen)
    {
        pbuf = set_ie(pbuf, _TIMEZONE_IE_, priv->pmib->hs2Entry.timezone_ielen, priv->pmib->hs2Entry.timezone_ie, &fr_len);
    }
#endif
    
//#ifdef RTK_AC_SUPPORT
#if 0
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
		construct_vht_ie(priv, priv->pshare->working_channel);
		pbuf = set_ie(pbuf, EID_VHTCapability, priv->vht_cap_len, (unsigned char *)&priv->vht_cap_buf, &fr_len);
		pbuf = set_ie(pbuf, EID_VHTOperation, priv->vht_oper_len, (unsigned char *)&priv->vht_oper_buf, &fr_len);
		// operting mode
		{
			char tmp[8];
			memset(tmp, 0, 8);
			tmp[7] = 0x40;
			pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 8, tmp, &fr_len);
		}
		if(priv->pshare->rf_ft_var.opmtest&1)
		pbuf = set_ie(pbuf, EID_VHTOperatingMode, 1, (unsigned char *)&(priv->pshare->rf_ft_var.oper_mode_field), &fr_len);		
	}
#endif

#ifdef P2P_SUPPORT		
    /*fill WSC IE*/
/*cfg p2p cfg p2p*/	
	//if(	rtk_p2p_is_enabled(priv))	 
	if((rtk_p2p_chk_role(priv,P2P_DEVICE)) && (rtk_p2p_chk_state(priv,P2P_S_LISTEN)))
	{
		if(pmib->wscEntry.probe_rsp_ielen){
			memcpy(pbuf, pmib->wscEntry.probe_rsp_ie, pmib->wscEntry.probe_rsp_ielen);
			pbuf += pmib->wscEntry.probe_rsp_ielen;
			fr_len += pmib->wscEntry.probe_rsp_ielen;
		}
	}
	else
#endif
#ifdef WIFI_SIMPLE_CONFIG
	{
		if (!priv->pshare->rf_ft_var.NDSi_support
		&& priv->pmib->dot11StationConfigEntry.dot11AclMode!=ACL_allow){
			if (pmib->wscEntry.wsc_enable && pmib->wscEntry.probe_rsp_ielen) {
				memcpy(pbuf, pmib->wscEntry.probe_rsp_ie, pmib->wscEntry.probe_rsp_ielen);
				pbuf += pmib->wscEntry.probe_rsp_ielen;
				fr_len += pmib->wscEntry.probe_rsp_ielen;
			}
		}
	}
#endif

#ifdef P2P_SUPPORT
    /*fill P2P IE*/
/*cfg p2p cfg p2p*/	
	if( rtk_p2p_is_enabled(priv) && (rtk_p2p_chk_role(priv,P2P_TMP_GO)||rtk_p2p_chk_role(priv,P2P_PRE_GO))){
		if(priv->p2pPtr->probe_rps_to_p2p_dev){
			need_include_p2pie = 1;
			priv->p2pPtr->probe_rps_to_p2p_dev = 0;
		}
	}
/*cfg p2p cfg p2p*/
	if ( ((rtk_p2p_chk_role(priv,P2P_DEVICE)) && (rtk_p2p_chk_state(priv,P2P_S_LISTEN))) 
		 ||  need_include_p2pie ) 
	{
		if(priv->p2pPtr->p2p_probe_rsp_ie_len){
			memcpy(pbuf, priv->p2pPtr->p2p_probe_rsp_ie, priv->p2pPtr->p2p_probe_rsp_ie_len);
			pbuf += priv->p2pPtr->p2p_probe_rsp_ie_len ;
			fr_len += priv->p2pPtr->p2p_probe_rsp_ie_len ;
		}
	}	
#endif



	SetFrameSubType((phdr), WIFI_PROBERSP);
	memcpy((void *)GetAddr2Ptr((phdr)), GET_MY_HWADDR, MACADDRLEN);

#ifdef CONFIG_RTL_WAPI_SUPPORT
		if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
		{
			SAVE_INT_AND_CLI(flags);
			priv->wapiCachedBuf = pbuf+2;
			wapiSetIE(priv);
			pbuf[0] = _EID_WAPI_;
			pbuf[1] = priv->wapiCachedLen;
			pbuf += priv->wapiCachedLen+2;
			fr_len += priv->wapiCachedLen+2;
			RESTORE_INT(flags);
		}
#endif

#ifdef P2P_SUPPORT		
/*cfg p2p cfg p2p*/
	if(	(rtk_p2p_chk_role(priv,P2P_DEVICE)) && (rtk_p2p_chk_state(priv,P2P_S_LISTEN)))	{
		memcpy((void *)GetAddr3Ptr((phdr)), GET_MY_HWADDR , MACADDRLEN);
	}
	else
#endif
	{
		memcpy((void *)GetAddr3Ptr((phdr)), bssid, MACADDRLEN);
	}

	return fr_len;
}


/**
 *	@brief	issue probe response
 *
 *	- Timestamp \n - Beacon interval \n - Capability \n - SSID \n - Support rate \n - DS Parameter set \n \n
 *	+-------+-------+----+----+--------+	\n
 *	| Frame control | DA | SA |	BSS ID |	\n
 *	+-------+-------+----+----+--------+	\n
 *	\n
 *	+-----------+-----------------+------------+------+--------------+------------------+-----------+	\n
 *	| Timestamp | Beacon interval | Capability | SSID | Support rate | DS Parameter set | ERP info.	|	\n
 *	+-----------+-----------------+------------+------+--------------+------------------+-----------+	\n
 *
 *	\param	priv	device info.
 *	\param	da	address
 *	\param	sid	SSID
 *	\param	ssid_len	SSID length
 *	\param 	set_privacy	Use Robust security network
 */
//static 	;  extern for P2P_SUPPORT
void issue_probersp(struct rtl8192cd_priv *priv, unsigned char *da,
				UINT8 *ssid, int ssid_len, int set_privacy, UINT8 is_11b_only)
{
#ifdef CONFIG_RTK_MESH
	issue_probersp_MP(priv, da, ssid, ssid_len, set_privacy, FALSE, is_11b_only);
}


void issue_probersp_MP(struct rtl8192cd_priv *priv, unsigned char *da,
				UINT8 *ssid, int ssid_len, int set_privacy, UINT8 is_11s, UINT8 is_11b_only)
{
//	UINT8 meshiearray[32];	// mesh IE buffer (Max byte is mesh_ie_MeshID)
#endif
	unsigned int z = 0;
#if defined(HS2_SUPPORT) && defined(HS2_CLIENT_TEST)
	unsigned char	*pbuf;
#endif

	DECLARE_TXINSN(txinsn);

//	pmib= GET_MIB(priv);
//	bssid = pmib->dot11StationConfigEntry.dot11Bssid;
#ifdef CONFIG_RTK_MESH
	txinsn.is_11s = is_11s;
#endif
	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;


#ifdef P2P_SUPPORT
	if(rtk_p2p_is_enabled(priv) ){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	txinsn.pframe  = get_mgtbuf_from_poll(priv);

	if (txinsn.pframe == NULL)
		goto issue_probersp_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);

	if (txinsn.phdr == NULL)
		goto issue_probersp_fail;

	memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

	for (z = 0; z < PSP_OUI_NUM; z++) {
		if (is_11b_only && (da[0] == PSP_OUI[z][0]) &&
			(da[1] == PSP_OUI[z][1]) &&
			(da[2] == PSP_OUI[z][2])) {
				is_11b_only = 0xf;
				printMac(da);
				break;
		}
	}	


#ifdef HS2_SUPPORT
#ifdef HS2_CLIENT_TEST
	printk("issue_Probereq_MP\n");
	if ((priv->pshare->rf_ft_var.swq_dbg == 30) || (priv->pshare->rf_ft_var.swq_dbg == 31))
    {
		printk("Probe Request to SSID [Hotspot 2.0]\n");
        strcpy(ssid, "Hotspot 2.0");
        ssid[11] = '\0';
        ssid_len = strlen(ssid);
        pbuf = set_ie(pbuf, _SSID_IE_, ssid_len, ssid, &txinsn.fr_len);
    }
    else if ((priv->pshare->rf_ft_var.swq_dbg == 32) || (priv->pshare->rf_ft_var.swq_dbg == 33) || (priv->pshare->rf_ft_var.swq_dbg == 34) || (priv->pshare->rf_ft_var.swq_dbg == 35) || (priv->pshare->rf_ft_var.swq_dbg == 36) || (priv->pshare->rf_ft_var.swq_dbg == 37) || (priv->pshare->rf_ft_var.swq_dbg == 38) || (priv->pshare->rf_ft_var.swq_dbg == 39))
    {
        pbuf = set_ie(pbuf, _SSID_IE_, 0, ssid, &txinsn.fr_len);
    }
    else
#endif
#endif

#ifdef CONFIG_RTK_MESH
	txinsn.fr_len = fill_probe_rsp_content(priv, txinsn.phdr, txinsn.pframe, ssid, ssid_len, set_privacy, is_11s, is_11b_only);
#else
	txinsn.fr_len = fill_probe_rsp_content(priv, txinsn.phdr, txinsn.pframe, ssid, ssid_len, set_privacy, 0, is_11b_only);
#endif

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return;

issue_probersp_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}


/**
 *	@brief	STA issue prob request
 *
 *	+---------------+-----+------+-----------------+--------------------------+	\n
 *	| Frame Control | ... | SSID | Supported Rates | Extended Supported Rates |	\n
 *	+---------------+-----+------+-----------------+--------------------------+	\n
 *	@param	priv	device
 *	@param	ssid	ssid name
 *	@param	ssid_len	ssid length
 */
static void issue_probereq(struct rtl8192cd_priv *priv, unsigned char *ssid, int ssid_len, unsigned char *da)
{
#ifdef CONFIG_RTK_MESH
	issue_probereq_MP(priv, ssid, ssid_len, da, FALSE);
}


void issue_probereq_MP(struct rtl8192cd_priv *priv, unsigned char *ssid, int ssid_len, unsigned char *da, int is_11s)
{
	UINT8           meshiearray[32];	// mesh IE buffer (Max byte is mesh_ie_MeshID)
#endif

	struct wifi_mib *pmib;
	unsigned char	*hwaddr, *pbuf;
	unsigned char	*pbssrate=NULL;
	int		bssrate_len;
	DECLARE_TXINSN(txinsn);

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return;
#endif


#if	0	//def DFS
#ifdef UNIVERSAL_REPEATER
	if(under_apmode_repeater(priv))
	{
		unsigned int channel = priv->pmib->dot11RFEntry.dot11channel; 
		unsigned char issue_ok = 0;
		unsigned int tmp_opmode =0;
		if(is_DFS_channel(channel))
		{
			if((OPMODE & WIFI_ASOC_STATE) && (OPMODE & WIFI_STATION_STATE))
			{
				issue_ok = 1;
			}	
			
			if(IS_ROOT_INTERFACE(priv))
			{
				tmp_opmode = GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.opmode;

				if((tmp_opmode & WIFI_ASOC_STATE) && (tmp_opmode & WIFI_STATION_STATE))
				{
					issue_ok = 1;
				}
			}
			
			//printk("DFS Channel=%d, issue_probeReq=%d\n", channel, issue_ok);
			
			if(issue_ok == 0)
				return;		
		}
	}
#endif
#endif

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

	pmib = GET_MIB(priv);

	hwaddr = pmib->dot11OperationEntry.hwaddr;
#ifdef CONFIG_RTK_MESH
	txinsn.is_11s = is_11s;
#endif
	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;

#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);

	if (pbuf == NULL)
		goto issue_probereq_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);

	if (txinsn.phdr == NULL)
		goto issue_probereq_fail;

	memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

	pbuf = set_ie(pbuf, _SSID_IE_, ssid_len, ssid, &txinsn.fr_len);

	/*fill supported rates*/ 

#ifdef P2P_SUPPORT/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
		get_bssrate_set(priv, _SUPPORTED_RATES_NO_CCK_, &pbssrate, &bssrate_len);	
	}else
#endif
	{
		get_bssrate_set(priv, _SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len);	
	}

	
	pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_ , bssrate_len , pbssrate, &txinsn.fr_len);

	if (get_bssrate_set(priv, _EXT_SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len))
		pbuf = set_ie(pbuf, _EXT_SUPPORTEDRATES_IE_ , bssrate_len , pbssrate, &txinsn.fr_len);


#ifdef RTK_AC_SUPPORT	// WDS-VHT support
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
			//WDEBUG("construct_vht_ie\n");
			construct_vht_ie(priv, priv->pshare->working_channel);
			pbuf = set_ie(pbuf, EID_VHTCapability, priv->vht_cap_len, (unsigned char *)&priv->vht_cap_buf, &txinsn.fr_len);
			pbuf = set_ie(pbuf, EID_VHTOperation, priv->vht_oper_len, (unsigned char *)&priv->vht_oper_buf, &txinsn.fr_len);
		}
#endif

#ifdef P2P_SUPPORT		
	if (rtk_p2p_is_enabled(priv) )
	{		/*add wsc ie*/
		if(pmib->wscEntry.probe_req_ielen){
			memcpy(pbuf, pmib->wscEntry.probe_req_ie, pmib->wscEntry.probe_req_ielen);
			pbuf += pmib->wscEntry.probe_req_ielen;
			txinsn.fr_len += pmib->wscEntry.probe_req_ielen;
		}
	}
	else
#endif
#ifdef WIFI_SIMPLE_CONFIG
	{
		if (pmib->wscEntry.wsc_enable && pmib->wscEntry.probe_req_ielen) {
			memcpy(pbuf, pmib->wscEntry.probe_req_ie, pmib->wscEntry.probe_req_ielen);
			pbuf += pmib->wscEntry.probe_req_ielen;
			txinsn.fr_len += pmib->wscEntry.probe_req_ielen;
		}
	}
#endif

/*cfg p2p cfg p2p*/
#ifdef P2P_SUPPORT
	if (rtk_p2p_is_enabled(priv) )
	{
		if(priv->p2pPtr->p2p_probe_req_ie_len){
			memcpy(pbuf, priv->p2pPtr->p2p_probe_req_ie, priv->p2pPtr->p2p_probe_req_ie_len);			
			pbuf += priv->p2pPtr->p2p_probe_req_ie_len ;
			txinsn.fr_len += priv->p2pPtr->p2p_probe_req_ie_len ;			
		}
	}	
#endif
/*cfg p2p cfg p2p*/

#ifdef HS2_SUPPORT
#ifdef HS2_CLIENT_TEST
	if (priv->pshare->rf_ft_var.swq_dbg == 30)
    {
		// HS2.0 AP does not transmit a probe response frame
		// HESSID is wrong.
		
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_, frlen=0;
        unsigned char tmp[]={0x02,0x50,0x6F,0x9A,0x00,0x00,0x01};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 7, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 31)
    {
		// HS2.0 AP does not transmit a probe response frame
		// HESSID is wrong.
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_, frlen=0;
        unsigned char tmp[]={0x02,0x00,0x00,0x50,0x6F,0x9A,0x00,0x00,0x01};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 9, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 32)
    {
		// APUT transmits Probe Response Message
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_, frlen=0;
        unsigned char tmp[]={0x02,0x00,0x33,0x44,0x55,0x66,0x77}; // HESSID = redir_mac, please refer to next line

		memcpy(&tmp[1], priv->pmib->hs2Entry.redir_mac, 6);

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

		frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 7, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 33)
    {
		// APUT transmits Probe Response Message
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_, frlen=0;
        unsigned char tmp[]={0x02,0x00,0x00,0x00,0x33,0x44,0x55,0x66,0x77}; // HESSID = redir_mac, please refer to next line

		memcpy(&tmp[3], priv->pmib->hs2Entry.redir_mac, 6);

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 9, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 34)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_, frlen=0;
        unsigned char tmp[]={0x03}; // HESSID is not present 

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 1, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 35)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_, frlen=0;
        unsigned char tmp[]={0x03,0x00,0x00}; // HESSID is not present

		temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 3, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 36)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_, frlen=0;
        unsigned char tmp[]={0x02,0xff,0xff,0xff,0xff,0xff,0xff};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 7, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 37)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_, frlen=0;
        unsigned char tmp[]={0x02,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 9, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
	else if (priv->pshare->rf_ft_var.swq_dbg == 38)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_, frlen=0;
        unsigned char tmp[]={0x0f,0xff,0xff,0xff,0xff,0xff,0xff};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 7, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 39)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_, frlen=0;
        unsigned char tmp[]={0x0f,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 9, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
#endif
#endif


#ifdef A4_STA
    if(priv->pshare->rf_ft_var.a4_enable == 2) {
        pbuf = construct_ecm_tvm_ie(priv, pbuf, &txinsn.fr_len, BIT0);
    }
#endif

#ifdef CONFIG_RTK_MESH	// mesh_profile Configure by WEB in the future, Maybe delete, Preservation before delete
	if((TRUE == is_11s) && (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && (TRUE == priv->mesh_profile[0].used)
			&& (MESH_PEER_LINK_CAP_NUM(priv) > 0))
	{
// ==== modified by GANTOE for site survey 2008/12/25 ====
		if(priv->auto_channel == 0)
			pbuf = set_ie(pbuf, _MESH_ID_IE_, 0, "", &txinsn.fr_len);
		else
			pbuf = set_ie(pbuf, _MESH_ID_IE_, mesh_ie_MeshID(priv, meshiearray, FALSE), meshiearray, &txinsn.fr_len);
		pbuf = set_ie(pbuf, _WLAN_MESH_CAP_IE_, mesh_ie_WLANMeshCAP(priv, meshiearray), meshiearray, &txinsn.fr_len);
	}
#endif

	SetFrameSubType(txinsn.phdr, WIFI_PROBEREQ);

	if (da)
		memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN); // unicast
	else
		memset((void *)GetAddr1Ptr((txinsn.phdr)), 0xff, MACADDRLEN); // broadcast
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), hwaddr, MACADDRLEN);
	//nctu note
	// spec define ProbeREQ Address 3 is BSSID or wildcard) (Refer: Draft 1.06, Page 12, 7.2.3, Line 27~28)
	memset((void *)GetAddr3Ptr((txinsn.phdr)), 0xff, MACADDRLEN);

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS){

        return;
    }else{
        NDEBUG("tx probe_req fail!!\n");    
    }
issue_probereq_fail:
    NDEBUG("tx probe_req fail!!\n");    
    if(RTL_R8(TXPAUSE)){
        NDEBUG("!!!tx_pause_val[%X]\n",RTL_R8(TXPAUSE));
    } 

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}


#if defined(CLIENT_MODE) && defined(WIFI_11N_2040_COEXIST)
static void issue_coexist_mgt(struct rtl8192cd_priv *priv)
{
	unsigned char	*pbuf;
	unsigned int len = 0, ch_len=0, i=0;
	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_coexist_mgt_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_coexist_mgt_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _PUBLIC_CATEGORY_ID_;
	pbuf[1] = _2040_COEXIST_ACTION_ID_;
	len+=2;

	pbuf[2] = _2040_BSS_COEXIST_IE_;
	pbuf[3] = 1;
	len+=2;

	if (priv->intolerant_timeout)
		pbuf[4] = _20M_BSS_WIDTH_REQ_;
	else
		pbuf[4] = 0;
	len+=1;

	if (priv->bg_ap_timeout) {
		pbuf[5] = _2040_Intolerant_ChRpt_IE_;
		pbuf[7] = 0; /*set category*/
		for (i=0; i<14; i++) {
			if (priv->bg_ap_timeout_ch[i]) {
				pbuf[8+ch_len] = i+1;/*set channels*/
				ch_len++;
			}
		}
		pbuf[6] = ch_len+1;
		len += (pbuf[6]+2);
	}

	txinsn.fr_len = len;
	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

	DEBUG_INFO("Coexist-mgt sent to AP\n");

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return;

issue_coexist_mgt_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
	return;
}
#endif


#ifdef WIFI_WMM

#if 0
void issue_DELBA(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char TID, unsigned char initiator){
	unsigned char	*pbuf;
	unsigned short	delba_para = 0;
	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE	
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_DELBA_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_DELBA_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _BLOCK_ACK_CATEGORY_ID_;
	pbuf[1] = _DELBA_ACTION_ID_;
	delba_para = initiator << 11 | TID << 12;	// assign buffer size | assign TID | set Immediate Block Ack
	pbuf[2] = initiator << 3 | TID << 4;
	pbuf[3] = 0;
	pbuf[4] = 38;//reason code
	pbuf[5] = 0;

	/* set the immediate next seq number of the "TID", as Block Ack Starting Seq*/

	txinsn.fr_len = _DELBA_Frame_Length;

	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

	DEBUG_INFO("issue_DELBAreq sent to AID %d, token %d TID %d size %d seq %d\n",
		pstat->aid, pstat->dialog_token, TID, max_size, pstat->AC_seq[TID]);

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return;

issue_DELBA_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
	return;
}
#endif

void issue_ADDBAreq(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char TID)
{
	unsigned char	*pbuf;
	unsigned short	ba_para = 0;
	int max_size;
	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;

#ifdef CONFIG_IEEE80211W		
	if(pstat)
		txinsn.isPMF = pstat->isPMF;
	else
		txinsn.isPMF = 0;	
#endif	
	
#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_ADDBAreq_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_ADDBAreq_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	if (!(++pstat->dialog_token))	// dialog token set to a non-zero value
		pstat->dialog_token++;

	pbuf[0] = _BLOCK_ACK_CATEGORY_ID_;
	pbuf[1] = _ADDBA_Req_ACTION_ID_;
	pbuf[2] = pstat->dialog_token;

	if (should_restrict_Nrate(priv, pstat))
		max_size = 1;
	else {
#ifdef CONFIG_RTL8196B_GW_8M
		if (pstat->IOTPeer==HT_IOT_PEER_BROADCOM)
			max_size = _ADDBA_Maximum_Buffer_Size_ / 2;
	else
#endif
		max_size = _ADDBA_Maximum_Buffer_Size_;
	}

	ba_para = (max_size<<6) | (TID<<2) | BIT(1);	// assign buffer size | assign TID | set Immediate Block Ack

#if defined(SUPPORT_RX_AMSDU_AMPDU)
	if(AMSDU_ENABLE >= 2)
	ba_para |= BIT(0);			// AMSDU
#endif

	pbuf[3] = ba_para & 0x00ff;
	pbuf[4] = (ba_para & 0xff00) >> 8;

	// set Block Ack Timeout value to zero, to disable the timeout
	pbuf[5] = 0;
	pbuf[6] = 0;

	// set the immediate next seq number of the "TID", as Block Ack Starting Seq
	pbuf[7] = ((pstat->AC_seq[TID] & 0xfff) << 4) & 0x00ff;
	pbuf[8] = (((pstat->AC_seq[TID] & 0xfff) << 4) & 0xff00) >> 8;

	txinsn.fr_len = _ADDBA_Req_Frame_Length_;
	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
#ifdef CONFIG_IEEE80211W
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

	DEBUG_INFO("ADDBA-req sent to AID %d, token %d TID %d size %d seq %d\n",
		pstat->aid, pstat->dialog_token, TID, max_size, pstat->AC_seq[TID]);
	/*
	panic_printk("ADDBA-req sent to AID %d, token %d TID %d size %d seq %d\n",
		pstat->aid, pstat->dialog_token, TID, max_size, pstat->AC_seq[TID]);
	*/
	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS) {
		//pstat->ADDBA_ready++;
		return;
	}

issue_ADDBAreq_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
	return;
}

#ifdef HS2_SUPPORT
#ifdef HS2_CLIENT_TEST

//issue_GASreq for client test used
int issue_GASreq(struct rtl8192cd_priv *priv, DOT11_HS2_GAS_REQ *gas_req, unsigned short qid)
{
	unsigned char	*pbuf;
	struct stat_info *pstat;
#if defined(WIFI_WMM)
	int ret;
#endif	
	DECLARE_TXINSN(txinsn);
	
	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE	
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);

	txinsn.phdr = get_wlanhdr_from_poll(priv);

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	if (qid != 10000)
	{
		if (qid < 500)
		{	
			if (qid == 0)
				gas_req->Reqlen = 0;
			else if (qid == 270)
				gas_req->Reqlen = 26;	
			else if (qid == 271)
				gas_req->Reqlen = 27;
			else if (qid == 272)
				gas_req->Reqlen = 15;
			else
				gas_req->Reqlen = 6;
		}
		else if(qid == 501)
			gas_req->Reqlen = 12;
		else if(qid == 502)
			gas_req->Reqlen = 12;
		else if(qid == 503)
			gas_req->Reqlen = 24;		
		else if(qid == 504)
			gas_req->Reqlen = 36;
		else if(qid == 505)
			gas_req->Reqlen = 44;
		else if(qid == 506)
			gas_req->Reqlen = 11;
		else if(qid == 507)
			gas_req->Reqlen = 11;
		else if(qid == 508)
			gas_req->Reqlen = 25;
		else if(qid == 509)
			gas_req->Reqlen = 19;
	}
	else
		gas_req->Reqlen = 10;
	
	if ((OPMODE & WIFI_ASOC_STATE) == 0)
	{		
		//unsigned char tmpmac[]={0x00,0xe0,0x4c,0x09,0x08,0x10};
		//memcpy((GET_MIB(priv))->dot11StationConfigEntry.dot11Bssid, tmpmac, 6);
		memcpy((GET_MIB(priv))->dot11StationConfigEntry.dot11Bssid, priv->pmib->hs2Entry.redir_mac, 6);
	}	
	memcpy(gas_req->MACAddr , BSSID, 6);
//	gas_req->MACAddr[1] = 0x33;
//	gas_req->MACAddr[2] = 0x44;
//	gas_req->MACAddr[3] = 0x55;
//	gas_req->MACAddr[4] = 0x66;
//	gas_req->MACAddr[5] = 0x77;
	
	pbuf[0] = _PUBLIC_CATEGORY_ID_;
	if (qid != 0)
		pbuf[1] = _GAS_INIT_REQ_ACTION_ID_;
	else
		pbuf[1] = _GAS_COMBACK_REQ_ACTION_ID_;
		
	pbuf[2] = gas_req->Dialog_token;
	
	if (qid != 0)
	{
		// refer to Fig. 8-354 in IEEE 802.11 - 2012
		pbuf[3] = 108; // element ID: 108 (Advertisement Protocol IE)
		pbuf[4] = 2;  // length = 0x 02
		pbuf[5] = 0x00; // Query Response Length Limit
		pbuf[6] = 0;  // Advertisement Protcol ID = 0x0 (ANQP)
	if (priv->pshare->rf_ft_var.swq_dbg == 12)
		pbuf[6] = 1;	// Advertisement Protcol ID = 0x1 (MIH) 
	else	
		pbuf[6] = 0;	//  Advertisement Protcol ID = 0x0 (ANQP)
	pbuf[7] = gas_req->Reqlen & 0x00ff;	// Query Request length
	pbuf[8] = (gas_req->Reqlen & 0xff00) >> 8; // Query Request length
	pbuf[9] = 256 & 0x00ff; // Info ID = 256 ( Query List)
	// Query Request fields
	// Info ID = 256 ( Query List)
	pbuf[10] = (256 & 0xff00) >> 8;
	
	if (qid != 10000)
	{
		if (qid < 500)
		{
			if (qid == 270)
			{
				pbuf[11] = 2 & 0x00ff;			// length
				pbuf[12] = (2 & 0xff00) >> 8;
				pbuf[13] = 261 & 0x00ff;		// InfoID: Roaming list
				pbuf[14] = (261 & 0xff00) >> 8; // InfoID: Roaming list
				pbuf[15] = 0xdd;
				pbuf[16] = 0xdd;
				pbuf[17] = 0x06;
				pbuf[18] = 0x00;
				pbuf[19] = 0x50;
				pbuf[20] = 0x6f;
				pbuf[21] = 0x9a;
				pbuf[22] = 0x11;
				pbuf[23] = 0x04; // WAN Metrics
				pbuf[24] = 0x00;
				pbuf[25] = 0xdd;
				pbuf[26] = 0xdd;
				pbuf[27] = 0x06;
				pbuf[28] = 0x00;
				pbuf[29] = 0x50;
				pbuf[30] = 0x6f;
				pbuf[31] = 0x9a;
				pbuf[32] = 0x11;
				pbuf[33] = 0x07; // Operating Class Indication
				pbuf[34] = 0x00;
				txinsn.fr_len += 35;
			}
			else if (qid == 271)
			{
				pbuf[11] = 2 & 0x00ff;
				pbuf[12] = (2 & 0xff00) >> 8;
				pbuf[13] = 261 & 0x00ff;		// InfoID: Roaming list
				pbuf[14] = (261 & 0xff00) >> 8; // InfoID: Roaming list
				pbuf[15] = 0xdd;
				pbuf[16] = 0xdd;
				pbuf[17] = 0x06;
				pbuf[18] = 0x00;
				pbuf[19] = 0x50;
				pbuf[20] = 0x6f;
				pbuf[21] = 0x9a;
				pbuf[22] = 0x0b;
				pbuf[23] = 0x00; // Subtype = 0 (Reserved)
				pbuf[24] = 0x00;
				pbuf[25] = 0xdd;
				pbuf[26] = 0xdd;
				pbuf[27] = 0x07;
				pbuf[28] = 0x00;
				pbuf[29] = 0x50;
				pbuf[30] = 0x6f;
				pbuf[31] = 0x9a;
				pbuf[32] = 0x11;
				pbuf[33] = 0x01; // HS Query List
				pbuf[34] = 0x00;
				pbuf[35] = 0x07; // Query Operating Class Indication
				txinsn.fr_len += 24;
			} 
			else if (qid  == 272) {
				pbuf[11] = 2& 0x00ff;
				pbuf[12] = (2& 0xff00) >> 8;
				pbuf[13] = 261 & 0x00ff;		// InfoID: Roaming list
				pbuf[14] = (261 & 0xff00) >> 8; // InfoID: Roaming list
				pbuf[15] = 0xdd;
				pbuf[16] = 0xdd;
				pbuf[17] = 0x05;
				pbuf[18] = 0x00;
				pbuf[19] = 0x50;
				pbuf[20] = 0x6f;
				pbuf[21] = 0x9a;
				pbuf[22] = 0x0b;
				pbuf[23] = 0x00; // Subtype = 0 (Reserved)
				txinsn.fr_len += 36;
			}
			else
			{
				// ANQP Query List (Fig. 8-403 in IEEE 802.11-2012)
				// length = 2
				pbuf[11] = 2 & 0x00ff;	
				pbuf[12] = (2 & 0xff00) >> 8;
				// ANQP Query ID
				pbuf[13] = qid & 0x00ff;
				pbuf[14] = (qid & 0xff00) >> 8;
				txinsn.fr_len += 15;
			}
		}
		else if(qid == 501)
		{
			pbuf[9] = 56797 & 0x00ff;
			pbuf[10] = (56797 & 0xff00) >> 8;
			pbuf[11] = 0x08;
			pbuf[12] = 0;
			//OI
			pbuf[13] =0x50;
			pbuf[14] =0x6f;
			pbuf[15] =0x9a;
			pbuf[16] =0x11;			
			pbuf[17] =0x1; // HS query list
			pbuf[18] =0x0;
			//payload
			pbuf[19] =0x2; // HS Capability List
			pbuf[20] =0x3; // Operator Friendly Name
			txinsn.fr_len += 21;
		}
		else if(qid == 502)
		{
			pbuf[9] = 56797 & 0x00ff;
			pbuf[10] = (56797 & 0xff00) >> 8;
			pbuf[11] = 0x08;
			pbuf[12] = 0;
			//OI
			pbuf[13] =0x50;
			pbuf[14] =0x6f;
			pbuf[15] =0x9a;			
			pbuf[16] =0x11; 
			pbuf[17] =0x1; // HS query list
			pbuf[18] =0x0;
			//payload
			pbuf[19] =0x4;
			pbuf[20] =0x5;
			txinsn.fr_len += 21;
		}
		else if(qid == 505)
		{
			pbuf[9] = 56797 & 0x00ff;
            pbuf[10] = (56797 & 0xff00) >> 8;
            pbuf[11] = 0x08;
            pbuf[12] = 0;
			pbuf[13] =0x50;
            pbuf[14] =0x6f;
            pbuf[15] =0x9a;
			pbuf[16] =0x11;
			pbuf[17] =0x1; // HS query list
			pbuf[18] =0x0;
			pbuf[19] =0x4;
            pbuf[20] =0x5;
			pbuf[21] = 56797 & 0x00ff;
            pbuf[22] = (56797 & 0xff00) >> 8;
            pbuf[23] = 0x06;
            pbuf[24] = 0;
            pbuf[25] =0x50;
            pbuf[26] =0x6f;
            pbuf[27] =0x9a;
            pbuf[28] =0x0b;
            pbuf[29] =0x0; // Reserved
			pbuf[30] =0x0;
			pbuf[31] = 56797 & 0x00ff;
            pbuf[32] = (56797 & 0xff00) >> 8;
            pbuf[33] = 21;
            pbuf[34] = 0;
            pbuf[35] =0x50;
            pbuf[36] =0x6f;
            pbuf[37] =0x9a;
            pbuf[38] =0x11;
            pbuf[39] =0x06; // Subtype: NAI Home Realm Query
			pbuf[40] =0x0;
			pbuf[41] =0x1;
            pbuf[42] =0x0;
            pbuf[43] =0xc;
            pbuf[44] ='e';
            pbuf[45] ='x';
            pbuf[46] ='a';
            pbuf[47] ='m';
            pbuf[48] ='p';
            pbuf[49] ='l';
            pbuf[50] ='e';
            pbuf[51] ='.';
            pbuf[52] ='c';
            pbuf[53] ='o';
            pbuf[54] ='m';

            txinsn.fr_len += 55;

		}
		else if(qid == 503)
		{
			pbuf[9] = 56797 & 0x00ff;
			pbuf[10] = (56797 & 0xff00) >> 8;
			pbuf[11] = 21;
			pbuf[12] = 0;
			//OI
			pbuf[13] =0x50;
			pbuf[14] =0x6f;
			pbuf[15] =0x9a;
			//TYPE
			pbuf[16] =0x11;
			//home realm query
			pbuf[17] =0x6;
			pbuf[18] =0x0;
			//payload
			pbuf[19] =0x1;
			pbuf[20] =0x0;
			pbuf[21] =0xc;
			pbuf[22] ='e';
			pbuf[23] ='x';
			pbuf[24] ='a';
			pbuf[25] ='m';
			pbuf[26] ='p';
			pbuf[27] ='l';
			pbuf[28] ='e';
			pbuf[29] ='.';
			pbuf[30] ='c';
			pbuf[31] ='o';
			pbuf[32] ='m';
			txinsn.fr_len += 33;
		}
		else if (qid == 506)
		{
			pbuf[9] = 56797 & 0x00ff;
            pbuf[10] = (56797 & 0xff00) >> 8;
            pbuf[11] = 7;
            pbuf[12] = 0;
			pbuf[13] =0x50;
            pbuf[14] =0x6f;
            pbuf[15] =0x9a;
			pbuf[16] =0x11;
			pbuf[17] =0x6;
			pbuf[18] =0x0;
			pbuf[19] =0x0;
			txinsn.fr_len += 20;
		}
		else if (qid == 507)
        {
            pbuf[9] = 56797 & 0x00ff;
            pbuf[10] = (56797 & 0xff00) >> 8;
            pbuf[11] = 7;
            pbuf[12] = 0;
            pbuf[13] =0x50;
            pbuf[14] =0x6f;
            pbuf[15] =0x9a;
            pbuf[16] =0x11;
            pbuf[17] =0x1;
            pbuf[18] =0x0;
			pbuf[19] =0x7;
            txinsn.fr_len += 20;
        }
		else if (qid == 504)
		{
			pbuf[9] = 56797 & 0x00ff;
            pbuf[10] = (56797 & 0xff00) >> 8;
            pbuf[11] = 32;
            pbuf[12] = 0;
			pbuf[13] =0x50;
            pbuf[14] =0x6f;
            pbuf[15] =0x9a;
			pbuf[16] =0x11;
			pbuf[17] =0x6;
			pbuf[18] =0x0;
			pbuf[19] =0x2;
			pbuf[20] =0x0;
            pbuf[21] =0x9;
            pbuf[22] ='c';
            pbuf[23] ='i';
            pbuf[24] ='s';
            pbuf[25] ='c';
            pbuf[26] ='o';
            pbuf[27] ='.';
            pbuf[28] ='c';
            pbuf[29] ='o';
            pbuf[30] ='m';

            pbuf[31] =0x0;
            pbuf[32] =0xc;
            pbuf[33] ='e';
            pbuf[34] ='x';
            pbuf[35] ='a';
            pbuf[36] ='m';
            pbuf[37] ='p';
            pbuf[38] ='l';
            pbuf[39] ='e';
            pbuf[40] ='4';
            pbuf[41] ='.';
            pbuf[42] ='c';
            pbuf[43] ='o';
            pbuf[44] ='m';
            txinsn.fr_len += 45;
		}
		else if (qid == 508)
        {
            pbuf[9] = 56797 & 0x00ff;
            pbuf[10] = (56797 & 0xff00) >> 8;
            pbuf[11] = 15;
            pbuf[12] = 0;
            pbuf[13] =0x50;
            pbuf[14] =0x6f;
            pbuf[15] =0x9a;
            pbuf[16] =0x11;
            pbuf[17] =0xa;
            pbuf[18] =0x0;
			pbuf[19] ='1';
			pbuf[20] ='3';
			pbuf[21] ='5';
			pbuf[22] ='7';
			pbuf[23] ='1';			
			pbuf[24] ='6';
			pbuf[25] ='1';
			pbuf[26] ='4';
			pbuf[27] ='7';
			pbuf[28] ='5';
			pbuf[29] ='_';
			pbuf[30] ='w';
			pbuf[31] ='i';
			pbuf[32] ='f';
			pbuf[33] ='i';
			
            txinsn.fr_len += 34;
        }
		else if (qid == 509)
		{
			
			pbuf[11] = 4 & 0x00ff;			// length
			pbuf[12] = (4 & 0xff00) >> 8;
			pbuf[13] = 260 & 0x00ff;		// InfoID: Network Authentication Type
			pbuf[14] = (260 & 0xff00) >> 8; // InfoID: Network Authentication Type
			pbuf[15] = 263 & 0x00ff;		// InfoID: NAI Realm List
			pbuf[16] = (263 & 0xff00) >> 8; // InfoID: NAI Realm List
			pbuf[17] = 56797 & 0x00ff;
			pbuf[18] = (56797 & 0xff00) >> 8;
			pbuf[19] = 0x07;
			pbuf[20] = 0;
			//OI
			pbuf[21] =0x50;
			pbuf[22] =0x6f;
			pbuf[23] =0x9a;
			pbuf[24] =0x11;			
			pbuf[25] =0x1; // HS query list
			pbuf[26] =0x0;
			//payload
			pbuf[27] =0x8; // OSU Providers List
			txinsn.fr_len += 28;
		}
	}
	else
	{
		pbuf[11] = 6 & 0x00ff;
		pbuf[12] = (6 & 0xff00) >> 8;
		pbuf[13] = 263 & 0x00ff;
		pbuf[14] = (263 & 0xff00) >> 8;
		pbuf[15] = 264 & 0x00ff;
		pbuf[16] = (264 & 0xff00) >> 8;
		pbuf[17] = 268 & 0x00ff;
		pbuf[18] = (268 & 0xff00) >> 8;
		txinsn.fr_len += 19;
	}
	}
	else
	{
		txinsn.fr_len += 11;
	}
	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), gas_req->MACAddr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), gas_req->MACAddr, MACADDRLEN);
	
	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return SUCCESS;
}
#endif

int issue_GASrsp(struct rtl8192cd_priv *priv, DOT11_HS2_GAS_RSP *gas_rsp)
{
	unsigned char	*pbuf;
	struct stat_info *pstat;
#if defined(WIFI_WMM)
	int ret;
#endif	
	DECLARE_TXINSN(txinsn);
	
	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_GASrsp_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_GASrsp_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _PUBLIC_CATEGORY_ID_;
	pbuf[1] = gas_rsp->Action;
	pbuf[2] = gas_rsp->Dialog_token;
	pbuf[3] = gas_rsp->StatusCode & 0x00ff;
	pbuf[4] = (gas_rsp->StatusCode & 0xff00) >> 8;
	
	if (gas_rsp->Action == _GAS_INIT_RSP_ACTION_ID_)
	{
		pbuf[5] = gas_rsp->Comeback_delay & 0x00ff;
		pbuf[6] = (gas_rsp->Comeback_delay & 0xff00) >> 8;
		//advertisement protocol element
		pbuf[7] = 0x6c;
		pbuf[8] = 2;
		pbuf[9] = 0x20;
		pbuf[10] = 0;//gas_rsp->Advt_proto;
		//gas rsp
		pbuf[11] = gas_rsp->Rsplen & 0x00ff;
		pbuf[12] = (gas_rsp->Rsplen & 0xff00) >> 8;
		if (gas_rsp->Rsplen > 0)
			memcpy(&pbuf[13], gas_rsp->Rsp, gas_rsp->Rsplen);
		
		txinsn.fr_len += 13 +	gas_rsp->Rsplen;
	}
	else if (gas_rsp->Action == _GAS_COMBACK_RSP_ACTION_ID_)
	{
		pbuf[5] = gas_rsp->Rsp_fragment_id;
		pbuf[6] = gas_rsp->Comeback_delay & 0x00ff;
		pbuf[7] = (gas_rsp->Comeback_delay & 0xff00) >> 8;
		//advertisement protocol element
		pbuf[8] = 0x6c;
		pbuf[9] = 2;
		pbuf[10] = 0x20;
		pbuf[11] = 0;//gas_rsp->Advt_proto;
		//gas rsp
		pbuf[12] = gas_rsp->Rsplen & 0x00ff;
		pbuf[13] = (gas_rsp->Rsplen & 0xff00) >> 8;
		if (gas_rsp->Rsplen > 0)
			memcpy(&pbuf[14], gas_rsp->Rsp, gas_rsp->Rsplen);
	
		txinsn.fr_len += 14 +	gas_rsp->Rsplen;	
	}	
	else
	{
		//unknown action
		goto issue_GASrsp_fail;	
	}
	
	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), gas_rsp->MACAddr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	
	pstat = get_stainfo(priv, gas_rsp->MACAddr);
	
#if defined(WIFI_WMM)
	ret = check_dz_mgmt(priv, pstat, &txinsn);
	
	if (ret < 0)
		goto issue_GASrsp_fail;
	else if (ret==1)
		return SUCCESS;
	else
#endif
	{
		if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
			return SUCCESS;	
	}
	
issue_GASrsp_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
		
	return FAIL;	
}

int issue_BSS_TSM_req(struct rtl8192cd_priv *priv, DOT11_HS2_TSM_REQ *tsm_req)
{
	unsigned char	*pbuf;
	struct stat_info *pstat;
#if defined(WIFI_WMM)
	int ret;
#endif	

	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE	
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif
#ifdef CONFIG_IEEE80211W	
	struct stat_info *psta; 
	if(!memcmp(tsm_req->MACAddr,"\xff\xff\xff\xff\xff\xff",6)) {
		txinsn.isPMF = 1; //?????
	} else {
		psta = get_stainfo(priv,tsm_req->MACAddr);
		if(!psta) {
			printk("STA does not exist\n");
			return 0;
		}
		if(psta)
			txinsn.isPMF = psta->isPMF;
		else
			txinsn.isPMF = 0;
	}	
		//printk("deauth:txinsn.isPMF=%d\n",txinsn.isPMF);
#endif
	
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_BSS_TxMgmt_req_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_BSS_TxMgmt_req_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _WNM_CATEGORY_ID_;
	pbuf[1] = _BSS_TSMREQ_ACTION_ID_;
	pbuf[2] = tsm_req->Dialog_token;			//token
	pbuf[3] = tsm_req->Req_mode;				//request mode
	pbuf[4] = tsm_req->Disassoc_timer & 0xff;	//disassociation timer
	pbuf[5] = (tsm_req->Disassoc_timer & 0xff00) >> 8;
	pbuf[6] = tsm_req->Validity_intval;			//validity interval
	if (tsm_req->term_len != 0)
		memcpy(&pbuf[7], tsm_req->terminal_dur, 12); 
	if (tsm_req->url_len != 0)
		memcpy(&pbuf[7+tsm_req->term_len], tsm_req->Session_url, tsm_req->url_len);
	if (tsm_req->list_len != 0)
		memcpy(&pbuf[7+tsm_req->term_len+tsm_req->url_len], tsm_req->Candidate_list, tsm_req->list_len);

	txinsn.fr_len += 7+tsm_req->term_len+tsm_req->url_len+tsm_req->list_len;
	
	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
#ifdef CONFIG_IEEE80211W	
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), tsm_req->MACAddr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	
	pstat = get_stainfo(priv, tsm_req->MACAddr);
	
#if defined(WIFI_WMM)
	ret = check_dz_mgmt(priv, pstat, &txinsn);
	
	if (ret < 0)
		goto issue_BSS_TxMgmt_req_fail;
	else if (ret==1)
		return SUCCESS;
	else
#endif
	{
		if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
			return SUCCESS;	
	}
	
issue_BSS_TxMgmt_req_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
		
	return FAIL;	
}

int issue_DLS_rsp(struct rtl8192cd_priv *priv, unsigned short status, unsigned char *da, unsigned char *dest, unsigned char *src)
{
	unsigned char   *pbuf;
    struct stat_info *pstat;
#if defined(WIFI_WMM)
    int ret;
#endif

    DECLARE_TXINSN(txinsn);

    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;
    txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
    txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
    txinsn.fixed_rate = 1;

    pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
        goto issue_DLS_rsp_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
    if (txinsn.phdr == NULL)
        goto issue_DLS_rsp_fail;

    memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _DLS_CATEGORY_ID_;
    pbuf[1] = _DLS_RSP_ACTION_ID_;
    pbuf[2] = status & 0xff;
    pbuf[3] = (status >> 8) & 0xff;           //status code
	memcpy(&pbuf[4], dest, 6);
	memcpy(&pbuf[10], src, 6);

	txinsn.fr_len += 16;

	pstat = get_stainfo(priv, da);

	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);
    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
    memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

#if defined(WIFI_WMM)
    ret = check_dz_mgmt(priv, pstat, &txinsn);

    if (ret < 0)
        goto issue_DLS_rsp_fail;
    else if (ret==1)
        return SUCCESS;
    else
#endif
    {
        if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
            return SUCCESS;
    }
    
issue_DLS_rsp_fail:
    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);

    return FAIL;
}

#ifdef HS2_CLIENT_TEST
//issue_TSM Query for client test used
int issue_BSS_TSM_query(struct rtl8192cd_priv *priv, unsigned char *list, unsigned char list_len)
{
	unsigned char	*pbuf;
	struct stat_info *pstat;
#if defined(WIFI_WMM)
	int ret;
#endif	
	unsigned char tmpda[]={0x00,0x33,0x44,0x055,0x66,0x77};
	
	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE	
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_BSS_TxMgmt_query_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_BSS_TxMgmt_query_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _WNM_CATEGORY_ID_;
	pbuf[1] = _WNM_TSMQUERY_ACTION_ID_;
	pbuf[2] = 20;			//token
	pbuf[3] = 18;				//request mode

	if (list_len >0 )
		memcpy(&pbuf[4], list, list_len);
	txinsn.fr_len += 4 + list_len;
	
	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	
	pstat = get_stainfo(priv, BSSID);
	
#if defined(WIFI_WMM)
	ret = check_dz_mgmt(priv, pstat, &txinsn);
	
	if (ret < 0)
		goto issue_BSS_TxMgmt_query_fail;
	else if (ret==1)
		return SUCCESS;
	else
#endif
	{
		if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
			return SUCCESS;	
	}
	
issue_BSS_TxMgmt_query_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
		
	return FAIL;	
}

//issue_TSM response for client test used
int issue_BSS_TSM_rsp(struct rtl8192cd_priv *priv, unsigned char *token, unsigned char *list, unsigned char list_len)
{
	unsigned char	*pbuf;
	struct stat_info *pstat;
#if defined(WIFI_WMM)
	int ret;
#endif	
	unsigned char tmpda[]={0x00,0x33,0x44,0x055,0x66,0x77};
	
	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_BSS_TxMgmt_rsp_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_BSS_TxMgmt_rsp_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _WNM_CATEGORY_ID_;
	pbuf[1] = _BSS_TSMRSP_ACTION_ID_;
	pbuf[2] = *token;			//token
	pbuf[3] = 0;				//request mode
	pbuf[4] = 0;

	if (list_len != 0)
		memcpy(&pbuf[5], list, list_len);
		
	txinsn.fr_len += 5+list_len;
	
	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	
	pstat = get_stainfo(priv, BSSID);
	
#if defined(WIFI_WMM)
	ret = check_dz_mgmt(priv, pstat, &txinsn);
	
	if (ret < 0)
		goto issue_BSS_TxMgmt_rsp_fail;
	else if (ret==1)
		return SUCCESS;
	else
#endif
	{
		if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
			return SUCCESS;	
	}
	
issue_BSS_TxMgmt_rsp_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
		
	return FAIL;	
}
#endif
#endif

#if defined(WIFI_WMM)
#ifdef RTK_AC_SUPPORT //for 11ac logo

void issue_op_mode_notify(struct rtl8192cd_priv *priv, struct stat_info *pstat, char mode)
{
	unsigned char	*pbuf;
	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_opm_notification_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_opm_notification_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _VHT_ACTION_CATEGORY_ID_;
	pbuf[1] = _VHT_ACTION_OPMNOTIF_ID_;
	pbuf[2] = mode;
	txinsn.fr_len = _OPMNOTIF_Frame_Length_;

	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);


	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS) {
		return;
	}

issue_opm_notification_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
	return;
}

#endif

/*-------------------------------------------------------------------------------
	Check if packet should be queued
return value:
-1: fail
1: success
0: no queue
--------------------------------------------------------------------------------*/

int check_dz_mgmt(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct tx_insn* txcfg)
{
	if (pstat && ((pstat->state & (WIFI_SLEEP_STATE | WIFI_ASOC_STATE)) ==
			(WIFI_SLEEP_STATE | WIFI_ASOC_STATE))){
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		if (update_txinsn_stage1(priv, txcfg) == FALSE) {
			return -1;
		}
		
		txcfg->next_txpath = TXPATH_FIRETX;
		if (rtw_xmit_enqueue(priv, txcfg) == FALSE) {
			return -1;
		}
#elif defined(CONFIG_PCI_HCI)
		int ret;
		struct tx_insn *ptx_insn;
		ptx_insn = (struct tx_insn*)kmalloc(sizeof(struct tx_insn), GFP_ATOMIC);

		if (ptx_insn == NULL){
			printk("%s: not enough memory\n", __FUNCTION__);
			return -1;
		}
		memcpy((void *)ptx_insn, (void *)txcfg, sizeof(struct tx_insn));
		
		//printk("%s %d\n",__FUNCTION__,__LINE__);
		DEBUG_INFO("h= %d t=%d\n", (pstat->MGT_dz_queue->head), (pstat->MGT_dz_queue->tail));
		ret = enque(priv, &(pstat->MGT_dz_queue->head), &(pstat->MGT_dz_queue->tail),
					(unsigned long)(pstat->MGT_dz_queue->ptx_insn), NUM_DZ_MGT_QUEUE, (void *)ptx_insn);
		
		if (ret == FALSE) {
			kfree(ptx_insn);
			DEBUG_ERR("MGT_dz_queue full!\n");
			return -1;
		}
#endif
		
		return 1; // success
	}else{
		return 0; // no queue
	}
}
#endif

#ifdef CONFIG_IEEE80211W

void stop_sa_query(struct stat_info *pstat)
{
	PMFDEBUG("stop sa query\n");
	pstat->sa_query_count = 0;
	if (timer_pending(&pstat->SA_timer))
		del_timer(&pstat->SA_timer);
}


int check_sa_query_timeout(struct stat_info *pstat)
{
	if(pstat->sa_query_end <= jiffies) {
		PMFDEBUG("sa query time out\n");
		pstat->sa_query_timed_out = 1;
		pstat->sa_query_count = 0;
		if (timer_pending(&pstat->SA_timer))
			del_timer(&pstat->SA_timer);
		return 1;
	}	
	return 0;
}

void rtl8192cd_sa_query_timer(unsigned long task_priv)
{
	struct stat_info        *pstat = (struct stat_info *)task_priv;
	struct rtl8192cd_priv *priv = NULL;
	struct aid_obj *aidobj;

	if(!pstat)
		return ;
	
	aidobj = container_of(pstat, struct aid_obj, station);
	priv = aidobj->priv;

	
	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

    if(pstat->sa_query_count > 0 && check_sa_query_timeout(pstat)) {
		PMFDEBUG("%s(%d)\n", __FUNCTION__, __LINE__);
		return;
    }
	if(pstat->sa_query_count < SA_QUERY_MAX_NUM) {
		PMFDEBUG("send sa query again\n");
		pstat->sa_query_count++;
		issue_SA_Query_Req(priv->dev,pstat->hwaddr);
	}
	mod_timer(&pstat->SA_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(SA_QUERY_RETRY_TO));
}

int issue_SA_Query_Req(struct net_device *dev, unsigned char *da)
{
	#ifdef NETDEV_NO_PRIV
	struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	struct rtl8192cd_priv	*priv = (struct rtl8192cd_priv *)dev->priv;
#endif

	struct stat_info *pstat;
	unsigned char	*pbuf;
	int ret;
	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
	
	txinsn.fixed_rate = 1;
	
	pstat = get_stainfo(priv, da);

	if(pstat)
		txinsn.isPMF = pstat->isPMF;
	else
		txinsn.isPMF = 0;

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_SA_Query_Req_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_SA_Query_Req_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _SA_QUERY_CATEGORY_ID_;
	pbuf[1] = _SA_QUERY_REQ_ACTION_ID_;

	get_random_bytes(&pstat->SA_TID[pstat->sa_query_count], sizeof(unsigned short));	
	
	pbuf[2] = pstat->SA_TID[pstat->sa_query_count] & 0xff;
	pbuf[3] = (pstat->SA_TID[pstat->sa_query_count] & 0xff00) >> 8;
#ifdef CONFIG_IEEE80211W_CLI_DEBUG		
	panic_printk("SA TID=%02x%02x\n", pbuf[2], pbuf[3]);
#endif
	txinsn.fr_len = 4;
	
	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
	
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 	
		
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	
	if (pstat == NULL)
		goto issue_SA_Query_Req_fail;
		panic_printk("%s(%d)\n", __FUNCTION__, __LINE__);
		
	txinsn.pstat = pstat;
	
#if defined(WIFI_WMM)
	ret = check_dz_mgmt(priv, pstat, &txinsn);
		panic_printk("%s(%d)\n", __FUNCTION__, __LINE__);
	if (ret < 0)
		goto issue_SA_Query_Req_fail;
	else if (ret==1) {
		panic_printk("%s(%d)\n", __FUNCTION__, __LINE__);
		return SUCCESS;
	}
	else
#endif
	{
		panic_printk("%s(%d)\n", __FUNCTION__, __LINE__);
		if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS) {
			panic_printk("%s(%d)\n", __FUNCTION__, __LINE__);
			return SUCCESS;	
		}
	}
	
issue_SA_Query_Req_fail:
	panic_printk("%s(%d)\n", __FUNCTION__, __LINE__);

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
		
	return FAIL;
}


int issue_SA_Query_Rsp(struct net_device *dev, unsigned char *da, unsigned char *trans_id)
{
	#ifdef NETDEV_NO_PRIV
	struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	struct rtl8192cd_priv	*priv = (struct rtl8192cd_priv *)dev->priv;
#endif

	struct stat_info *pstat;
	unsigned char	*pbuf;
	int ret;
	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
	
	txinsn.fixed_rate = 1;
	
	pstat = get_stainfo(priv, da);

	if(pstat)
		txinsn.isPMF = pstat->isPMF;
	else
		txinsn.isPMF = 0;

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_SA_Query_Rsp_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_SA_Query_Rsp_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _SA_QUERY_CATEGORY_ID_;
	pbuf[1] = _SA_QUERY_RSP_ACTION_ID_;
	memcpy(pbuf+2,trans_id, 2);
#ifdef CONFIG_IEEE80211W_CLI_DEBUG
	panic_printk("TID= %02x%02x\n", pbuf[2], pbuf[3]); 
#endif

	txinsn.fr_len = 4;
	
	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
	
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 	

	PMFDEBUG("\n");	
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	
	if (pstat == NULL)
		goto issue_SA_Query_Rsp_fail;
		
	txinsn.pstat = pstat;
	
#if defined(WIFI_WMM)
	ret = check_dz_mgmt(priv, pstat, &txinsn);
	if (ret < 0)
		goto issue_SA_Query_Rsp_fail;
	else if (ret==1) {
		return SUCCESS;
	}
	else
#endif
	{
		if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
			return SUCCESS;	
	}
	
issue_SA_Query_Rsp_fail:
	PMFDEBUG("issue_SA_Query_Rsp_fail\n");

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
		
	return FAIL;
}
#endif

int issue_ADDBArsp(struct rtl8192cd_priv *priv, unsigned char *da, unsigned char dialog_token,
				unsigned char TID, unsigned short status_code, unsigned short timeout)
{
	unsigned char	*pbuf;
	unsigned short	ba_para = 0;
	struct stat_info *pstat;
	int max_size;
#if defined(WIFI_WMM)
	int ret;
#endif

	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_ADDBArsp_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_ADDBArsp_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _BLOCK_ACK_CATEGORY_ID_;
	pbuf[1] = _ADDBA_Rsp_ACTION_ID_;
	pbuf[2] = dialog_token;
	pbuf[3] = status_code & 0x00ff;
	pbuf[4] = (status_code & 0xff00) >> 8;

	pstat = get_stainfo(priv, da);
 
#ifdef CONFIG_IEEE80211W	
	 if(pstat)
		 txinsn.isPMF = pstat->isPMF;
	 else
		 txinsn.isPMF = 0;
		 //printk("deauth:txinsn.isPMF=%d\n",txinsn.isPMF);
#endif
	if (pstat && should_restrict_Nrate(priv, pstat))
		max_size = 1;
	else {
#ifdef CONFIG_RTL8196B_GW_8M
	if (pstat->IOTPeer==HT_IOT_PEER_BROADCOM)
		max_size = _ADDBA_Maximum_Buffer_Size_ / 2;
	else
#endif
		max_size = _ADDBA_Maximum_Buffer_Size_;
	}

	ba_para = (max_size<<6) | (TID<<2) | BIT(1);	// assign buffer size | assign TID | set Immediate Block Ack

#if defined(SUPPORT_RX_AMSDU_AMPDU)
	if(AMSDU_ENABLE >= 2)
	ba_para |= BIT(0);
#endif

#ifdef RTK_AC_SUPPORT //for 11ac logo
	if((AC_SIGMA_MODE == AC_SIGMA_APUT) && (AMSDU_ENABLE >= 1))
	ba_para |= BIT(0);
#endif

	pbuf[5] = ba_para & 0x00ff;
	pbuf[6] = (ba_para & 0xff00) >> 8;
	pbuf[7] = timeout & 0x00ff;
	pbuf[8] = (timeout & 0xff00) >> 8;

	txinsn.fr_len += _ADDBA_Rsp_Frame_Length_;

	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

	DEBUG_INFO("ADDBA-rsp sent to AID %d, token %d TID %d size %d status %d\n",
		get_stainfo(priv, da)->aid, dialog_token, TID, max_size, status_code);
#if defined(WIFI_WMM)
	ret = check_dz_mgmt(priv, pstat, &txinsn);
	
	if (ret < 0)
		goto issue_ADDBArsp_fail;
	else if (ret==1)
		return SUCCESS;
	else
#endif
	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return SUCCESS;

issue_ADDBArsp_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
	return FAIL;
}
#endif


#ifdef RTK_WOW
void issue_rtk_wow(struct rtl8192cd_priv *priv, unsigned char *da)
{
	unsigned char	*pbuf;
	unsigned int i;
	DECLARE_TXINSN(txinsn);

	if (!(OPMODE & WIFI_AP_STATE)) {
		DEBUG_WARN("rtk_wake_up pkt should be sent in AP mode!!\n");
		return;
	}

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	
	txinsn.fixed_rate = 1;

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto send_rtk_wake_up_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto send_rtk_wake_up_fail;

	memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

	SetFrameSubType(txinsn.phdr, WIFI_DATA);
	SetFrDs(txinsn.phdr);
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);

	// sync stream
	memset((void *)pbuf, 0xff, MACADDRLEN);
	pbuf += MACADDRLEN;
	txinsn.fr_len += MACADDRLEN;

	for(i=0; i<16; i++) {
		memcpy((void *)pbuf, da, MACADDRLEN);
		pbuf += MACADDRLEN;
		txinsn.fr_len += MACADDRLEN;
	}

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS) {
		DEBUG_INFO("RTK wake up pkt sent\n");
		return;
	}
	else {
		DEBUG_ERR("Fail to send RTK wake up pkt\n");
	}

send_rtk_wake_up_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}
#endif


#ifdef CONFIG_RTL_92D_SUPPORT
int clnt_ss_check_band(struct rtl8192cd_priv *priv, unsigned int channel)
{
#ifdef CLIENT_MODE
	if (OPMODE & (WIFI_STATION_STATE|WIFI_ADHOC_STATE)) {
		if (priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY) {
			if (channel > 14 && priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G){
				//printk("change to 5G %d\n", channel);
				// stop BB
				PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
				priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;
				priv->pshare->iqk_5g_done = 0;
				//priv->pmib->dot11BssType.net_work_type = (WIRELESS_11A|WIRELESS_11N);
				UpdateBBRFVal8192DE(priv);
				PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);
				return 1;
			}
			
			if (channel <= 14 && priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G){
				//printk("change to 2G %d\n", channel);
				PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
				priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
				priv->pshare->iqk_2g_done = 0;
				//priv->pmib->dot11BssType.net_work_type = (WIRELESS_11B|WIRELESS_11G|WIRELESS_11N);
				//PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);
				UpdateBBRFVal8192DE(priv);
				PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);
				return 1;
			}
		}
	}
#endif
	return 0;
}
#endif


/**
 *	@brief	Process Site Survey
 *
 *	set site survery, reauth. , reassoc, idle_timer and proces Site survey \n
 *	PS: ss_timer is site survey timer	\n
 */
void start_clnt_ss(struct rtl8192cd_priv *priv)
{

#if !defined(SMP_SYNC) || defined(CONFIG_RTK_MESH) 
	unsigned long	flags;
#endif

#ifdef SUPPORT_MULTI_PROFILE
	int j;
#endif

/*cfg p2p cfg p2p*/
#if defined(CONFIG_P2P_RTK_SUPPORT) && defined(RTK_NL80211)
	/*cfg p2p wpas support*/
    u8 tx_pause_val;

	if (!(priv->drv_state & DRV_STATE_OPEN)){
        NDEBUG("NOT STARTED YET!\n");        
		return;
    }

    if(priv->pshare->rtk_remain_on_channel){
        NDEBUG3("deny scan by rtk_remain_on_channel\n");
        return;
    }

    if(priv->pshare->deny_scan_myself){
        NDEBUG3("deny scan by deny_scan_myself\n");
        return;
    }    

    if(priv->pmib->miscEntry.func_off){
        NDEBUG3("chk!!!func_off on\n");
    }

    tx_pause_val=RTL_R8(TXPAUSE);
    if(tx_pause_val){
        NDEBUG3("!!!tx_pause_val[%X]\n",tx_pause_val);
    }    
#endif
/*cfg p2p cfg p2p*/
	if (timer_pending(&priv->ss_timer))
		del_timer(&priv->ss_timer);
#ifdef CLIENT_MODE
	if (PENDING_REAUTH_TIMER)
		DELETE_REAUTH_TIMER;
	if (PENDING_REASSOC_TIMER)
		DELETE_REASSOC_TIMER;
	if (timer_pending(&priv->idle_timer))
		del_timer(&priv->idle_timer);
#endif

#ifdef P2P_SUPPORT
/*cfg p2p cfg p2p*/
	if( rtk_p2p_is_enabled(priv) && (rtk_p2p_chk_role(priv,P2P_DEVICE))&&(rtk_p2p_chk_state(priv,P2P_S_LISTEN))){
		NDEBUG("p2p device listen mode don't SS!!\n");
		return;
	}
#endif					

#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
	if(IS_OUTSRC_CHIP(priv))
#endif
	priv->pshare->bScanInProcess = TRUE;
#endif

	OPMODE_VAL(OPMODE & (~WIFI_SITE_MONITOR));

	SAVE_INT_AND_CLI(flags);
#ifdef SMART_REPEATER_MODE
	if (priv->ss_req_ongoing == SSFROM_REPEATER_VXD){
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
		if(priv->pmib->dot11StationConfigEntry.sc_enabled == 0){
			if(get_ss_level(priv) > SS_LV_WSTA){
				priv->site_survey_times = SS_COUNT-2;
			}else {
				priv->site_survey_times = SS_COUNT-1;
			}
		}else{
			priv->site_survey_times = 0;
		}
#else
		if(get_ss_level(priv) > SS_LV_WSTA){
			priv->site_survey_times = SS_COUNT-2;
		}
		else{
			priv->site_survey_times = SS_COUNT-1;
			}
#endif	
	} else
#endif
	{
#if defined(RTK_NL80211)
		if(rtk_p2p_is_enabled(priv) && priv->ss_ssidlen){
            priv->site_survey_times=0;
        }else{
			//brian, only scan all channels one round, because Hostapd will scan 5 times by default
    		priv->site_survey_times = SS_COUNT-2;        
        }
#else
		priv->site_survey_times = 0;			//normal case ss SS_COUNT times
#endif
	}


// mark by david ---------------
#if 0
	if (priv->pmib->dot11RFEntry.dot11ch_low != 0)
		priv->site_survey->ss_channel = priv->pmib->dot11RFEntry.dot11ch_low;
	else
#endif
//--------------------2007-04-14


#if 0 //defined(CONFIG_RTL_92D_SUPPORT) && !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
	if ((GET_CHIP_VER(priv) == VERSION_8192D) && priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY &&
		priv->auto_channel!=1) {
		//printk(">>>>>>>>>%s\n",__FUNCTION__);
		
		priv->site_survey->bk_nwtype = priv->pmib->dot11BssType.net_work_type; //backup network type
		priv->pmib->dot11BssType.net_work_type = (WIRELESS_11B|WIRELESS_11G|WIRELESS_11A|WIRELESS_11N);
		if (!get_available_channel(priv)) {
			DEBUG_ERR("(%s) Fail to get available channel\n", __FUNCTION__);
			priv->pmib->dot11BssType.net_work_type = priv->site_survey->bk_nwtype;
			return;
		}
	} 
#endif

	priv->site_survey->ss_channel = priv->available_chnl[0];
	priv->site_survey->count = 0;
	priv->site_survey->hidden_ap_found = 0;
	
#if defined(CONFIG_RTL_92D_SUPPORT) && !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
	if ((GET_CHIP_VER(priv) == VERSION_8192D) && priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY) {
		clnt_ss_check_band(priv, priv->site_survey->ss_channel);
	}
#endif
//let vxd can do ss under 5G
#if	0	//def UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv) ||
		(IS_VXD_INTERFACE(priv) && priv->pmib->wscEntry.wsc_enable))
#endif

	{
#ifdef DFS
		if (!priv->pmib->dot11DFSEntry.disable_DFS && is_DFS_channel(priv->site_survey->ss_channel))
			priv->pmib->dot11DFSEntry.disable_tx = 1;
		else
			priv->pmib->dot11DFSEntry.disable_tx = 0;
#endif

		if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_10)
			priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_10;
		else if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_5)
			priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_5;
		else
			priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
		if(!rtk_sc_is_channel_fixed(priv))
#endif
		{
			SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
			SwChnl(priv, priv->site_survey->ss_channel, priv->pshare->offset_2nd_chan);
		}

#if defined(RTK_NL80211)
		//by brian, trigger channel load evaluation after channel switched
		start_bbp_ch_load(priv, (45*1000)/4);
#endif
		
#if defined(CONFIG_RTL_92D_SUPPORT) && !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
		if ((GET_CHIP_VER(priv) == VERSION_8192D) && (priv->pmib->dot11RFEntry.macPhyMode == SINGLEMAC_SINGLEPHY)) 
			PHY_IQCalibrate(priv);
#endif

#ifdef UNIVERSAL_REPEATER
		if (IS_VXD_INTERFACE(priv) && priv->pmib->wscEntry.wsc_enable)
			GET_ROOT(priv)->pmib->miscEntry.func_off = 1;
#endif
	}

	memset((void *)priv->site_survey->bss, 0, sizeof(struct bss_desc)*MAX_BSS_NUM);
#ifdef WIFI_SIMPLE_CONFIG
	if (priv->ss_req_ongoing == 2)
		memset((void *)priv->site_survey->wscie, 0, sizeof(struct wps_ie_info)*MAX_BSS_NUM);
#endif

#if defined(WIFI_WPAS) || defined(RTK_NL80211)
    	if (priv->ss_req_ongoing == 2)
    		memset((void *)&priv->site_survey->wpa_ie, 0, sizeof(struct wpa_ie_info)*MAX_BSS_NUM);
    	if (priv->ss_req_ongoing == 2)
    		memset((void *)&priv->site_survey->rsn_ie, 0, sizeof(struct rsn_ie_info)*MAX_BSS_NUM);
    	if (priv->ss_req_ongoing == 2)
    		memset((void *)&priv->site_survey->rtk_p2p_ie, 0, sizeof(struct p2p_ie_info)*MAX_BSS_NUM);
#endif 

#if defined(TESTCHIP_SUPPORT) && defined(CONFIG_RTL_92C_SUPPORT)
	if(IS_TEST_CHIP(priv))
		RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) | DIS_TSF_UPDATE);
	else
#endif
		RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) | DIS_TSF_UPDATE_N);

#if defined(CLIENT_MODE)
#if defined(TESTCHIP_SUPPORT) && defined(CONFIG_RTL_92C_SUPPORT)
	if( IS_TEST_CHIP(priv) ) {		
		if ((OPMODE & WIFI_STATION_STATE) || (OPMODE & WIFI_ADHOC_STATE))
			RTL_W32(RCR, RTL_R32(RCR) & ~RCR_CBSSID);
	} else
#endif
	{
        /*	
		if (OPMODE & WIFI_STATION_STATE)
			RTL_W32(RCR, RTL_R32(RCR) & ~RCR_CBSSID);
		else 
              */
		if((OPMODE & WIFI_ADHOC_STATE))
			RTL_W32(RCR, RTL_R32(RCR) & ~RCR_CBSSID_ADHOC);
	}
#endif

	DIG_for_site_survey(priv, TRUE);
#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		if (priv->pshare->DNC_on){
			PHY_SetBBReg(priv, 0x870, bMaskDWord, 0x07000700);
		}
	}
#endif	
	OPMODE_VAL(OPMODE | WIFI_SITE_MONITOR);
	RESTORE_INT(flags);
#ifdef CONFIG_RTL_NEW_AUTOCH
	if (priv->auto_channel == 1) {
		reset_FA_reg(priv);

		if (OPMODE & WIFI_AP_STATE)
			RTL_W32(RXERR_RPT, RXERR_RPT_RST);
	}
#endif



            
    if( (priv->site_survey->hidden_ap_found == HIDE_AP_FOUND_DO_ACTIVE_SSAN ) ||
		!is_passive_channel(priv, priv->pmib->dot11StationConfigEntry.dot11RegDomain, priv->site_survey->ss_channel))            
	{
	
        #ifdef CONFIG_RTK_MESH
        //  GANTOE for site survey 2008/12/25 ====
		if(GET_MIB(priv)->dot1180211sInfo.mesh_enable)
			issue_probereq_MP(priv, NULL, 0, NULL, TRUE);
		else
        #endif
		{
		    /*no assigned SSID*/
			if (priv->ss_ssidlen == 0){

                #ifdef P2P_SUPPORT						
				/*cfg p2p cfg p2p*/				
					if( rtk_p2p_is_enabled(priv) && ((rtk_p2p_chk_role(priv,P2P_DEVICE))||(rtk_p2p_chk_role(priv,P2P_CLIENT))) ){
   						P2P_DEBUG("p2p scan (chann:%d)\n",priv->site_survey->ss_channel);
    					issue_probereq(priv, "DIRECT-", 7, NULL);
				}else
                #endif
				{
					//STADEBUG("issue_probereq no assigned SSID\n");	
					issue_probereq(priv, NULL, 0, NULL);
				}
		
			}else{   /*has assigned SSID*/
                #ifdef SUPPORT_MULTI_PROFILE	/*per channel tx multi probe_req by profile_num*/ 		
				if (priv->pmib->ap_profile.enable_profile && priv->pmib->ap_profile.profile_num > 0) {
					for(j=0;j<priv->pmib->ap_profile.profile_num;j++) {									
						NDEBUG3("issue_probereq,ssid[%s],ch=[%d]\n",priv->pmib->ap_profile.profile[j].ssid,priv->site_survey->ss_channel);
						issue_probereq(priv, priv->pmib->ap_profile.profile[j].ssid, strlen(priv->pmib->ap_profile.profile[j].ssid), NULL);
					}												
				}
				else	
                #endif		
				{						
					NDEBUG3("issue_probereq,ssid[%s],ch[%d]\n",priv->ss_ssid,priv->site_survey->ss_channel);
					issue_probereq(priv, priv->ss_ssid, priv->ss_ssidlen, NULL);
				}
                
			}
		}
	}


#ifdef CONFIG_RTK_MESH
	//GANTOE for site survey 2008/12/25 ====
	if(priv->auto_channel & 0x30)
	{
		SET_PSEUDO_RANDOM_NUMBER(flags);
		flags %= SS_RAND_DEFER;
	} else
		flags=0;
#endif

	    /*how long stady on current channel -start*/

	//if (should_passive_scan(priv))// replaced by is_passive_channel
	if(is_passive_channel(priv, priv->pmib->dot11StationConfigEntry.dot11RegDomain, priv->site_survey->ss_channel))        
	{
		if(priv->pmib->miscEntry.passive_ss_int) {
			mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pmib->miscEntry.passive_ss_int)
			#ifdef CONFIG_RTK_MESH //GANTOE for site survey 2008/12/25
				+ ( flags ) // for the deafness problem
			#endif
			);
		} else {
		mod_timer(&priv->ss_timer, jiffies + SS_PSSV_TO
		#ifdef CONFIG_RTK_MESH //GANTOE for site survey 2008/12/25
			+ ( flags ) // for the deafness problem
		#endif
		);
		}
	}else
#ifdef P2P_SUPPORT
/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
		/*search phase (only 1,6,11) use 120ms*/
		//P2P_DEBUG("%dms\n",P2P_SEARCH_TIME_V);
        mod_timer(&priv->ss_timer, jiffies + P2P_SEARCH_TIME);
	}else
#endif
	{
#ifdef CONFIG_RTL_NEW_AUTOCH
	if (priv->auto_channel == 1){
		mod_timer(&priv->ss_timer, jiffies + SS_AUTO_CHNL_TO
#ifdef CONFIG_RTK_MESH 		//GANTOE for site survey 2008/12/25
			+ ( flags ) // for the deafness problem
#endif
		);
	}else
#endif
	{
		mod_timer(&priv->ss_timer, jiffies + SS_TO
		#ifdef CONFIG_RTK_MESH 		//GANTOE for site survey 2008/12/25
			+ ( flags ) // for the deafness problem
		#endif
		);
	}
	
	}
    /*how long stady on current channel -end*/

    
}

static void ProfileSort (struct rtl8192cd_priv *priv, int CompareType , void  *base, int nel, int width)
{
	int wgap, i, j, k;
	unsigned char tmp;

	if ((nel > 1) && (width > 0)) {
		//assert( nel <= ((size_t)(-1)) / width ); /* check for overflow */
		wgap = 0;
		do {
			wgap = 3 * wgap + 1;
		} while (wgap < (nel-1)/3);

        /* From the above, we know that either wgap == 1 < nel or */
		/* ((wgap-1)/3 < (int) ((nel-1)/3) <= (nel-1)/3 ==> wgap <  nel. */
        
		wgap *= width;			/* So this can not overflow if wnel doesn't. */
		nel *= width;			/* Convert nel to 'wnel' */
		do {
			i = wgap;
			do {
				j = i;
				do {
					register unsigned char *a;
					register unsigned char *b;

					j -= wgap;
					a = (unsigned char *)(j + ((char *)base));
					b = a + wgap;
					if ( compareTpyeByProfile(priv,a, b,CompareType) <= 0 ) {
						break;
					}
					k = width;
					do {
						tmp = *a;
						*a++ = *b;
						*b++ = tmp;
					} while ( --k );
				} while (j >= wgap);
				i += width;
			} while (i < nel);
			wgap = (wgap - width)/3;
		} while (wgap);
	}
}


static void qsort (void  *base, int nel, int width,
				int (*comp)(const void *, const void *))
{
	int wgap, i, j, k;
	unsigned char tmp;

	if ((nel > 1) && (width > 0)) {
		//assert( nel <= ((size_t)(-1)) / width ); /* check for overflow */
		wgap = 0;
		do {
			wgap = 3 * wgap + 1;
		} while (wgap < (nel-1)/3);
		/* From the above, we know that either wgap == 1 < nel or */
		/* ((wgap-1)/3 < (int) ((nel-1)/3) <= (nel-1)/3 ==> wgap <  nel. */
		wgap *= width;			/* So this can not overflow if wnel doesn't. */
		nel *= width;			/* Convert nel to 'wnel' */
		do {
			i = wgap;
			do {
				j = i;
				do {
					register unsigned char *a;
					register unsigned char *b;

					j -= wgap;
					a = (unsigned char *)(j + ((char *)base));
					b = a + wgap;
					if ( (*comp)(a, b) <= 0 ) {
						break;
					}
					k = width;
					do {
						tmp = *a;
						*a++ = *b;
						*b++ = tmp;
					} while ( --k );
				} while (j >= wgap);
				i += width;
			} while (i < nel);
			wgap = (wgap - width)/3;
		} while (wgap);
	}
}


static int get_profile_index(struct rtl8192cd_priv *priv ,char* SSID2Search)
{
     int idx=0;
     int len1=0;
     int len2=0;     
     len1 = strlen(SSID2Search);

    
     for(idx=0 ; idx < priv->pmib->ap_profile.profile_num ; idx++){
         len2 = strlen(priv->pmib->ap_profile.profile[idx].ssid);
         if(len1==len2){
            if(!strcmp(priv->pmib->ap_profile.profile[idx].ssid,SSID2Search)){
                return idx;                
            }
         }
     }
     return -1;

}
static int compareTpyeByProfile(struct rtl8192cd_priv *priv , const void *entry1, const void *entry2 , int CompareType)
{
    int result1=0;
    int result2=0;
    switch(CompareType){
        case COMPARE_BSS:               
                result1=get_profile_index(priv,((struct bss_desc *)entry1)->ssid);
                result2=get_profile_index(priv,((struct bss_desc *)entry2)->ssid);
                break;
        case COMPARE_WSCIE:
                result1=get_profile_index(priv,((struct wps_ie_info *)entry1)->ssid);
                result2=get_profile_index(priv,((struct wps_ie_info *)entry2)->ssid);
                break;
#ifdef WIFI_WPAS                
        case COMPARE_WPAIE:
                result1=get_profile_index(priv,((struct wpa_ie_info *)entry1)->ssid);
                result2=get_profile_index(priv,((struct wpa_ie_info *)entry2)->ssid);
                break;  
        case COMPARE_RSNIE:
                result1=get_profile_index(priv,((struct rsn_ie_info *)entry1)->ssid);
                result2=get_profile_index(priv,((struct rsn_ie_info *)entry2)->ssid);
                break;
#endif                
        default:
            STADEBUG("unknow, check!!!\n\n");
    }

	/*result more small then list at more front*/
    if (  result1 < result2 )
        return -1;

    if (  result1 > result2 )
        return 1;

    return 0; 

}

static int compareBSS(const void *entry1, const void *entry2)
{
	if (((struct bss_desc *)entry1)->rssi > ((struct bss_desc *)entry2)->rssi)
		return -1;

	if (((struct bss_desc *)entry1)->rssi < ((struct bss_desc *)entry2)->rssi)
		return 1;

	return 0;
}


#ifdef WIFI_SIMPLE_CONFIG
static int compareWpsIE(const void *entry1, const void *entry2)
{
	if (((struct wps_ie_info *)entry1)->rssi > ((struct wps_ie_info *)entry2)->rssi)
		return -1;

	if (((struct wps_ie_info *)entry1)->rssi < ((struct wps_ie_info *)entry2)->rssi)
		return 1;

	return 0;
}
#endif

#if defined(WIFI_WPAS) || defined(RTK_NL80211)
static int compareWpaIE(const void *entry1, const void *entry2)
{
	if (((struct wpa_ie_info *)entry1)->rssi > ((struct wpa_ie_info *)entry2)->rssi)
		return -1;

	if (((struct wpa_ie_info *)entry1)->rssi < ((struct wpa_ie_info *)entry2)->rssi)
		return 1;

	return 0;
}

static int compareRsnIE(const void *entry1, const void *entry2)
{
	if (((struct rsn_ie_info *)entry1)->rssi > ((struct rsn_ie_info *)entry2)->rssi)
		return -1;

	if (((struct rsn_ie_info *)entry1)->rssi < ((struct rsn_ie_info *)entry2)->rssi)
		return 1;

	return 0;
}
#endif

static void debug_print_bss(struct rtl8192cd_priv *priv)
{
	STADEBUG("Got ssid count %d\n", priv->site_survey->count);
#if 0
	int i;

	panic_printk("Got ssid count %d\n", priv->site_survey->count);
	panic_printk("SSID                 BSSID        ch  prd cap  bsc  oper ss sq bd 40m\n");
	for(i=0; i<priv->site_survey->count; i++)
	{
		char tmpbuf[33];
		UINT8 *mac = priv->site_survey->bss[i].bssid;

		memcpy(tmpbuf, priv->site_survey->bss[i].ssid, priv->site_survey->bss[i].ssidlen);
		if (priv->site_survey->bss[i].ssidlen < 20) {
			memset(tmpbuf+priv->site_survey->bss[i].ssidlen, ' ', 20-priv->site_survey->bss[i].ssidlen);
			tmpbuf[20] = '\0';
		}
		else
			tmpbuf[priv->site_survey->bss[i].ssidlen] = '\0';

		panic_printk("%s %02x%02x%02x%02x%02x%02x %2d %4d %04x %04x %04x %02x %02x %02x %3d\n",
			tmpbuf,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],priv->site_survey->bss[i].channel,
			priv->site_survey->bss[i].beacon_prd,priv->site_survey->bss[i].capability,
			(unsigned short)priv->site_survey->bss[i].basicrate,
			(unsigned short)priv->site_survey->bss[i].supportrate,
			priv->site_survey->bss[i].rssi,priv->site_survey->bss[i].sq,
			priv->site_survey->bss[i].network,
			((priv->site_survey->bss[i].t_stamp[1] & BIT(1)) ? 1 : 0)
			);
	}
    panic_printk("\n\n");
#endif
}


void rtl8192cd_ss_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	int idx, loop_finish=0;
	int AUTOCH_SS_COUNT=SS_COUNT;//AUTOCH_SS_SPEEDUP
	int i;

#ifdef SUPPORT_MULTI_PROFILE
	int jdx;
#endif
#if defined(CONFIG_P2P_RTK_SUPPORT) && defined(RTK_NL80211)
    u8 tx_pause_val;
#endif
	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

#if (defined( UNIVERSAL_REPEATER) || defined(MBSSID)) && defined(CLIENT_MODE)
	if(!priv->ss_req_ongoing) {
		if(!IS_ROOT_INTERFACE(priv)){
			start_repeater_ss(priv); // site survey by vxd or vap
    		return;
        }
	}
#endif    
/*cfg p2p cfg p2p*/
#if defined(CONFIG_P2P_RTK_SUPPORT) && defined(RTK_NL80211)

	if (!(priv->drv_state & DRV_STATE_OPEN)){
        NDEBUG3("abort ss_timer interface not opened yet\n");        
		return;
    }
    if(priv->pshare->deny_scan_myself){
        NDEBUG3("abort ss_timer by deny_scan_myself\n");
        goto abort_scan;
    }
    if(priv->pshare->rtk_remain_on_channel){
        NDEBUG3("abort ss_timer by rtk_remain_on_channel\n");
        goto abort_scan;

    }

    if( rtk_p2p_is_enabled(priv) && (rtk_p2p_chk_role(priv,P2P_DEVICE))&&(rtk_p2p_chk_state(priv,P2P_S_LISTEN))){
        NDEBUG3("stop ss_timer when P2P_S_LISTEN\n");
        goto abort_scan;        
    }


    if(priv->pmib->miscEntry.func_off){
        NDEBUG3("chk!!!func_off on\n");
    } 
    if(tx_pause_val=RTL_R8(TXPAUSE)){
        NDEBUG3("tx_pause_val[%X]\n",tx_pause_val);
    }
#endif   
/*cfg p2p cfg p2p*/
#ifdef CONFIG_RTK_MESH
    if(GET_MIB(priv)->dot1180211sInfo.mesh_enable && priv->auto_channel) {
        if(priv->mesh_swchnl_channel) {/* force abort mesh auto channel procedure*/
            priv->auto_channel = 1;
            goto skip_40M_ss;
        }            
    }
#endif

	STADEBUG("rtl8192cd_ss_timer,ss_channel=%d\n",priv->site_survey->ss_channel);
	
	SMP_LOCK(flags);

#ifdef MULTI_MAC_CLONE
	ACTIVE_ID = 0;
#endif

	for (idx=0; idx<priv->available_chnl_num; idx++)
		if (priv->site_survey->ss_channel == priv->available_chnl[idx])
			break;

#if defined(RTK_NL80211)
	//by brian, collect channel statistic here, move from 8192cd_expire_timer
	{
		priv->rtk->survey_info[idx].channel = priv->site_survey->ss_channel;
		if((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8812E)) {
			int val = read_bbp_ch_load(priv);
			if(val != -1) {
				priv->rtk->survey_info[idx].chbusytime = (val/1000)*5;
			} else {
				NDEBUG("Invalid channel load!\n");
			}
		} else {
			NDEBUG("%s %d Chip:%x not support chanel load!\n",__func__,__LINE__,GET_CHIP_VER(priv));
		}

		check_sta_throughput(priv, idx);
		read_snr_report(priv, idx);
	}
#endif

#ifdef CONFIG_RTL_NEW_AUTOCH
#ifdef AUTOCH_SS_SPEEDUP
	if (priv->auto_channel == 1)
		if(priv->pmib->miscEntry.autoch_ss_cnt>0)
			AUTOCH_SS_COUNT = priv->pmib->miscEntry.autoch_ss_cnt;
#endif
	if (priv->auto_channel == 1) {
		unsigned int ofdm_ok, cck_ok, ht_ok;

		if (!priv->site_survey->to_scan_40M && !priv->auto_channel_step) {
#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
			if (IS_OUTSRC_CHIP(priv))//outsrc chip use odemFAStatistic
#endif
			{
				odm_FalseAlarmCounterStatistics(ODMPTR);
				priv->chnl_ss_fa_count[idx] = ODMPTR->FalseAlmCnt.Cnt_all;
				priv->chnl_ss_cca_count[idx] = ODMPTR->FalseAlmCnt.Cnt_CCA_all;		
			} 
#endif // USE_OUT_SRC
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
			if (!IS_OUTSRC_CHIP(priv))
#endif
			{	
				hold_CCA_FA_counter(priv);
				_FA_statistic(priv);
				priv->chnl_ss_fa_count[idx] = priv->pshare->FA_total_cnt;
				priv->chnl_ss_cca_count[idx] = ((RTL_R8(0xa60)<<8)|RTL_R8(0xa61)) + RTL_R16(0xda0);

				release_CCA_FA_counter(priv);
			}
 #endif // !USE_OUT_SRC || _OUTSRC_COEXIST
		}

		RTL_W32(RXERR_RPT, 0 << RXERR_RPT_SEL_SHIFT);
		ofdm_ok = RTL_R16(RXERR_RPT);

		RTL_W32(RXERR_RPT, 3 << RXERR_RPT_SEL_SHIFT);
		cck_ok = RTL_R16(RXERR_RPT);

		RTL_W32(RXERR_RPT, 6 << RXERR_RPT_SEL_SHIFT);
		ht_ok = RTL_R16(RXERR_RPT);

		RTL_W32(RXERR_RPT, RXERR_RPT_RST);

		if (priv->site_survey->to_scan_40M) {
			unsigned int z=0, ch_begin=0, ch_end=priv->available_chnl_num, 
				current_ch = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x18, 0xff, 1);
			int idx_2G_end=-1;
#if defined(RTK_5G_SUPPORT) 
			int idx_5G_begin=-1;
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
				for (z=0; z<priv->available_chnl_num; z++) {
					if (priv->available_chnl[z] > 14) {
						idx_5G_begin = z;
						break;
					}
				}
				if (idx_5G_begin >= 0) 
					ch_begin = idx_5G_begin;

				for (z=ch_begin; z < ch_end; z++) {
					if ((priv->available_chnl[z] == (current_ch+2)) || (priv->available_chnl[z] == (current_ch-2))) {
						priv->chnl_ss_mac_rx_count_40M[z] = ofdm_ok + cck_ok + ht_ok;
					}
				}
			} else
#endif
			{
				for (z=0; z<priv->available_chnl_num; z++) {
					if (priv->available_chnl[z] <= 14)
						idx_2G_end = z;
					else
						break;
				}
				if (idx_2G_end >= 0) 
					ch_end = idx_2G_end+1;

				for (z=ch_begin; z < ch_end; z++) {
					if (priv->available_chnl[z] == current_ch) {
						priv->chnl_ss_mac_rx_count_40M[z] = ofdm_ok + cck_ok + ht_ok;
						break;
					}
				}
			}
		} else {
			priv->chnl_ss_mac_rx_count[idx] = ofdm_ok + cck_ok + ht_ok;
		}

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
		if (!priv->site_survey->to_scan_40M) {
			unsigned long val32 = 0;
			if ((GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8192E)
				&& priv->pmib->dot11RFEntry.acs_type && priv->auto_channel_step) {
				for (i=0; i<20; i++) {
					mdelay(1);
					if (RTL_R8(0x8b6) & BIT0)
						break;
				}
			
				if (priv->auto_channel_step == 1) {
					val32 = RTL_R32(0x8dc);
					priv->nhm_cnt[idx][9] = (val32 & 0x0000ff00) >> 8;
					priv->nhm_cnt[idx][8] = (val32 & 0x000000ff);
					val32 = RTL_R32(0x8d8);
					priv->nhm_cnt[idx][7] = (val32 & 0xff000000) >> 24;
					priv->nhm_cnt[idx][6] = (val32 & 0x00ff0000) >> 16;
					priv->nhm_cnt[idx][5] = (val32 & 0x0000ff00) >> 8;
				} else if (priv->auto_channel_step == 2) {
					val32 = RTL_R32(0x8d8);
					priv->nhm_cnt[idx][4] = RTL_R8(0x8dc);
					priv->nhm_cnt[idx][3] = (val32 & 0xff000000) >> 24;
					priv->nhm_cnt[idx][2] = (val32 & 0x00ff0000) >> 16;
					priv->nhm_cnt[idx][1] = (val32 & 0x0000ff00) >> 8;
					priv->nhm_cnt[idx][0] = (val32 & 0x000000ff);
				}
			}
		}
#endif
	}
#endif

	if (idx == (priv->available_chnl_num - 1) &&
		 priv->site_survey->hidden_ap_found != HIDE_AP_FOUND) {
				loop_finish = 1;						
	}
	else {
// mark by david ------------------------
#if 0
		if ((priv->pmib->dot11RFEntry.dot11ch_hi != 0) &&
			(priv->site_survey->ss_channel >= priv->pmib->dot11RFEntry.dot11ch_hi))
			loop_finish = 1;
		else
#endif
//--------------------------- 2007-04-14
		if (priv->site_survey->hidden_ap_found != HIDE_AP_FOUND) {			
#if defined(DFS)
			if(priv->site_survey->defered_ss) {
				priv->site_survey->defered_ss--;
			} else
#endif
			{
				priv->site_survey->ss_channel = priv->available_chnl[idx+1];
#if defined(DFS)
				priv->site_survey->defered_ss = should_defer_ss(priv);
#endif
			}

			if(priv->pmib->dot11RFEntry.disable_scan_ch14 && (priv->site_survey->ss_channel==14))
				loop_finish = 1;

			priv->site_survey->hidden_ap_found = 0;
		}
		else{ 
			STADEBUG("HIDE_AP_FOUND_DO_ACTIVE_SSAN\n"); 
			priv->site_survey->hidden_ap_found = HIDE_AP_FOUND_DO_ACTIVE_SSAN;
        }
#ifdef CONFIG_RTL_NEW_AUTOCH
		if ((priv->auto_channel == 1) && priv->site_survey->to_scan_40M) {
#if defined(RTK_5G_SUPPORT) 
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
				unsigned int current_ch = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x18, 0xff, 1);

				if (((priv->site_survey->ss_channel+2) == current_ch) || ((priv->site_survey->ss_channel-2) == current_ch)) {
					if ((idx+2) >= (priv->available_chnl_num - 1))
						loop_finish = 1;
					else
						priv->site_survey->ss_channel = priv->available_chnl[idx+2];
				}
			} else
#endif
			{
				if (priv->site_survey->ss_channel == 14)
					loop_finish = 1;
			}
		}
#endif
	}

	if (loop_finish) {
		priv->site_survey_times++;		
#ifdef SIMPLE_CH_UNI_PROTOCOL
		if(GET_MIB(priv)->dot1180211sInfo.mesh_enable && (priv->auto_channel & 0x30) )
		{
			if( priv->auto_channel == 0x10 )
			{
				if( priv->site_survey_times >= _11S_SS_COUNT1 ){
					if(priv->mesh_ChannelPrecedence == 0)/*not yet select channel or recieved others channel number*/
					{
						priv->pmib->dot11RFEntry.dot11channel = selectClearChannel(priv);
						SET_PSEUDO_RANDOM_NUMBER(priv->mesh_ChannelPrecedence);
					}
					priv->auto_channel = 0x20;
				}
			}
			else
			{
				if( priv->site_survey_times >= 	_11S_SS_COUNT1+_11S_SS_COUNT2)
					priv->auto_channel = 1;
			}
            if(priv->auto_channel == 1) {
                get_available_channel(priv);
            }
            else {
    			for(i=0; i<priv->available_chnl_num; i++)
    			{
    				get_random_bytes(&(idx), sizeof(idx));
    				idx %= priv->available_chnl_num;
    				loop_finish = priv->available_chnl[idx];
    				priv->available_chnl[idx] = priv->available_chnl[i];
    				priv->available_chnl[i] = loop_finish;
    			}
            }
			priv->site_survey->ss_channel = priv->available_chnl[0];
		}
		else
#endif

// only do multiple scan when site-survey request, david+2006-01-25
//		if (priv->site_survey_times < SS_COUNT)
		if (priv->ss_req_ongoing && priv->site_survey_times < AUTOCH_SS_COUNT) {//AUTOCH_SS_SPEEDUP
// mark by david ---------------------
            #if 0   // mark by david ---------------------
			// scan again
			if (priv->pmib->dot11RFEntry.dot11ch_low != 0)
				priv->site_survey->ss_channel = priv->pmib->dot11RFEntry.dot11ch_low;
			else
            #endif  //------------------------ 2007-04-14
			
			//if scan 40M, start channel from start_ch_40M
			if(priv->site_survey->to_scan_40M && priv->site_survey->start_ch_40M!=0)
				priv->site_survey->ss_channel = priv->site_survey->start_ch_40M;
			else          
				priv->site_survey->ss_channel = priv->available_chnl[0];

			if(priv->pmib->miscEntry.ss_loop_delay) {
				//STADEBUG("loop delay for %d miliseconads\n",priv->pmib->miscEntry.ss_loop_delay);
				if(timer_pending(&priv->ss_timer))
					del_timer(&priv->ss_timer);
				mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pmib->miscEntry.ss_loop_delay));

				if(priv->ss_req_ongoing == SSFROM_REPEATER_VXD) {
					//STADEBUG("RollBack to ROOT's ch[%d] between loop\n",GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel);
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
					if(!rtk_sc_is_channel_fixed(priv))
#endif
					{
						SwBWMode(GET_ROOT(priv), GET_ROOT(priv)->pshare->CurrentChannelBW, GET_ROOT(priv)->pshare->offset_2nd_chan);
						SwChnl(GET_ROOT(priv), GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel, GET_ROOT(priv)->pshare->offset_2nd_chan);
					}
#if defined(DFS)
					priv->pmib->dot11DFSEntry.disable_tx = 0;
#endif
				}
				return;
			}
		}
#ifdef CONFIG_RTL_NEW_AUTOCH
		else if ((priv->auto_channel == 1) && !priv->site_survey->to_scan_40M) {
			unsigned int z=0, ch_begin=0, ch_end=priv->available_chnl_num;
			int idx_2G_end=-1;
			unsigned int proc_nhm = 0;
#if defined(RTK_5G_SUPPORT) 
			int idx_5G_begin=-1;
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
				for (z=0; z<priv->available_chnl_num; z++) {
					if (priv->available_chnl[z] > 14) {
						idx_5G_begin = z;
						break;
					}
				}
				if (idx_5G_begin < 0) 
					goto skip_40M_ss;
			} else 
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
			if ((GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8192E)
				&& priv->pmib->dot11RFEntry.acs_type && !priv->auto_channel_step) {
				priv->auto_channel_step = 1;
			
				memset(priv->nhm_cnt, 0, sizeof(priv->nhm_cnt));
				
				RTL_W8(TXPAUSE, STOP_BCN);
			
				RTL_W8(0xc50, 0x3e);
				if (get_rf_mimo_mode(priv) != MIMO_1T1R)
					RTL_W8(0xc58, 0x3e);
				
				RTL_W16(0x896, 0x61a8);
				RTL_W16(0x892, 0xffff);
				RTL_W32(0x898, 0x82786e64);
				RTL_W32(0x89c, 0xffffff8c);
				PHY_SetBBReg(priv, 0xE28, bMaskByte0, 0xff);
				PHY_SetBBReg(priv, 0x890, BIT8|BIT9|BIT10, 3);
			
				priv->site_survey_times = 0;
				priv->site_survey->ss_channel = priv->available_chnl[0];
				proc_nhm = 1;
			}
			else if ((GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8192E)
				&& priv->pmib->dot11RFEntry.acs_type && priv->auto_channel_step == 1) {
				priv->auto_channel_step = 2;
			
				RTL_W8(0xc50, 0x2a);
				if (get_rf_mimo_mode(priv) != MIMO_1T1R)
					RTL_W8(0xc58, 0x2a);
				
				RTL_W32(0x898, 0x5a50463c);
				RTL_W32(0x89c, 0xffffff64);
			
				priv->site_survey_times = 0;
				priv->site_survey->ss_channel = priv->available_chnl[0];
				proc_nhm = 1;
			}
			else
#endif
			{
				if (priv->auto_channel_step) {
					RTL_W8(TXPAUSE, RTL_R8(TXPAUSE)&~STOP_BCN);
					priv->auto_channel_step = 0;
				}
				
				for (z=0; z<priv->available_chnl_num; z++) {
					if (priv->available_chnl[z] < 14)
						idx_2G_end = z;
					else
						break;
				}
				if (idx_2G_end >= 0) 
					ch_end = idx_2G_end+1;

				for (z=ch_begin; z < ch_end; z++)
					if ((priv->available_chnl[z] >= 5) && (priv->available_chnl[z] < 14))
						break;
				if (z == ch_end)
					goto skip_40M_ss;
			}

			if (!proc_nhm) {
				priv->site_survey->to_scan_40M++;
				priv->site_survey->ss_channel = priv->available_chnl[z];
				priv->site_survey->start_ch_40M = priv->available_chnl[z];//when scan 40M, record start ch
				//priv->site_survey_times = 0;//Do not reset the ss_time because it will rescan SS_COUNT again //AUTOCH_SS_SPEEDUP
				priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20_40;
			}
		}
#endif
		else {
#ifdef CONFIG_RTL_NEW_AUTOCH
skip_40M_ss:
			priv->site_survey->to_scan_40M = 0;
#endif
/*cfg p2p cfg p2p*/
#if defined(CONFIG_P2P_RTK_SUPPORT) && defined(RTK_NL80211)
abort_scan:
#endif
			// scan end			
			OPMODE_VAL(OPMODE & ~WIFI_SITE_MONITOR);
            //STADEBUG("End of scan\n"); 
            #if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
			if (GET_CHIP_VER(priv) == VERSION_8192D){
				if (priv->pshare->DNC_on){
					PHY_SetBBReg(priv, 0x870, bMaskDWord, 0x07600760);
				}
			}
            #endif

            #if defined(CONFIG_RTL_92D_SUPPORT)
			if (GET_CHIP_VER(priv) == VERSION_8192D)
				clnt_ss_check_band(priv, priv->pmib->dot11RFEntry.dot11channel); 
            #endif

/*cfg p2p cfg p2p; remove
            #if 0 //defined(CONFIG_RTL_92D_SUPPORT) && !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
			if ((GET_CHIP_VER(priv) == VERSION_8192D) && priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY &&
					priv->auto_channel!=1) {
				clnt_ss_check_band(priv, priv->pmib->dot11RFEntry.dot11channel); 
				//clnt_load_IQK_res(priv);
				priv->pmib->dot11BssType.net_work_type = priv->site_survey->bk_nwtype;
				if (!get_available_channel(priv)) {
					DEBUG_ERR("(%s) Fail to get available channel\n", __FUNCTION__);
				}
			}
            #endif
*/
			DIG_for_site_survey(priv, FALSE);
#ifdef USE_OUT_SRC
			priv->pshare->bScanInProcess = FALSE;
#endif
#if defined(TESTCHIP_SUPPORT) && defined(CONFIG_RTL_92C_SUPPORT)
		if( IS_TEST_CHIP(priv) ) {
				if ((OPMODE & WIFI_STATION_STATE) || (OPMODE & WIFI_ADHOC_STATE)) {
#ifdef UNIVERSAL_REPEATER
					if (IS_ROOT_INTERFACE(priv) && !netif_running(GET_VXD_PRIV(priv)->dev))
#endif
						RTL_W32(RCR, RTL_R32(RCR) | RCR_CBSSID);
				}
			} else
#endif
			{
               #if 0
				if (OPMODE & WIFI_STATION_STATE) {
				#if defined(UNIVERSAL_REPEATER) || defined(RTK_NL80211)	//wrt-adhoc
					if (IS_ROOT_INTERFACE(priv) && !netif_running(GET_VXD_PRIV(priv)->dev))
				#endif
						RTL_W32(RCR, RTL_R32(RCR) | RCR_CBSSID);
				}
				else 
                #endif									
#ifdef RTK_NL80211 //wrt-adhoc
				if (IS_ROOT_INTERFACE(priv) && !netif_running(GET_VXD_PRIV(priv)->dev))
#endif								
				if (OPMODE & WIFI_ADHOC_STATE)
				{
					RTL_W32(RCR, RTL_R32(RCR) | RCR_CBSSID_ADHOC);
					RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) & ~STOP_BCN); 	// Re-enable beacon 
				}
			}



            #ifdef DFS
			if (!GET_ROOT(priv)->pmib->dot11DFSEntry.disable_DFS &&
				(timer_pending(&GET_ROOT(priv)->ch_avail_chk_timer)))
				GET_ROOT(priv)->pmib->dot11DFSEntry.disable_tx = 1;
			else
				GET_ROOT(priv)->pmib->dot11DFSEntry.disable_tx = 0;
            #endif
            
			if (priv->ss_req_ongoing ) 
			{
				GET_ROOT(priv)->pshare->CurrentChannelBW = GET_ROOT(priv)->pshare->is_40m_bw;
				STADEBUG("RollBack to ROOT's ch[%d] becoz scan done\n",GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel);
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
				if(!rtk_sc_is_channel_fixed(priv))
#endif
				{
					SwBWMode(GET_ROOT(priv), GET_ROOT(priv)->pshare->CurrentChannelBW, GET_ROOT(priv)->pshare->offset_2nd_chan);
					SwChnl(GET_ROOT(priv), GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel, GET_ROOT(priv)->pshare->offset_2nd_chan);
				}
                #ifdef CONFIG_RTL_92D_SUPPORT
				if (GET_CHIP_VER(priv) == VERSION_8192D
                    #ifdef UNIVERSAL_REPEATER
					&& IS_ROOT_INTERFACE(priv)
                    #endif
				)
					PHY_IQCalibrate(priv);
                #endif
			}	
            #ifdef UNIVERSAL_REPEATER
			if (IS_VXD_INTERFACE(priv) && priv->pmib->wscEntry.wsc_enable)
				GET_ROOT(priv)->pmib->miscEntry.func_off = 0;
            #endif



            #ifdef SUPPORT_MULTI_PROFILE
            if( priv->pmib->ap_profile.sortbyprofile && priv->pmib->ap_profile.enable_profile && priv->pmib->ap_profile.profile_num > 0){

                // sort by profile
                ProfileSort(priv,COMPARE_BSS, priv->site_survey->bss, priv->site_survey->count, sizeof(struct bss_desc));
                #ifdef WIFI_SIMPLE_CONFIG
                ProfileSort(priv,COMPARE_WSCIE, priv->site_survey->wscie, priv->site_survey->count, sizeof(struct wps_ie_info));
                #endif				
                #if defined(WIFI_WPAS) || defined(RTK_NL80211)
                    ProfileSort(priv,COMPARE_WPAIE, priv->site_survey->wpa_ie, priv->site_survey->count, sizeof(struct wpa_ie_info));
                    ProfileSort(priv,COMPARE_RSNIE, priv->site_survey->rsn_ie, priv->site_survey->count, sizeof(struct rsn_ie_info));
                    ProfileSort(priv,COMPARE_P2PIE, priv->site_survey->rtk_p2p_ie, priv->site_survey->count, sizeof(struct p2p_ie_info));                    
                #endif
            }else
            #endif            
            {
                // sort by rssi
    			qsort(priv->site_survey->bss, priv->site_survey->count, sizeof(struct bss_desc), compareBSS);
                #ifdef WIFI_SIMPLE_CONFIG
    			qsort(priv->site_survey->wscie, priv->site_survey->count, sizeof(struct wps_ie_info), compareWpsIE);
                #endif
                #if defined(WIFI_WPAS) || defined(RTK_NL80211)
    				qsort(priv->site_survey->wpa_ie, priv->site_survey->count, sizeof(struct wpa_ie_info), compareWpaIE);
	    			qsort(priv->site_survey->rsn_ie, priv->site_survey->count, sizeof(struct rsn_ie_info), compareRsnIE);
	    			qsort(priv->site_survey->rtk_p2p_ie, priv->site_survey->count, sizeof(struct p2p_ie_info), compareP2PIE);                    
                #endif

            }
            
    		debug_print_bss(priv);            

#ifdef RTK_NL80211  //nl_clnt
			SMP_UNLOCK(flags);
			NDEBUG2("end of scan,==>realtek_cfg80211_inform_ss_result\n");
			realtek_cfg80211_inform_ss_result(priv);

            /*cfg p2p note:  priv-> ss_req_ongoing be set to 0 in event_indicate_cfg80211()*/
            #ifdef P2P_SUPPORT  /*cfg p2p cfg p2p*/
            if(rtk_p2p_is_enabled(priv)==CFG80211_P2P){
                /*under p2p case , even result count==0 report it*/
		        event_indicate_cfg80211(priv, NULL, CFG80211_SCAN_DONE, NULL);
            }else
            #endif  /*cfg p2p cfg p2p*/
            {
                if(priv->site_survey->count)            
                    event_indicate_cfg80211(priv, NULL, CFG80211_SCAN_DONE, NULL);
            }

			SMP_LOCK(flags);			
#endif
			if (priv->auto_channel == 1) {
                #ifdef SIMPLE_CH_UNI_PROTOCOL
				if(!GET_MIB(priv)->dot1180211sInfo.mesh_enable)
                #endif
                {
    				priv->pmib->dot11RFEntry.dot11channel = selectClearChannel(priv);
                    #ifdef P2P_SUPPORT/*cfg p2p cfg p2p*/
                    if(rtk_p2p_is_enabled(priv)==PROPERTY_P2P){
                        P2P_DEBUG("auto channel select to[%d],ch changed call rtk_p2p_init again\n\n",priv->pmib->dot11RFEntry.dot11channel);
                        rtk_p2p_init(priv);
                    }
                    #endif/*cfg p2p cfg p2p*/
    				DEBUG_INFO("auto channel select ch %d\n", priv->pmib->dot11RFEntry.dot11channel);
                    #if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
    				LOG_START_MSG();
                    #endif
                    #if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
    				LOG_START_MSG();
                    #endif
                }
#ifdef DFS
                #if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
				if (IS_ROOT_INTERFACE(priv))
                #endif
				{
				 	if(!priv->pmib->dot11DFSEntry.disable_DFS 
					&& is_DFS_channel(priv->pmib->dot11RFEntry.dot11channel) && (OPMODE & WIFI_AP_STATE)) {
						if (timer_pending(&priv->DFS_timer))
							del_timer(&priv->DFS_timer);

						if (timer_pending(&priv->ch_avail_chk_timer))
							del_timer(&priv->ch_avail_chk_timer);

						if (timer_pending(&priv->dfs_det_chk_timer))
							del_timer(&priv->dfs_det_chk_timer);

						init_timer(&priv->ch_avail_chk_timer);
						priv->ch_avail_chk_timer.data = (unsigned long) priv;
						priv->ch_avail_chk_timer.function = rtl8192cd_ch_avail_chk_timer;

						if ((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI) &&
							((priv->pmib->dot11RFEntry.dot11channel >= 120) &&
							(priv->pmib->dot11RFEntry.dot11channel <= 132)))
							mod_timer(&priv->ch_avail_chk_timer, jiffies + CH_AVAIL_CHK_TO_CE);
						else
							mod_timer(&priv->ch_avail_chk_timer, jiffies + CH_AVAIL_CHK_TO);

						init_timer(&priv->DFS_timer);
						priv->DFS_timer.data = (unsigned long) priv;
						priv->DFS_timer.function = rtl8192cd_DFS_timer;

						/* DFS activated after 5 sec; prevent switching channel due to DFS false alarm */
						mod_timer(&priv->DFS_timer, jiffies + RTL_SECONDS_TO_JIFFIES(5));

						init_timer(&priv->dfs_det_chk_timer);
						priv->dfs_det_chk_timer.data = (unsigned long) priv;
						priv->dfs_det_chk_timer.function = rtl8192cd_dfs_det_chk_timer;

						mod_timer(&priv->dfs_det_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.dfs_det_period*10));

						DFS_SetReg(priv);

						if (!priv->pmib->dot11DFSEntry.CAC_enable) {
							del_timer_sync(&priv->ch_avail_chk_timer);
							mod_timer(&priv->ch_avail_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(200));
						}
				 	}
	

					/* disable all of the transmissions during channel availability check */
					priv->pmib->dot11DFSEntry.disable_tx = 0;
					if (!priv->pmib->dot11DFSEntry.disable_DFS &&
					is_DFS_channel(priv->pmib->dot11RFEntry.dot11channel) && (OPMODE & WIFI_AP_STATE)){
						priv->pmib->dot11DFSEntry.disable_tx = 1;
					}
				}
#endif /* DFS */


				
				if (OPMODE & WIFI_AP_STATE)
					priv->auto_channel = 0;
				else
					priv->auto_channel = 2;

				priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
				if(!rtk_sc_is_channel_fixed(priv))
#endif
				{
					SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
					SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
				}
#ifdef CONFIG_RTL_92D_SUPPORT
				if (GET_CHIP_VER(priv) == VERSION_8192D)
					PHY_IQCalibrate(priv);
#endif

				priv->ht_cap_len = 0;	// re-construct HT IE
				init_beacon(priv);
#ifdef SIMPLE_CH_UNI_PROTOCOL
				printk("scan finish, sw ch to (#%d), init beacon\n", priv->pmib->dot11RFEntry.dot11channel);
#endif
#ifdef MBSSID
				if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
					for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
						priv->pvap_priv[i]->pmib->dot11RFEntry.dot11channel = priv->pmib->dot11RFEntry.dot11channel;
						priv->pvap_priv[i]->ht_cap_len = 0;	// re-construct HT IE

						if (IS_DRV_OPEN(priv->pvap_priv[i]))
							init_beacon(priv->pvap_priv[i]);
					}
				}
#endif

#ifdef CLIENT_MODE
#ifdef HS2_CLIENT_TEST
				JOIN_RES = STATE_Sta_Ibss_Idle;
#else
				if (JOIN_RES == STATE_Sta_Ibss_Idle) {
					RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_ADHOC & NETYPE_Mask) << NETYPE_SHIFT));
					mod_timer(&priv->idle_timer, jiffies + RTL_SECONDS_TO_JIFFIES(5));
				}
#endif
#endif



				if (priv->ss_req_ongoing) {
					priv->site_survey->count_backup = priv->site_survey->count;
					memcpy(priv->site_survey->bss_backup, priv->site_survey->bss, sizeof(struct bss_desc)*priv->site_survey->count);
					priv->ss_req_ongoing = 0;
				}

#if defined(CONFIG_RTL_NEW_AUTOCH) && defined(SS_CH_LOAD_PROC)
				record_SS_report(priv);
#endif

			}
			// backup the bss database
			else if (priv->ss_req_ongoing) {
#ifdef P2P_SUPPORT
                /*cfg p2p cfg p2p*/
                if((rtk_p2p_is_enabled(priv)==PROPERTY_P2P)
                    && P2P_DISCOVERY){
                    //for keep  priv->site_survey->count_backup when P2P discovery
                }
                else
#endif
				{
					priv->site_survey->count_backup = priv->site_survey->count;
					memcpy(priv->site_survey->bss_backup, priv->site_survey->bss, sizeof(struct bss_desc)*priv->site_survey->count);
				}				

/*cfg p2p;remove*/
/*cfg p2p;remove*/
#ifdef CLIENT_MODE
                #ifdef P2P_SUPPORT/*cfg p2p cfg p2p*/
                if(rtk_p2p_is_enabled(priv)&& P2P_DISCOVERY){
                    /*P2P discovery duration no need start_clnt_lookup*/
                }else
                #endif/*cfg p2p cfg p2p*/
				if(priv->ss_req_ongoing != SSFROM_WSC && priv->ss_req_ongoing != SSFROM_REPEATER_VXD) {
    				if (JOIN_RES == STATE_Sta_Ibss_Idle) {
                        STADEBUG("start_clnt_lookup(RESCAN)\n");    
    					start_clnt_lookup(priv, RESCAN);
    				}
    				else if (JOIN_RES == STATE_Sta_Auth_Success){
    					start_clnt_assoc(priv);
    				}
    				else if (JOIN_RES == STATE_Sta_Roaming_Scan) {
                        STADEBUG("start_clnt_lookup(RESCAN)\n");    
    					start_clnt_lookup(priv, RESCAN);
    				}
    				else if(JOIN_RES == STATE_Sta_No_Bss) {
                        STADEBUG("start_clnt_lookup(RESCAN)\n");    
    					JOIN_RES_VAL(STATE_Sta_Roaming_Scan);
    					start_clnt_lookup(priv, RESCAN);
    				}

				}
#endif

#ifdef SMART_REPEATER_MODE
				if (priv->ss_req_ongoing == SSFROM_REPEATER_VXD) {
					// VXD Interface
#ifdef SUPPORT_MULTI_PROFILE
					if (GET_MIB(priv)->ap_profile.enable_profile &&
							GET_MIB(priv)->ap_profile.profile_num > 0) {
						SSID2SCAN_LEN = strlen(GET_MIB(priv)->ap_profile.profile[priv->profile_idx].ssid);
						memcpy(SSID2SCAN, GET_MIB(priv)->ap_profile.profile[priv->profile_idx].ssid, SSID2SCAN_LEN);					
					}
					else
#endif			
					{					
						SSID2SCAN_LEN = GET_MIB(priv)->dot11StationConfigEntry.dot11SSIDtoScanLen;
						memcpy(SSID2SCAN, GET_MIB(priv)->dot11StationConfigEntry.dot11SSIDtoScan, SSID2SCAN_LEN);
					}
					priv->site_survey->count_target = priv->site_survey->count;
					memcpy(priv->site_survey->bss_target, priv->site_survey->bss, sizeof(struct bss_desc)*priv->site_survey->count);					
					priv->join_index = -1;
					priv->join_res = STATE_Sta_Min; 

					start_clnt_lookup(priv, DONTRESCAN);

        		            //STADEBUG("call set_vxd_rescan\n");
   					if(priv->join_res == STATE_Sta_Min)
   			           set_vxd_rescan(priv,RESCAN_BY_NEXTTIME);

				}
#endif			

				priv->ss_req_ongoing = 0;
                //STADEBUG("set priv->ss_req_ongoing to 0\n\n\n");				

#ifdef WIFI_WPAS
				event_indicate_wpas(priv, NULL, WPAS_SCAN_DONE, NULL);
#endif
			}
#ifdef CLIENT_MODE
			else if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) {
				priv->site_survey->count_target = priv->site_survey->count;
				memcpy(priv->site_survey->bss_target, priv->site_survey->bss, sizeof(struct bss_desc)*priv->site_survey->count);
				priv->join_index = -1;
#ifdef HS2_CLIENT_TEST
#else
                #ifdef P2P_SUPPORT/*cfg p2p cfg p2p*/
                if(rtk_p2p_is_enabled(priv)&& P2P_DISCOVERY){
                    /*P2P discovery duration no need start_clnt_lookup*/
                }else
                #endif  /*cfg p2p cfg p2p*/              
				if (JOIN_RES == STATE_Sta_Roaming_Scan){
                    STADEBUG("start_clnt_lookup(DONTRESCAN)\n");
					start_clnt_lookup(priv, DONTRESCAN);
                }

#endif
			}
#endif
			else {
				DEBUG_ERR("Faulty scanning\n");
			}
			
#ifdef CONFIG_RTL_COMAPI_WLTOOLS
			wake_up_interruptible(&priv->ss_wait);
#endif

#ifdef CHECK_BEACON_HANGUP
			priv->pshare->beacon_wait_cnt = 2;
#endif
#ifdef CONFIG_RTL8672
			OPMODE_VAL(OPMODE & (~WIFI_WAIT_FOR_CHANNEL_SELECT));
#endif
			SMP_UNLOCK(flags);
			/*cfg p2p cfg p2p*/
            #ifdef P2P_SUPPORT
            if((rtk_p2p_is_enabled(priv))==PROPERTY_P2P
                && P2P_DISCOVERY){
                P2P_DEBUG("Scan end\n");
                /* execute p2p_find_timer() */                             
            	mod_timer(&priv->p2pPtr->p2p_find_timer_t, jiffies + RTL_MILISECONDS_TO_JIFFIES(100));   	
            }
#endif
            NDEBUG2("endofscan\n");
			/*cfg p2p cfg p2p*/
			return;
		}
	}



   /*switch channel; now, change RF channel... start*/

    #ifdef CONFIG_RTL_92D_SUPPORT
	int band_switch = 0;
    #endif
	
	// now, change RF channel...
    #ifdef DFS
	if (!priv->pmib->dot11DFSEntry.disable_DFS && is_DFS_channel(priv->site_survey->ss_channel)){
		priv->pmib->dot11DFSEntry.disable_tx = 1;
	}else{
		priv->pmib->dot11DFSEntry.disable_tx = 0;
	}
    #endif

    #if defined(CONFIG_RTL_92D_SUPPORT) && !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
		if ((GET_CHIP_VER(priv) == VERSION_8192D) && priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY) {
			band_switch = clnt_ss_check_band(priv, priv->site_survey->ss_channel);		
		}
    #endif

    #ifdef CONFIG_RTL_NEW_AUTOCH
	if (priv->site_survey->to_scan_40M) {
        #if defined(RTK_5G_SUPPORT) 
		if (GET_ROOT(priv)->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
				if((priv->site_survey->ss_channel>144) ? ((priv->site_survey->ss_channel-1)%8) : (priv->site_survey->ss_channel%8)) {
					//STADEBUG("SwChnl=%d,2ndoffsetCh=%d\n",priv->site_survey->ss_channel,HT_2NDCH_OFFSET_ABOVE);
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
					if(!rtk_sc_is_channel_fixed(priv))
#endif
					{
						SwChnl(GET_ROOT(priv), priv->site_survey->ss_channel, HT_2NDCH_OFFSET_ABOVE);
						SwBWMode(GET_ROOT(priv), priv->pshare->CurrentChannelBW, HT_2NDCH_OFFSET_ABOVE);
					}
				} else {
					//STADEBUG("SwChnl=%d,2ndoffsetCh=%d\n",priv->site_survey->ss_channel,HT_2NDCH_OFFSET_BELOW);
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
					if(!rtk_sc_is_channel_fixed(priv))
#endif
					{
						SwChnl(GET_ROOT(priv), priv->site_survey->ss_channel, HT_2NDCH_OFFSET_BELOW);
						SwBWMode(GET_ROOT(priv), priv->pshare->CurrentChannelBW, HT_2NDCH_OFFSET_BELOW);
					}
				}

		} else
        #endif
		{
			/* set channel >= 5 for algo requirement */
            //STADEBUG("SwChnl=%d,2ndoffsetCh=%d\n",priv->site_survey->ss_channel,HT_2NDCH_OFFSET_BELOW);
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
			if(!rtk_sc_is_channel_fixed(priv))
#endif
			{
				SwChnl(GET_ROOT(priv), priv->site_survey->ss_channel, HT_2NDCH_OFFSET_BELOW);
				SwBWMode(GET_ROOT(priv), priv->pshare->CurrentChannelBW, HT_2NDCH_OFFSET_BELOW);
			}
		}
	}
    else
    #endif
	{
        #if defined(UNIVERSAL_REPEATER) && defined(SMART_REPEATER_MODE)
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
		if(rtk_sc_during_simple_config_scan(priv)){

			int max_bw = HT_CHANNEL_WIDTH_20;
			int sc_scan_offset = HT_2NDCH_OFFSET_DONTCARE;

			//panic_printk("%s %d (idx=%d, channel=%d)+++\n", __FUNCTION__, __LINE__, idx, priv->available_chnl[idx]);

			if(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
				max_bw = HT_CHANNEL_WIDTH_20_40;

			if(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
				max_bw = HT_CHANNEL_WIDTH_80;

			if(max_bw != HT_CHANNEL_WIDTH_20){
				sc_scan_offset = rtk_sc_get_scan_offset(GET_ROOT(priv), priv->site_survey->ss_channel, max_bw); 
			}

			GET_ROOT(priv)->pshare->offset_2nd_chan = sc_scan_offset;
			GET_ROOT(priv)->pshare->CurrentChannelBW = max_bw;

			priv->pshare->offset_2nd_chan = sc_scan_offset;
			priv->pshare->CurrentChannelBW = max_bw;

			if(!rtk_sc_is_channel_fixed(priv)) {
				SwBWMode(GET_ROOT(priv), max_bw, sc_scan_offset);
				SwChnl(GET_ROOT(priv), priv->site_survey->ss_channel, sc_scan_offset);
			}
	
			if( (priv->site_survey->ss_channel >= 5) && (priv->site_survey->ss_channel <= 9)
				&& (priv->pshare->offset_2nd_chan  == HT_2NDCH_OFFSET_BELOW) && (idx >= 0))
				priv->site_survey->ss_channel = priv->available_chnl[(idx)];
			

		}
		else
#endif
		if(priv->site_survey->defered_ss) {
		//STADEBUG("Between B53&B56, stay AP's ch for %d miliseconds\n",priv->pmib->miscEntry.ss_delay);
		//STADEBUG("SwChnl[%d],2ndCh[%d]\n",GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel,GET_ROOT(priv)->pshare->offset_2nd_chan);                
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
			if(!rtk_sc_is_channel_fixed(priv))
#endif
			{
				SwBWMode(GET_ROOT(priv), GET_ROOT(priv)->pshare->CurrentChannelBW, GET_ROOT(priv)->pshare->offset_2nd_chan);
				SwChnl(GET_ROOT(priv), GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel, GET_ROOT(priv)->pshare->offset_2nd_chan);
			}
			priv->pmib->dot11DFSEntry.disable_tx = 0;

		} else 
        #endif
		{
			//STADEBUG("SwChnl=%d,2ndoffsetCh=%d\n",priv->site_survey->ss_channel,priv->pshare->offset_2nd_chan);
			#if defined(CONFIG_RTL_SIMPLE_CONFIG)
			if(!rtk_sc_is_channel_fixed(priv))
			#endif
			SwChnl(GET_ROOT(priv), priv->site_survey->ss_channel, priv->pshare->offset_2nd_chan);
            #ifdef CONFIG_RTL_92D_SUPPORT
			if ((GET_CHIP_VER(priv) == VERSION_8192D) && 
					(priv->pmib->dot11RFEntry.macPhyMode == SINGLEMAC_SINGLEPHY) && band_switch)
				PHY_IQCalibrate(GET_ROOT(priv));
            #endif
            #ifdef CONFIG_RTL_NEW_AUTOCH
			if (priv->auto_channel == 1) {
				reset_FA_reg(priv);

			#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
				if ((GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8192E)
					&& priv->pmib->dot11RFEntry.acs_type && priv->auto_channel_step) {
						PHY_SetBBReg(priv, 0x890, BIT1, 0);
						PHY_SetBBReg(priv, 0x890, BIT1, 1);
				}
			#endif
			}
            #endif
		}
	}    
	/*switch channel-end*/
#if defined(RTK_NL80211)
	//by brian, trigger channel load evaluation after channel switched
	start_bbp_ch_load(priv, (45*1000)/4);
#endif
    /*TX probe_request -- start*/
    if (priv->site_survey->hidden_ap_found == HIDE_AP_FOUND_DO_ACTIVE_SSAN ||
			!is_passive_channel(priv , priv->pmib->dot11StationConfigEntry.dot11RegDomain, priv->site_survey->ss_channel))            
	{                
            #ifdef SIMPLE_CH_UNI_PROTOCOL
			if(GET_MIB(priv)->dot1180211sInfo.mesh_enable)
				issue_probereq_MP(priv, "MESH-SCAN", 9, NULL, TRUE);
			else
            #endif				
			if (priv->ss_ssidlen == 0){

                #ifdef P2P_SUPPORT
				/*cfg p2p cfg p2p*/
				if( rtk_p2p_is_enabled(priv)&&                     /*PROPERTY_P2P case run this way*/
					((rtk_p2p_chk_role(priv,P2P_DEVICE))||(rtk_p2p_chk_role(priv,P2P_CLIENT))) ){
					P2P_DEBUG("p2p scan (chann:%d)\n",priv->site_survey->ss_channel);							
					issue_probereq(priv, "DIRECT-", 7, NULL);
				}/*cfg p2p cfg p2p*/
                else
                #endif					
				{
					if (!priv->auto_channel_step) {
						issue_probereq(priv, NULL, 0, NULL);
					}
				}
			}else{
                #ifdef SUPPORT_MULTI_PROFILE	/*send multi probe_req by profile_num*/	
				if (priv->pmib->ap_profile.enable_profile && priv->pmib->ap_profile.profile_num > 0) {
					for(jdx=0;jdx<priv->pmib->ap_profile.profile_num;jdx++) {									
       					 STADEBUG("issue_probereq[%s],ch=[%d]\n",priv->pmib->ap_profile.profile[jdx].ssid , priv->site_survey->ss_channel);
						issue_probereq(priv, priv->pmib->ap_profile.profile[jdx].ssid, strlen(priv->pmib->ap_profile.profile[jdx].ssid), NULL);
					}												
				}
				else	
                #endif								
				{
					STADEBUG("issue_probereq,ssid[%s],ch[%d]\n",priv->ss_ssid,priv->site_survey->ss_channel);
					issue_probereq(priv, priv->ss_ssid, priv->ss_ssidlen, NULL);
				}

    		}
	}

    /*TX probe_request -- End*/    
    SMP_UNLOCK(flags);


	/*now, scheduling next ss_timer ; the time util to next timer executed is how long DUT stady in this channel*/ 
	// now, start another timer again.

	if(is_passive_channel(priv , priv->pmib->dot11StationConfigEntry.dot11RegDomain, priv->site_survey->ss_channel))
	{
#if defined(UNIVERSAL_REPEATER) && defined(SMART_REPEATER_MODE)
		if(priv->site_survey->defered_ss) {
			if(priv->pmib->miscEntry.ss_delay) {
				mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pmib->miscEntry.ss_delay));
			} else {
				if(priv->pmib->miscEntry.passive_ss_int) {
					mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pmib->miscEntry.passive_ss_int));
				} else {
					mod_timer(&priv->ss_timer, jiffies + SS_PSSV_TO);
				}
			}
		}
		else {		
			if(priv->pmib->miscEntry.passive_ss_int) {
				mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pmib->miscEntry.passive_ss_int));
			} else {
				mod_timer(&priv->ss_timer, jiffies + SS_PSSV_TO);
			}
		}

#else
		if(priv->pmib->miscEntry.passive_ss_int) {
			mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pmib->miscEntry.passive_ss_int));
		} else {
			mod_timer(&priv->ss_timer, jiffies + SS_PSSV_TO);
		}
#endif

	} 
	else

	{
#ifdef CONFIG_RTL_NEW_AUTOCH
		if (priv->auto_channel == 1){
		#ifdef AUTOCH_SS_SPEEDUP
			if(priv->pmib->miscEntry.autoch_ss_to != 0)
				mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pmib->miscEntry.autoch_ss_to));
			else
		#endif
			if (priv->auto_channel_step)
				mod_timer(&priv->ss_timer, jiffies + SS_AUTO_CHNL_NHM_TO);
			else
				mod_timer(&priv->ss_timer, jiffies + SS_AUTO_CHNL_TO);
		}else 
#endif	
        {
			#ifdef P2P_SUPPORT	// 2013
			/*cfg p2p cfg p2p*/
            if(rtk_p2p_is_enabled(priv)&& (rtk_p2p_chk_role(priv,P2P_DEVICE) || rtk_p2p_chk_role(priv,P2P_CLIENT))){
                /*search phase (only 1,6,11) use 120ms*/
                //P2P_DEBUG("[%d]ms\n",P2P_SEARCH_TIME_V);
                mod_timer(&priv->ss_timer, jiffies + P2P_SEARCH_TIME);
            }else
			#endif
            {
    	    	mod_timer(&priv->ss_timer, jiffies + SS_TO);
            }
        }
	}
}


/**
 *	@brief  get WPA/WPA2 information
 *
 *	use 1 timestamp (32-bit variable) to carry WPA/WPA2 info \n
 *	1st 16-bit:                 WPA \n
 *  |          auth       |              unicast cipher              |              multicast cipher            |	\n
 *     15    14    13   12      11      10     9     8       7      6      5       4      3     2       1      0	\n
 *	+-----+-----+----+-----+--------+------+-----+------+-------+-----+--------+------+-----+------+-------+-----+	\n
 *	| Rsv | PSK | 1X | Rsv | WEP104 | CCMP | Rsv | TKIP | WEP40 | Grp | WEP104 | CCMP | Rsv | TKIP | WEP40 | Grp |	\n
 *	+-----+-----+----+-----+--------+------+-----+------+-------+-----+--------+------+-----+------+-------+-----+	\n
 *	2nd 16-bit:                 WPA2 \n
 *            auth       |              unicast cipher              |              multicast cipher            |	\n
 *	  15    14    13   12      11      10     9     8       7      6      5       4      3     2       1      0		\n
 *  +-----+-----+----+-----+--------+------+-----+------+-------+-----+--------+------+-----+------+-------+-----+	\n
 *	| Rsv | PSK | 1X | Rsv | WEP104 | CCMP | Rsv | TKIP | WEP40 | Grp | WEP104 | CCMP | Rsv | TKIP | WEP40 | Grp |	\n
 *  +-----+-----+----+-----+--------+------+-----+------+-------+-----+--------+------+-----+------+-------+-----+	\n
 */
static void get_security_info(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, int index)
{
	int i, len, result;
	unsigned char *p, *pframe, *p_uni, *p_auth, val;
	unsigned short num;
	unsigned char OUI1[] = {0x00, 0x50, 0xf2};
	unsigned char OUI2[] = {0x00, 0x0f, 0xac};
#if defined(CONFIG_RTL_WAPI_SUPPORT)
	const unsigned char OUI3[] = {0x00, 0x14, 0x72};
#endif
#ifdef  CONFIG_IEEE80211W_CLI
	unsigned short	rsnie_cap;
#endif

	//WPS2DOTX
	unsigned char *awPtr  =  (unsigned char *)&priv->site_survey->wscie[index].data;
	int foundtimes = 0;
	unsigned char *ptmp = NULL;
	unsigned int lentmp=0;
	unsigned int totallen=0;
	//WPS2DOTX
	
	pframe = get_pframe(pfrinfo);
	priv->site_survey->bss[index].t_stamp[0] = 0;

	p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;
	len = 0;
	result = 0;
	do {
		p = get_ie(p, _RSN_IE_1_, &len,
			pfrinfo->pktlen - (p - pframe));
		if ((p != NULL) && (len > 18))
		{
			if (memcmp((p + 2), OUI1, 3))
				goto next_tag;
			if (*(p + 5) != 0x01)
				goto next_tag;
			if (memcmp((p + 8), OUI1, 3))
				goto next_tag;
			val = *(p + 11);
			priv->site_survey->bss[index].t_stamp[0] |= BIT(val);
			p_uni = p + 12;
			memcpy(&num, p_uni, 2);
			num = le16_to_cpu(num);
			for (i=0; i<num; i++) {
				if (memcmp((p_uni + 2 + 4 * i), OUI1, 3))
					goto next_tag;
				val = *(p_uni + 2 + 4 * i + 3);
				priv->site_survey->bss[index].t_stamp[0] |= (BIT(val) << 6);
			}
			p_auth = p_uni + 2 + 4 * num;
			memcpy(&num, p_auth, 2);
			num = le16_to_cpu(num);
			for (i=0; i<num; i++) {
				if (memcmp((p_auth + 2 + 4 * i), OUI1, 3))
					goto next_tag;
				val = *(p_auth + 2 + 4 * i + 3);
				priv->site_survey->bss[index].t_stamp[0] |= (BIT(val) << 12);
			}
			result = 1;
		}
next_tag:
		if (p != NULL)
			p = p + 2 + len;
	} while ((p != NULL) && (result != 1));

	if (result != 1)
	{
		priv->site_survey->bss[index].t_stamp[0] = 0;
	}

	p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;
	len = 0;
	result = 0;
	do {
		p = get_ie(p, _RSN_IE_2_, &len,
			pfrinfo->pktlen - (p - pframe));
		if ((p != NULL) && (len > 12))
		{
			if (memcmp((p + 4), OUI2, 3))
				goto next_id;
			val = *(p + 7);
			priv->site_survey->bss[index].t_stamp[0] |= (BIT(val) << 16);
			p_uni = p + 8;
			memcpy(&num, p_uni, 2);
			num = le16_to_cpu(num);
			for (i=0; i<num; i++) {
				if (memcmp((p_uni + 2 + 4 * i), OUI2, 3))
					goto next_id;
				val = *(p_uni + 2 + 4 * i + 3);
				priv->site_survey->bss[index].t_stamp[0] |= (BIT(val) << 22);
			}
			p_auth = p_uni + 2 + 4 * num;
			memcpy(&num, p_auth, 2);
			num = le16_to_cpu(num);
			for (i=0; i<num; i++) {
				if (memcmp((p_auth + 2 + 4 * i), OUI2, 3))
					goto next_id;
				val = *(p_auth + 2 + 4 * i + 3);
				priv->site_survey->bss[index].t_stamp[0] |= (BIT(val) << 28);
#ifdef CONFIG_IEEE80211W_CLI
				memcpy(&rsnie_cap, (p_auth + 2 + 4 * i + 4), 2);
				priv->wpa_global_info->rsnie_cap = le16_to_cpu(rsnie_cap);	

				if((priv->wpa_global_info->rsnie_cap & BIT(6)) && (priv->wpa_global_info->rsnie_cap & BIT(7)))
					priv->site_survey->bss[index].t_stamp[1] |= PMF_REQ;
				else if(priv->wpa_global_info->rsnie_cap & BIT(7))
					priv->site_survey->bss[index].t_stamp[1]	|= PMF_CAP;
				else
					priv->site_survey->bss[index].t_stamp[1]	|= PMF_NONE;
#endif 
			}
			result = 1;
		}
next_id:
		if (p != NULL)
			p = p + 2 + len;
	} while ((p != NULL) && (result != 1));

	if (result != 1)
	{
		priv->site_survey->bss[index].t_stamp[0] &= 0x0000ffff;
	}

#if defined(CONFIG_RTL_WAPI_SUPPORT)
	p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;
	len = 0;
	result = 0;
	do {
		p = get_ie(p, _EID_WAPI_, &len,
			pfrinfo->pktlen - (p - pframe));
		if ((p != NULL) && (len >19))
		{
			i = *((uint16*)(p+4));
			if (i != 0x0100)
				goto next_id_wapi;
			if (memcmp((p + 6), OUI3, 3) || *(p+9)!=2)
				goto next_id_wapi;
			
			val = *(p + 9);
			priv->site_survey->bss[index].t_stamp[0] = SECURITY_INFO_WAPI;
			break;
		}
next_id_wapi:
		if (p != NULL)
			p = p + 2 + len;
	} while ((p != NULL) && (result != 1));
#endif

#ifdef WIFI_SIMPLE_CONFIG
/* WPS2DOTX*/
#ifndef MULTI_MAC_CLONE
#if defined(WIFI_WPAS) || defined(RTK_NL80211)
	if (OPMODE & WIFI_STATION_STATE) 
#else
	if (priv->ss_req_ongoing == 2)  //simple-config scan-req
#endif
#endif
	{
		//ptmp = pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_;
		ptmp = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;		

		for (;;)
		{
			ptmp = get_ie(ptmp, _WPS_IE_, (int *)&lentmp,pfrinfo->pktlen - (ptmp - pframe));
			
			if (ptmp != NULL) {
				if (!memcmp(ptmp+2, WSC_IE_OUI, 4)) {
					foundtimes ++;
					if(foundtimes ==1){
						memcpy(awPtr , ptmp ,lentmp + 2);
						awPtr+= (lentmp + 2);
						totallen += (lentmp + 2);
					}else{
						memcpy(awPtr , ptmp+2+4 ,lentmp-4);
						awPtr+= (lentmp-4);
						totallen += (lentmp-4);						
					}					
				}
			}
			else{
				awPtr[0]='\0';
				break;
			}

			ptmp = ptmp + lentmp + 2;
		}
		if(foundtimes){
        	/*cfg p2p cfg p2p*/
			/*Save WSC IE*/ 
			priv->site_survey->wscie[index].wps_ie_len = totallen;
			//debug_out("WSC_IE",priv->site_survey.ie[index].data,totallen);
			//get the first wps ie to see the
			//search_wsc_tag(unsigned char *data, unsigned short id, int len, &target_len);
			priv->site_survey->bss[index].t_stamp[1] |= BIT(8);  //  set t_stamp[1] bit 8 when AP supports WPS
		} else {
		    priv->site_survey->bss[index].t_stamp[1] &= ~BIT(8);  // clear t_stamp[1] bit 8 when AP not supports WPS(do not have wps IE)
		}
	}

#endif
/* WPS2DOTX*/
}

#ifdef P2P_SUPPORT
/* type:  WIFI_BEACON: from beacon, WIFI_PROBERSP: from prob rsp*/
int p2p_collect_bss_info(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, int type)
{
    int index;
    int len;
    unsigned char  *pframe, *sa, channel_tmp;
    int role;		
    unsigned char *ptmp=NULL;
    unsigned char *SsidPtr=NULL;
    int ssidlen = 0;

    /* p2p IE must support fragment and reassembly ;
    	this method is sample for support reassembly*/  
    static	unsigned char ReAssem_p2pie[MAX_REASSEM_P2P_IE];
    static	unsigned char ReAssem_wscie[MAX_REASSEM_P2P_IE];		

    int IEfoundtimes = 0 ;
    unsigned char *p2pIEPtr = ReAssem_p2pie ;
    int p2pIElen=0;

    unsigned char *wscIEPtr = ReAssem_wscie ;
    int wscIElen=0;	

    unsigned char* wsc_tag_ptr=NULL;
    int tag_len = 0;        
   

    struct device_info_s p2p_devic_info;
    memset(&p2p_devic_info,0,sizeof(struct device_info_s));

    pframe = get_pframe(pfrinfo);
    sa = GetAddr2Ptr(pframe);

    if (priv->site_survey->count_p2p >= MAX_BSS_NUM){
        P2P_DEBUG("bss count > MAX_BSS_NUM!!!\n");
        return 0;
    }

    /* chk DA is broadcast or my p2p device-addr (default use MY-HW-ADDR as p2p device addrree)*/
    if( memcmp(GetAddr1Ptr(pframe), "\xff\xff\xff\xff\xff\xff", 6) 
        && memcmp(GetAddr1Ptr(pframe), GET_MY_HWADDR, 6))
    {
        P2P_DEBUG("DA mismatch!\n");
        return 0;
    }

	
    /*get SSID*/ 
    SsidPtr = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_, _SSID_IE_, &ssidlen,
    pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_);


    /* chk ssid == wildcard SSID ("DIRECT-")*/ 
    /*cfg p2p cfg p2p*/
    if(SsidPtr && !memcmp(SsidPtr+2,"DIRECT-",7))
    {
		P2P_DEBUG("device: %02x%02x%02x:%02x%02x%02x\n" ,
			sa[0],sa[1],sa[2],sa[3],sa[4],sa[5]);					

    }else{	
        return 0;
    }


	/*----------------------------find P2P IE -----------------------------------start*/

	/*  Get P2P IE from Probe_RSP */
    IEfoundtimes = 0;
    len = 0 ;	
    ptmp = pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_ ;			

    /*support ReAssemble*/	
    for (;;)
    {
        ptmp = get_ie(ptmp, _P2P_IE_, &len,	
        pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_ - len);			

        if (ptmp) {
            if (!memcmp(ptmp+2, WFA_OUI_PLUS_TYPE, 4)) {
                IEfoundtimes ++;
                memcpy(p2pIEPtr , ptmp+6 ,len-4);
                p2pIEPtr+= (len-4);
            }

        }
        else{
            break;
        }
        ptmp = ptmp + len + 2;

    }


			
    if(IEfoundtimes){
        p2pIElen = (int)(((unsigned long)p2pIEPtr)-((unsigned long)ReAssem_p2pie));		
        if(p2pIElen > MAX_REASSEM_P2P_IE){
            P2P_DEBUG("\n\n	ReAssem WSC IE exceed MAX_REASSEM_P2P_IE , chk!!!\n\n");
            return 0;
        }

    }else{
        return 0;
    }	
	/*----------------------------find P2P IE -----------------------------------end*/


	/*----------------------------find WSC IE -----------------------------------start*/

	/*  Get WSC IE from Probe Rsp */
    IEfoundtimes = 0;
    len = 0;

    ptmp = pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_ ;
    for (;;)
    {
        ptmp = get_ie(ptmp, _WPS_IE_, &len,	
        pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_ - len);			

        if (ptmp) {
            if (!memcmp(ptmp+2, WSC_IE_OUI, 4)) {

                IEfoundtimes ++;
                memcpy(wscIEPtr , ptmp+6 ,len-4);
                wscIEPtr+= (len-4);
            }
        }
        else{
            break;
        }
        ptmp = ptmp + len + 2;

    }

    if(IEfoundtimes){
        wscIElen = (int)(((unsigned long)wscIEPtr)-((unsigned long)ReAssem_wscie));
        if(wscIElen > MAX_REASSEM_P2P_IE){
            P2P_DEBUG("\n\n	ReAssem WSC IE exceed MAX_REASSEM_P2P_IE , chk!!!\n\n");
            return 0;			
        }
    }else{
        P2P_DEBUG("no found wsc IE \n");					
        return 0 ;
    }	
    /*----------------------------find wsc IE -----------------------------------end*/


    /*--- record rx form which channel from _DSSET_IE_ for later use---*/	
    ptmp = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_, _DSSET_IE_, &len,
    pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_);


    if (ptmp != NULL){
        channel_tmp = *(ptmp+2);
    }else{
        channel_tmp = priv->site_survey->ss_channel;	
    }	
	/*--- record rx form which channel from _DSSET_IE_ for later use---*/	


	
	/*-----chk target is GO or device-----*/
	//p2pIEPtr = ReAssem_p2pie;
		/*cfg p2p cfg p2p*/
    if((role = p2p_get_role_from_ie(priv,ReAssem_p2pie,p2pIElen)) == 0) {       
        P2P_DEBUG("no found TAG_P2P_CAPABILITY attribute\n");					
        return 0;
    }
    
    if(type == WIFI_PROBERSP){  
        if(p2p_get_device_info(priv,ReAssem_p2pie,p2pIElen,&p2p_devic_info) == FAIL) {
            P2P_DEBUG("no P2P DEVICE_INFO attribute in prob rsp frame\n");                    
            return 0;
        }
    }
    else if(type == WIFI_BEACON){
        if(p2p_get_GO_p2p_info(priv,ReAssem_p2pie,p2pIElen,&p2p_devic_info) == FAIL){
            /*no include device id addr in P2P IE*/ 
            P2P_DEBUG("no P2P DEVICE_ID attribute in beacon frame\n");                    
            return 0;
        }
    }
    else {
        return 0;
    }
    
    p2p_get_GO_wsc_info(priv,ReAssem_wscie,wscIElen,&p2p_devic_info);            

	/*-----chk target is GO or device-----*/
	
	/*---find free site_survey bss---*/
	// search if exist
    for(index = 0  ; index<priv->site_survey->count_p2p ; index++){
        if(!memcmp(priv->site_survey->bss_p2p[index].p2paddress, p2p_devic_info.dev_address , MACADDRLEN))
            break;
    }
    if(index == priv->site_survey->count_p2p){
        priv->site_survey->count_p2p++;
    }


    // now recored this bss info
    priv->site_survey->bss_p2p[index].channel = channel_tmp ;
    priv->site_survey->bss_p2p[index].p2prole = role;		

    memcpy(priv->site_survey->bss_p2p[index].p2paddress,p2p_devic_info.dev_address ,MACADDRLEN);
    //memcpy(priv->site_survey->bss_backup[index].p2pdevname , p2p_devic_info.devname ,33);	
    strcpy(priv->site_survey->bss_p2p[index].p2pdevname , p2p_devic_info.devname);	
    priv->site_survey->bss_p2p[index].p2pwscconfig = p2p_devic_info.config_method;		

    if(role == R_P2P_GO){
        memcpy(priv->site_survey->bss_p2p[index].ssid , SsidPtr+2	, ssidlen);	
        priv->site_survey->bss_p2p[index].ssid[ssidlen]='\0';
        //P2P_DEBUG("ssid=%s\n",priv->site_survey->bss_backup[index].ssid);
    }

    return SUCCESS;
}

#endif

#ifdef	CONFIG_IEEE80211W_CLI

unsigned char SHA256_AKM_SUITE[] = {0x00, 0x0F, 0xAC, 0x06};
unsigned char Doll1X_SHA256_AKM_SUITE[] = {0x00, 0x0F, 0xAC, 0x05};
unsigned char Doll1X_AKM_SUITE[] = {0x00, 0x0F, 0xAC, 0x01};


void add_sha256_akm(struct rtl8192cd_priv *priv)
{
	WPA_GLOBAL_INFO *pGblInfo = priv->wpa_global_info;
	unsigned char AKM_buff[KEY_AKM_LEN];
	
	OCTET_STRING AKM;
	memcpy(AKM_buff, SHA256_AKM_SUITE, KEY_AKM_LEN);
	AKM.Octet = AKM_buff;
	AKM.Length = KEY_AKM_LEN;
	Message_setSha256AKM(pGblInfo->AuthInfoElement, AKM);
}
#endif

/**
 *	@brief	After site survey, collect BSS information to site_survey->bss[index]
 *
 *	The function can find site
 *  Later finish site survey, call the function get BSS informat.
 */
int collect_bss_info(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	int i, index, len=0;
	unsigned char *addr, *p, *pframe, *sa, channel=0;
	UINT32	basicrate=0, supportrate=0, hiddenAP=0;
	UINT16	val16;
	struct wifi_mib *pmib;
	DOT11_WPA_MULTICAST_CIPHER wpaMulticastCipher;
	unsigned char OUI1[] = {0x00, 0x50, 0xf2};
	DOT11_WPA2_MULTICAST_CIPHER wpa2MulticastCipher;
	unsigned char OUI2[] = {0x00, 0x0f, 0xac};

#if defined(P2P_SUPPORT) && defined(RTK_NL80211)        
	/*cfg p2p cfg p2p*/
    u8* IE_START_PTR;
    int IE_LEN =  pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_;	
    unsigned char *WSCIEPtr = NULL ;
    int WSCIElen=0;
	unsigned char *p2pIEPtr2 = NULL;    
	/*cfg p2p cfg p2p*/	
#endif    

    #ifdef P2P_SUPPORT  
	static	unsigned char ReAssem_p2pie[MAX_REASSEM_P2P_IE];
	int IEfoundtimes=0;
	unsigned char *p2pIEPtr = ReAssem_p2pie ;
	int p2pIElen=0;
    #endif

#ifdef SUPPORT_MULTI_PROFILE
	int jdx=0;
	int found=0;
	int ssid_len=0;
#endif	

	pframe = get_pframe(pfrinfo);
#ifdef CONFIG_RTK_MESH
// GANTOE for site survey 2008/12/25 ====
	if(pfrinfo->is_11s)
		addr = GetAddr2Ptr(pframe);
	else
#endif
		addr = GetAddr3Ptr(pframe);

	sa = GetAddr2Ptr(pframe);
	pmib = GET_MIB(priv);
#if defined(P2P_SUPPORT) && defined(RTK_NL80211)    
	/*cfg p2p cfg p2p*/
    IE_START_PTR = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_ ;

    /*-------------find wsc IE -----------------------------------*/
	WSCIEPtr = rtk_get_wps_ie(IE_START_PTR , IE_LEN , NULL , &WSCIElen);

    /*-------------find P2P IE -----------------------------------*/
    p2pIEPtr2 = rtk_get_p2p_ie(IE_START_PTR , IE_LEN , NULL , &p2pIElen);

/*-------------for p2p check da,ssid,wsc,p2p etc-----------------------*/    
	if(rtk_p2p_is_enabled(priv)==CFG80211_P2P && rtk_p2p_chk_role(priv,P2P_DEVICE)){
	    /* chk DA is broadcast or my p2p device-addr 
	              (default use MY-HW-ADDR as p2p device addrree)*/
	    if( memcmp(GetAddr1Ptr(pframe), "\xff\xff\xff\xff\xff\xff", 6) 
	        && memcmp(GetAddr1Ptr(pframe), GET_MY_HWADDR, 6)){


	        return 0;   
	    }            
	    /* chk ssid == wildcard SSID ("DIRECT-")*/ 
	    unsigned char *SSIDIEPtr=NULL;
	    int SSIDIELen = 0;  

	    SSIDIEPtr = get_ie(IE_START_PTR, _SSID_IE_, &SSIDIELen , IE_LEN);    
	    if(SSIDIEPtr && !memcmp(SSIDIEPtr+2,"DIRECT-",7))
	    {
	      // NDEBUG("device: %02x%02x%02x:%02x%02x%02x\n",sa[0],sa[1],sa[2],sa[3],sa[4],sa[5]); 
	    }else{  
	        //NDEBUG("ignore\n");                    
	        return 0;
	    }
	    
	    /*---------chk P2P IE------*/      
	    if(p2pIEPtr2==NULL){
	        NDEBUG2("ignore\n");                    
	        return 0; 
	    }        
	    /*---------chk WPS IE------*/      
	    if(WSCIEPtr==NULL){
	        NDEBUG2("ignore\n");                    
	        return 0; 
	    } 
	    
	}
#endif
	/*cfg p2p cfg p2p*/

#ifdef WIFI_11N_2040_COEXIST
	if (priv->pmib->dot11nConfigEntry.dot11nCoexist &&
		(priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N|WIRELESS_11G)) && (
#ifdef CLIENT_MODE
		(OPMODE & WIFI_STATION_STATE) ||
#endif
		priv->pshare->is_40m_bw)) {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_,
			&len, pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p == NULL) {
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _DSSET_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if (p != NULL)
				channel = *(p+2);			
			if (OPMODE & WIFI_AP_STATE) {
				if (channel && (channel <= 14) && (priv->pmib->dot11nConfigEntry.dot11nCoexist_ch_chk ?				
				 (channel != priv->pmib->dot11RFEntry.dot11channel) : 1)) {				

					if(!priv->bg_ap_timeout) {
						priv->bg_ap_timeout = 60;
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)						
						update_RAMask_to_FW(priv, 1);
#endif
						SetTxPowerLevel(priv);
					}

					//priv->bg_ap_timeout = 60;
				}
			}
#ifdef CLIENT_MODE
			else if ((OPMODE & WIFI_STATION_STATE) && priv->coexist_connection) {
				if (channel && (channel <= 14)) {
#if 0
//#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
					if(!priv->bg_ap_timeout) {
						priv->bg_ap_timeout = 180;
						update_RAMask_to_FW(priv, 1);
					}
#endif					
					priv->bg_ap_timeout = 180;
					priv->bg_ap_timeout_ch[channel-1] = 180;
					channel = 0;
				}
			}
#endif
		}
#ifdef CLIENT_MODE
		else if ((OPMODE & WIFI_STATION_STATE) && priv->coexist_connection) {
			/*
			 *	check if there is any 40M intolerant field set by other 11n AP
			 */
			struct ht_cap_elmt *ht_cap=(struct ht_cap_elmt *)(p+2);
			if (cpu_to_le16(ht_cap->ht_cap_info) & _HTCAP_40M_INTOLERANT_)
				priv->intolerant_timeout = 180;
		}
#endif
	}
#endif

	if (priv->site_survey->count >= MAX_BSS_NUM)
		return 0;

	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _DSSET_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if (p != NULL)
		channel = *(p+2);
	else {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p !=  NULL) 
			channel = *(p+2);
		else {
			if (priv->site_survey->ss_channel > 14 && !priv->site_survey->defered_ss)
				channel = priv->site_survey->ss_channel;	
			else {
				DEBUG_INFO("Beacon/Probe rsp doesn't carry channel info\n");
				return SUCCESS;
			}
		}
	}

#if defined(CONFIG_RTL_SIMPLE_CONFIG)
		if(priv->simple_config_status == 2 || priv->simple_config_status == 3)
                {
		if(!memcmp(priv->pmib->dot11StationConfigEntry.dot11DesiredBssid, sa, MACADDRLEN)) {
			int found_target_bss = 0;
			
			for(i=0; i<priv->site_survey->count; i++) {
				if(!memcmp(priv->pmib->dot11StationConfigEntry.dot11DesiredBssid, priv->site_survey->bss[i].bssid, MACADDRLEN)) {
					found_target_bss =1;
					break;
				}
				}

			if(!found_target_bss)
				goto collect_this_bss;
                        }
                }
#endif

	for(i=0; i<priv->site_survey->count; i++) {
		if (!memcmp((void *)addr, priv->site_survey->bss[i].bssid, MACADDRLEN)) {
#if defined(CLIENT_MODE) && defined(WIFI_WMM) && defined(WMM_APSD)  //  WMM STA
			if ((OPMODE & WIFI_STATION_STATE) && QOS_ENABLE && APSD_ENABLE 
				&& (channel == priv->site_survey->bss[i].channel)) {  // get WMM IE / WMM Parameter IE
				p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;
				for (;;) {
					p = get_ie(p, _RSN_IE_1_, &len,
						pfrinfo->pktlen - (p - pframe));
					if (p != NULL) {
						if (!memcmp(p+2, WMM_PARA_IE, 6)) {
							priv->site_survey->bss[i].t_stamp[1] |= BIT(0);  //  set t_stamp[1] bit 0 when AP supports WMM

							if (*(p+8) & BIT(7))
								priv->site_survey->bss[i].t_stamp[1] |= BIT(3);  //  set t_stamp[1] bit 3 when AP supports UAPSD
							else
								priv->site_survey->bss[i].t_stamp[1] &= ~(BIT(3));  //  reset t_stamp[1] bit 3 when AP not support UAPSD
							break;
						}
					} else {
						priv->site_survey->bss[i].t_stamp[1] &= ~(BIT(0)|BIT(3));  //  reset t_stamp[1] bit 0 when AP not support WMM & UAPSD
						break;
					}
					p = p + len + 2;
				}
			}
#endif

			if ((unsigned char)pfrinfo->rssi > priv->site_survey->bss[i].rssi) {
				priv->site_survey->bss[i].rssi = (unsigned char)pfrinfo->rssi;
				#ifdef WIFI_SIMPLE_CONFIG
				priv->site_survey->wscie[i].rssi = priv->site_survey->bss[i].rssi;
				#endif
				#if defined(WIFI_WPAS) || defined(RTK_NL80211)

					priv->site_survey->wpa_ie[i].rssi = priv->site_survey->bss[i].rssi;
					priv->site_survey->rsn_ie[i].rssi = priv->site_survey->bss[i].rssi;
					priv->site_survey->rtk_p2p_ie[i].rssi = priv->site_survey->bss[i].rssi;                    

				#endif
				if (channel == priv->site_survey->bss[i].channel) {
					if ((unsigned char)pfrinfo->sq > priv->site_survey->bss[i].sq)
						priv->site_survey->bss[i].sq = (unsigned char)pfrinfo->sq;
				} else {
					priv->site_survey->bss[i].channel = channel;
					priv->site_survey->bss[i].sq = (unsigned char)pfrinfo->sq;
				}
			}
			return SUCCESS;
		}
	}

	// checking SSID
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SSID_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

	if ((p == NULL) ||		// NULL AP case 1
		(len == 0) ||		// NULL AP case 2
		(*(p+2) == '\0'))	// NULL AP case 3 (like 8181/8186)
	{
		if (priv->ss_req_ongoing && pmib->miscEntry.show_hidden_bss)
			hiddenAP = 1;
		else if (priv->auto_channel == 1)
			hiddenAP = 1;
#ifdef CONFIG_RTK_MESH        
		else if(pfrinfo->is_11s)
			hiddenAP = 1;
#endif        
		else {
#ifdef CLIENT_MODE
			if ((OPMODE & WIFI_STATION_STATE) && 
		    //!priv->ss_req_ongoing &&	//20131218 , mark it for hidden + dfs ch AP connection
			//		!priv->auto_channel && 
			is_passive_channel(priv , priv->pmib->dot11StationConfigEntry.dot11RegDomain, priv->site_survey->ss_channel)) 
			{
                /*For eg: ch60 now and hidden_ap_found=1 ; next time ss_timer will keep at ch60=> issue probe_req; next time ss_timer change to ch64 and on going...                 
				hidden_ap_found status machine as below
                            1) 0->1,1st ch60
                            next time
                            2)1->2, 2nd ch 60, and don't 2->1(chk in here) else will bring loop,ch 60,60,60,60,60......util hidden AP gone
                            */
                if(	priv->site_survey->hidden_ap_found != HIDE_AP_FOUND_DO_ACTIVE_SSAN){
	                //STADEBUG("hidden_ap_found=1\n");
					priv->site_survey->hidden_ap_found = HIDE_AP_FOUND;
				}
			}
#endif	
			DEBUG_INFO("drop beacon/probersp due to null ssid\n");
			return 0;
		}
	}

	// if scan specific SSID
	if (priv->ss_ssidlen > 0) 
    {
        /*when multiProfile enable,we send probe_req to all dev on profiles list , so chk if ssid match with ssid on profiles list*/
#ifdef SUPPORT_MULTI_PROFILE		
		if (priv->pmib->ap_profile.enable_profile && priv->pmib->ap_profile.profile_num > 0) {
			found = 0;			
			for(jdx=0;jdx<priv->pmib->ap_profile.profile_num;jdx++) {									
				ssid_len = strlen(priv->pmib->ap_profile.profile[jdx].ssid);
				if ((ssid_len == len) && !memcmp(priv->pmib->ap_profile.profile[jdx].ssid, p+2, len)) {
					//STADEBUG("Found ssid=%s,jdx=%d\n", priv->pmib->ap_profile.profile[jdx].ssid , jdx);
					found = 1;
                    break;
                }
			}			
            
			if(found == 0)
				return 0;
		}
		else	
#endif
		if ((priv->ss_ssidlen != len) || memcmp(priv->ss_ssid, p+2, len)){
			/*cfg p2p cfg p2p*/
#if defined(P2P_SUPPORT) && defined(RTK_NL80211)    
			//CFG80211_P2P [1] case: both dev,GC run collect_bss_info()
			//PROPERTY_P2P [2] case: dev run p2p_collect_bss_info, others run this function
            if(rtk_p2p_is_enabled(priv)){   // under p2p mode when ssid = DIRECT-* , don't ignore, has checked above
				// let it go on
            }else
#endif
			{
                NDEBUG("ignore!\n");
    			return 0;
            }
        }
	}

#ifdef CLIENT_MODE
// mantis#2523
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SSID_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

	if( p && (SSID_LEN == len) && !memcmp(SSID, p+2, len)) {
		memcpy(priv->rx_timestamp, pframe+WLAN_HDR_A3_LEN, 8);
	}	
#endif

	//printk("priv->ss_ssid = %s, priv->ss_ssidlen=%d\n", priv->ss_ssid, priv->ss_ssidlen);

	// if scan specific SSID && WPA2 enabled
	if (priv->ss_ssidlen > 0) {
		// search WPA2 IE
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _RSN_IE_2_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p != NULL) {
			// RSN IE
			// 0	1	23	4567
			// ID	Len	Versin	GroupCipherSuite
#ifdef	CONFIG_IEEE80211W_CLI
			if((2 == WPA_GET_LE16(p+14))){ //AKM num >= 2	
				if(!memcmp((p+20), SHA256_AKM_SUITE, sizeof(SHA256_AKM_SUITE))){
					priv->support_sha256 = TRUE;
					add_sha256_akm(priv);
				}
			}else if(!memcmp((p+16), SHA256_AKM_SUITE, sizeof(SHA256_AKM_SUITE))){
				priv->support_sha256 = TRUE;
				add_sha256_akm(priv);
			}else
				priv->support_sha256 = FALSE;
#endif

			if ((len > 7) && (pmib->dot11RsnIE.rsnie[0] == _RSN_IE_2_) &&
					(pmib->dot11RsnIE.rsnie[7] != *(p+7)) &&
					!memcmp((p + 4), OUI2, 3)) {
				// set WPA2 Multicast Cipher as same as AP's
				//printk("WPA2 Multicast Cipher = %d\n", *(p+7));
				pmib->dot11RsnIE.rsnie[7] = *(p+7);
			}
#ifndef WITHOUT_ENQUEUE
			wpa2MulticastCipher.EventId = DOT11_EVENT_WPA2_MULTICAST_CIPHER;
			wpa2MulticastCipher.IsMoreEvent = 0;
			wpa2MulticastCipher.MulticastCipher = *(p+7);
			DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&wpa2MulticastCipher,
						sizeof(DOT11_WPA2_MULTICAST_CIPHER));
#endif
			event_indicate(priv, NULL, -1);

		}
	}

	// david, reported multicast cipher suite for WPA
	// if scan specific SSID && WPA2 enabled
	if (priv->ss_ssidlen > 0) {
		// search WPA IE, should skip that not real RSNIE (eg. Asus WL500g-Deluxe)
		p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;
		len = 0;
		do {
			p = get_ie(p, _RSN_IE_1_, &len,
				pfrinfo->pktlen - (p - pframe));
			if (p != NULL) {
				if ((len > 11) && (pmib->dot11RsnIE.rsnie[0] == _RSN_IE_1_) &&
						(pmib->dot11RsnIE.rsnie[11] != *(p+11)) &&
						!memcmp((p + 2), OUI1, 3) &&
						(*(p + 5) == 0x01)) {
					// set WPA Multicast Cipher as same as AP's
					pmib->dot11RsnIE.rsnie[11] = *(p+11);

#ifndef WITHOUT_ENQUEUE
					wpaMulticastCipher.EventId = DOT11_EVENT_WPA_MULTICAST_CIPHER;
					wpaMulticastCipher.IsMoreEvent = 0;
					wpaMulticastCipher.MulticastCipher = *(p+11);
					DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&wpaMulticastCipher,
							sizeof(DOT11_WPA_MULTICAST_CIPHER));
#endif
					event_indicate(priv, NULL, -1);
				}
			}
			if (p != NULL)
				p = p + 2 + len;
		} while (p != NULL);
	}

	for(i=0; i<priv->available_chnl_num; i++) {
		if (channel == priv->available_chnl[i])
			break;
	}
	if (i == priv->available_chnl_num)	// receive the adjacent channel that is not our domain
		return 0;

	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SUPPORTEDRATES_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if (p != NULL) {
		for(i=0; i<len; i++) {
			if (p[2+i] & 0x80)
				basicrate |= get_bit_value_from_ieee_value(p[2+i] & 0x7f);
			supportrate |= get_bit_value_from_ieee_value(p[2+i] & 0x7f);
		}
	}

	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if (p != NULL) {
		for(i=0; i<len; i++) {
			if (p[2+i] & 0x80)
				basicrate |= get_bit_value_from_ieee_value(p[2+i] & 0x7f);
			supportrate |= get_bit_value_from_ieee_value(p[2+i] & 0x7f);
		}
	}

	if (channel <= 14)
	{
#ifdef P2P_SUPPORT
			/*cfg p2p cfg p2p*/
		if(rtk_p2p_is_enabled(priv)){
			/*under P2P mode allow no support B rate*/ 
		}
		else
#endif
		if (!(pmib->dot11BssType.net_work_type & WIRELESS_11B)){
			if (((basicrate & 0xff0) == 0) && ((supportrate & 0xff0) == 0)){
				return 0;
			}
		}

		if (!(pmib->dot11BssType.net_work_type & WIRELESS_11G)){
			if (((basicrate & 0xf) == 0) && ((supportrate & 0xf) == 0)){
				return 0;
			}
		}
	}
#ifdef P2P_SUPPORT	
    if(rtk_p2p_is_enabled(priv) && rtk_p2p_chk_role(priv,P2P_PRE_CLIENT)){
        p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_ ;
        
        /*support ReAssemble*/  
        for (;;){
            
            p = get_ie(p, _P2P_IE_, &len,   
                pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_ - len);              
            if (p) {
                if (!memcmp(p+2, WFA_OUI_PLUS_TYPE, 4)) {
                    IEfoundtimes ++;
                    memcpy(p2pIEPtr , p+2+4 ,len-4);
                    p2pIEPtr+= (len-4);             
                }
            }
            else{
                break;
            }
            p = p + len + 2;
            
        }
        
        if(IEfoundtimes){
            if(IEfoundtimes>1){
                P2P_DEBUG("ReAssembly p2p IE\n");
            }       
            p2pIElen = (int)(((unsigned long)p2pIEPtr)-((unsigned long)ReAssem_p2pie));     
        
            if(p2pIElen > MAX_REASSEM_P2P_IE){
                P2P_DEBUG("\n\n reassemble P2P IE exceed MAX_REASSEM_P2P_IE , chk!!!\n\n");
                return 0;           
            }else{
                /*if target AP support management function ; skip it*/
                if(P2P_filter_manage_ap(priv , ReAssem_p2pie , p2pIElen )){
                    return 0;
                }
            }
        }

    }	
#endif  


#if defined(CONFIG_RTL_SIMPLE_CONFIG)
collect_this_bss:
#endif
	/*
	 * okay, recording this bss...
	 */
	index = priv->site_survey->count;
	priv->site_survey->count++;

	memcpy(priv->site_survey->bss[index].bssid, addr, MACADDRLEN);

	if (hiddenAP) {
		priv->site_survey->bss[index].ssidlen = 0;
		memset((void *)(priv->site_survey->bss[index].ssid),0, 32);
	}
	else {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SSID_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		priv->site_survey->bss[index].ssidlen = len;
		memcpy((void *)(priv->site_survey->bss[index].ssid), (void *)(p+2), len);
        /*add for sorting by profile */
        #ifdef WIFI_SIMPLE_CONFIG
		memcpy((void *)(priv->site_survey->wscie[index].ssid), (void *)(p+2), len);        
        #endif
    
		#if defined(WIFI_WPAS) || defined(RTK_NL80211)
			memcpy((void *)(priv->site_survey->wpa_ie[index].ssid), (void *)(p+2), len);        
			memcpy((void *)(priv->site_survey->rsn_ie[index].ssid), (void *)(p+2), len);                
			memcpy((void *)(priv->site_survey->rtk_p2p_ie[index].ssid), (void *)(p+2), len);                            
        #endif
	}
#ifdef CONFIG_RTK_MESH
	// GANTOE for site survey 2008/12/25 ====
	//Mesh ID
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _MESH_ID_IE_, (int *)&len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if(NULL == p)
	{
		priv->site_survey->bss[index].meshidlen = 0;
		priv->site_survey->bss[index].meshid[0] = '\0';
	}
	else
	{
		priv->site_survey->bss[index].meshidlen = (len > MESH_ID_LEN ? MESH_ID_LEN : len);
		memcpy((void *)(priv->site_survey->bss[index].meshid), (void *)(p + 2), priv->site_survey->bss[index].meshidlen);
	}
#endif

	// we use t_stamp to carry other info so don't get timestamp here
#if 0
	memcpy(&val32, (pframe + WLAN_HDR_A3_LEN), 4);
	priv->site_survey->bss[index].t_stamp[0] = le32_to_cpu(val32);

	memcpy(&val32, (pframe + WLAN_HDR_A3_LEN + 4), 4);
	priv->site_survey->bss[index].t_stamp[1] = le32_to_cpu(val32);
#endif

	memcpy(&val16, (pframe + WLAN_HDR_A3_LEN + 8 ), 2);
	priv->site_survey->bss[index].beacon_prd = le16_to_cpu(val16);

	memcpy(&val16, (pframe + WLAN_HDR_A3_LEN + 8 + 2), 2);
	priv->site_survey->bss[index].capability = le16_to_cpu(val16);

	if ((priv->site_survey->bss[index].capability & BIT(0)) &&
		!(priv->site_survey->bss[index].capability & BIT(1)))
		priv->site_survey->bss[index].bsstype = WIFI_AP_STATE;
	else if (!(priv->site_survey->bss[index].capability & BIT(0)) &&
		(priv->site_survey->bss[index].capability & BIT(1)))
		priv->site_survey->bss[index].bsstype = WIFI_ADHOC_STATE;
	else
		priv->site_survey->bss[index].bsstype = 0;

	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _TIM_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if (p != NULL)
		priv->site_survey->bss[index].dtim_prd = *(p+3);

	priv->site_survey->bss[index].channel = channel;
	priv->site_survey->bss[index].basicrate = basicrate;
	priv->site_survey->bss[index].supportrate = supportrate;

	memcpy(priv->site_survey->bss[index].bdsa, sa, MACADDRLEN);

	priv->site_survey->bss[index].rssi = (unsigned char)pfrinfo->rssi;
	priv->site_survey->bss[index].sq = (unsigned char)pfrinfo->sq;

#ifdef WIFI_SIMPLE_CONFIG
	priv->site_survey->wscie[index].rssi = priv->site_survey->bss[index].rssi;
#endif
	#if defined(WIFI_WPAS) || defined(RTK_NL80211)
		priv->site_survey->wpa_ie[index].rssi = priv->site_survey->bss[index].rssi;	
		priv->site_survey->rsn_ie[index].rssi = priv->site_survey->bss[index].rssi;
		priv->site_survey->rtk_p2p_ie[index].rssi = priv->site_survey->bss[index].rssi;
	#endif

	if (channel >= 36)
		priv->site_survey->bss[index].network |= WIRELESS_11A;
	else {
		if ((basicrate & 0xff0) || (supportrate & 0xff0))
			priv->site_survey->bss[index].network |= WIRELESS_11G;
		if ((basicrate & 0xf) || (supportrate & 0xf))
			priv->site_survey->bss[index].network |= WIRELESS_11B;
	}

	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if (p !=  NULL) {
		struct ht_cap_elmt *ht_cap=(struct ht_cap_elmt *)(p+2);
		if (cpu_to_le16(ht_cap->ht_cap_info) & _HTCAP_SUPPORT_CH_WDTH_)
			priv->site_survey->bss[index].t_stamp[1] |= BIT(1);
		else
			priv->site_survey->bss[index].t_stamp[1] &= ~(BIT(1));
		priv->site_survey->bss[index].network |= WIRELESS_11N;
	} else {
		priv->site_survey->bss[index].t_stamp[1] &= ~(BIT(1));
	}
#ifdef RTK_AC_SUPPORT
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTCapability, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if ((p !=  NULL) && (len <= sizeof(struct vht_cap_elmt))) {
		priv->site_survey->bss[index].network |= WIRELESS_11AC;
	}

	//Check if 80M AP
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTOperation, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

	priv->site_survey->bss[index].t_stamp[1] &= ~(BSS_BW_MASK << BSS_BW_SHIFT);

	if ((p !=  NULL) && (len <= sizeof(struct vht_oper_elmt))) {					
		if (p[2] == 1) {
			priv->site_survey->bss[index].t_stamp[1] |= (HT_CHANNEL_WIDTH_AC_80 << BSS_BW_SHIFT);
		}
	}
#endif
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

	if (p !=  NULL) {
		struct ht_info_elmt *ht_info=(struct ht_info_elmt *)(p+2);
		if (!(ht_info->info0 & _HTIE_STA_CH_WDTH_))
			priv->site_survey->bss[index].t_stamp[1] &= ~(BIT(1)|BIT(2));
		else {
			if ((ht_info->info0 & _HTIE_2NDCH_OFFSET_BL_) == _HTIE_2NDCH_OFFSET_NO_)
				priv->site_survey->bss[index].t_stamp[1] &= ~(BIT(1)|BIT(2));
			else
			if ((ht_info->info0 & _HTIE_2NDCH_OFFSET_BL_) == _HTIE_2NDCH_OFFSET_BL_)
				priv->site_survey->bss[index].t_stamp[1] |= BIT(2);
			else
				priv->site_survey->bss[index].t_stamp[1] &= ~(BIT(2));
		}
	}
	else
		priv->site_survey->bss[index].t_stamp[1] &= ~(BIT(1)|BIT(2));

	// get WPA/WPA2 information
	get_security_info(priv, pfrinfo, index);

	// Parse Multi Stage Element
	p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_; len = 0;
	for (;;)
	{
		unsigned char oui[] = { 0x00, 0x0d, 0x02 };
		unsigned char oui_type =7, ver1 = 0x01, ver2 = 0x00;
		p = get_ie(p, _RSN_IE_1_, &len,
			pfrinfo->pktlen - (p - pframe));
		if (p != NULL) {
			if (!memcmp(p+2, oui, 3) && (*(p+2+3) == oui_type)) {
				if ( (*(p+2+4) == ver1) && (*(p+2+5) == ver2))
				{	
					switch(*(p+2+6)) {
						case 0x80: priv->site_survey->bss[index].stage = 1; 
									break;
						case 0x40: priv->site_survey->bss[index].stage = 2;
									break;						
						case 0x20: priv->site_survey->bss[index].stage = 3; 
									break;
						case 0x10: priv->site_survey->bss[index].stage = 4; 
									break;
						case 0x08: priv->site_survey->bss[index].stage = 5; 
									break;
						default:
							priv->site_survey->bss[index].stage = 0;		
					};
				} else {
					priv->site_survey->bss[index].stage = 0;
				}
				break;
			}
		}
		else
			break;
		p = p + len + 2;
	}

    #if defined( P2P_SUPPORT	) && defined(RTK_NL80211)
    /*Get P2P IE*/ 
    if(rtk_p2p_is_enabled(priv)){
        priv->site_survey->rtk_p2p_ie[index].p2p_ie_len=0;

        if(p2pIEPtr){
           memcpy(priv->site_survey->rtk_p2p_ie[index].data , p2pIEPtr , p2pIElen); 
           
           priv->site_survey->rtk_p2p_ie[index].p2p_ie_len = p2pIElen;
           
           NDEBUG2("P2P IE LEN[%d],index[%d]\n",p2pIElen,index);
        }
    }	
    #endif
			/*cfg p2p*/
#ifdef WDS
	if (priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsNum) {
		// look for ERP rate. if no ERP rate existed, thought it is a legacy AP
		unsigned char supportedRates[32];
		int supplen=0;

		struct stat_info *pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
		if (pstat && (pstat->state & WIFI_WDS)) {
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_,
					_SUPPORTEDRATES_IE_, &len,
					pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if (p) {
				if (len>8)
					len=8;
				memcpy(&supportedRates[supplen], p+2, len);
				supplen += len;
			}

			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_,
					_EXT_SUPPORTEDRATES_IE_, &len,
					pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if (p) {
				if (len>8)
					len=8;
				memcpy(&supportedRates[supplen], p+2, len);
				supplen += len;
			}

			get_matched_rate(priv, supportedRates, &supplen, 0);
			update_support_rate(pstat, supportedRates, supplen);
			if (supplen == 0)
				pstat->current_tx_rate = 0;
			else {
				if (priv->pmib->dot11WdsInfo.entry[pstat->wds_idx].txRate == 0) {
					pstat->current_tx_rate = find_rate(priv, pstat, 1, 0);
					//pstat->upper_tx_rate = 0;	// unused
				}
			}

			// Customer proprietary IE
			if (priv->pmib->miscEntry.private_ie_len) {
				p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_,
					priv->pmib->miscEntry.private_ie[0], &len,
					pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
				if (p) {
					memcpy(pstat->private_ie, p, len + 2);
					pstat->private_ie_len = len + 2;
				}
			}

			// Realtek proprietary IE
			pstat->is_realtek_sta = FALSE;
			pstat->IOTPeer = HT_IOT_PEER_UNKNOWN;			
			p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_; len = 0;
			for (;;) {
				p = get_ie(p, _RSN_IE_1_, &len,
					pfrinfo->pktlen - (p - pframe));
				if (p != NULL) {
					if (!memcmp(p+2, Realtek_OUI, 3) && *(p+2+3) == 2) { /*found realtek out and type == 2*/
                        pstat->is_realtek_sta = TRUE;
                        pstat->IOTPeer = HT_IOT_PEER_REALTEK;

                        if (*(p+2+3+2) & RTK_CAP_IE_AP_CLIENT)
                            pstat->IOTPeer = HT_IOT_PEER_RTK_APCLIENT;

                        if(*(p+2+3+2) & RTK_CAP_IE_WLAN_8192SE) 					
                            pstat->IOTPeer = HT_IOT_PEER_REALTEK_92SE;

                        if (*(p+2+3+2) & RTK_CAP_IE_USE_AMPDU)
                            pstat->is_forced_ampdu = TRUE;
                        else
                            pstat->is_forced_ampdu = FALSE;
#ifdef RTK_WOW
                        if (*(p+2+3+2) & RTK_CAP_IE_USE_WOW)						
                            pstat->IOTPeer = HT_IOT_PEER_REALTEK_WOW;			
#endif
                        if (*(p+2+3+2) & RTK_CAP_IE_WLAN_88C92C)						
                            pstat->IOTPeer = HT_IOT_PEER_REALTEK_81XX;

                        if (*(p+2+3+3) & ( RTK_CAP_IE_8812_BCUT | RTK_CAP_IE_8812_CCUT))						
                            pstat->IOTPeer = HT_IOT_PEER_REALTEK_8812;		
						break;
					}
				}
				else
					break;

				p = p + len + 2;
			}

#ifdef WIFI_WMM
			if (QOS_ENABLE) {
				p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;
				for (;;) {
					p = get_ie(p, _RSN_IE_1_, &len,
							pfrinfo->pktlen - (p - pframe));
					if (p != NULL) {
						if (!memcmp(p+2, WMM_PARA_IE, 6)) {
							pstat->QosEnabled = 1;
							break;
						}
					}
					else {
						pstat->QosEnabled = 0;
						break;
					}
					p = p + len + 2;
				}
			}
#endif
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
				p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_, &len,
					pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
				if (p !=  NULL) {
					pstat->ht_cap_len = len;
					memcpy((unsigned char *)&pstat->ht_cap_buf, p+2, len);
					if (cpu_to_le16(pstat->ht_cap_buf.ht_cap_info) & _HTCAP_AMSDU_LEN_8K_) {
						pstat->is_8k_amsdu = 1;
						pstat->amsdu_level = 7935 - sizeof(struct wlan_hdr);
					}
					else {
						pstat->is_8k_amsdu = 0;
						pstat->amsdu_level = 3839 - sizeof(struct wlan_hdr);
					}
				}
				else
					pstat->ht_cap_len = 0;
			}
			if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_)){
				pstat->tx_bw = HT_CHANNEL_WIDTH_20_40;
			}else{
				pstat->tx_bw = HT_CHANNEL_WIDTH_20;
			}
#ifdef RTK_AC_SUPPORT
		//WDS-VHT support
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
			{
				p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTCapability, &len,
						pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

				if ((p !=  NULL) && (len <= sizeof(struct vht_cap_elmt))) {
					pstat->vht_cap_len = len;
					memcpy((unsigned char *)&pstat->vht_cap_buf, p+2, len);
					//WDEBUG("vht_cap len = %d \n",len);		
					//WDEBUG("vht_cap vht_cap_info:%04X \n",pstat->vht_cap_buf.vht_cap_info);	
					//WDEBUG("vht_cap vht_support_mcs[0]=[%04X] ; vht_support_mcs[1]=[%04X] \n", pstat->vht_cap_buf.vht_support_mcs[0],pstat->vht_cap_buf.vht_support_mcs[1]);					
				}

				p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTOperation, &len,
						pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

				if ((p !=  NULL) && (len <= sizeof(struct vht_oper_elmt))) {
					pstat->vht_oper_len = len;
					memcpy((unsigned char *)&pstat->vht_oper_buf, p+2, len);
					//WDEBUG("vht_oper len = %d \n",len);
					//WDEBUG("vht_oper: 0[%02x],1[%02x],2[%02x] \n",pstat->vht_oper_buf.vht_oper_info[0],pstat->vht_oper_buf.vht_oper_info[1],pstat->vht_oper_buf.vht_oper_info[2]);						
					//WDEBUG("vht_oper [%02X]\n",pstat->vht_oper_buf.vht_basic_msc);					
				}


				if ((pstat->vht_cap_len))	{
					switch(cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & 0x3) {
						default:
						case 0:	
							pstat->is_8k_amsdu = 0;
							pstat->amsdu_level = 3895 - sizeof(struct wlan_hdr);
							break;
						case 1:
							pstat->is_8k_amsdu = 1;
							pstat->amsdu_level = 7991 - sizeof(struct wlan_hdr);
							break;
						case 2:	
							pstat->is_8k_amsdu = 1;
							pstat->amsdu_level = 11454 - sizeof(struct wlan_hdr);	
							break;
					}

					if (priv->vht_oper_buf.vht_oper_info[0] == 1) {
						pstat->tx_bw = HT_CHANNEL_WIDTH_80;
						priv->pshare->is_40m_bw	= HT_CHANNEL_WIDTH_80;	
						//WDEBUG("pstat->tx_bw=80M\n");
					}
				}

				p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTOperatingMode, &len,
					pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
				if ((p !=  NULL) && (len == 1)) {
						// check self capability....					
						if((p[2] &3) <= priv->pmib->dot11nConfigEntry.dot11nUse40M)
							pstat->tx_bw = p[2] &3;
						pstat->nss = ((p[2]>>4)&0x7)+1;											
						printk("[%s %d]receive opering mode data = %02X \n",__FUNCTION__,__LINE__ ,p[2]);
				}				
			
			}
#endif			
#ifdef CONFIG_RTL_8812_SUPPORT
			if(GET_CHIP_VER(priv)== VERSION_8812E){
				UpdateHalRAMask8812(priv, pstat, 3);
				//WDEBUG("UpdateHalRAMask8812\n");
				UpdateHalMSRRPT8812(priv, pstat, INCREASE);			
				//WDEBUG("UpdateHalMSRRPT8812\n");				
			}
#endif
#ifdef  CONFIG_WLAN_HAL
			if (IS_HAL_CHIP(priv)){
				GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
			}
#endif

			
		}
	}
#endif

#ifdef WIFI_WMM  //  WMM STA
	if (QOS_ENABLE) {  // get WMM IE / WMM Parameter IE
		p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;
		for (;;) {
			p = get_ie(p, _RSN_IE_1_, &len,
				pfrinfo->pktlen - (p - pframe));
			if (p != NULL) {
				if ((!memcmp(p+2, WMM_IE, 6)) || (!memcmp(p+2, WMM_PARA_IE, 6))) {
					priv->site_survey->bss[index].t_stamp[1] |= BIT(0);  //  set t_stamp[1] bit 0 when AP supports WMM
#if defined(CLIENT_MODE) && defined(WMM_APSD)
					if ((OPMODE & WIFI_STATION_STATE) && APSD_ENABLE) {
						if (!memcmp(p+2, WMM_PARA_IE, 6)) {
							if (*(p+8) & BIT(7))
								priv->site_survey->bss[index].t_stamp[1] |= BIT(3);  //  set t_stamp[1] bit 3 when AP supports UAPSD
							else
								priv->site_survey->bss[index].t_stamp[1] &= ~(BIT(3));  //  reset t_stamp[1] bit 3 when AP not support UAPSD
							break;
						} else {
							priv->site_survey->bss[index].t_stamp[1] &= ~(BIT(3));  //  reset t_stamp[1] bit 3 when AP not support UAPSD
						}
					} else
#endif
						break;
				}
			} else {
				priv->site_survey->bss[index].t_stamp[1] &= ~(BIT(0));  //  reset t_stamp[1] bit 0 when AP not support WMM
#if defined(CLIENT_MODE) && defined(WMM_APSD)
				if ((OPMODE & WIFI_STATION_STATE) && APSD_ENABLE)
					priv->site_survey->bss[index].t_stamp[1] &= ~(BIT(3));  //  reset t_stamp[1] bit 3 when AP not support UAPSD
#endif
				break;
			}
			p = p + len + 2;
		}
	}
#endif

#if defined(WIFI_WPAS) || defined(RTK_NL80211)
			priv->site_survey->bss[index].rsn_ie_len = 0;			/*cfg p2p cfg p2p*/
			priv->site_survey->bss[index].wpa_ie_len = 0;			/*cfg p2p cfg p2p*/

			p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;
			p = get_ie(p, _RSN_IE_2_, &len,
				pfrinfo->pktlen - (p - pframe));
			if ((p != NULL) && (len > 7)) {

                priv->site_survey->bss[index].rsn_ie_len = len + 2;			/*cfg p2p cfg p2p*/
                memcpy(priv->site_survey->bss[index].rsn_ie , p, len + 2);			/*cfg p2p cfg p2p*/

			}			

			len = 0;
			p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;
			do {
				p = get_ie(p, _RSN_IE_1_, &len,
					pfrinfo->pktlen - (p - pframe));
				if ((p != NULL) && (len > 11) 			/*cfg p2p cfg p2p*/
					 && (!memcmp((p + 2), OUI1, 4)) ) {
                    priv->site_survey->bss[index].wpa_ie_len = len+2;
					memcpy(priv->site_survey->bss[index].wpa_ie, p, len + 2);
				}			/*cfg p2p cfg p2p*/
				if (p != NULL)
					p = p + 2 + len;
			} while (p != NULL);

#endif


#if defined(CONFIG_RTL_SIMPLE_CONFIG)
	{
			if(!memcmp(priv->pmib->dot11StationConfigEntry.dot11DesiredBssid, sa, 6)) {
				int bss_bw = HT_CHANNEL_WIDTH_20_40;
				int bss_offset = 0;			
	
#ifdef RTK_AC_SUPPORT			
				if(GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A){
					if((priv->site_survey->bss[index].t_stamp[1] & (BSS_BW_MASK << BSS_BW_SHIFT)) 
					== (HT_CHANNEL_WIDTH_80 << BSS_BW_SHIFT))
					bss_bw = HT_CHANNEL_WIDTH_80;									
				}
#endif
	
				if ((priv->site_survey->bss[index].t_stamp[1] & (BIT(1) | BIT(2))) == (BIT(1) | BIT(2)))
					bss_offset = HT_2NDCH_OFFSET_BELOW;
				else if ((priv->site_survey->bss[index].t_stamp[1] & (BIT(1) | BIT(2))) == BIT(1))
					bss_offset = HT_2NDCH_OFFSET_ABOVE;
				else { 
					if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_10)
						bss_bw = HT_CHANNEL_WIDTH_10;
					else if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_5)
						bss_bw = HT_CHANNEL_WIDTH_5;
					else
						bss_bw = HT_CHANNEL_WIDTH_20;	
			
					bss_offset = HT_2NDCH_OFFSET_DONTCARE;
				}
	
		
				if(channel){
					
					if(priv->simple_config_could_fix)
					if(priv->simple_config_status == 2 || priv->simple_config_status == 3) {
							
						if(priv->pmib->dot11StationConfigEntry.sc_fix_channel == 0) {
							if(priv->pmib->dot11StationConfigEntry.sc_debug)
								panic_printk("Simple Config switch to remote AP's channel %d BW %d Offset %d\n", 
								channel, bss_bw, bss_offset);
											
							//sync channel
   							priv->pmib->dot11RFEntry.dot11channel = channel;

    						//sync bw
    						priv->pmib->dot11nConfigEntry.dot11nUse40M = bss_bw;
   							priv->pshare->CurrentChannelBW = bss_bw;
    						priv->pshare->is_40m_bw = bss_bw; 

    						//sync offset
    						priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = bss_offset;
    						priv->pshare->offset_2nd_chan =  bss_offset;					

							SwBWMode(priv, bss_bw, bss_offset);
							SwChnl(priv, channel, bss_offset);
							
							rtk_sc_set_value(SC_FIX_CHANNEL, channel);
							priv->pmib->dot11StationConfigEntry.sc_fix_bw = bss_bw;
							priv->pmib->dot11StationConfigEntry.sc_fix_offset = bss_offset;
							
						}
										
						if(priv->simple_config_status == 3)
							rtk_sc_check_security(priv, &(priv->site_survey->bss[index]));
												
						return SUCCESS;
					}
		
				}
				
			}
			
	}
#endif


	return SUCCESS;
}


void assign_tx_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo)
{
	int tx_rate=0;
	UINT8 rate;
	int auto_rate;

#ifdef WDS
	if (pstat->state & WIFI_WDS) {
		auto_rate =	(priv->pmib->dot11WdsInfo.entry[pstat->wds_idx].txRate==0) ? 1: 0;
		tx_rate = priv->pmib->dot11WdsInfo.entry[pstat->wds_idx].txRate;
	}
	else
#endif
	{
		auto_rate = priv->pmib->dot11StationConfigEntry.autoRate;
		tx_rate = priv->pmib->dot11StationConfigEntry.fixedTxRate;
	}

	if (auto_rate || 
#ifdef RTK_AC_SUPPORT
		( is_fixedVHTTxRate(priv, pstat) && !(pstat->vht_cap_len)) ||
#endif
		( should_restrict_Nrate(priv, pstat) && is_fixedMCSTxRate(priv, pstat))) {
#if 0
		// if auto rate, select highest or lowest rate depending on rssi
		if (pfrinfo && pfrinfo->rssi > 30)
			pstat->current_tx_rate = find_rate(priv, pstat, 1, 0);
		else
			pstat->current_tx_rate = find_rate(priv, pstat, 0, 0);
#endif
		pstat->current_tx_rate = find_rate(priv, pstat, 1, 0);

	}
	else {
		// see if current fixed tx rate of mib is existed in supported rates set
		rate = get_rate_from_bit_value(tx_rate);
		if (match_supp_rate(pstat->bssrateset, pstat->bssratelen, rate))
			tx_rate = (int)rate;
		if (tx_rate == 0) // if not found, use highest supported rate for current tx rate
			tx_rate = find_rate(priv, pstat, 1, 0);

#if	defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE)
		if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A)||(GET_CHIP_VER(priv)== VERSION_8814A)){
			pstat->current_tx_rate = rate;
		}else
#endif
		{
			pstat->current_tx_rate = tx_rate;
		}
// ToDo: 
// fixed 2T rate, but STA is 1R...		
	}

	if ((pstat->MIMO_ps & _HT_MIMO_PS_STATIC_) && is_2T_rate(pstat->current_tx_rate)) {
#ifdef RTK_AC_SUPPORT
		if( is_VHT_rate(pstat->current_tx_rate)){
			pstat->current_tx_rate = _NSS1_MCS9_RATE_;
		}else
#endif
		{
			pstat->current_tx_rate = _MCS7_RATE_;	// when HT MIMO Static power save is set and rate > MCS7, fix rate to MCS7
		}
	}


	if (pfrinfo)
		pstat->rssi = pfrinfo->rssi;	// give the initial value to pstat->rssi

	pstat->ht_current_tx_info = 0;
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat->ht_cap_len) {
		if (priv->pshare->is_40m_bw && ((pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)||(pstat->tx_bw == HT_CHANNEL_WIDTH_80))) {
			pstat->ht_current_tx_info |= TX_USE_40M_MODE;
			if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M &&
				(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_)))
				pstat->ht_current_tx_info |= TX_USE_SHORT_GI;
		}
		else {
			if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M &&
				(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_)))
				pstat->ht_current_tx_info |= TX_USE_SHORT_GI;
		}
	}

	if (priv->pshare->rf_ft_var.rssi_dump && pfrinfo)
	{
#ifdef  RTK_AC_SUPPORT	
		if(is_VHT_rate(pstat->current_tx_rate))
			printk("[%d] rssi=%d%% assign rate %s%d %d\n", pstat->aid, pfrinfo->rssi,
			"VHT NSS", (((pstat->current_tx_rate-VHT_RATE_ID)/10)+1), ((pstat->current_tx_rate-VHT_RATE_ID)%10));
		else
#endif
		printk("[%d] rssi=%d%% assign rate %s%d\n", pstat->aid, pfrinfo->rssi,
			is_MCS_rate(pstat->current_tx_rate)? "MCS" : "",
			is_MCS_rate(pstat->current_tx_rate)? (pstat->current_tx_rate-HT_RATE_ID) : pstat->current_tx_rate/2);
	}
}


// Assign aggregation method automatically.
// We according to the following rule:
// 1. Rtl8190: AMPDU
// 2. Broadcom: AMSDU
// 3. Station who supports only 4K AMSDU receiving: AMPDU
// 4. Others: AMSDU
void assign_aggre_mthod(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat->ht_cap_len) {
#ifdef RTK_AC_SUPPORT //for 11ac logo		
		if ((AMPDU_ENABLE) && (AMSDU_ENABLE >= 2)) {
#ifdef SUPPORT_TX_AMSDU
			if(pstat->AMSDU_AMPDU_support && priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G) {		
				if((pstat->aggre_mthd != AGGRE_MTHD_MPDU_AMSDU) && ((pstat->tx_avarage >> 17) > priv->pshare->rf_ft_var.amsdu_th)) {
					//printk("TP:%d Mbps --> amsdu mode\n", (pstat->tx_avarage >> 17));
					pstat->aggre_mthd = AGGRE_MTHD_MPDU_AMSDU;
				} else if((pstat->aggre_mthd == AGGRE_MTHD_MPDU_AMSDU) && ((pstat->tx_avarage >> 17) < priv->pshare->rf_ft_var.amsdu_th2)) {
					//printk("TP:%d Mbps --> ampdu mode\n", (pstat->tx_avarage >> 17));
					pstat->aggre_mthd = AGGRE_MTHD_MPDU;
				}		
			}
			else
#endif
				pstat->aggre_mthd = AGGRE_MTHD_MPDU;
		}
		else
#endif // RTK_AC_SUPPORT

		if ((AMPDU_ENABLE == 1) || (AMSDU_ENABLE == 1))		// auto assignment
			pstat->aggre_mthd = AGGRE_MTHD_MPDU;
		else if ((AMPDU_ENABLE >= 2) && (AMSDU_ENABLE == 0))
			pstat->aggre_mthd = AGGRE_MTHD_MPDU;
		else if ((AMPDU_ENABLE == 0) && (AMSDU_ENABLE >= 2))
			pstat->aggre_mthd = AGGRE_MTHD_MSDU;									//5.2.38
		else
			pstat->aggre_mthd = AGGRE_MTHD_NONE;
	}
	else
		pstat->aggre_mthd = AGGRE_MTHD_NONE;

	if (should_restrict_Nrate(priv, pstat) && (pstat->aggre_mthd != AGGRE_MTHD_NONE))
		pstat->aggre_mthd = AGGRE_MTHD_NONE;

// Client mode IOT issue, Button 2009.07.17
// we won't restrict N rate with 8190
#ifdef CLIENT_MODE
	if(OPMODE & WIFI_STATION_STATE)
	{
		if((pstat->IOTPeer !=HT_IOT_PEER_REALTEK_92SE) && pstat->is_realtek_sta && pstat->is_legacy_encrpt)
			pstat->aggre_mthd = AGGRE_MTHD_NONE;
	}
#endif

//	if(pstat->sta_in_firmware != 1 && priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _NO_PRIVACY_)
//		pstat->aggre_mthd = AGGRE_MTHD_NONE;
}


void assign_aggre_size(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat->ht_cap_len) {
		if ((priv->pmib->dot11nConfigEntry.dot11nAMPDUSendSz == 8) ||
			(priv->pmib->dot11nConfigEntry.dot11nAMPDUSendSz == 16) ||
			(priv->pmib->dot11nConfigEntry.dot11nAMPDUSendSz == 32)) {
			if (priv->pmib->dot11nConfigEntry.dot11nAMPDUSendSz == 8)
				pstat->diffAmpduSz = 0x44444444;
			else if (priv->pmib->dot11nConfigEntry.dot11nAMPDUSendSz == 16)
				pstat->diffAmpduSz = 0x88888888;
			else
				pstat->diffAmpduSz = 0xffffffff;
		} else {
			unsigned int ampdu_para = pstat->ht_cap_buf.ampdu_para & 0x03;
			pstat->diffAmpduSz = RTL_R32(AGGLEN_LMT);
			if ((!ampdu_para) || (ampdu_para == 1)) {
				if ((pstat->diffAmpduSz & 0xf) > 4*(ampdu_para+1))
					pstat->diffAmpduSz = (pstat->diffAmpduSz & ~0xf) | 0x4*(ampdu_para+1);
				if (((pstat->diffAmpduSz & 0xf0) >> 4) > 4*(ampdu_para+1))
					pstat->diffAmpduSz = (pstat->diffAmpduSz & ~0xf0) | 0x40*(ampdu_para+1);
				if (((pstat->diffAmpduSz & 0xf00) >> 8) > 4*(ampdu_para+1))
					pstat->diffAmpduSz = (pstat->diffAmpduSz & ~0xf00) | 0x400*(ampdu_para+1);
				if (((pstat->diffAmpduSz & 0xf000) >> 12) > 4*(ampdu_para+1))
					pstat->diffAmpduSz = (pstat->diffAmpduSz & ~0xf000) | 0x4000*(ampdu_para+1);
				if (((pstat->diffAmpduSz & 0xf0000) >> 16) > 4*(ampdu_para+1))
					pstat->diffAmpduSz = (pstat->diffAmpduSz & ~0xf0000) | 0x40000*(ampdu_para+1);
				if (((pstat->diffAmpduSz & 0xf00000) >> 20) > 4*(ampdu_para+1))
					pstat->diffAmpduSz = (pstat->diffAmpduSz & ~0xf00000) | 0x400000*(ampdu_para+1);
				if (((pstat->diffAmpduSz & 0xf000000) >> 24) > 4*(ampdu_para+1))
					pstat->diffAmpduSz = (pstat->diffAmpduSz & ~0xf000000) | 0x4000000*(ampdu_para+1);
				if (((pstat->diffAmpduSz & 0xf0000000) >> 28) > 4*(ampdu_para+1))
					pstat->diffAmpduSz = (pstat->diffAmpduSz & ~0xf0000000) | 0x40000000*(ampdu_para+1);
			}
		}
	 	DEBUG_INFO("assign aggregation size: %d\n", 8<<(pstat->ht_cap_buf.ampdu_para & 0x03));
	}

	//if (pstat->ht_cap_len)
	//	pstat->maxAggNum = ((1<<(pstat->ht_cap_buf.ampdu_para & 0x03))*5);
	if (pstat->ht_cap_len) {
		if((pstat->ht_cap_buf.ampdu_para & 0x03) == 3)
			pstat->maxAggNum = 42;
		else if((pstat->ht_cap_buf.ampdu_para & 0x03) == 2)
			pstat->maxAggNum = 21;
		else if((pstat->ht_cap_buf.ampdu_para & 0x03) == 1)
			pstat->maxAggNum = 10;
		else
			pstat->maxAggNum = 5;
	}
#ifdef RTK_AC_SUPPORT
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) && (pstat->vht_cap_len)) 
		pstat->maxAggNum = ((1<<((cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & 0x3800000)>>MAX_RXAMPDU_FACTOR_S))*5);
#endif	

	if(pstat->maxAggNum >= 0x3F)
		pstat->maxAggNum = 0x3F;
}

#ifdef SUPPORT_MONITOR
void rtl8192cd_chan_switch_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	
	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;
	if(priv->is_monitor_mode==TRUE)
	{
		if(((priv->chan_num%priv->available_chnl_num)==0)&&(priv->chan_num>0))
			priv->chan_num = 0;
		else
			priv->chan_num++;
	
		priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv, priv->available_chnl[priv->chan_num], priv->pshare->offset_2nd_chan);
		mod_timer(&priv->chan_switch_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pmib->miscEntry.chan_switch_time));	
	}	
}
#endif

#ifndef USE_WEP_DEFAULT_KEY
void set_keymapping_wep(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	struct wifi_mib	*pmib = GET_MIB(priv);

//	if ((GET_ROOT(priv)->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) &&
#ifdef MULTI_MAC_CLONE
	if (!IEEE8021X_FUN &&
#else
	if (!SWCRYPTO && !IEEE8021X_FUN &&
#endif
		((pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) ||
		 (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)))
	{
		pstat->dot11KeyMapping.dot11Privacy = pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
		pstat->keyid = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) {
			pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen = 5;
			memcpy(pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKey.skey,
				   pmib->dot11DefaultKeysTable.keytype[pstat->keyid].skey, 5);
		}
		else {
			pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen = 13;
			memcpy(pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKey.skey,
				   pmib->dot11DefaultKeysTable.keytype[pstat->keyid].skey, 13);
		}

		DEBUG_INFO("going to set %s unicast key for sta %02X%02X%02X%02X%02X%02X, id=%d\n",
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)?"WEP40":"WEP104",
			pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
			pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5], pstat->keyid);
		if (!SWCRYPTO) {
			int retVal;
			retVal = CamDeleteOneEntry(priv, pstat->hwaddr, pstat->keyid, 0);
			if (retVal) {
				priv->pshare->CamEntryOccupied--;
				pstat->dot11KeyMapping.keyInCam = FALSE;
			}
#ifdef MULTI_MAC_CLONE
			if ((OPMODE & WIFI_STATION_STATE) && priv->pmib->ethBrExtInfo.macclone_enable)
				retVal = CamAddOneEntry(priv, pstat->sa_addr, pstat->keyid,
					pstat->dot11KeyMapping.dot11Privacy<<2, 0, pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKey.skey);
			else			
#endif			
			retVal = CamAddOneEntry(priv, pstat->hwaddr, pstat->keyid,
				pstat->dot11KeyMapping.dot11Privacy<<2, 0, pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKey.skey);
			if (retVal) {
				priv->pshare->CamEntryOccupied++;
				pstat->dot11KeyMapping.keyInCam = TRUE;
			}
			else {
				if (pstat->aggre_mthd != AGGRE_MTHD_NONE)
					pstat->aggre_mthd = AGGRE_MTHD_NONE;
			}
		}
	}
}
#endif


/*-----------------------------------------------------------------------------
OnAssocReg:
	--> Reply DeAuth or AssocRsp
Capability Info, Listen Interval, SSID, SupportedRates
------------------------------------------------------------------------------*/
unsigned int OnAssocReq(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	struct wifi_mib		*pmib;
	struct stat_info	*pstat;
	unsigned char		*pframe, *p;
	unsigned char		rsnie_hdr[4]={0x00, 0x50, 0xf2, 0x01};
#ifdef RTL_WPA2
	unsigned char		rsnie_hdr_wpa2[2]={0x01, 0x00};
#endif
	int		len;
#ifndef SMP_SYNC
	unsigned long		flags;
#endif
	DOT11_ASSOCIATION_IND     Association_Ind;
	DOT11_REASSOCIATION_IND   Reassociation_Ind;
	unsigned char		supportRate[32];
	int					supportRateNum;
	unsigned int		status = _STATS_SUCCESSFUL_;
	unsigned short		frame_type, ie_offset=0, val16;
	unsigned int z = 0;

#ifdef P2P_SUPPORT
	unsigned char ReAssem_p2pie[MAX_REASSEM_P2P_IE];
	int IEfoundtimes=0;
	unsigned char *p2pIEPtr = ReAssem_p2pie ;
	int p2pIElen;
#endif
	pmib = GET_MIB(priv);
	pframe = get_pframe(pfrinfo);
	pstat = get_stainfo(priv, GetAddr2Ptr(pframe));

	if (!(OPMODE & WIFI_AP_STATE))
		return FAIL;

#ifdef WDS
	if (pmib->dot11WdsInfo.wdsPure)
		return FAIL;
#endif

	if (pmib->miscEntry.func_off || pmib->miscEntry.raku_only)
		return FAIL;

#ifdef DFS
	if (priv->pshare->rf_ft_var.dfs_det_period)
		priv->det_asoc_clear = 500 / priv->pshare->rf_ft_var.dfs_det_period;
	else
		priv->det_asoc_clear = 50;
#endif

#ifdef CONFIG_RTK_MESH

// KEY_MAP_KEY_PATCH_0223
	if((pmib->dot1180211sInfo.mesh_enable && !(GET_MIB(priv)->dot1180211sInfo.mesh_ap_enable)) || pmib->dot1180211sInfo.meshSilence)
// 2008.05.16
//		((pmib->dot11sKeysTable.dot11Privacy && pmib->dot11sKeysTable.keyInCam == FALSE )
//		||( (OPMODE & WIFI_AP_STATE) && !(GET_MIB(priv)->dot1180211sInfo.mesh_ap_enable))))
	{
		return FAIL;
	}
// KEY_MAP_KEY_PATCH_0223
// 2008.05.16
#endif

	frame_type = GetFrameSubType(pframe);
	if (frame_type == WIFI_ASSOCREQ)
		ie_offset = _ASOCREQ_IE_OFFSET_;
	else // WIFI_REASSOCREQ
		ie_offset = _REASOCREQ_IE_OFFSET_;

	if (pstat == (struct stat_info *)NULL)
	{
		status = _RSON_CLS2_;
		goto asoc_class2_error;
	}

	// check if this stat has been successfully authenticated/assocated
	if (!((pstat->state) & WIFI_AUTH_SUCCESS))
	{
		status = _RSON_CLS2_;
		goto asoc_class2_error;
	}

	if (priv->assoc_reject_on)
	{
		status = _STATS_OTHER_;
		goto OnAssocReqFail;
	}
#ifdef CONFIG_IEEE80211W
	if(((pstat->state) & WIFI_ASOC_STATE) &&
		pstat->isPMF &&
		!pstat->sa_query_timed_out &&
		pstat->sa_query_count) 	{
		check_sa_query_timeout(pstat);
	}
	
	if(((pstat->state) & WIFI_ASOC_STATE) &&
		pstat->isPMF &&
		!pstat->sa_query_timed_out) {
		PMFDEBUG("%s(%d)\n", __FUNCTION__, __LINE__);
		status = _STATS_ASSOC_REJ_TEMP_;
		if(pstat->sa_query_count == 0) {			
			pstat->sa_query_start = jiffies;			
			pstat->sa_query_end = jiffies + RTL_MILISECONDS_TO_JIFFIES(SA_QUERY_MAX_TO);
		}
		if (frame_type == WIFI_ASSOCREQ)
			issue_asocrsp(priv, status, pstat, WIFI_ASSOCRSP);
		else
			issue_asocrsp(priv, status, pstat, WIFI_REASSOCRSP);
		
		if(pstat->sa_query_count == 0) {
			PMFDEBUG("sa_query_end=%lu, sa_query_start=%lu\n", pstat->sa_query_end, pstat->sa_query_start);
			pstat->sa_query_count++;
			issue_SA_Query_Req(priv->dev,pstat->hwaddr);
			PMFDEBUG("%s(%d), settimer, %x\n", __FUNCTION__, __LINE__, &pstat->SA_timer);
			
			if(timer_pending(&pstat->SA_timer))
				del_timer(&pstat->SA_timer);

			pstat->SA_timer.data = (unsigned long) pstat;
			pstat->SA_timer.function = rtl8192cd_sa_query_timer;
			mod_timer(&pstat->SA_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(SA_QUERY_RETRY_TO));				
			PMFDEBUG("%s(%d), settimer end\n", __FUNCTION__, __LINE__);
		}					
		return FAIL;
	}
	pstat->sa_query_timed_out = 0;
#endif
#ifdef CONFIG_RTL_WLAN_DOS_FILTER
	if (block_sta_time)
	{
		int i;
		for (i=0; i<MAX_BLOCK_MAC;i++)
		{
			if (memcmp(pstat->hwaddr, block_mac[i], 6) == 0)
			{
				status = _STATS_OTHER_;
				goto OnAssocReqFail;
			}				
		}
	}
#endif

	/* Rate adpative algorithm */
	if (pstat->check_init_tx_rate)
		pstat->check_init_tx_rate = 0;

	// now we should check all the fields...

	// checking SSID
	p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _SSID_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);

	if (p == NULL)
	{
		status = _STATS_FAILURE_;
		goto OnAssocReqFail;
	}

	if (len == 0) // broadcast ssid, however it is not allowed in assocreq
		status = _STATS_FAILURE_;
	else
	{
		// check if ssid match
		if (memcmp((void *)(p+2), SSID, SSID_LEN))
			status = _STATS_FAILURE_;

		if (len != SSID_LEN)
			status = _STATS_FAILURE_;
	}

	// check if the supported is ok
	p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _SUPPORTEDRATES_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);

	if (len > 8)
		status = _STATS_RATE_FAIL_;
	else if (p == NULL) {
		DEBUG_WARN("Rx a sta assoc-req which supported rate is empty!\n");
		// use our own rate set as statoin used
		memcpy(supportRate, AP_BSSRATE, AP_BSSRATE_LEN);
		supportRateNum = AP_BSSRATE_LEN;
	}
	else {
		memcpy(supportRate, p+2, len);
		supportRateNum = len;

		p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _EXT_SUPPORTEDRATES_IE_ , &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
		if ((p !=  NULL) && (len <= 8)) {
			memcpy(supportRate+supportRateNum, p+2, len);
			supportRateNum += len;
		}
	}




#ifdef __DRAYTEK_OS__
	if (status == _STATS_SUCCESSFUL_) {
		status = cb_assoc_request(priv->dev, GetAddr2Ptr(pframe), pframe + WLAN_HDR_A3_LEN + _ASOCREQ_IE_OFFSET_,
				pfrinfo->pktlen-WLAN_HDR_A3_LEN-_ASOCREQ_IE_OFFSET_);
		if (status != _STATS_SUCCESSFUL_) {
			DEBUG_ERR("\rReject association from draytek OS, status=%d!\n", status);
			goto OnAssocReqFail;
		}
	}
#endif
#if 0	
#ifdef P2P_SUPPORT			/*cfg p2p cfg p2p*/
	if((rtk_p2p_is_enabled(priv)==PROPERTY_P2P)){
	}
	else
#endif
	{
	if (check_basic_rate(priv, supportRate, supportRateNum) == FAIL) {		// check basic rate. jimmylin 2004/12/02
		DEBUG_WARN("Rx a sta assoc-req which basic rates not match! %02X%02X%02X%02X%02X%02X\n",
			pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
		if (priv->pmib->dot11OperationEntry.wifi_specific) {
			status = _STATS_RATE_FAIL_;
			goto OnAssocReqFail;
		}
	}
	}
#endif
	get_matched_rate(priv, supportRate, &supportRateNum, 0);
	update_support_rate(pstat, supportRate, supportRateNum);

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
		!isErpSta(pstat) &&
		(priv->pmib->dot11StationConfigEntry.legacySTADeny & WIRELESS_11B)) {
		status = _STATS_RATE_FAIL_;
		goto OnAssocReqFail;
	}

	val16 = cpu_to_le16(*(unsigned short*)((unsigned long)pframe + WLAN_HDR_A3_LEN));
	if (!(val16 & BIT(5))) // NOT use short preamble
		pstat->useShortPreamble = 0;
	else
		pstat->useShortPreamble = 1;

	pstat->state |= WIFI_ASOC_STATE;

	if (status != _STATS_SUCCESSFUL_)
		goto OnAssocReqFail;

    	// now the station is qualified to join our BSS...
        #if defined(BR_SHORTCUT) && defined(RTL_CACHED_BR_STA)
        release_brsc_cache(GetAddr2Ptr(pframe));
        #endif

#ifdef WIFI_WMM
	// check if there is WMM IE
	if (QOS_ENABLE) {
		p = pframe + WLAN_HDR_A3_LEN + ie_offset; len = 0;
		for (;;) {
			p = get_ie(p, _RSN_IE_1_, &len,
				pfrinfo->pktlen - (p - pframe));
			if (p != NULL) {
				if (!memcmp(p+2, WMM_IE, 6)) {
					pstat->QosEnabled = 1;
#ifdef WMM_APSD
					if (APSD_ENABLE)
						pstat->apsd_bitmap = *(p+8) & 0x0f;		// get QSTA APSD bitmap
#endif
					break;
				}
			}
			else {
				pstat->QosEnabled = 0;
#ifdef WMM_APSD
				pstat->apsd_bitmap = 0;
#endif
				break;
			}
			p = p + len + 2;
		}
	}
	else {
		pstat->QosEnabled = 0;
#ifdef WMM_APSD
		pstat->apsd_bitmap = 0;
#endif
	}
#endif
#ifdef RTK_AC_SUPPORT
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
	{
		p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, EID_VHTCapability, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);

		if ((p !=  NULL) && (len <= sizeof(struct vht_cap_elmt))) {
			pstat->vht_cap_len = len;
			memcpy((unsigned char *)&pstat->vht_cap_buf, p+2, len);
			/* For debugging
			SDEBUG("Receive vht_cap len = %d \n",len);		
			if (pstat->vht_cap_buf.vht_cap_info & cpu_to_le32(_VHTCAP_RX_STBC_CAP_)) {
				SDEBUG("STA support RX STBC\n");
			}
			if (pstat->vht_cap_buf.vht_cap_info & cpu_to_le32(_VHTCAP_RX_LDPC_CAP_)) {
				SDEBUG("STA support RX LDPC\n");
			}
			*/
		}
#if 1//for 2.4G VHT IE		
		else
		{
			if(priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_2G)
			{
				unsigned char vht_ie_id[] = {0x00, 0x90, 0x4c};
				p = pframe + WLAN_HDR_A3_LEN + ie_offset; 
				len = 0;			

				for (;;)
				{
					//printk("\nOUI limit=%d\n", pfrinfo->pktlen - (p - pframe));
					p = get_ie(p, _RSN_IE_1_, &len, pfrinfo->pktlen - (p - pframe));
					if (p != NULL) {
						#if 0//debug
						int i;
						for(i=0; i<len+2; i++){
							if((i%8)==0)
								panic_printk("\n");
							panic_printk("%02x ", *(p+i));						
						}
						panic_printk("\nlen=%d vht_len=%d\n",len, sizeof(struct vht_cap_elmt));
						#endif
						// Bcom VHT IE
						// {0xdd, 0x13} RSN_IE_1-221, length
						// {0x00, 0x90, 0x4c} oui // {0x04 0x08 } unknow
						// {0xbf, 0x0c} element id, length
						if (!memcmp(p+2, vht_ie_id, 3) && (*(p+7) == 0xbf) && ((*(p+8)) <= sizeof(struct vht_cap_elmt))) {
							pstat->vht_cap_len = *(p+8);
							memcpy((unsigned char *)&pstat->vht_cap_buf, p+9, pstat->vht_cap_len);
							//panic_printk("\n get vht ie in OUI!!! len=%d\n\n", pstat->vht_cap_len);
							break;
						}
					}
					else
						break;
					p = p + len + 2;
				}
			}
		}
#endif
		p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, EID_VHTOperation, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);

		if ((p !=  NULL) && (len <= sizeof(struct vht_oper_elmt))) {
			pstat->vht_oper_len = len;
			memcpy((unsigned char *)&pstat->vht_oper_buf, p+2, len);
//			SDEBUG("Receive vht_oper len = %d \n",len);	
		}
	}
#endif
// ====2011-0926 ;roll back ; ht issue 
#if 1
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _HT_CAP_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
		if ((p !=  NULL) && (len <= sizeof(struct ht_cap_elmt))) {
			pstat->ht_cap_len = len;
			memcpy((unsigned char *)&pstat->ht_cap_buf, p+2, len);
		}
		else {
			unsigned char old_ht_ie_id[] = {0x00, 0x90, 0x4c};
			p = pframe + WLAN_HDR_A3_LEN + ie_offset; len = 0;
			for (;;)
			{
				p = get_ie(p, _RSN_IE_1_, &len,
					pfrinfo->pktlen - (p - pframe));
				if (p != NULL) {
					if (!memcmp(p+2, old_ht_ie_id, 3) && (*(p+5) == 0x33) && ((len - 4) <= sizeof(struct ht_cap_elmt))) {
						pstat->ht_cap_len = len - 4;
						memcpy((unsigned char *)&pstat->ht_cap_buf, p+6, pstat->ht_cap_len);
						break;
					}
				}
				else
					break;

				p = p + len + 2;
			}
		}
	
		//AC mode only, deny N mode STA
#ifdef RTK_AC_SUPPORT		
		if (!pstat->vht_cap_len && (priv->pmib->dot11StationConfigEntry.legacySTADeny & (WIRELESS_11N ))) {
			DEBUG_ERR("AC mode only, deny non-AC STA association!\n");
			status = _STATS_RATE_FAIL_;
			goto OnAssocReqFail;
		}
#endif		
		if (pstat->ht_cap_len) {
			// below is the process to check HT MIMO power save
			unsigned char mimo_ps = ((cpu_to_le16(pstat->ht_cap_buf.ht_cap_info)) >> 2)&0x0003;
			pstat->MIMO_ps = 0;
			if (!mimo_ps)
				pstat->MIMO_ps |= _HT_MIMO_PS_STATIC_;
			else if (mimo_ps == 1)
				pstat->MIMO_ps |= _HT_MIMO_PS_DYNAMIC_;
			if (cpu_to_le16(pstat->ht_cap_buf.ht_cap_info) & _HTCAP_AMSDU_LEN_8K_) {
				pstat->is_8k_amsdu = 1;
				pstat->amsdu_level = 7935 - sizeof(struct wlan_hdr);
			}
			else {
				pstat->is_8k_amsdu = 0;
				pstat->amsdu_level = 3839 - sizeof(struct wlan_hdr);
			}

			if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))
				pstat->tx_bw = HT_CHANNEL_WIDTH_20_40;
#ifdef RTK_AC_SUPPORT
			if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) && (pstat->vht_cap_len)) 
			{
				switch(cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & 0x3) {
					default:
					case 0:	
						pstat->is_8k_amsdu = 0;
						pstat->amsdu_level = 3895 - sizeof(struct wlan_hdr);
						break;
					case 1:
						pstat->is_8k_amsdu = 1;
						pstat->amsdu_level = 7991 - sizeof(struct wlan_hdr);
						break;
					case 2:	
						pstat->is_8k_amsdu = 1;
						pstat->amsdu_level = 11454 - sizeof(struct wlan_hdr);	
						break;
				}
// force 4k
//				pstat->is_8k_amsdu = 0;
//				pstat->amsdu_level = 3895 - sizeof(struct wlan_hdr);
			}
#endif					
		}
		else {
			if(!priv->pmib->wscEntry.wsc_enable){
				if (priv->pmib->dot11StationConfigEntry.legacySTADeny & (WIRELESS_11G | WIRELESS_11A)) {
					DEBUG_ERR("Deny legacy STA association!\n");
					status = _STATS_RATE_FAIL_;
					goto OnAssocReqFail;
				}
	
			}
		}
#ifdef RTK_AC_SUPPORT
		if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)) {
			if (pstat->vht_cap_len && (priv->vht_oper_buf.vht_oper_info[0] == 1)) 
				pstat->tx_bw = HT_CHANNEL_WIDTH_80;

			p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, EID_VHTOperatingMode, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
			if ((p !=  NULL) && (len == 1)) {
				// check self capability....					
					if((p[2] &3) <= priv->pshare->CurrentChannelBW)
						pstat->tx_bw = p[2] &3;
					pstat->nss = ((p[2]>>4)&0x7)+1;											
					//printk("receive opering mode data = %x \n", p[2]);
			}


		}
#endif			

	}
#endif
// ====2011-0926 ; ht issue

#ifdef WIFI_WMM
	if (QOS_ENABLE) {
		if ((pstat->QosEnabled == 0) && pstat->ht_cap_len) {
			DEBUG_INFO("STA supports HT but doesn't support WMM, force WMM supported\n");
			pstat->QosEnabled = 1;
		}
	}
#endif


#ifdef P2P_SUPPORT	
	if(rtk_p2p_is_enabled(priv)){			/*cfg p2p cfg p2p*/
		p = pframe + WLAN_HDR_A3_LEN + ie_offset ;
		for (;;)
		{
			p = get_ie(p, _P2P_IE_, &len,	
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset - len);				
			if (p) {
				if (!memcmp(p+2, WFA_OUI_PLUS_TYPE, 4)) {
                    if(IEfoundtimes==0){
    					memcpy(p2pIEPtr , p ,len+2);
	    				p2pIEPtr += (len+2);	
                    }else{
                        if(len>4){
        					memcpy(p2pIEPtr , p+6 ,len-4);
	        				p2pIEPtr+= (len-4);	
                        }
                    }
					IEfoundtimes ++;                    
				}			/*cfg p2p cfg p2p*/
			}
			else{
				break;
			}
			p = p + len + 2;
			
		}

		if(IEfoundtimes){
			#if 0   // for debug			/*cfg p2p cfg p2p*/
			if(IEfoundtimes>1){
				P2P_DEBUG("ReAssembly p2p IE\n");
			}	
            #endif			/*cfg p2p cfg p2p*/
			p2pIElen = (int)(((unsigned long)p2pIEPtr)-((unsigned long)ReAssem_p2pie));		
			
			if(p2pIElen > MAX_REASSEM_P2P_IE){
				P2P_DEBUG("\n\n	reassemble P2P IE exceed MAX_REASSEM_P2P_IE , chk!!!\n\n");
			}else{
				/*just start record GC's info when i am Real GO*/

                pstat->is_p2p_client = 1;                                    
				if(rtk_p2p_is_enabled(priv)==PROPERTY_P2P){
   					P2P_DEBUG("GC come from:\n");				                    
   					printMac(pfrinfo->sa);                    
					P2P_on_assoc_req(priv,ReAssem_p2pie+6 , p2pIElen-6 , pfrinfo->sa);
				}else if(rtk_p2p_is_enabled(priv)==CFG80211_P2P){
    				ReAssem_p2pie[1] = p2pIElen-2;  // recompute len if this p2p ie has ReAssembly
                    memcpy(pstat->p2p_ie, ReAssem_p2pie, p2pIElen); // report to cfg event , when new sat added
                }
			}
		}
        else{
            memset(pstat->p2p_ie, 0 , 6);
        }
					/*cfg p2p cfg p2p*/
	} 
	
#endif


	// Realtek proprietary IE
	pstat->is_realtek_sta = FALSE;
	pstat->IOTPeer = HT_IOT_PEER_UNKNOWN;	
	p = pframe + WLAN_HDR_A3_LEN + ie_offset; len = 0;
	for (;;)
	{
		p = get_ie(p, _RSN_IE_1_, &len,
			pfrinfo->pktlen - (p - pframe));
		if (p != NULL) {
			if (!memcmp(p+2, Realtek_OUI, 3) && *(p+2+3) == 2) { /*found realtek out and type == 2*/
                pstat->is_realtek_sta = TRUE;
                pstat->IOTPeer = HT_IOT_PEER_REALTEK;

                if (*(p+2+3+2) & RTK_CAP_IE_AP_CLIENT)
                    pstat->IOTPeer = HT_IOT_PEER_RTK_APCLIENT;

                if(*(p+2+3+2) & RTK_CAP_IE_WLAN_8192SE) 					
                    pstat->IOTPeer = HT_IOT_PEER_REALTEK_92SE;

                if (*(p+2+3+2) & RTK_CAP_IE_USE_AMPDU)
                    pstat->is_forced_ampdu = TRUE;
                else
                    pstat->is_forced_ampdu = FALSE;
#ifdef RTK_WOW
                if (*(p+2+3+2) & RTK_CAP_IE_USE_WOW)						
                    pstat->IOTPeer = HT_IOT_PEER_REALTEK_WOW;			
#endif
                if (*(p+2+3+2) & RTK_CAP_IE_WLAN_88C92C)						
                    pstat->IOTPeer = HT_IOT_PEER_REALTEK_81XX;

                if (*(p+2+3+3) & ( RTK_CAP_IE_8812_BCUT | RTK_CAP_IE_8812_CCUT))						
                    pstat->IOTPeer = HT_IOT_PEER_REALTEK_8812;
				break;
			}
		}
		else
			break;

		p = p + len + 2;
	}

	// identify if this is Broadcom sta
	p = pframe + WLAN_HDR_A3_LEN + ie_offset; len = 0;

	for (;;)
	{
		unsigned char Broadcom_OUI1[]={0x00, 0x05, 0xb5};
		unsigned char Broadcom_OUI2[]={0x00, 0x0a, 0xf7};
		unsigned char Broadcom_OUI3[]={0x00, 0x10, 0x18};

		p = get_ie(p, _RSN_IE_1_, &len,
			pfrinfo->pktlen - (p - pframe));
		if (p != NULL) {
			if (!memcmp(p+2, Broadcom_OUI1, 3) ||
				!memcmp(p+2, Broadcom_OUI2, 3) ||
				!memcmp(p+2, Broadcom_OUI3, 3)) {

				pstat->IOTPeer= HT_IOT_PEER_BROADCOM;
			
				break;
			}
		}
		else
			break;

		p = p + len + 2;
	}

	// identify if this is ralink sta
	p = pframe + WLAN_HDR_A3_LEN + ie_offset; len = 0;

#if 0
//#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
	if(!IS_OUTSRC_CHIP(priv))
#endif	
	pstat->is_ralink_sta = FALSE;
#endif
	for (;;)
	{
		unsigned char Ralink_OUI1[]={0x00, 0x0c, 0x43};

		p = get_ie(p, _RSN_IE_1_, &len,
			pfrinfo->pktlen - (p - pframe));
		if (p != NULL) {
			if (!memcmp(p+2, Ralink_OUI1, 3)) {

				pstat->IOTPeer= HT_IOT_PEER_RALINK;

				break;
			}
		}
		else
			break;

		p = p + len + 2;
	}

	for (z = 0; z < HTC_OUI_NUM; z++) {
		if ((pstat->hwaddr[0] == HTC_OUI[z][0]) &&
			(pstat->hwaddr[1] == HTC_OUI[z][1]) &&
			(pstat->hwaddr[2] == HTC_OUI[z][2])) {
	
			pstat->IOTPeer = HT_IOT_PEER_HTC; 			

			break;
		}
	}
	
	if (!pstat->is_realtek_sta && (pstat->IOTPeer!=HT_IOT_PEER_BROADCOM) && pstat->IOTPeer!=HT_IOT_PEER_RALINK && pstat->IOTPeer!=HT_IOT_PEER_HTC) 
	{
		//unsigned int z = 0;

		for (z = 0; z < INTEL_OUI_NUM; z++) {
			if ((pstat->hwaddr[0] == INTEL_OUI[z][0]) &&
				(pstat->hwaddr[1] == INTEL_OUI[z][1]) &&
				(pstat->hwaddr[2] == INTEL_OUI[z][2])) {

				pstat->IOTPeer = HT_IOT_PEER_INTEL;				

				pstat->no_rts = 1;
				break;
			}
		}
	
	}

#ifdef MCR_WIRELESS_EXTEND
	if ((GET_CHIP_VER(priv)==VERSION_8812E) || (GET_CHIP_VER(priv)==VERSION_8192E) || (GET_CHIP_VER(priv)==VERSION_8814A)) {
		if (!memcmp(pstat->hwaddr, "\x00\x01\x02\x03\x04\x05", MACADDRLEN)) {
			pstat->IOTPeer = HT_IOT_PEER_CMW;
			priv->pshare->rf_ft_var.tx_pwr_ctrl = 0;
#ifdef CONFIG_WLAN_HAL_8814AE			
			if (GET_CHIP_VER(priv)==VERSION_8814A) {
				PHY_SetBBReg(priv, 0x93c, 0xfff00000, 0x002);
				if (priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_80) {
					PHY_SetBBReg(priv, 0x838, BIT0, 0x1);
					if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
						PHY_SetBBReg(priv, 0x82C, 0x00f00000, 0xa);
						PHY_SetBBReg(priv, 0x82C, 0x000f0000, 0x9);
						PHY_SetBBReg(priv, 0x838, 0x0f000000, 0x9);
						PHY_SetBBReg(priv, 0x838, 0x00f00000, 0x9);
						PHY_SetBBReg(priv, 0x838, 0x000f0000, 0x9);
						PHY_SetBBReg(priv, 0x840, 0x0000f000, 0x6);
					} else {
						PHY_SetBBReg(priv, 0x82C, 0x00f00000, 0xa);
						PHY_SetBBReg(priv, 0x82C, 0x000f0000, 0x8);
						PHY_SetBBReg(priv, 0x838, 0x0f000000, 0x7);
						PHY_SetBBReg(priv, 0x838, 0x00f00000, 0x7);
						PHY_SetBBReg(priv, 0x838, 0x000f0000, 0x7);
						PHY_SetBBReg(priv, 0x840, 0x0000f000, 0x7);
					} 
				} else {				
					if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
						PHY_SetBBReg(priv, 0x82c, BIT27|BIT26|BIT25|BIT24, 0x5);
						RTL_W8(0x830,0xa);	
					} else {
						PHY_SetBBReg(priv, 0x82c, BIT27|BIT26|BIT25|BIT24, 0x3);
						RTL_W8(0x830,0x8);
					}
				}
			} else
#endif						
#ifdef CONFIG_RTL_8812_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8812E) {
				PHY_SetBBReg(priv, 0x878, BIT(12)|BIT(13), 0);
			} else			
#endif
#ifdef CONFIG_WLAN_HAL_8192EE
			if (GET_CHIP_VER(priv)==VERSION_8192E) {
				RTL_W16(RESP_SIFS_CCK, 0x0808);
				RTL_W8(0xa2f, 0x0);
				RTL_W8(0xa07, 0x81);
				if (priv->pshare->rf_ft_var.mcr_ft)
					RTL_W8(0xa20, 0x10);				
				ODM_CmnInfoUpdate(ODMPTR, ODM_CMNINFO_ABILITY, ODMPTR->SupportAbility & (~ ODM_COMP_CCK_PD));				
			}
#endif			
		}
	}
#endif

#ifdef A4_STA
    if(priv->pshare->rf_ft_var.a4_enable) {
        if(priv->pshare->rf_ft_var.a4_enable == 2) {
            if(0 < parse_a4_ie(priv, pframe + WLAN_HDR_A3_LEN + ie_offset, pfrinfo->pktlen - (WLAN_HDR_A3_LEN + ie_offset))) {
                add_a4_client(priv, pstat);
            }       
        }
        a4_sta_update(GET_ROOT(priv), NULL, pstat->hwaddr);
    }
#endif

#ifdef TV_MODE
    if(priv->tv_mode_status & BIT1){ /*tv mode is auto*/
        if(0 < parse_tv_mode_ie(priv, pframe + WLAN_HDR_A3_LEN + ie_offset, pfrinfo->pktlen - (WLAN_HDR_A3_LEN + ie_offset))) {
            pstat->tv_auto_support = 1;
            priv->tv_mode_status |= BIT0; /* set tv mode to auto(enable)*/
        }
    }
#endif

	SAVE_INT_AND_CLI(flags);
	auth_list_del(priv, pstat);
	if (asoc_list_add(priv, pstat))
	{
		pstat->expire_to = priv->expire_to;
		//printk("wlan%d pstat->asoc_list = %p priv->asoc_list=%p\n",priv->pshare->wlandev_idx, pstat->asoc_list, priv->asoc_list);
		cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);
		check_sta_characteristic(priv, pstat, INCREASE);
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			construct_ht_ie(priv, priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);
	}
	RESTORE_INT(flags);

	assign_tx_rate(priv, pstat, pfrinfo);

#if defined(CONFIG_PCI_HCI)
#ifdef CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		//panic_printk("%s %d UpdateRAMask\n", __FUNCTION__, __LINE__);
		GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
		ODM_RAPostActionOnAssoc(ODMPTR);
		pstat->ratr_idx_init = pstat->ratr_idx;	
		//phydm_ra_dynamic_rate_id_on_assoc(ODMPTR, pstat->WirelessMode, pstat->ratr_idx_init);

	} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv)== VERSION_8812E) {
		UpdateHalRAMask8812(priv, pstat, 3);
	} else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
		add_RATid(priv, pstat);
#endif
	} else
#endif
	{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)	
		add_update_RATid(priv, pstat);
#endif
	}
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	update_STA_RATid(priv, pstat);
#endif
	assign_aggre_mthod(priv, pstat);
	assign_aggre_size(priv, pstat);

#if defined(WIFI_11N_2040_COEXIST_EXT)
	update_40m_staMap(priv, pstat, 0);
	checkBandwidth(priv);
#endif
	
	// Customer proprietary IE
	if (priv->pmib->miscEntry.private_ie_len) {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, priv->pmib->miscEntry.private_ie[0], &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
		if (p) {
			memcpy(pstat->private_ie, p, len + 2);
			pstat->private_ie_len = len + 2;
		}
	}

#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
	{
		SAVE_INT_AND_CLI(flags);
		if (pstat->wapiInfo==NULL)
		{
			pstat->wapiInfo = (wapiStaInfo*)kmalloc(sizeof(wapiStaInfo), GFP_ATOMIC);
			if (pstat->wapiInfo==NULL)
			{
				/*pstat->wapiInfo->wapiState = ST_WAPI_AE_IDLE;*/
				status = _RSON_UNABLE_HANDLE_;
				RESTORE_INT(flags);
				goto asoc_class2_error;
			}
			pstat->wapiInfo->priv = priv;
			wapiStationInit(pstat);
		}

		RESTORE_INT(flags);

		p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _EID_WAPI_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);

		if (p==NULL)
		{
			pstat->wapiInfo->wapiState = ST_WAPI_AE_IDLE;
			status = _RSON_IE_NOT_CONSISTENT_;
			goto asoc_class2_error;
		}

		memcpy(pstat->wapiInfo->asueWapiIE, p, len+2);
		pstat->wapiInfo->asueWapiIELength= len+2;

		/*	check for KM	*/
		if ((status=wapiIEInfoInstall(priv, pstat))!=_STATS_SUCCESSFUL_)
		{
			pstat->wapiInfo->wapiState = ST_WAPI_AE_IDLE;
			goto asoc_class2_error;
		}
	}
#endif

	DEBUG_INFO("%s %02X%02X%02X%02X%02X%02X\n",
		(frame_type == WIFI_ASSOCREQ)? "OnAssocReq" : "OnReAssocReq",
		pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);

	/* 1. If 802.1x enabled, get RSN IE (if exists) and indicate ASSOIC_IND event
	 * 2. Set dot118021xAlgrthm, dot11PrivacyAlgrthm in pstat
	 */
	if (IEEE8021X_FUN || IAPP_ENABLE || priv->pmib->wscEntry.wsc_enable)
	{
		p = pframe + WLAN_HDR_A3_LEN + ie_offset; len = 0;
		for(;;)
		{
#ifdef RTL_WPA2
			char tmpbuf[128];
			int buf_len=0;
			p = get_rsn_ie(priv, p, &len,
				pfrinfo->pktlen - (p - pframe));

			buf_len = sprintf(tmpbuf, "RSNIE len = %d, p = %s", len, (p==NULL? "NULL":"non-NULL"));
			if (p != NULL)
				buf_len += sprintf(tmpbuf+buf_len, ", ID = %02X\n", *(unsigned char *)p);
			else
				buf_len += sprintf(tmpbuf+buf_len, "\n");
			DEBUG_INFO("%s", tmpbuf);
#else
			p = get_ie(p, _RSN_IE_1_, &len,
				pfrinfo->pktlen - (p - pframe));
#endif
			/*cfg p2p cfg p2p
			if (p == NULL)
#ifdef WIFI_HAPD
			{	
				memset(pstat->wpa_ie, 0, 256);
				break;
			}
#else
				break;
#endif
			*/
			if (p == NULL){
                #if defined(WIFI_HAPD) || defined(RTK_NL80211)
                memset(pstat->wpa_ie, 0, 6);
                #endif          
                break;
            }

			/*cfg p2p cfg p2p*/
#if defined(WIFI_HAPD) || defined(RTK_NL80211)
			if((*(unsigned char *)p == _RSN_IE_1_)&& (len >= 4))
				{
					pstat->wpa_sta_info->RSNEnabled = BIT(0); 
					memcpy(pstat->wpa_ie, p, len+2);
				}
			else if((*(unsigned char *)p == _RSN_IE_2_) && (len >= 2))
				{
					pstat->wpa_sta_info->RSNEnabled = BIT(1); 
					memcpy(pstat->wpa_ie, p, len+2);
				}
#endif

#ifdef RTL_WPA2
			if ((*(unsigned char *)p == _RSN_IE_1_) && (len >= 4) && (!memcmp((void *)(p + 2), (void *)rsnie_hdr, 4))) {
#ifdef TLN_STATS
				pstat->enterpise_wpa_info = STATS_ETP_WPA;
#endif
				break;
}

			if ((*(unsigned char *)p == _RSN_IE_2_) && (len >= 2) && (!memcmp((void *)(p + 2), (void *)rsnie_hdr_wpa2, 2))) {
#ifdef TLN_STATS
				pstat->enterpise_wpa_info = STATS_ETP_WPA2;
#endif
				break;
			}
#else
			if ((len >= 4) && (!memcmp((void *)(p + 2), (void *)rsnie_hdr, 4))) {
#ifdef TLN_STATS
				pstat->enterpise_wpa_info = STATS_ETP_WPA;
#endif
				break;
			}
#endif

			p = p + len + 2;
		}

#ifdef WIFI_SIMPLE_CONFIG
/* WPS2DOTX   -start*/
		if (priv->pmib->wscEntry.wsc_enable & 2) { // work as AP (not registrar)
			unsigned char *ptmp;
			unsigned char *TagPtr=NULL;
			int IS_V2=0;			
			int Taglen = 0;
			int Taglent2 = 0;
			unsigned int lentmp = 0;
			unsigned char passWscIE=0;
			unsigned char both_band_cred = 0; 
			DOT11_WSC_ASSOC_IND wsc_Association_Ind;

		//================both_band_credential====================
		if(priv->pmib->wscEntry.both_band_multicredential){ 	
			ptmp = pframe + WLAN_HDR_A3_LEN + ie_offset;		
			for (;;)
			{
				ptmp = get_ie(ptmp, 221, (int *)&lentmp , pfrinfo->pktlen - (ptmp - pframe));
				if (ptmp != NULL) {
					if (!memcmp(ptmp+2, NEC_OUI, 3) && ptmp[5] == 0x06 && (ptmp[8] & BIT7)) {
						both_band_cred = 1;
						break;
					} 
				}
				else{
					break;
				}
				ptmp = ptmp + lentmp + 2;
			}				
		}
		//==================================================

			ptmp = pframe + WLAN_HDR_A3_LEN + ie_offset; 
			
			for (;;)
			{
				ptmp = get_ie(ptmp, _WPS_IE_, (int *)&lentmp , pfrinfo->pktlen - (ptmp - pframe));
				if (ptmp != NULL) {
					if ((!memcmp(ptmp+2, WSC_IE_OUI, 4)) && ((lentmp + 2) <= (MIN_NUM(PROBEIELEN,256)))) {//256 is size of pstat->wps_ie
#if (defined(RTK_NL80211) || defined(WIFI_HAPD)) && !defined(HAPD_DRV_PSK_WPS)
						//printk("copy wps_ie \n");
						memcpy(pstat->wps_ie, ptmp, lentmp+2);
#endif

						TagPtr = search_wsc_tag(ptmp+2+4, TAG_REQUEST_TYPE, lentmp-4, &Taglen);
						if (TagPtr && (*TagPtr <= MAX_REQUEST_TYPE_NUM)) {
							SME_DEBUG("found WSC_IE TAG_REQUEST_TYPE=%d (from %02x%02x%02x:%02x%02x%02x)\n",
								*TagPtr , pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
								 pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);	
							passWscIE = 1;
						}
						

						TagPtr = search_wsc_tag(ptmp+2+4, TAG_VENDOR_EXT, lentmp-4, &Taglen);
						if (TagPtr != NULL)	{
							if(!memcmp(TagPtr , WSC_VENDOR_OUI ,3 )){
								SME_DEBUG("Found WFA-vendor OUI!!\n");
								TagPtr = search_VendorExt_tag(TagPtr ,VENDOR_VERSION2 , Taglen , &Taglent2);
								if(TagPtr){
									IS_V2 = 1;
									SME_DEBUG("sme Rev version2(0x%x) ProReq\n",TagPtr[0]);
								}
							}				
						} 

						
						break;
					} else {
						if (TagPtr !=NULL){
							DEBUG_INFO("Found WSC_IE TAG_REQUEST_TYPE=%d", *TagPtr);
						}else{
							DEBUG_INFO("Found WSC_IE");
						}
							DEBUG_INFO(" from %02x%02x%02x:%02x%02x%02x, but the length(%d) of WSC_IE may be bigger than %d, Parse next WSC_IE\n", 
							pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], 
							pstat->hwaddr[4], pstat->hwaddr[5],	lentmp + 2, (MIN_NUM(PROBEIELEN,256)) );					
					}
				}
				else{
#if (defined(RTK_NL80211) || defined(WIFI_HAPD)) && !defined(HAPD_DRV_PSK_WPS)
					memset(pstat->wps_ie, 0, 256);
#endif
					break;
					}

				ptmp = ptmp + lentmp + 2;
			}

			memset(&wsc_Association_Ind, 0, sizeof(DOT11_WSC_ASSOC_IND));
			wsc_Association_Ind.EventId = DOT11_EVENT_WSC_ASSOC_REQ_IE_IND;
			memcpy((void *)wsc_Association_Ind.MACAddr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
			if (passWscIE) {
				if(both_band_cred){
					//indicates wscd for both_band_credential
					memcpy((void *)wsc_Association_Ind.AssocIE, "\x10\xFF\x00\x01\x01", 5); // T(0x10ff)L(1)V(1)
					memcpy((void *)wsc_Association_Ind.AssocIE + 5, (void *)(ptmp), wsc_Association_Ind.AssocIELen);					
					panic_printk("[%s]%d both_band_cred_ind =1 \n",__FUNCTION__,__LINE__);
				}else{
					memcpy((void *)wsc_Association_Ind.AssocIE, (void *)(ptmp), wsc_Association_Ind.AssocIELen);
				}
				wsc_Association_Ind.wscIE_included = 1;
				wsc_Association_Ind.AssocIELen = lentmp + 2;

			}
			else {
				/*modify for WPS2DOTX SUPPORT*/
				if(IS_V2==0)
				{	
					/*when sta is wps1.1 case then should be run below path*/ 
					if (IEEE8021X_FUN &&
						(pstat->AuthAlgrthm == _NO_PRIVACY_) && // authentication is open
						(p == NULL)) // No SSN or RSN IE
					{ 
						wsc_Association_Ind.wscIE_included = 1; //treat this case as WSC IE included
						SME_DEBUG("Association : auth open & no SSN or RSN IE , for wps1.1 case\n");
					}
				}
			}

             /*   wscIE_included :
                  case 1:  make sure STA include WSC IE
                  case 2:  because auth == open & no SSN or RSN IE ;so we
                           treat this case as WSC IE included
             */
			/*modify for WPS2DOTX SUPPORT*/
			if ((wsc_Association_Ind.wscIE_included == 1) || !IEEE8021X_FUN){
#ifdef INCLUDE_WPS
			
				wps_NonQueue_indicate_evt(priv ,
					(UINT8 *)&wsc_Association_Ind,sizeof(DOT11_WSC_ASSOC_IND));
#else

				DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&wsc_Association_Ind,
						sizeof(DOT11_WSC_ASSOC_IND));
#endif
			}
			/*modify for WPS2DOTX SUPPORT*/
			if (wsc_Association_Ind.wscIE_included == 1) {
				pstat->state |= WIFI_WPS_JOIN;
				goto OnAssocReqSuccess;
			}
// Brad add for DWA-652 WPS interoperability 2008/03/13--------
			if ((pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
     				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_) &&
     				!IEEE8021X_FUN)
				pstat->state |= WIFI_WPS_JOIN;
//------------------------- end

		}
/* WPS2DOTX   -end*/		
#endif

// ====2011-0926 ;roll back ; ht issue
#if 1
	if(priv->pmib->wscEntry.wsc_enable) {
		if (!pstat->ht_cap_len && (priv->pmib->dot11StationConfigEntry.legacySTADeny & (WIRELESS_11G | WIRELESS_11A))) {
			DEBUG_ERR("Deny legacy STA association!\n");
			status = _STATS_RATE_FAIL_;
			SAVE_INT_AND_CLI(flags);
			asoc_list_del(priv, pstat);
			cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
			check_sta_characteristic(priv, pstat, DECREASE);
			RESTORE_INT(flags);
			goto OnAssocReqFail;
		}
	}
#endif
// ====2011-0926 end


	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
		(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _NO_PRIVACY_))
	{
		int mask_mcs_rate = 0;
		if 	((pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) ||
			 (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_))
			mask_mcs_rate = 2;
#ifdef CONFIG_RTL_WAPI_SUPPORT	
		else if(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WAPI_SMS4_) {
			mask_mcs_rate = 0;
		}
#endif
		else {
			if (p == NULL)
				mask_mcs_rate = 1;
			else {
				if (*p == _RSN_IE_1_) {
					if (is_support_wpa_aes(priv,  p, len+2) != 1)
						mask_mcs_rate = 1;
				}
				else if (*p == _RSN_IE_2_) {
					if (is_support_wpa2_aes(priv,  p, len+2) != 1)
						mask_mcs_rate = 1;
				}
				else
						mask_mcs_rate = 1;
			}
		}

		if (mask_mcs_rate) {
			pstat->is_legacy_encrpt = mask_mcs_rate;
			assign_tx_rate(priv, pstat, pfrinfo);
#if defined(CONFIG_PCI_HCI)
#ifdef CONFIG_WLAN_HAL
			if (IS_HAL_CHIP(priv)) {
				//panic_printk("%s %d UpdateRAMask\n", __FUNCTION__, __LINE__);
				GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
				ODM_RAPostActionOnAssoc(ODMPTR);
				pstat->ratr_idx_init = pstat->ratr_idx;		
				//phydm_ra_dynamic_rate_id_on_assoc(ODMPTR, pstat->WirelessMode, pstat->ratr_idx_init);				
			} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
			if(GET_CHIP_VER(priv)== VERSION_8812E) {
				UpdateHalRAMask8812(priv, pstat, 3);
			} else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
				add_RATid(priv, pstat);
#endif
			} else
#endif
			{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
				add_update_RATid(priv, pstat);
#endif
			}
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			update_STA_RATid(priv, pstat);
#endif
			assign_aggre_mthod(priv, pstat);
		}
	}

#ifndef WITHOUT_ENQUEUE
		if (frame_type == WIFI_ASSOCREQ)
		{
			memcpy((void *)Association_Ind.MACAddr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
			Association_Ind.EventId = DOT11_EVENT_ASSOCIATION_IND;
			Association_Ind.IsMoreEvent = 0;
			if (p == NULL)
				Association_Ind.RSNIELen = 0;
			else
			{
				DEBUG_INFO("assoc indication rsnie len=%d\n", len);
#ifdef RTL_WPA2
				// inlcude ID and Length
				Association_Ind.RSNIELen = len + 2;
				memcpy((void *)Association_Ind.RSNIE, (void *)(p), Association_Ind.RSNIELen);
#else
				Association_Ind.RSNIELen = len;
				memcpy((void *)Association_Ind.RSNIE, (void *)(p + 2), len);
#endif
			}
			// indicate if 11n sta associated
			Association_Ind.RSNIE[MAXRSNIELEN-1] = ((pstat->ht_cap_len==0) ? 0 : 1);

			DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Association_Ind,
						sizeof(DOT11_ASSOCIATION_IND));
		}
		else
		{
			memcpy((void *)Reassociation_Ind.MACAddr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
			Reassociation_Ind.EventId = DOT11_EVENT_REASSOCIATION_IND;
			Reassociation_Ind.IsMoreEvent = 0;
			if (p == NULL)
				Reassociation_Ind.RSNIELen = 0;
			else
			{
				DEBUG_INFO("assoc indication rsnie len=%d\n", len);
#ifdef RTL_WPA2
				// inlcude ID and Length
				Reassociation_Ind.RSNIELen = len + 2;
				memcpy((void *)Reassociation_Ind.RSNIE, (void *)(p), Reassociation_Ind.RSNIELen);
#else
				Reassociation_Ind.RSNIELen = len;
				memcpy((void *)Reassociation_Ind.RSNIE, (void *)(p + 2), len);
#endif
			}
			memcpy((void *)Reassociation_Ind.OldAPaddr,
				(void *)(pframe + WLAN_HDR_A3_LEN + _CAPABILITY_ + _LISTEN_INTERVAL_), MACADDRLEN);

			// indicate if 11n sta associated
			Reassociation_Ind.RSNIE[MAXRSNIELEN-1] = ((pstat->ht_cap_len==0) ? 0 : 1);

			DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Reassociation_Ind,
						sizeof(DOT11_REASSOCIATION_IND));
		}
#endif // WITHOUT_ENQUEUE

#ifdef RTK_NL80211
	//printk("pstat=0x%x at %d: %02x %02x %02x\n", pstat, __LINE__, pstat->wpa_ie[0], pstat->wpa_ie[1], pstat->wpa_ie[2]);
	//event_indicate_cfg80211(priv, GetAddr2Ptr(pframe), CFG80211_NEW_STA, pstat);
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
		{
			int id;
			unsigned char *pIE;
			int ie_len;

#if !(defined(__ECOS) && defined(CONFIG_SDIO_HCI))
			LOG_MSG("A wireless client is associated - %02X:%02X:%02X:%02X:%02X:%02X\n",
				*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
				*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
#endif

			if (frame_type == WIFI_ASSOCREQ)
				id = DOT11_EVENT_ASSOCIATION_IND;
			else
				id = DOT11_EVENT_REASSOCIATION_IND;

#ifdef RTL_WPA2
			ie_len = len + 2;
			pIE = p;
#else
			ie_len = len;
			pIE = p + 2;
#endif
			psk_indicate_evt(priv, id, GetAddr2Ptr(pframe), pIE, ie_len);
		}
#endif // INCLUDE_WPA_PSK

#ifdef RTK_NL80211
		//printk("pstat=0x%x at %d: %02x %02x %02x\n", pstat, __LINE__, pstat->wpa_ie[0], pstat->wpa_ie[1], pstat->wpa_ie[2]);
		//event_indicate_cfg80211(priv, GetAddr2Ptr(pframe), CFG80211_NEW_STA, pstat);
#else //RTK_NL80211
#ifdef WIFI_HAPD
		event_indicate_hapd(priv, GetAddr2Ptr(pframe), HAPD_REGISTERED, NULL);
#ifdef HAPD_DRV_PSK_WPS
		event_indicate(priv, GetAddr2Ptr(pframe), 1);
#endif
#else
		event_indicate(priv, GetAddr2Ptr(pframe), 1);
#endif
#ifdef WIFI_WPAS
		//printk("_Eric WPAS_REGISTERED at %s %d \n", __FUNCTION__, __LINE__);
		event_indicate_wpas(priv, GetAddr2Ptr(pframe), WPAS_REGISTERED, NULL);
#endif
#ifdef RTK_NL80211
		event_indicate_cfg80211(priv, GetAddr2Ptr(pframe), CFG80211_CONNECT_RESULT, NULL);
#endif
#endif //RTK_NL80211
	}
#ifdef HS2_SUPPORT
	calcu_sta_v6ip(pstat);
#endif

//#ifndef INCLUDE_WPA_PSK
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
	if (!IEEE8021X_FUN &&
			!(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
			 priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_))
			LOG_MSG_NOTICE("Wireless PC connected;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
				*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
				*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
#elif defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	if (!IEEE8021X_FUN &&
			!(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
			 priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_))
			LOG_MSG_NOTICE("Wireless PC connected;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
				*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
				*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
#elif defined(CONFIG_RTL8196B_TLD)
	if (!IEEE8021X_FUN &&
			!(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
			 priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_)) {
			if (!list_empty(&priv->wlan_acl_list)) {
				LOG_MSG_DEL("[WLAN access allowed] from MAC: %02x:%02x:%02x:%02x:%02x:%02x,\n",
				*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
				*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
			}
	}
#elif defined(__ECOS) && defined(CONFIG_SDIO_HCI)
	if (priv->sta_status_cb_func)
		priv->sta_status_cb_func("wlan0", GetAddr2Ptr(pframe), ((frame_type == WIFI_ASSOCREQ)? WIFI_STA_STATUS_ASSOCIATED : WIFI_STA_STATUS_REASSOCIATED));
#else
	LOG_MSG("A wireless client is associated - %02X:%02X:%02X:%02X:%02X:%02X\n",
			*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
			*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
#endif
//#endif

#ifndef RTK_NL80211 //OPENWRT_RADIUS
	if (IEEE8021X_FUN || IAPP_ENABLE || priv->pmib->wscEntry.wsc_enable) {
#ifndef __DRAYTEK_OS__
		if (IEEE8021X_FUN &&	// in WPA, let user daemon check RSNIE and decide to accept or not
			(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
			 priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_)) {
			 
#ifdef BEAMFORMING_SUPPORT
			//panic_printk("%s, %x\n", __FUNCTION__, cpu_to_le32(pstat->vht_cap_buf.vht_cap_info));
				if(GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8814A)
				if((priv->pmib->dot11RFEntry.txbf == 1)  && ((pstat->ht_cap_len && (pstat->ht_cap_buf.txbf_cap)) 
#ifdef RTK_AC_SUPPORT	
					|| (pstat->vht_cap_len && (cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & (BIT(SU_BFEE_S)|BIT(SU_BFER_S))))
#endif	
				)){
					Beamforming_Enter(priv, pstat);
				}
#endif	

			return SUCCESS;
		}
#endif
	}
#endif

#ifdef WIFI_SIMPLE_CONFIG
OnAssocReqSuccess:
#endif

	if (frame_type == WIFI_ASSOCREQ)
		issue_asocrsp(priv, status, pstat, WIFI_ASSOCRSP);
	else
		issue_asocrsp(priv, status, pstat, WIFI_REASSOCRSP);

#ifdef BEAMFORMING_SUPPORT
	if(GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8814A)
	if((priv->pmib->dot11RFEntry.txbf == 1) &&
		((pstat->ht_cap_len && (pstat->ht_cap_buf.txbf_cap)) 
#ifdef RTK_AC_SUPPORT	
		||(pstat->vht_cap_len && (cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & (BIT(SU_BFEE_S)|BIT(SU_BFER_S))))
#endif
		)) {
			Beamforming_Enter(priv, pstat);
	}
#endif

//#ifdef BR_SHORTCUT
#if 0
	clear_shortcut_cache();
#endif

	update_fwtbl_asoclst(priv, pstat);

/*update mesh proxy table*/
#if defined(CONFIG_RTK_MESH)
    mesh_proxy_update(priv, pframe);

    #ifdef BR_SHORTCUT
    clear_shortcut_cache();
    #endif // BR_SHORTCUT
#endif
#ifdef RTK_NL80211
	/*cfg p2p cfg p2p*/
	//NDEBUG("pstat=0x%x: %02x %02x %02x\n", pstat, pstat->wpa_ie[0], pstat->wpa_ie[1], pstat->wpa_ie[2]);
	event_indicate_cfg80211(priv, GetAddr2Ptr(pframe), CFG80211_NEW_STA, pstat);
#else //RTK_NL80211

#ifdef WIFI_HAPD
	event_indicate_hapd(priv, GetAddr2Ptr(pframe), HAPD_REGISTERED, NULL);
#ifdef HAPD_DRV_PSK_WPS
	event_indicate(priv, GetAddr2Ptr(pframe), 1);
#endif
#else
	event_indicate(priv, GetAddr2Ptr(pframe), 1);
#endif
#ifdef WIFI_WPAS
	//printk("_Eric WPAS_REGISTERED at %s %d\n", __FUNCTION__, __LINE__);
	event_indicate_wpas(priv, GetAddr2Ptr(pframe), WPAS_REGISTERED, NULL);
#endif
#endif //RTK_NL80211

#ifndef USE_WEP_DEFAULT_KEY
	set_keymapping_wep(priv, pstat);
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (priv->pmib->wapiInfo.wapiType==wapiTypeCert)
	{
		wapiAssert(pstat->wapiInfo->wapiState==ST_WAPI_AE_IDLE);
		
		WAPI_LOCK(&pstat->wapiInfo->lock);
		pstat->wapiInfo->wapiRetry = 0;
		WAPI_UNLOCK(&pstat->wapiInfo->lock);
		
		wapiReqActiveCA(pstat);
		return SUCCESS;
	}
	else if (priv->pmib->wapiInfo.wapiType==wapiTypePSK)
	{
		wapiAssert(pstat->wapiInfo->wapiState==ST_WAPI_AE_IDLE);
		
		WAPI_LOCK(&pstat->wapiInfo->lock);
		pstat->wapiInfo->wapiRetry = 0;
		WAPI_UNLOCK(&pstat->wapiInfo->lock);
		
		wapiSetBK(pstat);
		if (wapiSendUnicastKeyAgrementRequeset(priv, pstat)==WAPI_RETURN_SUCCESS)
			return SUCCESS;
		else
			return FAIL;
	}
#endif

	return SUCCESS;

asoc_class2_error:

	issue_deauth(priv,	(void *)GetAddr2Ptr(pframe), status);
	if (pstat){
		free_stainfo(priv, pstat);
	}
	return FAIL;

OnAssocReqFail:

	if (frame_type == WIFI_ASSOCREQ)
		issue_asocrsp(priv, status, pstat, WIFI_ASSOCRSP);
	else
		issue_asocrsp(priv, status, pstat, WIFI_REASSOCRSP);
	return FAIL;
}

#ifdef P2P_SUPPORT
int is_brate(unsigned char rrate)
{
	if(rrate==0x82 || rrate==0x84 ||  rrate==0x8b || rrate==0x96 )
		return 1;
	else
		return 0;
}
#endif

unsigned int OnProbeReq(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	struct wifi_mib	*pmib;
	unsigned char	*pframe, *p;
	unsigned char	*ssidptrx=NULL;
	unsigned int	len;
	unsigned char	*bssid, is_11b_only=0;    
    #ifdef WDS
	unsigned int i;
    #endif
    #ifdef P2P_SUPPORT
	int idx=0;
	int brateonly = 1;
	static unsigned char ReAssem_p2pie[MAX_REASSEM_P2P_IE];
	unsigned char *p2pIEPtr = ReAssem_p2pie ;	
	int IEfoundtimes=0;
	int p2pIElen=0;
    #endif
    #if	defined(WIFI_SIMPLE_CONFIG) || defined(P2P_SUPPORT)
	static	unsigned char tmp_assem_wscie[512];
	unsigned char *awPtr = tmp_assem_wscie ;	
	unsigned int foundtimes=0;		
	int lenx =	0;    
	unsigned char *ptmp;
	unsigned int lentmp;
    #endif		

	bssid  = BSSID;
	pmib   = GET_MIB(priv);
	pframe = get_pframe(pfrinfo);

	if (!IS_DRV_OPEN(priv))
		return FAIL;

	if (priv->auto_channel == 1)
		return FAIL;

#ifdef P2P_SUPPORT
        /*allow (p2p device mode && under listen state) can process probe_req frame*/			/*cfg p2p cfg p2p*/
		if(  rtk_p2p_is_enabled(priv) && 
			((rtk_p2p_chk_role(priv,P2P_DEVICE)) && (rtk_p2p_chk_state(priv,P2P_S_LISTEN)))){
				
            /*do no thing just allow go on*/ 
		}
		else
#endif
		{	
			if (!((OPMODE & WIFI_AP_STATE) || (OPMODE & WIFI_ADHOC_STATE))
#ifdef MP_TEST
			|| priv->pshare->rf_ft_var.mp_specific
#endif
			)
				return FAIL;

		}

#ifdef WDS
	if (pmib->dot11WdsInfo.wdsEnabled && pmib->dot11WdsInfo.wdsPure) {
		if (pmib->dot11WdsInfo.wdsNum) {
			for (i = 0; i < pmib->dot11WdsInfo.wdsNum; i++) {
				if (!memcmp(pmib->dot11WdsInfo.entry[i].macAddr, (char *)GetAddr2Ptr(pframe), MACADDRLEN)) {
					break;
				}
			}
			if (i == pmib->dot11WdsInfo.wdsNum) {
				return FAIL;
			}
		}
		else{
			return FAIL;
		}
	}
#endif

	if (pmib->miscEntry.func_off || pmib->miscEntry.raku_only)
		return FAIL;
#ifdef CLIENT_MODE
	if ((OPMODE & WIFI_ADHOC_STATE) &&
			(!priv->ibss_tx_beacon || (OPMODE & WIFI_SITE_MONITOR)))
		return FAIL;
#endif

#ifdef CONFIG_RTK_MESH
	if(pfrinfo->is_11s)
		return OnProbeReq_MP(priv, pfrinfo);
#endif

    /*PSP IOT*/ 
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, (int *)&len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);
	if (p == NULL) {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _SUPPORTEDRATES_IE_,  (int *)&len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);
		if( (p == NULL) || ( len<=4))
			is_11b_only = 1;
	}
    /*PSP IOT*/ 
    
    #ifdef WIFI_SIMPLE_CONFIG
	if (priv->pmib->wscEntry.wsc_enable & 2) { // work as AP (not registrar)
		ptmp = pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_; lentmp = 0;
		for (;;)
		{
			ptmp = get_ie(ptmp, _WPS_IE_, (int *)&lentmp,
			pfrinfo->pktlen - (ptmp - pframe));
			if (ptmp != NULL) {
				if (!memcmp(ptmp+2, WSC_IE_OUI, 4)) {
					foundtimes ++;
					if(foundtimes ==1){
						if ( (lentmp + 2 ) > PROBEIELEN)
						{
							DEBUG_WARN("[%s] WPS_IE length is too big =%d\n", __FUNCTION__, (lentmp+2));
							foundtimes--;
							break;
						} else {
							memcpy(awPtr , ptmp ,lentmp + 2);
							awPtr+= (lentmp + 2);
							lenx += (lentmp + 2);
						}
					}else{
					    if ( (lenx + lentmp-4 ) > PROBEIELEN)
						{
							DEBUG_WARN("[%s] Total length of several WPS_IE is too big =%d, do not include the last WSC IEs\n", __FUNCTION__, (lenx+lentmp-4));
							foundtimes--;
							break;
						} else {
							memcpy(awPtr , ptmp+2+4 ,lentmp-4);
							awPtr+= (lentmp-4);
							lenx += (lentmp-4);
						}
					}					
				}
			}
			else{
				break;
			}

			ptmp = ptmp + lentmp + 2;
		}
		if(foundtimes){
			lenx = (int)(((unsigned long)awPtr)-((unsigned long)tmp_assem_wscie));	
			if(foundtimes>1){
				tmp_assem_wscie[1] = lenx-2;
				//debug_out("ReAss probe_Req wsc_ie ",tmp_assem_wscie,lenx);					
			}
			wsc_forward_probe_request(priv, pframe, tmp_assem_wscie, lenx);						
		}else{	// 
			if( search_wsc_pbc_probe_sta(priv, (unsigned char *)GetAddr2Ptr(pframe))==1){
                DOT11_WSC_PIN_IND wsc_ind;
                wsc_ind.EventId = DOT11_EVENT_WSC_RM_PBC_STA ;
                wsc_ind.IsMoreEvent = 0;
				memcpy(wsc_ind.code,(unsigned char *)GetAddr2Ptr(pframe),6);
                DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)&wsc_ind, sizeof(DOT11_WSC_PIN_IND));
                event_indicate(priv, NULL, -1);  
			}
		}		
	}
    /* WPS2DOTX   */

    #ifdef HS2_SUPPORT
	if (priv->pmib->hs2Entry.interworking_ielen)
	{
		unsigned char *ptmp;
		unsigned int  hs_len;
		ptmp = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _EXTENDED_CAP_IE_, (int *)&hs_len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);

		if (ptmp)
		{
			unsigned char tmp[12];
	
			if (hs_len >= 4)
			{	
				memcpy(tmp, ptmp+2, hs_len);
		
				if (tmp[3] & 0x80)	//interworking capability bit
				{
					ptmp = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _INTERWORKING_IE_, (int *)&hs_len,
							pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);
					
					if (ptmp)
					{
						memcpy(tmp, ptmp+2, hs_len);
						//printk("Check Interworking element, hs_len=%d\n",hs_len);
						//check ant match or not
						if ((tmp[0] & 0x0f) != (priv->pmib->hs2Entry.interworking_ie[0] & 0x0f) && ((tmp[0] & 0x0f) != 15)) //access network type not match
						{
							//printk("ant (access network type) not match\n");	
							goto OnProbeReqFail;	
						}
					
						hs_len -= 1;
						if (hs_len > 1)
						{
							//printk("Case hs_len = %d\n",hs_len);
							if (hs_len == 8)
							{
								if (memcmp(tmp+3, priv->pmib->hs2Entry.interworking_ie+3, 6) && memcmp(tmp+3, "\xff\xff\xff\xff\xff\xff", 6)) //hessid not match
								{
									//printk("no match hessid1, %02x%02x%02x%02x%02x%02x\n",tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8]);
									goto OnProbeReqFail;
								}
							}
							else if (hs_len == 6)
							{
								if (memcmp(tmp+1, priv->pmib->hs2Entry.interworking_ie+3, 6) && memcmp(tmp+1, "\xff\xff\xff\xff\xff\xff", 6)) //hessid not match
								{
									//printk("no match hessid2, %02x%02x%02x%02x%02x%02x\n",tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6]);
									goto OnProbeReqFail;
								}
							}
							else {
								goto OnProbeReqFail;
							}
						}
					}
					else
						panic_printk("enable interworking bit, but no interworking ie!!\n");
				}
			}
		}
	}
    #endif
    #endif


#ifdef P2P_SUPPORT			/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv))
	{
							
		
		/*check P2P Wildcard SSID: The SSID field ��DIRECT-��.*/
		ssidptrx = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _SSID_IE_, (int *)&len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);

		if (ssidptrx == NULL){
			goto OnProbeReqFail;	
		}

		if(  len>=7 && !memcmp(ssidptrx+2,"DIRECT-",7))
		{
			//P2P_SME_P("chk (DIRECT-) ssid\n");
		}else{
			/*no included "DIRECT-" , rsp as normal dev*/ 
			if((rtk_p2p_chk_role(priv,P2P_DEVICE)) && (rtk_p2p_chk_state(priv,P2P_S_LISTEN))){
				goto OnProbeReqFail;		
			}else if(rtk_p2p_chk_role(priv,P2P_TMP_GO) || rtk_p2p_chk_role(priv,P2P_PRE_GO) ){
				goto normal_probe_req;
			}
		}
            
		if(is_11b_only){
            /* 1)P2P Devices shall not respond to Probe Request frames that indicate support for 11b rates only ;test plan 1.07,p25*/
            P2P_DEBUG("	all p2p devices don't care b rate only device\n");            
			goto OnProbeReqFail; // don't care b rate only device
		}


		/*------check include P2P IE ------*/
		ptmp = pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_;	
		/*support ReAssemble*/
		for (;;)
		{
			ptmp = get_ie(ptmp, _P2P_IE_, (int *)&len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_ - len);
			if (ptmp != NULL) {
				if (!memcmp(ptmp+2, WFA_OUI_PLUS_TYPE, 4)) {
					IEfoundtimes ++;
					memcpy(p2pIEPtr , ptmp+6 ,len-4);
					p2pIEPtr+= (len-4);	
				}
			}
			else{
				break;
			}
			ptmp = ptmp + len + 2;
			
		}
				
		if(IEfoundtimes){
			if(IEfoundtimes>1){
				P2P_DEBUG("ReAssembly p2p IE\n");
			}	
			p2pIElen = (int)(((unsigned long)p2pIEPtr)-((unsigned long)ReAssem_p2pie));		
			if(p2pIElen > MAX_REASSEM_P2P_IE){
				P2P_DEBUG("\n\n	reassemble P2P IE exceed MAX_REASSEM_P2P_IE , chk!!!\n\n");
			}else{
				P2P_on_probe_req(priv, pfrinfo, ReAssem_p2pie, p2pIElen);
				return SUCCESS;
			}
		}
		else{

			if((rtk_p2p_chk_role(priv,P2P_DEVICE)) && (rtk_p2p_chk_state(priv,P2P_S_LISTEN)))
				goto OnProbeReqFail;

		}	
		/*------chk include P2P IE ------*/

	}


normal_probe_req:
#endif
	ssidptrx = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _SSID_IE_, (int *)&len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);

	if (ssidptrx == NULL)
		goto OnProbeReqFail;

	if (len == 0) {
		if (HIDDEN_AP || TAKEOVER_HIDDEN_AP)
			goto OnProbeReqFail;
		else
			goto send_rsp;
	}

	if ((len != SSID_LEN) ||
			memcmp((void *)(ssidptrx+2), (void *)SSID, SSID_LEN)) {
		if ((len == 3) &&
				((*(ssidptrx+2) == 'A') || (*(ssidptrx+2) == 'a')) &&
				((*(ssidptrx+3) == 'N') || (*(ssidptrx+3) == 'n')) &&
				((*(ssidptrx+4) == 'Y') || (*(ssidptrx+4) == 'y'))) {
			if (pmib->dot11OperationEntry.deny_any)
				goto OnProbeReqFail;
			else
				if (HIDDEN_AP || TAKEOVER_HIDDEN_AP)
					goto OnProbeReqFail;
				else
					goto send_rsp;
		}
		else
			goto OnProbeReqFail;
	}
	
send_rsp:

    #ifdef CONFIG_RTK_MESH
    if(pmib->dot1180211sInfo.meshSilence) {
        return FAIL;
    }    
    else if( pmib->dot1180211sInfo.mesh_enable && !pmib->dot1180211sInfo.mesh_ap_enable) {
        issue_probersp(priv, GetAddr2Ptr(pframe), NULL, 0, 1, is_11b_only);
    }
    else
    #endif      
    {
		if(priv->pmib->dot11StationConfigEntry.probe_info_enable){
			if (check_probe_sta_rssi_valid(priv,GetAddr2Ptr(pframe),pfrinfo->rssi))
				add_probe_req_sta(priv,GetAddr2Ptr(pframe),pfrinfo->rssi);	
		}
        issue_probersp(priv, GetAddr2Ptr(pframe), SSID, SSID_LEN, 1, is_11b_only);
    }
	return SUCCESS;

OnProbeReqFail:

	return FAIL;
}


unsigned int OnProbeRsp(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
#ifdef WDS
    struct stat_info *pstat = NULL;
#endif

#if defined(CONFIG_RTL_WAPI_SUPPORT)
    uint8		*p;
    int			len;
#endif

#if defined(WDS)||defined(CONFIG_RTL_WAPI_SUPPORT)
    unsigned char *pframe = get_pframe(pfrinfo);
#endif
// ==== modified by GANTOE for site survey 2008/12/25 ====
    if (OPMODE & WIFI_SITE_MONITOR)
    {

#ifdef P2P_SUPPORT
        /*cfg p2p cfg p2p*/
        if( (rtk_p2p_is_enabled(priv)==PROPERTY_P2P) && P2P_DISCOVERY)
        {
            p2p_collect_bss_info(priv, pfrinfo, WIFI_PROBERSP);
        }
        else
#endif
        {
            collect_bss_info(priv, pfrinfo);
        }
		
#if defined(CONFIG_RTL_WAPI_SUPPORT)
		p = get_ie(pframe + WLAN_HDR_A3_LEN, _EID_WAPI_, 
			&len, pfrinfo->pktlen);
		if (p)
		{
			memcpy(priv->aeWapiIE, p, len+2);
			priv->aeWapiIELength = len+2;
		}
#endif
	}
#ifdef WDS
	else if ((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11WdsInfo.wdsEnabled &&
		priv->pmib->dot11WdsInfo.wdsNum) {
		pstat = get_stainfo(priv, (unsigned char *)GetAddr2Ptr(pframe));
		if (pstat && (pstat->state & WIFI_WDS)) {
			collect_bss_info(priv, pfrinfo);
			#if 0
			if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))
				pstat->tx_bw = HT_CHANNEL_WIDTH_20_40;
			else
				pstat->tx_bw = HT_CHANNEL_WIDTH_20;
			#endif

#if defined(CONFIG_PCI_HCI)
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
				add_RATid(priv, pstat);
#endif
			} else
#endif
			{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)			
				add_update_RATid(priv, pstat);
#endif
			}
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			update_STA_RATid(priv, pstat);
#endif
			assign_aggre_mthod(priv, pstat);
			assign_tx_rate(priv, pstat, pfrinfo);
			assign_aggre_size(priv, pstat);
#ifdef TX_SHORTCUT
			/*Clear WDS station tx sc info*/
			memset(pstat->tx_sc_ent, 0x0, sizeof(pstat->tx_sc_ent));
#endif
			if (!pstat->wds_probe_done){
				//WDEBUG("\n=>wds_probe_done=1\n\n");
#ifdef CONFIG_WLAN_HAL					
				if(IS_HAL_CHIP(priv)){
					GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat, INCREASE);
				// Have to inform firmware that wds is connectted, so RA can operate correctly			
				}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
				if(GET_CHIP_VER(priv)== VERSION_8812E){
					UpdateHalMSRRPT8812(priv, pstat, INCREASE); 					
				}
#endif
				pstat->wds_probe_done = 1;
			}
		}
	}
#endif
#ifdef CONFIG_RTK_MESH	// ==== GANTOE ====
	if(pfrinfo->is_11s)
		return OnProbeRsp_MP(priv, pfrinfo);
#endif

	return SUCCESS;
}

unsigned int OnBeacon(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	int i, len;
	unsigned char *p, *pframe, channel;

	if (OPMODE & WIFI_SITE_MONITOR) {
		collect_bss_info(priv, pfrinfo);
		return SUCCESS;
	}

	pframe = get_pframe(pfrinfo);

	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _DSSET_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if (p != NULL)
		channel = *(p+2);
	else
		channel = priv->pmib->dot11RFEntry.dot11channel;

	// If used as AP in G mode, need monitor other 11B AP beacon to enable
	// protection mechanism
#ifdef WDS
	// if WDS is used, need monitor other WDS AP beacon to decide tx rate
	if (priv->pmib->dot11WdsInfo.wdsEnabled ||
		((OPMODE & WIFI_AP_STATE) && (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G|WIRELESS_11A)) &&
		 (channel == priv->pmib->dot11RFEntry.dot11channel)))
#else
	if ((OPMODE & WIFI_AP_STATE) &&
		(priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G|WIRELESS_11A)) &&
		(channel == priv->pmib->dot11RFEntry.dot11channel))
#endif
	{
		// look for ERP rate. if no ERP rate existed, thought it is a legacy AP
		unsigned char supportedRates[32];
		int supplen=0, legacy=1;

		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_,
				_SUPPORTEDRATES_IE_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p) {
			if (len>8)
				len=8;
			memcpy(&supportedRates[supplen], p+2, len);
			supplen += len;
		}

		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_,
				_EXT_SUPPORTEDRATES_IE_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p) {
			if (len>8)
				len=8;
			memcpy(&supportedRates[supplen], p+2, len);
			supplen += len;
		}

#ifdef WDS
		if (priv->pmib->dot11WdsInfo.wdsEnabled) {
			struct stat_info *pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
			if (pstat && !(pstat->state & WIFI_WDS_RX_BEACON)) {
				get_matched_rate(priv, supportedRates, &supplen, 0);
				update_support_rate(pstat, supportedRates, supplen);
				if (supplen == 0)
					pstat->current_tx_rate = 0;
				else {
					if (priv->pmib->dot11WdsInfo.entry[pstat->wds_idx].txRate == 0) {
						pstat->current_tx_rate = find_rate(priv, pstat, 1, 0);
						//pstat->upper_tx_rate = 0;	// unused
					}
				}

				// Customer proprietary IE
				if (priv->pmib->miscEntry.private_ie_len) {
					p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_,
						priv->pmib->miscEntry.private_ie[0], &len,
						pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
					if (p) {
						memcpy(pstat->private_ie, p, len + 2);
						pstat->private_ie_len = len + 2;
					}
				}

				// Realtek proprietary IE
                pstat->is_realtek_sta = FALSE;				
				p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_; len = 0;
				for (;;)
				{
					p = get_ie(p, _RSN_IE_1_, &len,
					pfrinfo->pktlen - (p - pframe));
					if (p != NULL) {
						if (!memcmp(p+2, Realtek_OUI, 3) && *(p+2+3) == 2) { /*found realtek out and type == 2*/
							pstat->is_realtek_sta = TRUE;
							break;
						}
					}
					else
						break;
					p = p + len + 2;
				}

#ifdef WIFI_WMM
				if (QOS_ENABLE) {
					p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;
					for (;;) {
						p = get_ie(p, _RSN_IE_1_, &len,
								pfrinfo->pktlen - (p - pframe));
						if (p != NULL) {
							if (!memcmp(p+2, WMM_PARA_IE, 6)) {
								pstat->QosEnabled = 1;
								break;
							}
						}
						else {
							pstat->QosEnabled = 0;
							break;
						}
						p = p + len + 2;
					}
				}
#endif
				if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
					p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_, &len,
						pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
					if (p !=  NULL) {
						pstat->ht_cap_len = len;
						memcpy((unsigned char *)&pstat->ht_cap_buf, p+2, len);
						if (cpu_to_le16(pstat->ht_cap_buf.ht_cap_info) & _HTCAP_AMSDU_LEN_8K_) {
							pstat->is_8k_amsdu = 1;
							pstat->amsdu_level = 7935 - sizeof(struct wlan_hdr);
						}
						else {
							pstat->is_8k_amsdu = 0;
							pstat->amsdu_level = 3839 - sizeof(struct wlan_hdr);
						}
					}
					else
						pstat->ht_cap_len = 0;
				}

				if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))
					pstat->tx_bw = HT_CHANNEL_WIDTH_20_40;
				else
					pstat->tx_bw = HT_CHANNEL_WIDTH_20;


				/*-----------------get VHT info------------------*/				
#ifdef RTK_AC_SUPPORT		//WDS-VHT support
				if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
				{
					p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTCapability, &len,
							pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

					if ((p !=  NULL) && (len <= sizeof(struct vht_cap_elmt))) {
						pstat->vht_cap_len = len;
						memcpy((unsigned char *)&pstat->vht_cap_buf, p+2, len);
						//WDEBUG("vht_cap len = %d \n",len);		
						//WDEBUG("vht_cap vht_cap_info:%04X \n",pstat->vht_cap_buf.vht_cap_info);	
						//WDEBUG("vht_cap vht_support_mcs[0]=[%04X] ; vht_support_mcs[1]=[%04X] \n", pstat->vht_cap_buf.vht_support_mcs[0],pstat->vht_cap_buf.vht_support_mcs[1]);

					}

					p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTOperation, &len,
							pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

					if ((p !=  NULL) && (len <= sizeof(struct vht_oper_elmt))) {
						pstat->vht_oper_len = len;
						memcpy((unsigned char *)&pstat->vht_oper_buf, p+2, len);
						//WDEBUG("vht_oper len = %d \n",len);
						//WDEBUG("vht_oper: 0[%02x],1[%02x],2[%02x] \n",pstat->vht_oper_buf.vht_oper_info[0],pstat->vht_oper_buf.vht_oper_info[1],pstat->vht_oper_buf.vht_oper_info[2]);						
						//WDEBUG("vht_oper [%02X]\n",pstat->vht_oper_buf.vht_basic_msc);
						
					}

					if(pstat->vht_cap_len){
						switch(cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & 0x3) {
							default:				
							case 0:	
								pstat->is_8k_amsdu = 0;
								pstat->amsdu_level = 3895 - sizeof(struct wlan_hdr);
								break;
							case 1:
								pstat->is_8k_amsdu = 1;
								pstat->amsdu_level = 7991 - sizeof(struct wlan_hdr);
								break;
							case 2:	
								pstat->is_8k_amsdu = 1;
								pstat->amsdu_level = 11454 - sizeof(struct wlan_hdr);	
								break;
						}

						if (priv->vht_oper_buf.vht_oper_info[0] == 1) {
							pstat->tx_bw = HT_CHANNEL_WIDTH_80;
							priv->pshare->is_40m_bw	= HT_CHANNEL_WIDTH_80;	
							//WDEBUG("pstat->tx_bw=80M\n");
						}
						
					}

					p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTOperatingMode, &len,
						pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
					if ((p !=  NULL) && (len == 1)) {
							// check self capability....					
							if((p[2] &3) <= priv->pmib->dot11nConfigEntry.dot11nUse40M)
								pstat->tx_bw = p[2] &3;
							pstat->nss = ((p[2]>>4)&0x7)+1;											
							//WDEBUG("receive opering mode data = %02X\n" ,p[2]);
					}

#ifdef CONFIG_RTL_8812_SUPPORT
					if(GET_CHIP_VER(priv)== VERSION_8812E){

						//WDEBUG("UpdateHalRAMask8812 \n");
						UpdateHalRAMask8812(priv, pstat, 3);						
						//WDEBUG("UpdateHalMSRRPT8812 \n");
						UpdateHalMSRRPT8812(priv, pstat, INCREASE);
					}
#endif
#ifdef  CONFIG_WLAN_HAL
					if (IS_HAL_CHIP(priv)){
						GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
					}
#endif

				}
#endif	
				/*-----------------get VHT info------------------*/								

				assign_tx_rate(priv, pstat, pfrinfo);
				assign_aggre_mthod(priv, pstat);
				assign_aggre_size(priv, pstat);
				pstat->state |= WIFI_WDS_RX_BEACON;
			}

			if (pstat && pstat->state & WIFI_WDS) {
				pstat->beacon_num++;
				if (!pstat->wds_probe_done){
					pstat->wds_probe_done = 1;
					collect_bss_info(priv, pfrinfo);
#ifdef CONFIG_WLAN_HAL					
				if(IS_HAL_CHIP(priv)){
					GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat, INCREASE);
				// Have to inform firmware that wds is connectted, so RA can operate correctly			
				}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
				if(GET_CHIP_VER(priv)== VERSION_8812E){
					UpdateHalMSRRPT8812(priv, pstat, INCREASE); 					
				}
#endif
					assign_aggre_mthod(priv, pstat);
					assign_aggre_size(priv, pstat);					
					//WDEBUG("\n=>wds_probe_done=1\n\n");					
				}
			}
		}
#endif

		for (i=0; i<supplen; i++) {
			if (!is_CCK_rate(supportedRates[i]&0x7f)) {
				legacy = 0;
				break;
			}
		}

		// look for ERP IE and check non ERP present
		if (legacy == 0) {
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _ERPINFO_IE_,
					&len, pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if (p && (*(p+2) & BIT(0)))
				legacy = 1;
		}

		if (legacy) {
			if (!priv->pmib->dot11StationConfigEntry.olbcDetectDisabled &&
							priv->pmib->dot11ErpInfo.olbcDetected==0) {
				priv->pmib->dot11ErpInfo.olbcDetected = 1;
				check_protection_shortslot(priv);
				DEBUG_INFO("OLBC detected\n");
			}
			if (priv->pmib->dot11ErpInfo.olbcDetected)
				priv->pmib->dot11ErpInfo.olbcExpired = DEFAULT_OLBC_EXPIRE;
		}
	}

	if ((OPMODE & WIFI_AP_STATE) &&
		(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) 
//		&& (channel == priv->pmib->dot11RFEntry.dot11channel)
		) 
		{
		if (!priv->pmib->dot11StationConfigEntry.protectionDisabled &&
				!priv->pmib->dot11StationConfigEntry.olbcDetectDisabled) {
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_,
				&len, pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if (p == NULL)
				priv->ht_legacy_obss_to = 60;	
		}

		if (!priv->pmib->dot11StationConfigEntry.protectionDisabled &&
				!priv->pmib->dot11StationConfigEntry.nmlscDetectDisabled) {
		
				p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_IE_, &len,
						pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
				if (p !=  NULL) {
					struct ht_info_elmt *ht_info=(struct ht_info_elmt *)(p+2);
					if (len) {
						unsigned int prot_mode =  (cpu_to_le16(ht_info->info1) & 0x03);
						if (prot_mode == _HTIE_OP_MODE3_)
							priv->ht_nomember_legacy_sta_to= 60;	
					}
				}		
		}
	}

#ifdef WIFI_11N_2040_COEXIST
	if (priv->pmib->dot11nConfigEntry.dot11nCoexist && (OPMODE & WIFI_AP_STATE) &&
		(priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N|WIRELESS_11G)) && 
		priv->pshare->is_40m_bw) {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_,
			&len, pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if ((p == NULL) && bg_ap_rssi_chk(priv, pfrinfo, channel)) {

			if (channel && (priv->pmib->dot11nConfigEntry.dot11nCoexist_ch_chk ?
			(channel != priv->pmib->dot11RFEntry.dot11channel) :1))
			{			
				if(!priv->bg_ap_timeout) {
					priv->bg_ap_timeout = 60;
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)					
					update_RAMask_to_FW(priv, 1);
#endif					
					SetTxPowerLevel(priv);
				}
				//priv->bg_ap_timeout = 60;
			}
		}
	}
#endif


#ifdef CONFIG_RTK_MESH
    if(pfrinfo->is_11s) {
        OnBeacon_MP(priv, pfrinfo);
    }
#endif

	return SUCCESS;
}


unsigned int OnDisassoc(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char *pframe;
	struct  stat_info   *pstat;
	unsigned char *sa;
	unsigned short reason;
	DOT11_DISASSOCIATION_IND Disassociation_Ind;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	pframe = get_pframe(pfrinfo);
	sa = GetAddr2Ptr(pframe);
	pstat = get_stainfo(priv, sa);

	if (pstat == NULL)
		return 0;

#ifdef CONFIG_IEEE80211W
	if(pstat->isPMF) {
		pstat->isPMF = 0; // transmit unprotected mgmt frame
		issue_disassoc(priv, sa, 7);
	}
#endif

#ifdef RTK_WOW
	if (pstat->is_rtk_wow_sta)
		return 0;
#endif

#ifdef P2P_SUPPORT
    P2P_DEBUG("	............on DisAssoc\n");
    MAC_PRINT(pstat->hwaddr);
    /*cfg p2p cfg p2p*/	
    if((rtk_p2p_is_enabled(priv) == PROPERTY_P2P) && (rtk_p2p_chk_role(priv,P2P_TMP_GO))){
        if(pstat->is_p2p_client){
            p2p_client_remove(priv,pstat);
        }
    }
#endif

	reason = cpu_to_le16(*(unsigned short *)((unsigned long)pframe + WLAN_HDR_A3_LEN ));
	DEBUG_INFO("receiving disassoc from station %02X%02X%02X%02X%02X%02X reason %d\n",
		pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
		pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5], reason);

#ifdef MULTI_MAC_CLONE
	__del_mclone_addr(priv, pstat->hwaddr);//this function only AP in.
#endif

	SAVE_INT_AND_CLI(flags);

	if (asoc_list_del(priv, pstat))
	{
		if (pstat->expire_to > 0)
		{
			cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
			check_sta_characteristic(priv, pstat, DECREASE);
		}
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E && pstat->sta_in_firmware == 1) {
			RTL8188E_MACID_NOLINK(priv, 1, REMAP_AID(pstat));
			RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
            pstat->txpause_flag = 0;            
		}
#endif

#ifdef CONFIG_WLAN_HAL
        if(IS_HAL_CHIP(priv))
        {
            if(pstat && (REMAP_AID(pstat) < 128))
            {
                DEBUG_WARN("%s %d OnDisassoc, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
                GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0 , REMAP_AID(pstat));                
                pstat->txpause_flag = 0;
            }
            else
            {
                DEBUG_WARN(" MACID sleep only support 128 STA \n");
            }
        }
#endif

	}

#ifdef CONFIG_RTL8186_KB
	if (priv->pmib->dot11OperationEntry.guest_access || (pstat && pstat->ieee8021x_ctrlport == DOT11_PortStatus_Guest))
	{
		if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == 0)
		{
			/* hotel style guest access */
			set_guestmacinvalid(priv, sa);
		}
	}
#endif

	// Need change state back to autehnticated
	if (IEEE8021X_FUN)
	{
#ifndef WITHOUT_ENQUEUE
		memcpy((void *)Disassociation_Ind.MACAddr, (void *)sa, MACADDRLEN);
		Disassociation_Ind.EventId = DOT11_EVENT_DISASSOCIATION_IND;
		Disassociation_Ind.IsMoreEvent = 0;
		Disassociation_Ind.Reason = reason;
		Disassociation_Ind.tx_packets = pstat->tx_pkts;
		Disassociation_Ind.rx_packets = pstat->rx_pkts;
		Disassociation_Ind.tx_bytes   = pstat->tx_bytes;
		Disassociation_Ind.rx_bytes   = pstat->rx_bytes;
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Disassociation_Ind,
					sizeof(DOT11_DISASSOCIATION_IND));
#endif
	}

	release_stainfo(priv, pstat);
	init_stainfo(priv, pstat);
	pstat->state |= WIFI_AUTH_SUCCESS;
	pstat->expire_to = priv->assoc_to;
	auth_list_add(priv, pstat);

	RESTORE_INT(flags);

	if (IEEE8021X_FUN)
	{
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
		psk_indicate_evt(priv, DOT11_EVENT_DISASSOCIATION_IND, sa, NULL, 0);
#endif
	}

#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
	if (priv->sta_status_cb_func)
		priv->sta_status_cb_func("wlan0", sa, WIFI_STA_STATUS_DISASSOCIATED);
#else
	LOG_MSG("A wireless client is disassociated - %02X:%02X:%02X:%02X:%02X:%02X\n",
		*sa, *(sa+1), *(sa+2), *(sa+3), *(sa+4), *(sa+5));
#endif
#ifdef RTK_NL80211
	event_indicate_cfg80211(priv, sa, CFG80211_DEL_STA, NULL);
#endif
#ifdef WIFI_HAPD
	event_indicate_hapd(priv, sa, HAPD_EXIRED, NULL);
#ifdef HAPD_DRV_PSK_WPS
	event_indicate(priv, sa, 2);
#endif
#else
	event_indicate(priv, sa, 2);
#endif


	return SUCCESS;
}


unsigned int OnAuth(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned int		privacy,seq, len;
#if !defined(SMP_SYNC) || (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))    
	unsigned long		flags=0;
#endif
	struct list_head	*phead, *plist;
	struct wlan_acl_node *paclnode;
	unsigned int		acl_mode;

	struct wifi_mib		*pmib;
	struct stat_info	*pstat=NULL;
	unsigned char		*pframe, *sa, *p;
	unsigned int		res=FAIL;
	UINT16				algorithm;
	int					status, alloc_pstat=0;
	struct ac_log_info	*log_info;

	pmib = GET_MIB(priv);

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	UINT8	isMeshMP = FALSE, prevState = MP_UNUSED;
#endif
	acl_mode = priv->pmib->dot11StationConfigEntry.dot11AclMode;
	pframe = get_pframe(pfrinfo);
	sa = GetAddr2Ptr(pframe);
	pstat = get_stainfo(priv, sa);

	if (!IS_DRV_OPEN(priv))
		return FAIL;
	if (!(OPMODE & WIFI_AP_STATE))
		return FAIL;

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH) 	// Is Mesh MP?
	if ((1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && (NULL != pstat) && isPossibleNeighbor(pstat)) {
		prevState = pstat->mesh_neighbor_TBL.State;
		isMeshMP = TRUE;
	}
#endif
#ifdef WDS
#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	if ((pmib->dot11WdsInfo.wdsPure) && (isMeshMP==FALSE))
#else
	if (pmib->dot11WdsInfo.wdsPure)
#endif
		return FAIL;
#endif

	if (pmib->miscEntry.func_off || pmib->miscEntry.raku_only)
		return FAIL;

	privacy = priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm;

	seq = cpu_to_le16(*(unsigned short *)((unsigned long)pframe + WLAN_HDR_A3_LEN + 2));

	algorithm = cpu_to_le16(*(unsigned short *)((unsigned long)pframe + WLAN_HDR_A3_LEN));

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	MESH_DEBUG_MSG("\nMesh: Auth START !! seq=%d\n", seq);

	if (FALSE == isMeshMP)
#endif
	{
	if (GetPrivacy(pframe))
	{
		int use_keymapping=0;
		status = wep_decrypt(priv, pfrinfo, pfrinfo->pktlen,
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm, use_keymapping);

		if (status == FALSE)
		{
			SAVE_INT_AND_CLI(flags);
#ifdef CONFIG_RTL8196B_TLD
			LOG_MSG_DEL("[WLAN access rejected: incorrect security] from MAC address: %02x:%02x:%02x:%02x:%02x:%02x,\n",
				sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);
#endif

			DEBUG_ERR("wep-decrypt a Auth frame error!\n");
			status = _STATS_CHALLENGE_FAIL_;
			goto auth_fail;
		}

		seq = cpu_to_le16(*(unsigned short *)((unsigned long)pframe + WLAN_HDR_A3_LEN + 4 + 2));
		algorithm = cpu_to_le16(*(unsigned short *)((unsigned long)pframe + WLAN_HDR_A3_LEN + 4));
	}
#ifdef WIFI_SIMPLE_CONFIG
#ifndef CONFIG_RTL8196B_TLD
	else {
		if (pmib->wscEntry.wsc_enable && (seq == 1) && (algorithm == 0))
			privacy = 0;
	}
#endif
#endif

	DEBUG_INFO("auth alg=%x, seq=%X\n", algorithm, seq);

	if (privacy == 2 &&
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_40_PRIVACY_ &&
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_104_PRIVACY_)
		privacy = 0;

	if ((algorithm > 0 && privacy == 0) ||	// rx a shared-key auth but shared not enabled
		(algorithm == 0 && privacy == 1) )	// rx a open-system auth but shared-key is enabled
	{
		SAVE_INT_AND_CLI(flags);
		DEBUG_ERR("auth rejected due to bad alg [alg=%d, auth_mib=%d] %02X%02X%02X%02X%02X%02X\n",
			algorithm, privacy, sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);
#ifdef CONFIG_RTL8196B_TLD
		LOG_MSG_DEL("[WLAN access rejected: incorrect security] from MAC address: %02x:%02x:%02x:%02x:%02x:%02x,\n",
			sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);
#endif
		status = _STATS_NO_SUPP_ALG_;
		goto auth_fail;
	}

	// STA ACL check;nctu note
	SAVE_INT_AND_CLI(flags);
	SMP_LOCK_ACL(flags);

	phead = &priv->wlan_acl_list;
	plist = phead->next;
	//check sa
	if (acl_mode == 1)		// 1: positive check, only those on acl_list can be connected.
		res = FAIL;
	else
		res = SUCCESS;

	while(plist != phead)
	{
		paclnode = list_entry(plist, struct wlan_acl_node, list);
		plist = plist->next;
		if (!memcmp((void *)sa, paclnode->addr, 6)) {
			if (paclnode->mode & 2) { // deny
				res = FAIL;
				break;
			}
			else {
				res = SUCCESS;
				break;
			}
		}
	}

	RESTORE_INT(flags);
	SMP_UNLOCK_ACL(flags);

#if defined(CONFIG_RTK_MESH) && defined(MESH_ESTABLISH_RSSI_THRESHOLD)
		if (pfrinfo->rssi < priv->mesh_fake_mib.establish_rssi_threshold)
			res = FAIL;
#endif

#ifdef __DRAYTEK_OS__
	if (res == SUCCESS) {
		if (cb_auth_request(priv->dev, sa) != 0)
			res = FAIL;
	}
#endif

	if (res != SUCCESS) {
		DEBUG_ERR("auth abort because ACL!\n");

		log_info = aclog_lookfor_entry(priv, sa);
		if (log_info) {
			aclog_update_entry(log_info, sa);

			if (log_info->cur_cnt == 1) { // first time trigger
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
				LOG_MSG_DROP("Unauthorized wireless PC try to connect;note:%02x:%02x:%02x:%02x:%02x:%02x;\n",
					*sa, *(sa+1), *(sa+2), *(sa+3), *(sa+4), *(sa+5));
#elif defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
				LOG_MSG_DROP("Unauthorized wireless PC try to connect;note:%02x:%02x:%02x:%02x:%02x:%02x;\n",
					*sa, *(sa+1), *(sa+2), *(sa+3), *(sa+4), *(sa+5));
#elif defined(CONFIG_RTL8196B_TLD)
				LOG_MSG_DEL("[WLAN access denied] from MAC: %02x:%02x:%02x:%02x:%02x:%02x,\n",
					*sa, *(sa+1), *(sa+2), *(sa+3), *(sa+4), *(sa+5));
#else
				LOG_MSG("A wireless client was rejected due to access control - %02X:%02X:%02X:%02X:%02X:%02X\n",
					*sa, *(sa+1), *(sa+2), *(sa+3), *(sa+4), *(sa+5));
#endif
				log_info->last_cnt = log_info->cur_cnt;

				if (priv->acLogCountdown == 0)
					priv->acLogCountdown = AC_LOG_TIME;
			}
		}
		return FAIL;
	}

	if (priv->pmib->dot11StationConfigEntry.supportedStaNum) {
		if (!pstat && priv->assoc_num >= priv->pmib->dot11StationConfigEntry.supportedStaNum) {
			DEBUG_ERR("Exceed the upper limit of supported clients...\n");
			status = _STATS_UNABLE_HANDLE_STA_;
			goto auth_fail;
		}
	}
	}	//if MESH is enable here is end of (FALSE == isMeshMP)

	// (below, share with Mesh) due to the free_statinfo in AUTH_TO, we should enter critical section here!
	SAVE_INT_AND_CLI(flags);

	if (pstat == NULL)	// STA only, other one, Don't detect peer MP myself, But peer MP detect me and send Auth request.;nctu note
	{
#ifdef CONFIG_RTK_MESH
		// Denied STA auth, When MP configure MAP "OFF"(beacon or ProbeREQ/RSP AP information OFF), and STA ignore these information.
		if (((1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && (0 == GET_MIB(priv)->dot1180211sInfo.mesh_ap_enable)) || 
			GET_MIB(priv)->dot1180211sInfo.meshSilence) {
			status = _STATS_OTHER_;
			goto auth_fail;
		}
#endif

		// allocate a new one
		DEBUG_INFO("going to alloc stainfo for sa=%02X%02X%02X%02X%02X%02X\n",  sa[0],sa[1],sa[2],sa[3],sa[4],sa[5]);
		pstat = alloc_stainfo(priv, sa, -1);

		if (pstat == NULL)
		{
			DEBUG_ERR("Exceed the upper limit of supported clients...\n");
			status = _STATS_UNABLE_HANDLE_STA_;
			goto auth_fail;
		}
		pstat->state = WIFI_AUTH_NULL;
		pstat->auth_seq = 0;	// clear in alloc_stainfo;nctu note
		pstat->tpcache_mgt = GetTupleCache(pframe);
	}
#ifdef CONFIG_IEEE80211W
	else if(pstat->isPMF)
	{		
		pstat->auth_seq = seq + 1;
		goto auth_success;
	}
#endif		
	else
	{	// close exist connection.;nctu note
		if (asoc_list_del(priv, pstat))
		{
#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
			if ((TRUE == isMeshMP)	// fix: 00000053 2007/12/11  NOTE!! Best solution detail see bug report !!
					&& ((pstat->mesh_neighbor_TBL.State == MP_SUPERORDINATE_LINK_UP) || (pstat->mesh_neighbor_TBL.State == MP_SUBORDINATE_LINK_UP)
					|| (pstat->mesh_neighbor_TBL.State == MP_SUPERORDINATE_LINK_DOWN) || (pstat->mesh_neighbor_TBL.State == MP_SUBORDINATE_LINK_DOWN_E)))
			{
#if 0
				LOG_MESH_MSG("OnAuth mesh_cnt_ASSOC_PeerLink_CAP(-)\n");
#endif
				mesh_cnt_ASSOC_PeerLink_CAP(priv, pstat, DECREASE);
				SMP_LOCK_MESH_MP_HDR(flags);
				if (!list_empty(&pstat->mesh_mp_ptr))	// add by Galileo
					list_del_init(&(pstat->mesh_mp_ptr));
				SMP_UNLOCK_MESH_MP_HDR(flags);

			}
#endif

			if (pstat->expire_to > 0)
			{
				cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
				check_sta_characteristic(priv, pstat, DECREASE);
			}
		}
		if (seq==1) {
#ifdef  SUPPORT_TX_MCAST2UNI
			int ipmc_num;
			struct ip_mcast_info ipmc[MAX_IP_MC_ENTRY];

			ipmc_num = pstat->ipmc_num;
			if (ipmc_num)
				memcpy(ipmc, pstat->ipmc, MAX_IP_MC_ENTRY * sizeof(struct ip_mcast_info));
#endif
			release_stainfo(priv, pstat);
			init_stainfo(priv, pstat);
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
			if (GET_CHIP_VER(priv)==VERSION_8188E)
#ifdef RATEADAPTIVE_BY_ODM
				ODM_RAInfo_Init(ODMPTR, pstat->aid);
#else
				RateAdaptiveInfoInit(&priv->pshare->RaInfo[pstat->aid]);
#endif
#endif

			pstat->tpcache_mgt = GetTupleCache(pframe);
#ifdef  SUPPORT_TX_MCAST2UNI
			if (ipmc_num) {
				pstat->ipmc_num = ipmc_num;
				memcpy(pstat->ipmc, ipmc, MAX_IP_MC_ENTRY * sizeof(struct ip_mcast_info));
			}
#endif

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
			if (TRUE == isMeshMP) {
				SMP_LOCK_MESH_MP_HDR(flags);
				if (!list_empty(&pstat->mesh_mp_ptr))
					list_del_init(&pstat->mesh_mp_ptr);
				SMP_UNLOCK_MESH_MP_HDR(flags);

				// Avoid pstat remain in system when can't auth, Not actually use
				pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_LISTEN_TO;
				pstat->mesh_neighbor_TBL.State = MP_LISTEN;
				pstat->state = WIFI_AUTH_NULL;
				SET_PSEUDO_RANDOM_NUMBER(pstat->mesh_neighbor_TBL.LocalLinkID);

				SMP_LOCK_MESH_MP_HDR(flags);
				list_add_tail(&(pstat->mesh_mp_ptr), &(priv->mesh_auth_hdr));
				SMP_UNLOCK_MESH_MP_HDR(flags);

				if (!(timer_pending(&priv->mesh_auth_timer)))	// start timer if stop
					mod_timer(&priv->mesh_auth_timer, jiffies + MESH_TIMER_TO);
			}
#endif

		}
	}

	auth_list_add(priv, pstat);

	if (pstat->auth_seq == 0)
		pstat->expire_to = priv->auth_to;

	// Authentication Sequence (STA only)
#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)	// Mesh can't check, Because recived seq=1 and 2
	if((FALSE == isMeshMP) && ((pstat->auth_seq + 1) != seq))
#else
	if ((pstat->auth_seq + 1) != seq)
#endif
	{
		DEBUG_ERR("(1)auth rejected because out of seq [rx_seq=%d, exp_seq=%d]!\n",
			seq, pstat->auth_seq+1);
		status = _STATS_OUT_OF_AUTH_SEQ_;
		goto auth_fail;
	}

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	if (algorithm == 0 && ((FALSE == isMeshMP && (privacy == 0 || privacy == 2)) || (TRUE == isMeshMP)))	// open auth (STA & mesh)
#else
	if (algorithm == 0 && (privacy == 0 || privacy == 2)) // STA only open auth
#endif
	{
#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
		if ((1 == seq) || ((2 == seq) && (TRUE == isMeshMP)))
#else
		if (seq == 1)
#endif
		{
#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
			if((2 == seq) && ((MP_OPEN_SENT != prevState) ||
				(_STATS_SUCCESSFUL_ != cpu_to_le16(*(unsigned short *)((unsigned long)pframe + WLAN_HDR_A3_LEN + 2 + 2)))))
				return FAIL;
#endif

			pstat->state &= ~WIFI_AUTH_NULL;
			pstat->state |= WIFI_AUTH_SUCCESS;
			pstat->expire_to = priv->assoc_to;
			pstat->AuthAlgrthm = algorithm;

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
			if (TRUE == isMeshMP) {	// Authentication Success

				SMP_LOCK_MESH_MP_HDR(flags);
				if (!list_empty(&pstat->mesh_mp_ptr))	// mesh_auth_hdr -> mesh_unEstablish_hdr
					list_del_init(&(pstat->mesh_mp_ptr));
				SMP_UNLOCK_MESH_MP_HDR(flags);

				if ((1 == seq) && (MP_OPEN_SENT != prevState)) { // Passive
					pstat->expire_to = priv->assoc_to;
					MESH_DEBUG_MSG("Mesh: Auth Successful... seq = '%d', state = '%d', And 'PASSIVE' connect\n", seq, prevState);
					pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_LISTEN_TO;
					pstat->mesh_neighbor_TBL.State = MP_LISTEN;
				} else { // Active (include associated)
					MESH_DEBUG_MSG("Mesh: Auth Successful... seq = '%d', state = '%d', And 'ACTIVE' connect\n", seq, prevState);
					pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_RETRY_TO;
					pstat->mesh_neighbor_TBL.retry = 0;
					pstat->mesh_neighbor_TBL.State = MP_OPEN_SENT;
				}

				SMP_LOCK_MESH_MP_HDR(flags);
				list_add_tail(&(pstat->mesh_mp_ptr), &(priv->mesh_unEstablish_hdr));
				SMP_UNLOCK_MESH_MP_HDR(flags);

				if (!(timer_pending(&priv->mesh_peer_link_timer)))		// start timer if stop
					mod_timer(&priv->mesh_peer_link_timer, jiffies + MESH_TIMER_TO);

			}
#endif

		}
		else
		{
			DEBUG_ERR("(2)auth rejected because out of seq [rx_seq=%d, exp_seq=%d]!\n",
				seq, pstat->auth_seq+1);
			status = _STATS_OUT_OF_AUTH_SEQ_;
			goto auth_fail;
		}
	}
	else // shared system or auto authentication (STA only).
	{
#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
		if (TRUE == isMeshMP) {
			MESH_DEBUG_MSG("Mesh: Auth rejected because You're Mesh MP, but use incorrect algorithm = %d!\n", algorithm);
			status = _STATS_CHALLENGE_FAIL_;
			goto auth_fail;
		}
#endif
		if (seq == 1)
		{
			//prepare for the challenging txt...
			get_random_bytes((void *)pstat->chg_txt, 128);
			pstat->state &= ~WIFI_AUTH_NULL;
			pstat->state |= WIFI_AUTH_STATE1;
			pstat->AuthAlgrthm = algorithm;
			pstat->auth_seq = 2;
		}
		else if (seq == 3)
		{
			//checking for challenging txt...
			p = get_ie(pframe + WLAN_HDR_A3_LEN + 4 + _AUTH_IE_OFFSET_, _CHLGETXT_IE_, (int *)&len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _AUTH_IE_OFFSET_ - 4);
			if ((p != NULL) && !memcmp((void *)(p + 2),pstat->chg_txt, 128))
			{
				pstat->state &= (~WIFI_AUTH_STATE1);
				pstat->state |= WIFI_AUTH_SUCCESS;
				// challenging txt is correct...
				pstat->expire_to = priv->assoc_to;
			}
			else
			{
				DEBUG_ERR("auth rejected because challenge failure!\n");
				status = _STATS_CHALLENGE_FAIL_;
#ifdef CONFIG_RTL8196B_TLD
				LOG_MSG_DEL("[WLAN access rejected: incorrect security] from MAC address: %02x:%02x:%02x:%02x:%02x:%02x,\n",
					sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);
#endif
				goto auth_fail;
			}
		}
		else
		{
			DEBUG_ERR("(3)auth rejected because out of seq [rx_seq=%d, exp_seq=%d]!\n",
				seq, pstat->auth_seq+1);
			status = _STATS_OUT_OF_AUTH_SEQ_;
			goto auth_fail;
		}
	}

	// Now, we are going to issue_auth...

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	if (TRUE == isMeshMP)
		pstat->auth_seq = 2;		// Mesh send seq=2 (response) only
	else
#endif
	pstat->auth_seq = seq + 1;
#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	if ((FALSE == isMeshMP) || ((1 == seq) && (TRUE == isMeshMP)))
#endif
	issue_auth(priv, pstat, (unsigned short)(_STATS_SUCCESSFUL_));

	if (pstat->state & WIFI_AUTH_SUCCESS)	// STA valid
		pstat->auth_seq = 0;

	RESTORE_INT(flags);

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	if ((TRUE == isMeshMP) && ( MP_OPEN_SENT == pstat->mesh_neighbor_TBL.State) && (2 == seq)) // Active 2==seq , auth success prevState is MP_OPEN_SENT
		issue_assocreq_MP(priv, pstat);
#endif

	return SUCCESS;
auth_success:

	issue_auth(priv, pstat, (unsigned short)(_STATS_SUCCESSFUL_));

	if (pstat->state & WIFI_AUTH_SUCCESS)	// STA valid
		pstat->auth_seq = 0;

	RESTORE_INT(flags);

#if defined(CONFIG_RTK_MESH) && defined(MESH_BOOTSEQ_AUTH)
	if ((TRUE == isMeshMP) && ( MP_OPEN_SENT == pstat->mesh_neighbor_TBL.State) && (2 == seq)) // Active 2==seq , auth success prevState is MP_OPEN_SENT
		issue_assocreq_MP(priv, pstat);
#endif

	return SUCCESS;

auth_fail:

	if ((OPMODE & WIFI_AP_STATE) && (pstat == NULL)) {
		pstat = (struct stat_info *)kmalloc(sizeof(struct stat_info), GFP_ATOMIC);
		if (pstat == NULL) {
			RESTORE_INT(flags);
			return FAIL;
		}

		alloc_pstat = 1;
		memset(pstat, 0, sizeof(struct stat_info));

		pstat->auth_seq = 2;
		memcpy(pstat->hwaddr, sa, 6);
		pstat->AuthAlgrthm = algorithm;
	}
	else {
		alloc_pstat = 0;
		pstat->auth_seq = seq + 1;
	}

	issue_auth(priv, pstat, (unsigned short)status);
#ifdef TLN_STATS
	stats_conn_status_counts(priv, status);
#endif

	if (alloc_pstat)
		kfree(pstat);

	SNMP_MIB_ASSIGN(dot11AuthenticateFailStatus, status);
	SNMP_MIB_COPY(dot11AuthenticateFailStation, sa, MACADDRLEN);

	RESTORE_INT(flags);

	return FAIL;
}



/**
 *	@brief	AP recived De-Authentication
 *
 */
unsigned int OnDeAuth(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char *pframe;
	struct  stat_info   *pstat;
	unsigned char *sa;
	unsigned short reason;
	DOT11_DISASSOCIATION_IND Disassociation_Ind;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	pframe = get_pframe(pfrinfo);
	sa = GetAddr2Ptr(pframe);
	pstat = get_stainfo(priv, sa);

	if (pstat == NULL)
		return 0;
#ifdef CONFIG_IEEE80211W
	if(pstat->isPMF) {
		pstat->isPMF = 0;
		issue_deauth(priv, sa, 6);
	}
#endif
#ifdef RTK_WOW
	if (pstat->is_rtk_wow_sta)
		return 0;
#endif

#ifdef P2P_SUPPORT
	P2P_DEBUG("on deauth\n");
	MAC_PRINT(pstat->hwaddr);
			/*cfg p2p cfg p2p*/	
	if((rtk_p2p_is_enabled(priv)) && (rtk_p2p_chk_role(priv,P2P_TMP_GO))){
		if(pstat->is_p2p_client)
			p2p_client_remove(priv,pstat);
	}
#endif

	reason = cpu_to_le16(*(unsigned short *)((unsigned long)pframe + WLAN_HDR_A3_LEN ));
	DEBUG_INFO("receiving deauth from station %02X%02X%02X%02X%02X%02X reason %d\n",
		pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
		pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5], reason);

#ifdef MULTI_MAC_CLONE
	__del_mclone_addr(priv, pstat->hwaddr);//this function only AP in.
#endif

	SAVE_INT_AND_CLI(flags);
	if (asoc_list_del(priv, pstat))
	{
		if (pstat->expire_to > 0)
		{
			cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
			check_sta_characteristic(priv, pstat, DECREASE);

#ifdef CONFIG_RTK_MESH
			if (isPossibleNeighbor(pstat))
			{
#if 0
				LOG_MESH_MSG("OnDeAuth mesh_cnt_ASSOC_PeerLink_CAP(-)\n");
#endif
				mesh_cnt_ASSOC_PeerLink_CAP(priv, pstat, DECREASE);
			}
#endif
		}
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E && pstat->sta_in_firmware == 1) {
			RTL8188E_MACID_NOLINK(priv, 1, REMAP_AID(pstat));
			RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
            pstat->txpause_flag = 0;            
		}
#endif

#ifdef CONFIG_WLAN_HAL
        if(IS_HAL_CHIP(priv))
        {
            if(pstat && (REMAP_AID(pstat) < 128))
            {
                DEBUG_WARN("%s %d OnDeAuth, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
                GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstat));                
                pstat->txpause_flag = 0;
            }
            else
            {
                DEBUG_WARN(" MACID sleep only support 128 STA \n");
            }
        }
#endif

	}
	RESTORE_INT(flags);
	
#ifdef BEAMFORMING_SUPPORT
	if (priv->pmib->dot11RFEntry.txbf == 1 && (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8814A) )
	{
		ODM_RT_TRACE(ODMPTR, BEAMFORMING_DEBUG, ODM_DBG_LOUD, ("%s,\n", __FUNCTION__));
		if(Beamforming_DeInitEntry(priv, pstat->hwaddr))
			Beamforming_Notify(priv);
	}
#endif


	free_stainfo(priv, pstat);

#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
	if (priv->sta_status_cb_func)
		priv->sta_status_cb_func("wlan0", sa, WIFI_STA_STATUS_DEAUTHENTICATED);
#else
	LOG_MSG("A wireless client is deauthenticated - %02X:%02X:%02X:%02X:%02X:%02X\n",
		*sa, *(sa+1), *(sa+2), *(sa+3), *(sa+4), *(sa+5));
#endif

	if (IEEE8021X_FUN)
	{
#ifndef WITHOUT_ENQUEUE
#ifdef CONFIG_RTK_MESH
		if (isSTA(pstat))
#endif
		{
		memcpy((void *)Disassociation_Ind.MACAddr, (void *)sa, MACADDRLEN);
		Disassociation_Ind.EventId = DOT11_EVENT_DISASSOCIATION_IND;
		Disassociation_Ind.IsMoreEvent = 0;
		Disassociation_Ind.Reason = reason;
		Disassociation_Ind.tx_packets = pstat->tx_pkts;
		Disassociation_Ind.rx_packets = pstat->rx_pkts;
		Disassociation_Ind.tx_bytes   = pstat->tx_bytes;
		Disassociation_Ind.rx_bytes   = pstat->rx_bytes;
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Disassociation_Ind,
					sizeof(DOT11_DISASSOCIATION_IND));
		}
#endif
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
		psk_indicate_evt(priv, DOT11_EVENT_DISASSOCIATION_IND, sa, NULL, 0);
#endif
	}

#ifdef RTK_NL80211
	event_indicate_cfg80211(priv, sa, CFG80211_DEL_STA, NULL);
#endif
#ifdef WIFI_HAPD
	event_indicate_hapd(priv, sa, HAPD_EXIRED, NULL);
#ifdef HAPD_DRV_PSK_WPS
	event_indicate(priv, sa, 2);
#endif
#else
	event_indicate(priv, sa, 2);
#endif

	return SUCCESS;
}


unsigned int OnWmmAction(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
#if defined(P2P_SUPPORT) || defined(HS2_SUPPORT)
	int needRdyAssoc=1;
#endif		
#ifdef CONFIG_RTK_MESH
	// please add the codes to check where the action frame is rreq, rrep or rrer
	// (check  the action field )
	if (pfrinfo->is_11s) {
		unsigned char  *pframe, *pFrameBody;
		unsigned char action_type = -1, category_type;
		int Is_6Addr = 0;

		pframe = get_pframe(pfrinfo);  // get frame data

		if (pframe!=0) {
			if(is_mesh_6addr_format_without_qos(pframe)) {
				pFrameBody = pframe + WLAN_HDR_A6_MESH_DATA_LEN;
				Is_6Addr = 1;
			} else
				pFrameBody = pframe + WLAN_HDR_A4_MESH_DATA_LEN;

			//pFrameBody = GetMeshMgtPtr(pframe);
			// reason = cpu_to_le16(*(unsigned short *)((unsigned long)pframe + WLAN_HDR_A4_MESH_MGT_LEN )); //len define ref to wifi.h
			category_type = *pFrameBody;
			action_type = *(pFrameBody+1);



#ifdef MESH_USE_METRICOP
			if(category_type == _CATEGORY_11K_ACTION_)
			{
				switch(action_type) {
					case ACTION_FILED_11K_LINKME_REQ:
						return On11kvLinkMeasureReq(priv, pfrinfo);
					case ACTION_FILED_11K_LINKME_REP:
						return On11kvLinkMeasureRep(priv, pfrinfo);
				}
			}
#endif

			if(category_type == _MESH_CATEGORY_ID_) {
                switch(action_type) {
                    case _MESH_LINK_REPORT_ACTION_ID_:
                        OnLocalLinkStateANNOU_MP(priv, pfrinfo);
                        break;
                    case _HWMP_MESH_PATHSEL_ACTION_ID_:
                    case _GANN_ACTION_ID_:
                        OnPathSelectionManagFrame(priv, pfrinfo, Is_6Addr);
                        break;

                    default:
                        printk("unknow action_type in OnWmmAction\n");
                        goto ACTIVE_NOT_11S;
                }

            }
            else if(category_type == _MULTIHOP_CATEGORY_ID_) {
                switch(action_type) {
#ifdef PU_STANDARD
                    case _PROXY_UPDATES_ACTION_ID_:
                        OnProxyUpdate_MP(priv,pfrinfo);
                        break;
                    case _PROXY_UPDATES_CONFIRM_ACTION_ID_:
                        OnProxyUpdateConfirm_MP(priv,pfrinfo);
                        break;
#endif
                    default:
                        printk("unknow action_type in OnWmmAction\n");
                        goto ACTIVE_NOT_11S;
                }

            }
            else {
				goto	ACTIVE_NOT_11S;
            }
		}
		return SUCCESS;
	}

ACTIVE_NOT_11S:
#endif	// CONFIG_RTK_MESH

#ifdef WIFI_WMM
	if (QOS_ENABLE
#ifdef P2P_SUPPORT
		|| rtk_p2p_is_enabled(priv)			/*cfg p2p cfg p2p*/
#endif
	) {
		unsigned char *sa = pfrinfo->sa;
		unsigned char *da = pfrinfo->da;
		struct stat_info *pstat = get_stainfo(priv, sa);
		unsigned char *pframe=NULL;
		unsigned char Category_field=0, Action_field=0, previous_mimo_ps=0;
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		unsigned char TID=0xff;
#else
		unsigned char TID=0;
#endif
		unsigned short blockAck_para=0, status_code=_STATS_SUCCESSFUL_, timeout=0, reason_code, max_size;
#ifdef TX_SHORTCUT
		unsigned int do_tx_slowpath = 0;
#endif
#ifdef HS2_SUPPORT
		DOT11_HS2_GAS_REQ   gas_req, gas_req2;
#endif

		// Reply in B/G mode to fix IOT issue with D-Link DWA-642
#if 0
		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) ||
				(pstat && pstat->ht_cap_len == 0)) {
			DEBUG_ERR("Drop Action frame!\n");
			return SUCCESS;
		}
#endif

		pframe = get_pframe(pfrinfo) + WLAN_HDR_A3_LEN;	//start of action frame content
		Category_field = pframe[0];
		Action_field = pframe[1];
#ifdef HS2_SUPPORT
        if((Category_field==_PUBLIC_CATEGORY_ID_) && ((Action_field==_GAS_INIT_REQ_ACTION_ID_)||(Action_field==_GAS_COMBACK_REQ_ACTION_ID_)||(Action_field==_GAS_INIT_RSP_ACTION_ID_)||(Action_field==_GAS_COMBACK_RSP_ACTION_ID_))) 
        {
			needRdyAssoc=0;
        }
        if ((Category_field==_DLS_CATEGORY_ID_) && (Action_field==_DLS_REQ_ACTION_ID_))
			needRdyAssoc=0;
#endif

		
#ifdef P2P_SUPPORT
			/*cfg p2p cfg p2p*/
		if(rtk_p2p_is_enabled(priv)){
			if(	(Category_field ==_PUBLIC_CATEGORY_ID_ && Action_field == _P2P_PUBLIC_ACTION_FIELD_) 
				//|| (Category_field==_VENDOR_ACTION_ID_)
				)
			{
				needRdyAssoc=0;
			}
		}
#endif

		if ((!IS_MCAST(da)) && (pstat 
#if defined(P2P_SUPPORT) || defined(HS2_SUPPORT)
			|| needRdyAssoc==0 
#endif				
			)) 
		{
			
			switch (Category_field) {
#ifdef DOT11H
                case _SPECTRUM_MANAGEMENT_CATEGORY_ID_:
                    switch (Action_field) {
                        case _TPC_REQEST_ACTION_ID_:
                            if(priv->pmib->dot11hTPCEntry.tpc_enable)
                                issue_TPC_report(priv, sa, pframe[2]);
                            break;
                        case _TPC_REPORT_ACTION_ID_:
                            break;
                        default:
                            break;
                    }
                    break;
#endif                    
				case _BLOCK_ACK_CATEGORY_ID_:
					switch (Action_field) {
						case _ADDBA_Req_ACTION_ID_:
							blockAck_para = pframe[3] | (pframe[4] << 8);
							timeout = 0; //pframe[5] | (pframe[6] << 8);
							pstat->AMSDU_AMPDU_support = blockAck_para & 0x1;
							TID = (blockAck_para>>2)&0x000f;
							max_size = (blockAck_para&0xffc0)>>6;
							DEBUG_INFO("ADDBA-req recv fr AID %d, token %d TID %d size %d timeout %d\n",
								pstat->aid, pframe[2], TID, max_size, timeout);
							if (!(blockAck_para & BIT(1)) || (pstat->ht_cap_len == 0)
							|| (pstat->sta_in_firmware != 1 && priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _NO_PRIVACY_)
							|| priv->pmib->dot11nConfigEntry.dot11nAddBAreject
							){	// 0=delayed BA, 1=immediate BA							
								status_code = _STATS_REQ_DECLINED_;
							}else{
								pstat->ADDBA_req_num[TID] = 0;
							}
							
#ifdef DZ_ADDBA_RSP
							if (pstat && ((pstat->state & (WIFI_SLEEP_STATE | WIFI_ASOC_STATE)) ==
											(WIFI_SLEEP_STATE | WIFI_ASOC_STATE))) {
								pstat->dz_addba.used = 1;
								pstat->dz_addba.dialog_token = pframe[2];
								pstat->dz_addba.TID = TID;
								pstat->dz_addba.status_code = status_code;
								pstat->dz_addba.timeout = timeout;
							}
							else 
#endif
							if (!issue_ADDBArsp(priv, sa, pframe[2], TID, status_code, timeout))
								DEBUG_ERR("issue ADDBA-rsp failed\n");
							break;
						case _DELBA_ACTION_ID_:
							TID = (pframe[3] & 0xf0) >> 4;
							pstat->ADDBA_ready[TID] = 0;
							pstat->ADDBA_req_num[TID] = 0;
							pstat->ADDBA_sent[TID] = 0;
							reason_code = pframe[4] | (pframe[5] << 8);
							DEBUG_INFO("DELBA recv from AID %d, TID %d reason %d\n", pstat->aid, TID, reason_code);
#ifdef TX_SHORTCUT
							do_tx_slowpath++;
#endif
							break;
						case _ADDBA_Rsp_ACTION_ID_:
							blockAck_para = pframe[5] | (pframe[6] << 8);
							status_code = pframe[3] | (pframe[4] << 8);
							pstat->AMSDU_AMPDU_support = blockAck_para & 0x1;
							TID = (blockAck_para>>2)&0x000f;
							max_size = (blockAck_para&0xffc0)>>6;
							if (status_code != _STATS_SUCCESSFUL_) {
								pstat->ADDBA_ready[TID] = 0;
							} else {
								DEBUG_INFO("%s %d increase ADDBA_ready, clear ADDBA_sent\n",__func__,__LINE__);
								pstat->ADDBA_ready[TID]++;
								pstat->ADDBA_sent[TID] = 0;
							}
							pstat->ADDBA_req_num[TID] = 0;
#ifdef TX_SHORTCUT
							do_tx_slowpath++;
#endif
							DEBUG_INFO("ADDBA-rsp recv fr AID %d, token %d TID %d size %d status %d\n",
								pstat->aid, pframe[2], TID, max_size, status_code);
							break;
						default:
							DEBUG_ERR("Error BA Action frame is received\n");
							goto error_frame;
							break;
					}
					break;

#if	defined(WIFI_11N_2040_COEXIST) || defined(P2P_SUPPORT) || defined(HS2_SUPPORT)
				case _PUBLIC_CATEGORY_ID_:
					switch (Action_field) {
						case _2040_COEXIST_ACTION_ID_:
							if (COEXIST_ENABLE) {
								if (!(OPMODE & WIFI_AP_STATE)) {
									DEBUG_WARN("Ignored Public Action frame received since this is not an AP\n");
									break;
								}
								if (!priv->pshare->is_40m_bw) {
									DEBUG_WARN("Ignored Public Action frame received since AP is 20m mode\n");
									break;
								}
								if (!(priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N|WIRELESS_11G))) {
									DEBUG_WARN("Ignored Public Action frame received since AP is not 2.4G band\n");
									break;
								}
								if (pframe[2] == _2040_BSS_COEXIST_IE_) {
									if (pframe[4] & (_40M_INTOLERANT_ |_20M_BSS_WIDTH_REQ_)) {
										if (pframe[4] & _40M_INTOLERANT_) {
											DEBUG_INFO("Public Action frame: force 20m by 40m intolerant\n");
										} else {
											DEBUG_INFO("Public Action frame: force 20m by 20m bss width req\n");
										}	

										setSTABitMap(&priv->switch_20_sta, pstat->aid);


#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
										update_RAMask_to_FW(priv, 1);									
#endif
										SetTxPowerLevel(priv);
									} else {
										if ((pframe[2+pframe[3]+2]) && (pframe[2+pframe[3]+2] == _2040_Intolerant_ChRpt_IE_)) {	
											int ch_idx, ch_len= pframe[2+pframe[3]+2+1]-1, ch; 							
											DEBUG_INFO("Public Action frame: force 20m by channel report\n");
											for(ch_idx=0; ch_idx < ch_len; ch_idx++) {	
												ch = pframe[2+pframe[3]+2+3 + ch_idx];
												if( ch && (ch<=14) && (priv->pmib->dot11nConfigEntry.dot11nCoexist_ch_chk?
												(ch != priv->pmib->dot11RFEntry.dot11channel):1) )
												{									
													setSTABitMap(&priv->switch_20_sta, pstat->aid);

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
													update_RAMask_to_FW(priv, 1);									
#endif
													SetTxPowerLevel(priv);
													break;
												}
											}
										} else {
											DEBUG_INFO("Public Action frame: cancel force 20m\n");
#if 0											

										    clearSTABitMap(&priv->switch_20_sta, pstat->aid);

#if defined(WIFI_11N_2040_COEXIST_EXT)
											clearSTABitMap(&priv->pshare->_40m_staMap, pstat->aid);
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
											update_RAMask_to_FW(priv, 0);									
#endif
#endif
										}
									}

#ifdef TX_SHORTCUT
									do_tx_slowpath++;
#endif
								} else {
									DEBUG_ERR("Error Public Action frame received\n");
								}
							} else {
								DEBUG_WARN("Public Action frame received but func off\n");
							}
							break;
#ifdef HS2_SUPPORT
                        case _GAS_INIT_REQ_ACTION_ID_:                          
                            HS2_DEBUG_INFO("Receive GAS Init REQ\n");
                            if (priv->pmib->hs2Entry.interworking_ielen)
                            {   
                                int tmplen = pframe[4];
                                gas_req.Reqlen = (pframe[5+tmplen+1] << 8) | pframe[4+tmplen+1];                                
                                memcpy((void *)gas_req.MACAddr, (void *)sa, MACADDRLEN);
                                gas_req.EventId = DOT11_EVENT_GAS_INIT_REQ;
                                gas_req.IsMoreEvent = 0;
                                gas_req.Dialog_token = pframe[2];
                                gas_req.Advt_proto = pframe[4+tmplen];
                                
                                memcpy(gas_req.Req, &pframe[5+tmplen+2], gas_req.Reqlen); //copy ANQP content
            
                                DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&gas_req,
                                        sizeof(DOT11_HS2_GAS_REQ));
                                
                                event_indicate(priv, sa, 1);        
                            }
                            
                            break;
                        case _GAS_COMBACK_REQ_ACTION_ID_:
                            if (priv->pmib->hs2Entry.interworking_ielen)
                            {
                                memcpy((void *)gas_req.MACAddr, (void *)sa, MACADDRLEN);
                                HS2_DEBUG_INFO("GAS comback REQ Action frame\n");
                                gas_req.EventId = DOT11_EVENT_GAS_COMEBACK_REQ;
                                gas_req.Dialog_token = pframe[2];
                                
                                DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&gas_req,
                                        sizeof(DOT11_HS2_GAS_REQ));
                                
                                event_indicate(priv, sa, 1);    
                            }
                            break;  
#ifdef HS2_CLIENT_TEST
                        case _GAS_INIT_RSP_ACTION_ID_:
                            {
                                HS2_DEBUG_INFO("GAS initial RSP Action frame\n");
                                if ((pframe[5] != 0) || (testflg == 1))
                                {
                                    memcpy((void *)gas_req.MACAddr, (void *)sa, MACADDRLEN);
                                    gas_req.Dialog_token = pframe[2];
                                    issue_GASreq(priv, &gas_req, 0);                                    
                                    testflg = 0;
                                }
                            }
                            break;
                        case _GAS_COMBACK_RSP_ACTION_ID_:
                            {
                                HS2_DEBUG_INFO("GAS combo RSP Action frame\n");
                                if ((pframe[5] & 0x80 )!= 0)
                                {
                                    memcpy((void *)gas_req.MACAddr, (void *)sa, MACADDRLEN);
                                    gas_req.Dialog_token = pframe[2];
                                    issue_GASreq(priv, &gas_req, 0);        
                                }
                            }
                            break;  
#endif
#endif

#ifdef P2P_SUPPORT
						case _P2P_PUBLIC_ACTION_FIELD_:
							if(!memcmp(&pframe[2] , WFA_OUI_PLUS_TYPE,4)){							
								P2P_on_public_action(priv,pfrinfo);
							}
							break;
#endif							
						default:
							DEBUG_INFO("Public Action frame received but not support yet\n");
							goto error_frame;
							break;
					}
					break;
#endif
#ifdef CONFIG_IEEE80211W
				case _SA_QUERY_CATEGORY_ID_:
					PMFDEBUG("%s, _SA_QUERY_CATEGORY_ID_\n", __FUNCTION__);
					if (Action_field == _SA_QUERY_REQ_ACTION_ID_) {		
						PMFDEBUG("recv SA req\n");
						issue_SA_Query_Rsp(priv->dev, sa, pframe+2);
						//if (IEEE8021X_FUN && priv->pmib->dot1180211AuthEntry.dot11EnablePSK) {		
						//	PMFDEBUG("recv SA req..2\n");
						//	psk_indicate_evt(priv, DOT11_EVENT_SA_QUERY, GetAddr2Ptr(pframe), pframe, 0);
						//}
					} else if (Action_field == _SA_QUERY_RSP_ACTION_ID_) {
						int idx;
						unsigned short trans_id = (unsigned short) (pframe[3] << 8) + pframe[2];
						for (idx = 1; idx <= pstat->sa_query_count; idx++) {
							if (pstat->SA_TID[idx] == trans_id)
								break;
						}
						if (idx > pstat->sa_query_count) { // No match
							PMFDEBUG("recv SA Rsp...not match\n");
							return SUCCESS;
						}
						stop_sa_query(pstat);
					}
					break;
#endif // CONFIG_IEEE80211W
#ifdef RTK_AC_SUPPORT
				case _VHT_ACTION_CATEGORY_ID_:
					if (Action_field == _VHT_ACTION_OPMNOTIF_ID_) {
//						int nss = ((pframe[4]>>4)&0x7);
						if((pframe[2] &3) <= priv->pmib->dot11nConfigEntry.dot11nUse40M)
							pstat->tx_bw = pframe[2] &3;
						pstat->nss = ((pframe[2]>>4)&0x7)+1;
//						pstat->vht_cap_buf.vht_support_mcs[0] |=   cpu_to_le32(0xffff);
//						pstat->vht_cap_buf.vht_support_mcs[0] &= ~ cpu_to_le32((1<<((nss+1)<<1))-1);
//						panic_printk("Action 21, operating mode:%d, %d", pstat->tx_bw, pstat->nss);
#ifdef CONFIG_RTL_8812_SUPPORT
						if(GET_CHIP_VER(priv)== VERSION_8812E)
							UpdateHalRAMask8812(priv, pstat, 3);
#endif
#ifdef  CONFIG_WLAN_HAL
						if (IS_HAL_CHIP(priv)){
							GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
						}
#endif


					}
					break;
#endif
				case _HT_CATEGORY_ID_:
					if (Action_field == _HT_MIMO_PS_ACTION_ID_) {
						previous_mimo_ps = pstat->MIMO_ps;
						pstat->MIMO_ps = 0;
						if (pframe[2] & BIT(0)) {
							if (pframe[2] & BIT(1))
								pstat->MIMO_ps|=_HT_MIMO_PS_DYNAMIC_;
							else
								pstat->MIMO_ps|=_HT_MIMO_PS_STATIC_;
						}
						if ((previous_mimo_ps|pstat->MIMO_ps)&_HT_MIMO_PS_STATIC_) {
							assign_tx_rate(priv, pstat, pfrinfo);
#if defined(CONFIG_PCI_HCI)
#ifdef CONFIG_WLAN_HAL
							if (IS_HAL_CHIP(priv)) {
								GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
							} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
							if(GET_CHIP_VER(priv)== VERSION_8812E) {
								UpdateHalRAMask8812(priv, pstat, 3);
							} else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
							if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
								add_RATid(priv, pstat);
#endif
							} else
#endif
							{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)	
								add_update_RATid(priv, pstat);
#endif
							}
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
							update_STA_RATid(priv, pstat);
#endif
						}
#ifdef TX_SHORTCUT
						if ((previous_mimo_ps|pstat->MIMO_ps)&_HT_MIMO_PS_DYNAMIC_) 
							do_tx_slowpath++;
#endif
						check_NAV_prot_len(priv, pstat, 0);
					} else {
						DEBUG_INFO("HT Action Frame is received but not support yet\n");
					}
					break;
#ifdef HS2_SUPPORT
            case _WNM_CATEGORY_ID_:
                switch (Action_field) {
                    case _WNM_TSMQUERY_ACTION_ID_:
                        {
                            DOT11_HS2_TSM_REQ tsm_req;
                            int can_list = 0;
                            int payload = pfrinfo->pktlen - WLAN_HDR_A3_LEN;
                            tsm_req.Dialog_token = pframe[2];
                            
                            HS2_DEBUG_INFO("got TSM QUERY frame\n");
                            if (payload > 4)
                            {
                                if (payload-4 > 100)
                                {
                                    tsm_req.list_len = 100;
                                    tsm_req.url_len = 0;
                                    HS2_DEBUG_ERR("payload too long!!\n");
                                    memcpy(tsm_req.Candidate_list, &pframe[4], 100);
                                    can_list = 1;
                                }
                                else
                                {
                                    tsm_req.list_len = payload-4;   
                                    tsm_req.url_len = 0;                
                                    memcpy(tsm_req.Candidate_list, &pframe[4], payload-4);
                                    can_list = 1;
                                }
                            }
                            else
                            {
                                tsm_req.url_len = 0;
                                tsm_req.list_len = 0;
                                can_list = 0;
                            }
                            memcpy(tsm_req.MACAddr, get_pframe(pfrinfo) + WLAN_ADDR_LEN + 4, MACADDRLEN);
                            tsm_req.Req_mode = priv->pmib->hs2Entry.reqmode | (can_list & 0x01);
                            tsm_req.Disassoc_timer = priv->pmib->hs2Entry.disassoc_timer;
                            if (can_list != 0)
                                tsm_req.Validity_intval = 200;
                            else
                                tsm_req.Validity_intval = priv->pmib->hs2Entry.validity_intval; 
                            issue_BSS_TSM_req(priv, &tsm_req);                              
                        }
                        break;
                    case _BSS_TSMRSP_ACTION_ID_:
                        HS2_DEBUG_INFO("got TSM RSP frame\n");
                        HS2_DEBUG_INFO("token=%d\n", pframe[2]);
                        HS2_DEBUG_INFO("status code=%d\n", pframe[3]);
                        break;
#ifdef HS2_CLIENT_TEST
                    case _BSS_TSMREQ_ACTION_ID_:
                        printk("got TSM REQ frame\n");
                        if (pfrinfo->pktlen - WLAN_HDR_A3_LEN > 7)
                            issue_BSS_TSM_rsp(priv, &pframe[2], &pframe[7], pfrinfo->pktlen - WLAN_HDR_A3_LEN-7);
                        else
                            issue_BSS_TSM_rsp(priv, &pframe[2], NULL, 0);
                        break;  
#endif
                }
                break;                                              
            case _DLS_CATEGORY_ID_:
                printk("dls..action=%d\n", Action_field);
                switch (Action_field) {
                    case _DLS_REQ_ACTION_ID_:
                        printk("recv DLS frame\n");

                        issue_DLS_rsp(priv, 48, sa, &pframe[2], &pframe[8]); // status = 48 means that Direct link is not allowed in the BSS by policy)
                    break;
                };
                break;
#endif // HS2_SUPPORT
#ifdef TDLS_SUPPORT
			case _TDLS_CATEGORY_ID_:
				printk("TDLS..action=%d, tdls_prohibited = %d\n", Action_field, priv->pmib->dot11OperationEntry.tdls_prohibited);		 
             break;
#endif
#ifdef P2P_SUPPORT
				case _VENDOR_ACTION_ID_:
					if(!memcmp(&pframe[1],WFA_OUI_PLUS_TYPE,WFA_OUI_PLUS_TYPE_LEN)){
						P2P_on_action(priv,pfrinfo);
					}
					break;	
#endif
				default:
					DEBUG_INFO("Action Frame is received but not support yet\n");
					break;
			}
#ifdef TX_SHORTCUT
			if (do_tx_slowpath) {
				/* let the first tx packet go through normal path and set fw properly */
				if (!priv->pmib->dot11OperationEntry.disable_txsc) {
					int i;
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
					int j;
					if (TID == 0xff) {
						for (i = 0; i < NR_NET80211_UP; ++i)
							for (j = 0; j < TX_SC_ENTRY_NUM; ++j)
								pstat->tx_sc_ent[i][j].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
					} else {
						for (i = 0; i < TX_SC_ENTRY_NUM; ++i)
							pstat->tx_sc_ent[TID][i].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
					}
#else
					for (i=0; i<TX_SC_ENTRY_NUM; i++) {
#ifdef CONFIG_WLAN_HAL
                        if (IS_HAL_CHIP(priv)) {
                            GET_HAL_INTERFACE(priv)->SetShortCutTxBuffSizeHandler(priv, pstat->tx_sc_ent[i].hal_hw_desc, 0);
                        } else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif // CONFIG_WLAN_HAL
                        {//not HAL
						pstat->tx_sc_ent[i].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
#ifdef TX_SCATTER
						pstat->tx_sc_ent[i].has_desc3 = 0;
#endif
						 }
					}
#endif
				}
			}
#endif
		} else {
			if (IS_MCAST(da)) {
				//DEBUG_ERR("Error Broadcast or Multicast Action Frame is received\n");
			}
			else {
				//DEBUG_ERR("Action Frame is received from non-associated station\n");
			}
		}
	}
error_frame:
#endif
	return SUCCESS;
}


unsigned int DoReserved(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	return SUCCESS;
}


#ifdef CLIENT_MODE
void update_bss(struct Dot11StationConfigEntry *dst, struct bss_desc *src)
{
	memcpy((void *)dst->dot11Bssid, (void *)src->bssid, MACADDRLEN);
	memset((void *)dst->dot11DesiredSSID, 0, sizeof(dst->dot11DesiredSSID));
	memcpy((void *)dst->dot11DesiredSSID, (void *)src->ssid, src->ssidlen);
	dst->dot11DesiredSSIDLen = src->ssidlen;
}



/**
 *	@brief	Authenticat success, Join a BSS
 *
 *	Set BSSID to hardware, Join BSS complete
 */
void join_bss(struct rtl8192cd_priv *priv)
{
	unsigned short	val16;
	unsigned long	val32;

	memcpy((void *)&val32, BSSID, 4);
	memcpy((void *)&val16, BSSID+4, 2);
	RTL_W32(BSSIDR, cpu_to_le32(val32));
	RTL_W16((BSSIDR + 4), cpu_to_le16(val16));
}


/**
 *	@brief	issue Association Request
 *
 *	STA find compatible network, and authenticate success, use this function send association request. \n
 *	+---------------+----+----+-------+------------+-----------------+------+-----------------+	\n
 *	| Frame Control | DA | SA | BSSID | Capability | Listen Interval | SSID | Supported Rates |	\n
 *	+---------------+----+----+-------+------------+-----------------+------+-----------------+	\n
 *	\n
 *	+--------------------+---------------------+-----------------+	\n
 *  | Ext. Support Rates | Realtek proprietary | RSN information |	\n
 *	+--------------------+---------------------+-----------------+	\n
 *
 *	PS: Reassociation Frame Body have Current AP Address field, But not implement.
 */
unsigned int issue_assocreq(struct rtl8192cd_priv *priv)
{
	unsigned short	val;
	struct wifi_mib *pmib;
	unsigned char	*bssid, *pbuf;
	unsigned char	*pbssrate=NULL;
	int		bssrate_len;
	unsigned char	supportRateSet[32];
	int		i, j, idx=0, supportRateSetLen=0, match=0;
	unsigned int	retval=0;
#ifdef WIFI_WMM
	int		k;
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
	unsigned long		flags;
#endif
#if 0 //def SUPPORT_MULTI_PROFILE		
    int found=0;
    int jdx=0;
    int ssid_len=0;
#endif

	DECLARE_TXINSN(txinsn);

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

	pmib= GET_MIB(priv);

	bssid = pmib->dot11Bss.bssid;

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
#ifdef P2P_SUPPORT				/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);

	if (pbuf == NULL)
		goto issue_assocreq_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);

	if (txinsn.phdr == NULL)
		goto issue_assocreq_fail;
	memset((void *)txinsn.phdr, 0, sizeof(struct  wlan_hdr));

	val = cpu_to_le16(pmib->dot11Bss.capability);

	if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm)
		val |= cpu_to_le16(BIT(4));

	if (SHORTPREAMBLE)
		val |= cpu_to_le16(BIT(5));
    
#ifdef DOT11H
    if(priv->pmib->dot11hTPCEntry.tpc_enable)
        val |= cpu_to_le16(BIT(8));	/* set spectrum mgt */
#endif    

	pbuf = set_fixed_ie(pbuf, _CAPABILITY_, (unsigned char *)&val, &txinsn.fr_len);

	val	 = cpu_to_le16(3);
	pbuf = set_fixed_ie(pbuf, _LISTEN_INTERVAL_, (unsigned char *)&val, &txinsn.fr_len);

	pbuf = set_ie(pbuf, _SSID_IE_, pmib->dot11Bss.ssidlen, pmib->dot11Bss.ssid, &txinsn.fr_len);

	if (pmib->dot11Bss.supportrate == 0)
	{
		// AP don't contain rate info in beacon/probe response
		// Use our rate in asoc req
		get_bssrate_set(priv, _SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len);
		pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_, bssrate_len, pbssrate, &txinsn.fr_len);

		//EXT supported rates.
		if (get_bssrate_set(priv, _EXT_SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len))
			pbuf = set_ie(pbuf, _EXT_SUPPORTEDRATES_IE_, bssrate_len, pbssrate, &txinsn.fr_len);
	}
	else
	{
		// See if there is any mutual supported rate
		for (i=0; dot11_rate_table[i]; i++) {
			int bit_mask = 1 << i;
			if (pmib->dot11Bss.supportrate & bit_mask) {
				val = dot11_rate_table[i];
				for (j=0; j<AP_BSSRATE_LEN; j++) {
					if (val == (AP_BSSRATE[j] & 0x7f)) {
						match = 1;
						break;
					}
				}
				if (match)
					break;
			}
		}

		// If no supported rates match, assoc fail!
		if (!match) {
			DEBUG_ERR("Supported rate mismatch!\n");
			retval = 1;
			goto issue_assocreq_fail;
		}

		// Use AP's rate info in asoc req
		for (i=0; dot11_rate_table[i]; i++) {
			int bit_mask = 1 << i;
			if (pmib->dot11Bss.supportrate & bit_mask) {
				val = dot11_rate_table[i];
				if (((pmib->dot11BssType.net_work_type == WIRELESS_11B) && is_CCK_rate(val)) ||
					(pmib->dot11BssType.net_work_type != WIRELESS_11B)) {
					if (pmib->dot11Bss.basicrate & bit_mask)
						val |= 0x80;

					supportRateSet[idx] = val;
					supportRateSetLen++;
					idx++;
				}
			}
		}

		if (supportRateSetLen == 0) {
			retval = 1;
			goto issue_assocreq_fail;
		}
		else if (supportRateSetLen <= 8)
			pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_ , supportRateSetLen , supportRateSet, &txinsn.fr_len);
		else {
			pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_, 8, supportRateSet, &txinsn.fr_len);
			pbuf = set_ie(pbuf, _EXT_SUPPORTEDRATES_IE_, supportRateSetLen-8, &supportRateSet[8], &txinsn.fr_len);
		}
	}


#ifdef DOT11H
    if(pmib->dot11hTPCEntry.tpc_enable) {
        pbuf = construct_power_capability_ie(priv, pbuf, &txinsn.fr_len);
        pbuf = construct_supported_channel_ie(priv, pbuf, &txinsn.fr_len);
    }
#endif
       /*RSN IE*/
#ifdef WIFI_SIMPLE_CONFIG
		if (!(pmib->wscEntry.wsc_enable && pmib->wscEntry.assoc_ielen))
#endif
		{
#if 0
//#ifdef SUPPORT_MULTI_PROFILE		
            /*echo profile maintain self RNSIE,when issue_assoc chk and try to use itself RSNIE , 
                            the fail case is when RTK AP use mixed mode we use wrong macst cipher type*/
            if ( priv->pmib->ap_profile.enable_profile && priv->pmib->ap_profile.profile_num > 0) {            

                int RSNIE_LEN=0;
                unsigned char RSNIE[128];               
                for(jdx=0 ; jdx<priv->pmib->ap_profile.profile_num ; jdx++) {                                   
    				ssid_len = strlen(priv->pmib->ap_profile.profile[jdx].ssid);                    
                    ///STADEBUG("be chk ssid=%s,idx=%d\n", priv->pmib->ap_profile.profile[jdx].ssid,jdx);                    
                    if((ssid_len == pmib->dot11Bss.ssidlen) && !memcmp(priv->pmib->ap_profile.profile[jdx].ssid , pmib->dot11Bss.ssid,ssid_len )) 
                    {
                        ///STADEBUG("Found;my[%s],target[%s]\n", priv->pmib->ap_profile.profile[jdx].ssid,pmib->dot11Bss.ssid); 
                        priv->profile_idx = jdx;
                        ///STADEBUG("call switch_profile(%d)\n",priv->profile_idx);                        
						switch_profile(priv, priv->profile_idx);
                        
                        if(priv->pmib->ap_profile.profile[jdx].MulticastCipher){

                            priv->wpa_global_info->MulticastCipher=priv->pmib->ap_profile.profile[jdx].MulticastCipher;
                            ///STADEBUG("MulticastCipher=%d\n", priv->wpa_global_info->MulticastCipher);                                                         
                            ///STADEBUG("dot11EnablePSK=[%d]\n", priv->pmib->dot1180211AuthEntry.dot11EnablePSK);                            
                            ConstructIE(priv, RSNIE,&RSNIE_LEN);                            
                            ///STADEBUG("AuthInfoElement len=%d\n", RSNIE_LEN);                            
                            if(RSNIE_LEN){
                                memcpy(pbuf, RSNIE,RSNIE_LEN);
                                pbuf += RSNIE_LEN;
                                txinsn.fr_len += RSNIE_LEN;
                                STADEBUG("Use profile's RSNIE[%s]\n",  priv->pmib->ap_profile.profile[jdx].ssid);                            
                            }

                        }
                        else{
                            ///STADEBUG("NO RSNIE\n");
                        }
                        break;
                    }
                    
                }           
                
                
            }
            else
#endif             
            {
                //STADEBUG("normal case [ssid=%s]\n", pmib->dot11Bss.ssid);                                        
    			if (pmib->dot11RsnIE.rsnielen) {
    				memcpy(pbuf, pmib->dot11RsnIE.rsnie, pmib->dot11RsnIE.rsnielen);
    				pbuf += pmib->dot11RsnIE.rsnielen;
    				txinsn.fr_len += pmib->dot11RsnIE.rsnielen;
    			}
    		}
	}

	if ((QOS_ENABLE) || (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)) {
		int count=0;
		struct bss_desc	*bss=NULL;

		if (priv->site_survey->count) {
			count = priv->site_survey->count;
			bss = priv->site_survey->bss;
		}
		else if (priv->site_survey->count_backup) {
			count = priv->site_survey->count_backup;
			bss = priv->site_survey->bss_backup;
		}

		for(k=0; k<count; k++) {
			if (!memcmp((void *)bssid, bss[k].bssid, MACADDRLEN)) {

#ifdef WIFI_WMM
				//  AP supports WMM when t_stamp[1] bit 0 is set
				if ((QOS_ENABLE) && (bss[k].t_stamp[1] & BIT(0))) {
#ifdef WMM_APSD
					if (APSD_ENABLE) {
						if (bss[k].t_stamp[1] & BIT(3))
							priv->uapsd_assoc++;
						else 
							priv->uapsd_assoc = 0;

						init_WMM_Para_Element(priv, priv->pmib->dot11QosEntry.WMM_IE);
					}
#endif
					pbuf = set_ie(pbuf, _RSN_IE_1_, _WMM_IE_Length_, GET_WMM_IE, &txinsn.fr_len);
				}
#endif
				if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
						(bss[k].network & WIRELESS_11N)) {

					int is_40m_bw, offset_chan;
#ifdef UNIVERSAL_REPEATER
					if (!IS_ROOT_INTERFACE(priv) && !GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11nUse40M)
						is_40m_bw=0;
					else
#endif						
						is_40m_bw = (bss[k].t_stamp[1] & BIT(1)) ? 1 : 0;
					
					if (is_40m_bw) {
						if (bss[k].t_stamp[1] & BIT(2))
							offset_chan = 1;
						else
							offset_chan = 2;
					}
					else
						offset_chan = 0;

					priv->ht_cap_len = 0;	// re-construct HT IE
					construct_ht_ie(priv, is_40m_bw, offset_chan);
					pbuf = set_ie(pbuf, _HT_CAP_, priv->ht_cap_len, (unsigned char *)&priv->ht_cap_buf, &txinsn.fr_len);
#ifdef RTK_AC_SUPPORT
					if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
						construct_vht_ie(priv, priv->pshare->working_channel);
						pbuf = set_ie(pbuf, EID_VHTCapability, priv->vht_cap_len, (unsigned char *)&priv->vht_cap_buf, &txinsn.fr_len);
						pbuf = set_ie(pbuf, EID_VHTOperation, priv->vht_oper_len, (unsigned char *)&priv->vht_oper_buf, &txinsn.fr_len);
					}
#endif
				}

				break;
			}
		}
	}

#ifdef WIFI_SIMPLE_CONFIG

	if (pmib->wscEntry.wsc_enable && pmib->wscEntry.assoc_ielen) {
		memcpy(pbuf, pmib->wscEntry.assoc_ie, pmib->wscEntry.assoc_ielen);
		pbuf += pmib->wscEntry.assoc_ielen;
		txinsn.fr_len += pmib->wscEntry.assoc_ielen;
	}
	if (priv->pmib->wscEntry.both_band_multicredential) {
		pbuf = set_ie(pbuf, 221, 7, "\x00\x0D\x02\x06\x01\02\01", &txinsn.fr_len);
	}
#endif

#ifdef A4_STA
    if(priv->pshare->rf_ft_var.a4_enable == 2) {
        pbuf = construct_ecm_tvm_ie(priv, pbuf, &txinsn.fr_len, BIT0);
    }
#endif

#ifdef TV_MODE
    if(priv->tv_mode_status > 0) {
        pbuf = construct_tv_mode_ie(priv, pbuf, &txinsn.fr_len);
    }
#endif    

#ifdef P2P_SUPPORT
			/*cfg p2p cfg p2p*/
	if((rtk_p2p_is_enabled(priv))){

		if(priv->p2pPtr->p2p_assocReq_ie_len) {
			
			memcpy(pbuf, priv->p2pPtr->p2p_assocReq_ie, priv->p2pPtr->p2p_assocReq_ie_len);
			
			pbuf += priv->p2pPtr->p2p_assocReq_ie_len;
			txinsn.fr_len += priv->p2pPtr->p2p_assocReq_ie_len;
		}

	}
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
		{
			SAVE_INT_AND_CLI(flags);
			priv->wapiCachedBuf = pbuf+2;
			wapiSetIE(priv);
			pbuf[0] = _EID_WAPI_;
			pbuf[1] = priv->wapiCachedLen;
			pbuf += priv->wapiCachedLen+2;
			txinsn.fr_len += priv->wapiCachedLen+2;
			RESTORE_INT(flags);
	}
#endif

	// Realtek proprietary IE
	if (priv->pshare->rtk_ie_len)
		pbuf = set_ie(pbuf, _RSN_IE_1_, priv->pshare->rtk_ie_len, priv->pshare->rtk_ie_buf, &txinsn.fr_len);

	// Customer proprietary IE
	if (priv->pmib->miscEntry.private_ie_len) {
		memcpy(pbuf, pmib->miscEntry.private_ie, pmib->miscEntry.private_ie_len);
		pbuf += pmib->miscEntry.private_ie_len;
		txinsn.fr_len += pmib->miscEntry.private_ie_len;
	}

	SetFrameSubType((txinsn.phdr), WIFI_ASSOCREQ);

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), bssid, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), bssid, MACADDRLEN);

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
	{
#ifdef RTK_NL80211 //wrt_clnt
		unsigned char *assocreq_ie = txinsn.pframe + 4; //ignore fix ies
		int assocreq_ie_len = (txinsn.fr_len-4);

		//printk("AssocReq Len = %d\n", assocreq_ie_len);
		if(assocreq_ie_len > MAX_ASSOC_REQ_LEN)
		{
			printk("AssocReq Len too LONG !!\n");
			memcpy(priv->rtk->clnt_info.assoc_req, assocreq_ie, MAX_ASSOC_REQ_LEN);
			priv->rtk->clnt_info.assoc_req_len = MAX_ASSOC_REQ_LEN;
		}
		else
		{
			memcpy(priv->rtk->clnt_info.assoc_req, assocreq_ie, assocreq_ie_len);
			priv->rtk->clnt_info.assoc_req_len = assocreq_ie_len;
		}
#endif
		return retval;
	}

issue_assocreq_fail:

	DEBUG_ERR("sending assoc req fail!\n");

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
	return retval;
}

#if defined( UNIVERSAL_REPEATER) || defined(MBSSID) || defined(CONFIG_RTK_MESH)
void ap_sync_chan_to_bss(struct rtl8192cd_priv *priv, int bss_channel, int bss_bw, int bss_offset)
{
    int i, j;
    struct rtl8192cd_priv *vap_priv;
    struct stat_info * pstat;
    STADEBUG("===>\n");
    for(j=0; j<NUM_STAT; j++)
    {
        if (priv->pshare->aidarray[j] && (priv->pshare->aidarray[j]->used == TRUE)
#ifdef WDS
            && !(priv->pshare->aidarray[j]->station.state & WIFI_WDS)
#endif
        ) {
            if (priv != priv->pshare->aidarray[j]->priv)
                continue;

            pstat = &(priv->pshare->aidarray[j]->station);
#ifdef CONFIG_RTK_MESH
            if(isPossibleNeighbor(pstat)) {
                cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
                mesh_cnt_ASSOC_PeerLink_CAP(priv, pstat, DECREASE);
                free_stainfo(priv, pstat);
            }
            else 
#endif
            {
                issue_deauth(priv, pstat->hwaddr, _RSON_DEAUTH_STA_LEAVING_);
            }
        }
    }
    delay_ms(10);

    //sync channel
    priv->pmib->dot11RFEntry.dot11channel = bss_channel;

    //sync bw
    priv->pmib->dot11nConfigEntry.dot11nUse40M = bss_bw;
    priv->pshare->CurrentChannelBW = bss_bw;
    priv->pshare->is_40m_bw = bss_bw; 

    //sync offset
    priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = bss_offset;
    priv->pshare->offset_2nd_chan =  bss_offset;

    //regen ht ie
    priv->ht_cap_len = 0;
    priv->ht_ie_len = 0;

    /*The  beacon IE content before TIM(TLV) need call init_beacon for update ;
           for example channle info(Direct Sequence Parameter Set)
           else content can update by update_beacon()
        */
    init_beacon(priv);

#ifdef MBSSID
	if (priv->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			vap_priv = priv->pvap_priv[i];
			if (vap_priv && IS_DRV_OPEN(vap_priv)) {
				for(j=0; j<NUM_STAT; j++)
				{
					if (priv->pshare->aidarray[j] && (priv->pshare->aidarray[j]->used == TRUE)) {
						if (vap_priv != priv->pshare->aidarray[j]->priv)
							continue;
						issue_deauth(vap_priv, priv->pshare->aidarray[j]->station.hwaddr, _RSON_DEAUTH_STA_LEAVING_);
					}
				}
				delay_ms(10);

				vap_priv->pmib->dot11RFEntry.dot11channel = bss_channel;           //sync channel
				vap_priv->pmib->dot11nConfigEntry.dot11nUse40M = bss_bw;           //sync bandwidth
				vap_priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = bss_offset;  //sync 2nd ch offset

				vap_priv->ht_cap_len = 0;                //regen ht ie
				vap_priv->ht_ie_len = 0;
				init_beacon(vap_priv);
			}
		}
	}
#endif

}
#endif

void clnt_switch_chan_to_bss(struct rtl8192cd_priv *priv)
{
	int bss_channel, bss_bw, bss_offset = 0;

    STADEBUG("===>\n");
	//sync channel
	bss_channel = priv->pmib->dot11Bss.channel;

	//sync bw & offset
	bss_bw = HT_CHANNEL_WIDTH_20_40; 
		
#ifdef RTK_AC_SUPPORT			
	if(GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A || GET_CHIP_VER(priv)==VERSION_8814A){
		if((priv->pmib->dot11Bss.t_stamp[1] & (BSS_BW_MASK << BSS_BW_SHIFT)) 
			== (HT_CHANNEL_WIDTH_80 << BSS_BW_SHIFT))
			bss_bw = HT_CHANNEL_WIDTH_80;									
	}
#endif

	if ((priv->pmib->dot11Bss.t_stamp[1] & (BIT(1) | BIT(2))) == (BIT(1) | BIT(2)))
		bss_offset = HT_2NDCH_OFFSET_BELOW;
	else if ((priv->pmib->dot11Bss.t_stamp[1] & (BIT(1) | BIT(2))) == BIT(1))
		bss_offset = HT_2NDCH_OFFSET_ABOVE;
	else { 
		if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_10)
			bss_bw = HT_CHANNEL_WIDTH_10;
		else if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_5)
			bss_bw = HT_CHANNEL_WIDTH_5;
		else
			bss_bw = HT_CHANNEL_WIDTH_20;	
		
		bss_offset = HT_2NDCH_OFFSET_DONTCARE;
	}


	//sync channel
	priv->pmib->dot11RFEntry.dot11channel = bss_channel;
	
	//sync bw
	priv->pmib->dot11nConfigEntry.dot11nUse40M = bss_bw;
	priv->pshare->CurrentChannelBW = bss_bw;
	priv->pshare->is_40m_bw = bss_bw; 

	//sync offset
	priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = bss_offset;
	priv->pshare->offset_2nd_chan =  bss_offset;

    /*3.4.6,wlanx-vax interface also can support STA mode*/
#if defined( UNIVERSAL_REPEATER) || defined(MBSSID)
	if( !IS_ROOT_INTERFACE(priv)
#ifdef MULTI_MAC_CLONE
		|| (OPMODE&WIFI_STATION_STATE)
#endif
	)
	{
		struct rtl8192cd_priv *priv_root = GET_ROOT(priv);
 
		priv->pshare->switch_chan_rp = bss_channel;
		priv->pshare->band_width_rp = bss_bw;
		priv->pshare->switch_2ndchoff_rp = bss_offset;
			 
		if(IS_DRV_OPEN(priv_root) || (priv_root->pmib->miscEntry.vap_enable))
		if ((priv_root->pmib->dot11RFEntry.dot11channel != bss_channel) ||
			(priv_root->pmib->dot11nConfigEntry.dot11nUse40M != bss_bw) ||
			(priv_root->pmib->dot11nConfigEntry.dot11n2ndChOffset != bss_offset)  ) {
				ap_sync_chan_to_bss(priv_root, bss_channel, bss_bw, bss_offset);
		}	 
		
		priv->pshare->switch_chan_rp = 0;	 
	}
#endif

	// when STA want to connect target AP make sure TX_PAUSE don't pause packet
    priv->site_survey->target_ap_found=1;    
	SwBWMode(priv, bss_bw, bss_offset);
	SwChnl(priv, bss_channel, bss_offset);
    priv->site_survey->target_ap_found=0;    

}


/**
 *	@brief	STA Authentication
 *
 *	STA process Authentication Request first step.
 */
void start_clnt_auth(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
	if((priv->pmib->dot11StationConfigEntry.sc_enabled == 1) && (priv->simple_config_status > 2) && priv->simple_config_status != 5)
	{
		if(rtk_sc_is_channel_fixed(priv))
		{
		return;
	}
	}
#endif


	SAVE_INT_AND_CLI(flags);

	OPMODE_VAL(OPMODE & ~(WIFI_AUTH_SUCCESS | WIFI_AUTH_STATE1 | WIFI_ASOC_STATE));
	OPMODE_VAL(OPMODE | WIFI_AUTH_NULL);
	REAUTH_COUNT_VAL(0);
	REASSOC_COUNT_VAL(0);
	AUTH_SEQ_VAL(1);

	if (PENDING_REAUTH_TIMER)
		DELETE_REAUTH_TIMER;
	if (PENDING_REASSOC_TIMER)
		DELETE_REASSOC_TIMER;

#ifdef CONFIG_RTK_MESH
    if(mesh_channel_switch_initiate(priv)) {/*check if we need to inform other mesh nodes first*/
        RESTORE_INT(flags);
        return;
    }
#endif


#ifdef MULTI_MAC_CLONE
	if (ACTIVE_ID == 0)
#endif
	{
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			if(priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY)
				clnt_ss_check_band(priv, priv->pmib->dot11Bss.channel); 
			reload_txpwr_pg(priv);
		}
#endif

#if 0 // do not switch to bw=20 when issue auth
		priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
		SwBWMode(priv, priv->pshare->CurrentChannelBW, 0);

		SwChnl(priv, priv->pmib->dot11Bss.channel, 0);
#else
		clnt_switch_chan_to_bss(priv); // Eric
#endif

		{
			unsigned int trigger_iqk = 0;
#ifdef CONFIG_RTL_92D_SUPPORT
			if ((GET_CHIP_VER(priv) == VERSION_8192D) && (priv->pmib->dot11Bss.channel > 14)) {
				priv->pshare->iqk_5g_done = 0;
				trigger_iqk++;
			} else
#endif
			{
				if (priv->pmib->dot11Bss.channel <= 14) {
					priv->pshare->iqk_2g_done = 0;
					trigger_iqk++;
				}
			}

			if (trigger_iqk)
				PHY_IQCalibrate(priv);
		}
	}

#ifdef CONFIG_WLAN_HAL_8192EE
		if (GET_CHIP_VER(priv) == VERSION_8192E) {
		if (OPMODE & WIFI_STATION_STATE) {
			if ((priv->pshare->CurrentChannelBW==0 && priv->pmib->dot11RFEntry.dot11channel==13) 
				||(priv->pshare->CurrentChannelBW==1 && priv->pmib->dot11RFEntry.dot11channel>=11))
				Check_92E_Spur_Valid(priv, false);
		}
		}
#endif

	MOD_REAUTH_TIMER(REAUTH_TO);

#ifdef MULTI_MAC_CLONE
	if ((ACTIVE_ID == 0) || ((GET_MIB(priv))->dot11OperationEntry.opmode & WIFI_ASOC_STATE))
#endif
	{
		DEBUG_INFO("start sending auth req\n");
		//STADEBUG("tx auth req\n");		
		issue_auth(priv, NULL, 0);
	}
	
#ifdef INCLUDE_WPA_PSK
	if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
	{
		SSID_LEN = priv->pmib->dot11Bss.ssidlen;
		memcpy(SSID, priv->pmib->dot11Bss.ssid, SSID_LEN);
		derivePSK(priv);
	}
#endif


	RESTORE_INT(flags);
}


/**
 *	@brief	client (STA) association
 *
 *	PS: clnt is client.
 */
void start_clnt_assoc(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	// now auth has succedded...let's perform assoc
	SAVE_INT_AND_CLI(flags);

	OPMODE_VAL(OPMODE & (~ (WIFI_AUTH_NULL | WIFI_AUTH_STATE1 | WIFI_ASOC_STATE)));
	OPMODE_VAL(OPMODE | (WIFI_AUTH_SUCCESS));
	JOIN_RES_VAL(STATE_Sta_Auth_Success);
	REAUTH_COUNT_VAL(0);
	REASSOC_COUNT_VAL(0);

	if (PENDING_REAUTH_TIMER)
		DELETE_REAUTH_TIMER;
	if (PENDING_REASSOC_TIMER)
		DELETE_REASSOC_TIMER;

	DEBUG_INFO("start sending assoc req\n");
	if (issue_assocreq(priv) == 0) {
		MOD_REASSOC_TIMER(REASSOC_TO);			
		RESTORE_INT(flags);
	}
	else {
		RESTORE_INT(flags);
        STADEBUG("start_clnt_lookup(DONTRESCAN)\n");        
		start_clnt_lookup(priv, DONTRESCAN);
	}
}


void clean_for_join(struct rtl8192cd_priv *priv)
{
	int i;
#ifndef SMP_SYNC
	unsigned long flags;
#endif
			/*cfg p2p cfg p2p ; remove*/
	SAVE_INT_AND_CLI(flags);

	for(i=0; i<NUM_STAT; i++) {
		if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (priv != priv->pshare->aidarray[i]->priv)
				continue;
#endif
#ifdef MULTI_MAC_CLONE
			__del_mclone_addr(priv, priv->pshare->aidarray[i]->station.sa_addr);
#else
			if ((free_stainfo(priv, &(priv->pshare->aidarray[i]->station))) == FALSE)
				DEBUG_ERR("free station %d fails\n", i);
#endif
		}
	}

	priv->assoc_num = 0;

	memset(BSSID, 0, MACADDRLEN);
    /*cfg p2p cfg p2p ; remove*/
    OPMODE_VAL(OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE));

	/*cfg p2p cfg p2p ; remove*/

	//P2P_DEBUG("\n\n\n");

	if ((OPMODE & WIFI_STATION_STATE) &&
			((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_) ||
			 (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_) ||
			 (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_WPA_MIXED_PRIVACY_))) {
		memset(&(priv->pmib->dot11GroupKeysTable), 0, sizeof(struct Dot11KeyMappingsEntry));
#ifdef UNIVERSAL_REPEATER
		if (IS_ROOT_INTERFACE(priv))
#endif
			CamResetAllEntry(priv);
	}

	if (IEEE8021X_FUN)
		priv->pmib->dot118021xAuthEntry.dot118021xcontrolport =
			priv->pmib->dot118021xAuthEntry.dot118021xDefaultPort;
	else
		priv->pmib->dot118021xAuthEntry.dot118021xcontrolport = 1;

	if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G|WIRELESS_11A)) {
		if (OPMODE & WIFI_ADHOC_STATE) {
			priv->pmib->dot11ErpInfo.nonErpStaNum = 0;
			check_protection_shortslot(priv);
			priv->pmib->dot11ErpInfo.longPreambleStaNum = 0;
		}
	}

	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
		priv->ht_legacy_sta_num = 0;

	JOIN_RES_VAL(STATE_Sta_No_Bss);
	priv->link_status = 0;
	//netif_stop_queue(priv->dev);		// don't start/stop queue dynamically
	priv->rxBeaconNumInPeriod = 0;
	memset(priv->rxBeaconCntArray, 0, sizeof(priv->rxBeaconCntArray));
	priv->rxBeaconCntArrayIdx = 0;
	priv->rxBeaconCntArrayWindow = 0;
	priv->rxBeaconPercentage = 0;
	priv->rxDataNumInPeriod = 0;
	memset(priv->rxDataCntArray, 0, sizeof(priv->rxDataCntArray));
	priv->rxMlcstDataNumInPeriod = 0;
//	priv->rxDataNumInPeriod_pre = 0;
//	priv->rxMlcstDataNumInPeriod_pre = 0;
	RESTORE_INT(flags);
}

unsigned int mod64(unsigned int A1, unsigned int A2, unsigned int b) 
{
	unsigned int r;
	r = A1%b;
	r = (r<<12) | ((A2>>20)&0x0fff);
	r %=b;
	r = (r<<12) | ((A2>>8)&0x0fff);
	r %=b;	
	r = (r<<8) | (A2&0xff);
	r %=b;
//	DEBUG_INFO("A1=%u, A2=%u, b=%u, r=%u\n", A1, A2, b, r);
	return r;
}

void updateTSF(struct rtl8192cd_priv *priv)
{
	UINT64 tsf;
	unsigned int r ;

	if (priv->beacon_period == 0)
		return;

	tsf = *((UINT64*)priv->rx_timestamp);
	tsf = le64_to_cpu(tsf);
	if( tsf > 1024) {
		r = mod64(tsf>>32, tsf&0xffffffff, priv->beacon_period*1024);
		tsf = tsf -r -1024;
		priv->prev_tsf = tsf;
		RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) | BIT(6));
		RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) & ~ (EN_BCN_FUNCTION));
		RTL_W32(TSFTR, (unsigned int)(tsf&0xffffffff));
		RTL_W32(TSFTR+4, (unsigned int)(tsf>>32));
		RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) | EN_BCN_FUNCTION);
		if(OPMODE & WIFI_STATION_STATE)
			RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) | DIS_ATIM);
		RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) ^ BIT(6));
	}
}


/**
 *	@brief	STA join BSS
 *	Join a BSS, In function, emit join request, Before association must be Authentication. \n
 *	[NOTE] TPT information element
 */
void start_clnt_join(struct rtl8192cd_priv *priv)
{
	struct wifi_mib *pmib = GET_MIB(priv);
	unsigned char null_mac[]={0,0,0,0,0,0};
	unsigned char random;
	int i;

#ifdef DFS
	if (priv->pmib->dot11DFSEntry.disable_tx)
		priv->pmib->dot11DFSEntry.disable_tx = 0;
#endif

#ifdef MULTI_MAC_CLONE
	if (ACTIVE_ID) {
		OPMODE_VAL(WIFI_STATION_STATE);
		JOIN_RES_VAL(STATE_Sta_No_Bss);
		start_clnt_auth(priv);
		return;							
	}
#endif

// stop ss_timer before join ------------------------
	if (timer_pending(&priv->ss_timer))
		del_timer(&priv->ss_timer);
//------------------------------- david+2007-03-10

#ifdef WIFI_SIMPLE_CONFIG
	if (priv->pmib->wscEntry.wsc_enable == 1) { //wps client mode
		if (priv->wps_issue_join_req)
			priv->wps_issue_join_req = 0;
		else {
			priv->recover_join_req = 1;
			return;
		}
	}
#endif

	// if found bss
	if (memcmp(pmib->dot11Bss.bssid, null_mac, MACADDRLEN))
	{
		priv->beacon_period = pmib->dot11Bss.beacon_prd;
		if (pmib->dot11Bss.bsstype & WIFI_AP_STATE)
		{
#ifdef WIFI_SIMPLE_CONFIG
			if (priv->pmib->wscEntry.wsc_enable == 1) //wps client mode
				priv->recover_join_req = 1;
#endif
			clean_for_join(priv);

		/*cfg p2p cfg p2p ; remove*/
			OPMODE_VAL(WIFI_STATION_STATE);
		/*cfg p2p cfg p2p ; remove*/

#ifdef UNIVERSAL_REPEATER
			if (IS_ROOT_INTERFACE(priv))
#endif
			{
				RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_INFRA & NETYPE_Mask) << NETYPE_SHIFT));
				updateTSF(priv);
#if defined(TESTCHIP_SUPPORT) && defined(CONFIG_RTL_92C_SUPPORT)
				if (IS_TEST_CHIP(priv))
					RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) & ~(DIS_TSF_UPDATE |DIS_SUB_STATE));
				else
#endif
					RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) & ~(DIS_TSF_UPDATE_N | DIS_SUB_STATE_N));

			}
			start_clnt_auth(priv);
			return;
		}
		else if (pmib->dot11Bss.bsstype == WIFI_ADHOC_STATE)
		{
			clean_for_join(priv);
			OPMODE_VAL(WIFI_ADHOC_STATE);
			update_bss(&pmib->dot11StationConfigEntry, &pmib->dot11Bss);
			pmib->dot11RFEntry.dot11channel = pmib->dot11Bss.channel;

			if (pmib->dot11BssType.net_work_type & WIRELESS_11N) {
				if (priv->pmib->dot11nConfigEntry.dot11nUse40M) {
					if (pmib->dot11Bss.t_stamp[1] & BIT(1))
						priv->pshare->is_40m_bw	= 1;
					else
						priv->pshare->is_40m_bw	= 0;

					if (priv->pshare->is_40m_bw) {
						if (pmib->dot11Bss.t_stamp[1] & BIT(2))
							priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
						else
							priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
					}
					else
						priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
				}
				else
					priv->pshare->is_40m_bw	= 0;

				priv->ht_cap_len = 0;
				priv->ht_ie_len = 0;

				priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
				SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
			}
#ifdef RTK_AC_SUPPORT		//ADHOC-VHT support
			if (pmib->dot11BssType.net_work_type & WIRELESS_11AC)
			{
				if (priv->pmib->dot11nConfigEntry.dot11nUse40M == 2) {
					priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_80;	
					priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
				}
				else if (priv->pmib->dot11nConfigEntry.dot11nUse40M == 1){
					if (pmib->dot11Bss.t_stamp[1] & BIT(1))
						priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_20_40;

					else
						priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_20;

					if (priv->pshare->is_40m_bw) {
						if (pmib->dot11Bss.t_stamp[1] & BIT(2))
							priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
						else
							priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
					}
					else
						priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
				}
				else
					priv->pshare->is_40m_bw = 0;

				if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_5)
					priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_5;
				else if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_10)					
					priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_10;
				else
					priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;	
				SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		}
			
#endif			
#ifdef CONFIG_RTL_92D_SUPPORT
			if ((GET_CHIP_VER(priv) == VERSION_8192D)&&(priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY)) {
				clnt_ss_check_band(priv, priv->pmib->dot11Bss.channel); 
			}
#endif
			SwChnl(priv, pmib->dot11Bss.channel, priv->pshare->offset_2nd_chan);

			{
				unsigned int trigger_iqk = 0;
#ifdef CONFIG_RTL_92D_SUPPORT
				if ((GET_CHIP_VER(priv) == VERSION_8192D) && (priv->pmib->dot11Bss.channel > 14)) {
					priv->pshare->iqk_5g_done = 0;
					trigger_iqk++;
				} else
#endif
				{
					if (priv->pmib->dot11Bss.channel <= 14) {
						priv->pshare->iqk_2g_done = 0;
						trigger_iqk++;
					}
				}

				if (trigger_iqk)
					PHY_IQCalibrate(priv);
			}

			DEBUG_INFO("Join IBSS: chan=%d, 40M=%d, offset=%d\n", pmib->dot11Bss.channel,
				priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);

			join_bss(priv);
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_ADHOC & NETYPE_Mask) << NETYPE_SHIFT));
			updateTSF(priv);
#if defined(TESTCHIP_SUPPORT) && defined(CONFIG_RTL_92C_SUPPORT)
			if (IS_TEST_CHIP(priv))
				RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) & ~(DIS_TSF_UPDATE ));
			else
#endif
				RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) & ~(DIS_TSF_UPDATE_N));
			
			JOIN_REQ_ONGOING_VAL(0);
			init_beacon(priv);
			JOIN_RES_VAL(STATE_Sta_Ibss_Active);

#ifdef RTK_NL80211 //wrt-adhoc
			if(IS_VXD_INTERFACE(priv))
				construct_ibss_beacon(priv);
#endif

			DEBUG_INFO("Join IBSS - %02X:%02X:%02X:%02X:%02X:%02X\n",
				BSSID[0], BSSID[1], BSSID[2], BSSID[3], BSSID[4], BSSID[5]);
			LOG_MSG("Join IBSS - %02X:%02X:%02X:%02X:%02X:%02X\n",
				BSSID[0], BSSID[1], BSSID[2], BSSID[3], BSSID[4], BSSID[5]);
			return;
		}
		else
			return;
	}

	// not found
	//if (OPMODE & WIFI_STATION_STATE) orig
	/*new ; under vxd if not found then stop ss and wait next time by timer*/	
	if ((OPMODE & WIFI_STATION_STATE) && priv->ss_req_ongoing != SSFROM_REPEATER_VXD)	
	{
		clean_for_join(priv);
#ifdef UNIVERSAL_REPEATER
		if (IS_ROOT_INTERFACE(priv))
#endif
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
		JOIN_RES_VAL(STATE_Sta_No_Bss);
		JOIN_REQ_ONGOING_VAL(0);
        STADEBUG("start_clnt_lookup(RESCAN)\n");
		start_clnt_lookup(priv, RESCAN);
		return;
	}
	else if (OPMODE & WIFI_ADHOC_STATE)
	{
		unsigned char tmpbssid[MACADDRLEN];
		int start_period;

		memset(tmpbssid, 0, MACADDRLEN);
		if (!memcmp(BSSID, tmpbssid, MACADDRLEN)) {
			// generate an unique Ibss ssid
#ifdef __ECOS
			{
				unsigned char random_buf[4];
				get_random_bytes(random_buf, 4);
				random = random_buf[3];
			}
#else
			get_random_bytes(&random, 1);
#endif
			tmpbssid[0] = 0x02;
			for (i=1; i<MACADDRLEN; i++)
				tmpbssid[i] = GET_MY_HWADDR[i-1] ^ GET_MY_HWADDR[i] ^ random;
			while(1) {
				for (i=0; i<priv->site_survey->count_target; i++) {
					if (!memcmp(tmpbssid, priv->site_survey->bss_target[i].bssid, MACADDRLEN)) {
						tmpbssid[5]++;
						break;
					}
				}
				if (i == priv->site_survey->count)
					break;
			}

			clean_for_join(priv);
			memcpy(BSSID, tmpbssid, MACADDRLEN);
			if (SSID_LEN == 0) {
				SSID_LEN = pmib->dot11StationConfigEntry.dot11DefaultSSIDLen;
				memcpy(SSID, pmib->dot11StationConfigEntry.dot11DefaultSSID, SSID_LEN);
			}

			pmib->dot11Bss.channel = pmib->dot11RFEntry.dot11channel;

			if (pmib->dot11BssType.net_work_type & WIRELESS_11N) {
				priv->pshare->is_40m_bw = priv->pmib->dot11nConfigEntry.dot11nUse40M;
				if (priv->pshare->is_40m_bw)
					priv->pshare->offset_2nd_chan = priv->pmib->dot11nConfigEntry.dot11n2ndChOffset;
				else
					priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;

				if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_5)
					priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_5;
				else if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_10)					
					priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_10;
				else
				priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
				SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
			}
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8192D){
				if (priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY)
					clnt_ss_check_band(priv, priv->pmib->dot11Bss.channel);
				reload_txpwr_pg(priv);
			}
#endif

			SwChnl(priv, pmib->dot11Bss.channel, priv->pshare->offset_2nd_chan);

			{
				unsigned int trigger_iqk = 0;
#ifdef CONFIG_RTL_92D_SUPPORT
				if ((GET_CHIP_VER(priv) == VERSION_8192D) && (priv->pmib->dot11Bss.channel > 14)) {
					priv->pshare->iqk_5g_done = 0;
					trigger_iqk++;
				} else
#endif
				{
					if (priv->pmib->dot11Bss.channel <= 14) {
						priv->pshare->iqk_2g_done = 0;
						trigger_iqk++;
					}
				}

				if (trigger_iqk)
					PHY_IQCalibrate(priv);
			}

			DEBUG_INFO("Start IBSS: chan=%d, 40M=%d, offset=%d\n", pmib->dot11Bss.channel,
				priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);
			DEBUG_INFO("Start IBSS - %02X:%02X:%02X:%02X:%02X:%02X\n",
				BSSID[0], BSSID[1], BSSID[2], BSSID[3], BSSID[4], BSSID[5]);

			join_bss(priv);

#ifdef RTK_NL80211 //wrt-adhoc
			if(IS_VXD_INTERFACE(priv))
				construct_ibss_beacon(priv);
#endif

			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_ADHOC & NETYPE_Mask) << NETYPE_SHIFT));
			updateTSF(priv);
#if defined(TESTCHIP_SUPPORT) && defined(CONFIG_RTL_92C_SUPPORT)
			if (IS_TEST_CHIP(priv))
				RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) & ~(DIS_TSF_UPDATE));
			else
#endif
				RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) & ~(DIS_TSF_UPDATE_N));

			
			priv->beacon_period = pmib->dot11StationConfigEntry.dot11BeaconPeriod;
			JOIN_RES_VAL(STATE_Sta_Ibss_Idle);
			JOIN_REQ_ONGOING_VAL(0);
			if (priv->auto_channel) {
				priv->auto_channel = 1;
				priv->ss_ssidlen = 0;
				DEBUG_INFO("start_clnt_ss, trigger by %s, ss_ssidlen=0\n", (char *)__FUNCTION__);
				RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
				start_clnt_ss(priv);
				return;
			}
			else
				init_beacon(priv);

			JOIN_RES_VAL(STATE_Sta_Ibss_Idle);
		}
		else {
			pmib->dot11Bss.channel = pmib->dot11RFEntry.dot11channel;

			if (pmib->dot11BssType.net_work_type & WIRELESS_11N) {
				priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
				SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
			}
#ifdef CONFIG_RTL_92D_SUPPORT
			if ((GET_CHIP_VER(priv) == VERSION_8192D)&&(priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY)) {
				clnt_ss_check_band(priv, priv->pmib->dot11Bss.channel); 
			}
#endif
			SwChnl(priv, pmib->dot11Bss.channel, priv->pshare->offset_2nd_chan);

			{
				unsigned int trigger_iqk = 0;
#ifdef CONFIG_RTL_92D_SUPPORT
				if ((GET_CHIP_VER(priv) == VERSION_8192D) && (priv->pmib->dot11Bss.channel > 14)) {
					priv->pshare->iqk_5g_done = 0;
					trigger_iqk++;
				} else
#endif
				{
					if (priv->pmib->dot11Bss.channel <= 14) {
						priv->pshare->iqk_2g_done = 0;
						trigger_iqk++;
					}
				}

				if (trigger_iqk)
					PHY_IQCalibrate(priv);
			}

			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_ADHOC & NETYPE_Mask) << NETYPE_SHIFT));

			DEBUG_INFO("Start IBSS: chan=%d, 40M=%d, offset=%d\n", pmib->dot11Bss.channel,
				priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);
			DEBUG_INFO("Start IBSS - %02X:%02X:%02X:%02X:%02X:%02X\n",
				BSSID[0], BSSID[1], BSSID[2], BSSID[3], BSSID[4], BSSID[5]);
			JOIN_RES_VAL(STATE_Sta_Ibss_Idle);
		}

		// start for more than scanning period, including random backoff
		start_period = UINT32_DIFF(jiffies, priv->jiffies_pre) / HZ + 1;
#ifdef __ECOS
		{
			unsigned char random_buf[4];
			get_random_bytes(random_buf, 4);
			random = random_buf[3];
		}
#else
		get_random_bytes(&random, 1);
#endif
		start_period += (random % 5);
		mod_timer(&priv->idle_timer, jiffies + RTL_SECONDS_TO_JIFFIES(start_period));

		LOG_MSG("Start IBSS - %02X:%02X:%02X:%02X:%02X:%02X\n",
			BSSID[0], BSSID[1], BSSID[2], BSSID[3], BSSID[4], BSSID[5]);

		return;
	}
	else
		return;
}


int check_bss_networktype(struct rtl8192cd_priv * priv, struct bss_desc *bss_target)
{
	int result;

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
		(priv->pmib->dot11StationConfigEntry.legacySTADeny & WIRELESS_11G) &&
		!(bss_target->network & WIRELESS_11N))
		result = FAIL;
	else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
		(priv->pmib->dot11StationConfigEntry.legacySTADeny & WIRELESS_11B) &&
		(bss_target->network == WIRELESS_11B))
		result = FAIL;
	else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
		(priv->pmib->dot11StationConfigEntry.legacySTADeny & WIRELESS_11B) &&
		(bss_target->network == WIRELESS_11B))
		result = FAIL;
	else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) &&
		(priv->pmib->dot11StationConfigEntry.legacySTADeny & WIRELESS_11N) &&
		!(bss_target->network & WIRELESS_11AC))
		result = FAIL;
	else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) &&
		(priv->pmib->dot11StationConfigEntry.legacySTADeny & WIRELESS_11A) &&
		(bss_target->network == WIRELESS_11A))
		result = FAIL;
	else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
		(priv->pmib->dot11StationConfigEntry.legacySTADeny & WIRELESS_11A) &&
		(bss_target->network == WIRELESS_11A))
		result = FAIL;
	else
		result = SUCCESS;

	if (result == FAIL) {
		DEBUG_ERR("Deny connect to a legacy AP!\n");
	}

	return result;
}

#ifdef MULTI_MAC_CLONE
void mclone_start_roaming(struct rtl8192cd_priv *priv)
{
	int i;

	if (MCLONE_NUM > 0) {
		for (i=0; i<MAX_MAC_CLONE_NUM; i++) {
			ACTIVE_ID = i+1;
			if (priv == priv->pshare->mclone_sta[i].priv) {
				if (OPMODE & WIFI_ASOC_STATE) {
					OPMODE_VAL(OPMODE & ~(WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE));
					JOIN_RES_VAL(STATE_Sta_No_Bss);
					start_clnt_join(priv);
				}
			}
		}
		ACTIVE_ID = 0;
	}
}
#endif


#if 1   //def SMART_REPEATER_MODE
int check_ap_security(struct rtl8192cd_priv *priv, struct bss_desc *bss)

{
#ifdef SUPPORT_MULTI_PROFILE
	if (GET_MIB(priv)->ap_profile.enable_profile && 
			GET_MIB(priv)->ap_profile.profile_num > 0) {

		if ((GET_MIB(priv)->ap_profile.profile[priv->profile_idx].encryption==0) && (bss->capability&BIT(4)))
			return 0;
		else if ((GET_MIB(priv)->ap_profile.profile[priv->profile_idx].encryption == 1) ||
			(GET_MIB(priv)->ap_profile.profile[priv->profile_idx].encryption == 2)) {
			if ((bss->capability&BIT(4))==0)
				return 0;
			else if (bss->t_stamp[0]!=0)
				return 0;
		}
		else if ((GET_MIB(priv)->ap_profile.profile[priv->profile_idx].encryption == 3) ||
			(GET_MIB(priv)->ap_profile.profile[priv->profile_idx].encryption == 4)) {
			if ((bss->capability&BIT(4))==0)
				return 0;
			else if (bss->t_stamp[0]==0)
				return 0;
		}

		if (check_bss_networktype(priv, bss)) 
			return 1;
	}
	else
#endif			
	{
		if ((GET_MIB(priv)->dot1180211AuthEntry.dot11PrivacyAlgrthm==_NO_PRIVACY_) && (bss->capability&BIT(4)))
			return 0;
		else if ((GET_MIB(priv)->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_) ||
			(GET_MIB(priv)->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)) {
			if ((bss->capability&BIT(4))==0)
				return 0;
			else if (bss->t_stamp[0]!=0)
				return 0;
		}
		else if ((GET_MIB(priv)->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_) ||
			(GET_MIB(priv)->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_)) {
			if ((bss->capability&BIT(4))==0)
				return 0;
			else if (bss->t_stamp[0]==0)
				return 0;
		}

		if (check_bss_networktype(priv, bss)) 
			return 1;
	}
	return 0;
}
#endif


#ifdef SUPPORT_MULTI_PROFILE
void  switch_profile(struct rtl8192cd_priv *priv, int idx)
{
	struct ap_profile *profile;
	int key_len;

	if (idx > priv->pmib->ap_profile.profile_num) {
		panic_printk("Invalid profile idx (%d), reset to 0.\n", idx);
		idx = 0;
	}

	profile = &priv->pmib->ap_profile.profile[idx];

	SSID2SCAN_LEN = strlen(profile->ssid);
	SSID_LEN = strlen(profile->ssid);
	memcpy(SSID2SCAN, profile->ssid, SSID2SCAN_LEN);
	memcpy(SSID, profile->ssid, SSID_LEN);

	OPMODE_VAL(WIFI_STATION_STATE);
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _NO_PRIVACY_;
	priv->pmib->dot1180211AuthEntry.dot11EnablePSK= 0;
	priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = profile->auth_type;

	if (profile->encryption == 1 || profile->encryption == 2) {
		if (profile->encryption == 1) {
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _WEP_40_PRIVACY_;
			key_len = 5;
		}
		else {
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _WEP_104_PRIVACY_;
			key_len = 13;
		}
		memcpy(&priv->pmib->dot11DefaultKeysTable.keytype[0], profile->wep_key1, key_len);
		memcpy(&priv->pmib->dot11DefaultKeysTable.keytype[1], profile->wep_key2, key_len);
		memcpy(&priv->pmib->dot11DefaultKeysTable.keytype[2], profile->wep_key3, key_len);
		memcpy(&priv->pmib->dot11DefaultKeysTable.keytype[3], profile->wep_key4, key_len);
		priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = profile->wep_default_key;

		priv->pmib->dot11GroupKeysTable.dot11Privacy = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
		memcpy(&priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
							&priv->pmib->dot11DefaultKeysTable.keytype[0].skey[0], key_len);
		priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = key_len;
		priv->pmib->dot11GroupKeysTable.keyid = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		priv->pmib->dot11GroupKeysTable.keyInCam = 0;
	}
	else if (profile->encryption == 3 || profile->encryption == 4) {
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _CCMP_PRIVACY_;
		if (profile->encryption == 3) {
			priv->pmib->dot1180211AuthEntry.dot11EnablePSK = PSK_WPA;
			priv->pmib->dot1180211AuthEntry.dot11WPACipher = profile->wpa_cipher;
		}
		else {
			priv->pmib->dot1180211AuthEntry.dot11EnablePSK = PSK_WPA2;
			priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = profile->wpa_cipher;
		}
		strcpy(priv->pmib->dot1180211AuthEntry.dot11PassPhrase, profile->wpa_psk);
	}

	if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK) {
		psk_init(priv);
		priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm= 1;
	}
	else	 {
		priv->pmib->dot11RsnIE.rsnielen = 0;
		priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm= 0;
	}
	
	if (should_forbid_Nmode(priv)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
			if (!priv->mask_n_band) 
				priv->mask_n_band = (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N | WIRELESS_11AC));
			priv->pmib->dot11BssType.net_work_type &= ~(WIRELESS_11N | WIRELESS_11AC);
		}
	}
	else {
		if (priv->mask_n_band) {
			priv->pmib->dot11BssType.net_work_type |= priv->mask_n_band;
			priv->mask_n_band = 0;				
		}		
	}	
}
#endif /* SUPPORT_MULTI_PROFILE */


unsigned int get_ava_2ndchoff(struct rtl8192cd_priv *priv, unsigned int channel, unsigned int bandwidth)
{

	int chan_offset = 0;

	if(bandwidth == HT_CHANNEL_WIDTH_20)
		return HT_2NDCH_OFFSET_DONTCARE;
	
	if(channel >=34){
		if((channel>144) ? ((channel-1)%8) : (channel%8)) {
			chan_offset = HT_2NDCH_OFFSET_ABOVE;
		} else {
			chan_offset = HT_2NDCH_OFFSET_BELOW;
		}
	}
	else
	{
		if(channel < 5)
			chan_offset = HT_2NDCH_OFFSET_ABOVE;
		else if(channel > 9)
			chan_offset = HT_2NDCH_OFFSET_BELOW;
		else
		{
			chan_offset = priv->pmib->dot11nConfigEntry.dot11n2ndChOffset;
			if(chan_offset == HT_2NDCH_OFFSET_DONTCARE)
				chan_offset = HT_2NDCH_OFFSET_BELOW;
		}
	}

	return chan_offset;

}


/**
 *	@brief	STA don't how to do
 *	popen:Maybe process client lookup and auth and assoc by IOCTL trigger
 *
 *	[Important]
 *	Exceed Authentication times, process this function.
 *	@param	rescan	: process rescan.
 */
void start_clnt_lookup(struct rtl8192cd_priv *priv, int rescan)
{
	struct wifi_mib *pmib = GET_MIB(priv);
	unsigned char null_mac[]={0,0,0,0,0,0};
	char tmpbuf[33];
	int i;
#ifdef SUPPORT_MULTI_PROFILE
	int j1, j2;
	int found = 0;
#endif	

#ifdef SMP_SYNC
	int locked=0;
#endif // SMP_SYNC

#ifdef P2P_SUPPORT
			/*cfg p2p cfg p2p */
	if((rtk_p2p_is_enabled(priv))&&(rtk_p2p_chk_role(priv,P2P_DEVICE))){
		P2P_DEBUG("P2P_DEVICE don't lookup\n");
		return;							
	}
#endif

#ifdef MULTI_MAC_CLONE
	if (ACTIVE_ID) {
		start_clnt_join(priv);
		return;
	}
#endif

#ifdef SMART_REPEATER_MODE
#ifdef RTK_NL80211 //wrt-adhoc
	if(IS_VXD_INTERFACE(priv) && (OPMODE & WIFI_ADHOC_STATE))
		printk("DO RESCAN for VXD ADHOC!! \n");
	else
#endif
	if(!IS_ROOT_INTERFACE(priv) && rescan)         
    {
		SDEBUG("STA(non-root)don't SS immediately wait next time by timer\n");
#ifdef SMP_SYNC		
		locked = 0;		
		SMP_TRY_LOCK(flags,locked);
#endif
        set_vxd_rescan(priv,rescan);  /*when rescan==2 means Roaming or AP's CH/2ndch/BW be changed*/       
#ifdef SMP_SYNC				
		if(locked)
			SMP_UNLOCK(flags);
#endif
		return;
	}		
#endif		



    //if (rescan || ((priv->site_survey->count_target > 0) && ((priv->join_index+1) >= priv->site_survey->count_target)))
	if (rescan || ((priv->site_survey->count_target > 0) && ((priv->join_index+1) > priv->site_survey->count_target)))
	{
#ifdef P2P_SUPPORT
			/*cfg p2p cfg p2p */
	if((rtk_p2p_is_enabled(priv)==PROPERTY_P2P) && rtk_p2p_chk_role(priv,P2P_CLIENT) )
	{
        rtk_p2p_set_state(priv,P2P_S_IDLE);         /*cfg p2p cfg p2p */
	}		
#endif
		JOIN_RES_VAL(STATE_Sta_Roaming_Scan);
		if (OPMODE & WIFI_SITE_MONITOR) // if scanning, scan later
			return;

#if	0	//def SUPPORT_MULTI_PROFILE
		if (priv->pmib->ap_profile.enable_profile && priv->pmib->ap_profile.profile_num > 0) {

			switch_profile(priv, 0);
		}
#endif

		priv->ss_ssidlen = SSID2SCAN_LEN;
		memcpy(priv->ss_ssid, SSID2SCAN, SSID2SCAN_LEN);
		DEBUG_INFO("start_clnt_ss, trigger by %s, ss_ssidlen=%d, rescan=%d\n", (char *)__FUNCTION__, priv->ss_ssidlen, rescan);
		priv->jiffies_pre = jiffies;
		if((OPMODE & WIFI_ADHOC_STATE) && rescan)	
			RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) | STOP_BCN); //when start Ad-hoc ss, disable beacon
		start_clnt_ss(priv);
#ifdef MULTI_MAC_CLONE
		mclone_start_roaming(priv);
#endif		
		return;
	}

#ifdef SMART_REPEATER_MODE

	if (priv->ss_req_ongoing != SSFROM_REPEATER_VXD)
#endif
    {
		memset(&pmib->dot11Bss, 0, sizeof(struct bss_desc));
    }
	
	
	if (SSID2SCAN_LEN > 0
#ifdef SUPPORT_MULTI_PROFILE		
		|| (priv->pmib->ap_profile.enable_profile && priv->pmib->ap_profile.profile_num > 0)
#endif		
		)
	{
		for (i=priv->join_index+1; i<priv->site_survey->count_target; i++)
		{
			// check SSID
#ifdef SUPPORT_MULTI_PROFILE			
            
			if (priv->pmib->ap_profile.enable_profile && priv->pmib->ap_profile.profile_num > 0) {
				int idx3,tmpidx;                
				int pidx = priv->profile_idx;
				found = 0;
				for(j2=0;j2<priv->pmib->ap_profile.profile_num;j2++) 
				{
					j1 = (pidx + j2) % priv->pmib->ap_profile.profile_num;	

						if(strlen(priv->pmib->ap_profile.profile[j1].ssid)==0){ 
							continue;
					}
					SSID2SCAN_LEN = strlen(priv->pmib->ap_profile.profile[j1].ssid);
					memcpy(SSID2SCAN, priv->pmib->ap_profile.profile[j1].ssid, SSID2SCAN_LEN);                    
                    SSID2SCAN[SSID2SCAN_LEN]='\0';

					//STADEBUG("Profile.ssid=%s, target.ssid=%s,rssi=[%d]\n",priv->pmib->ap_profile.profile[j1].ssid, priv->site_survey->bss_target[i].ssid,priv->site_survey->bss_target[i].rssi);
					if((priv->pmib->miscEntry.stage == 0) ||
						((priv->site_survey->bss_target[i].stage + 1) == priv->pmib->miscEntry.stage))
					if ((priv->site_survey->bss_target[i].ssidlen == SSID2SCAN_LEN) &&
					(!memcmp(SSID2SCAN, priv->site_survey->bss_target[i].ssid, SSID2SCAN_LEN))
					) {
						priv->profile_idx = j1;
                		//STADEBUG("  found[%s];switch_profile(%d)\n",SSID2SCAN ,priv->profile_idx);                        
						switch_profile(priv, priv->profile_idx);

						if(check_ap_security(priv, &priv->site_survey->bss_target[i])) {
							syncMulticastCipher(priv, &priv->site_survey->bss_target[i]);
							found = 1;
							break;
						}
					}						
				}												
				tmpidx = priv->profile_idx;
				for(idx3=0;idx3<priv->pmib->ap_profile.profile_num;idx3++){
		    			tmpidx++;
		    			tmpidx%=priv->pmib->ap_profile.profile_num;
		    			if(strlen(priv->pmib->ap_profile.profile[tmpidx].ssid)==0){
		    			}else{
						priv->profile_idx=tmpidx;
						break;
		    			}
				}
				STADEBUG("next to search profile_idx[%d]\n",priv->profile_idx);                        
			}
			else 				
			if((priv->pmib->miscEntry.stage == 0) ||
				((priv->site_survey->bss_target[i].stage + 1) == priv->pmib->miscEntry.stage))
			{		
				if ((priv->site_survey->bss_target[i].ssidlen == SSID2SCAN_LEN) &&
				(!memcmp(SSID2SCAN, priv->site_survey->bss_target[i].ssid, SSID2SCAN_LEN))) 
				{
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
					if(priv->simple_config_status == 3)
					{					
						rtk_sc_check_security(priv, &(priv->site_survey->bss_target[i]));
					}
#endif
					found = 1;
				}
				else
					found = 0;
			}	
#endif
			
#ifdef SUPPORT_MULTI_PROFILE
			if (found)
#else
			if((priv->pmib->miscEntry.stage == 0) ||
				((priv->site_survey->bss_target[i].stage + 1) == priv->pmib->miscEntry.stage))
			if ((priv->site_survey->bss_target[i].ssidlen == SSID2SCAN_LEN) &&
					(!memcmp(SSID2SCAN, priv->site_survey->bss_target[i].ssid, SSID2SCAN_LEN)))
#endif
			{			
#ifdef SUPPORT_CLIENT_MIXED_SECURITY			
                choose_cipher(priv, &priv->site_survey->bss_target[i]);
#endif
				syncMulticastCipher(priv, &priv->site_survey->bss_target[i]);
				// check BSSID
				if (!memcmp(pmib->dot11StationConfigEntry.dot11DesiredBssid, null_mac, MACADDRLEN) ||
					!memcmp(priv->site_survey->bss_target[i].bssid, pmib->dot11StationConfigEntry.dot11DesiredBssid, MACADDRLEN)
#ifdef SMART_REPEATER_MODE
					|| (priv->ss_req_ongoing == SSFROM_REPEATER_VXD)
#endif
					)
				{
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
					if(priv->simple_config_status == 3)
					{					
						rtk_sc_check_security(priv, &(priv->site_survey->bss_target[i]));
					}
#endif
					// check BSS type
					if (((OPMODE & WIFI_STATION_STATE) && (priv->site_survey->bss_target[i].bsstype == WIFI_AP_STATE)) ||
						((OPMODE & WIFI_ADHOC_STATE) && (priv->site_survey->bss_target[i].bsstype == WIFI_ADHOC_STATE))
#ifdef SMART_REPEATER_MODE
						|| ((priv->ss_req_ongoing == SSFROM_REPEATER_VXD) && (priv->site_survey->bss_target[i].bsstype == WIFI_AP_STATE))
#endif
						)
					{


						//check encryption ; if security setting no match with mine  use next site_survey->bss_target
						if ((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==_NO_PRIVACY_) && (priv->site_survey->bss_target[i].capability&BIT(4)))
							continue;
						else if ((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_) ||
							(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)) {
							if ((priv->site_survey->bss_target[i].capability&BIT(4))==0)
								continue;
							else if (priv->site_survey->bss_target[i].t_stamp[0]!=0)
								continue;
						}
						else if ((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_) ||
							(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_)) {
							if ((priv->site_survey->bss_target[i].capability&BIT(4))==0)
								continue;
							else if (priv->site_survey->bss_target[i].t_stamp[0]==0)
								continue;
						}
						if ((OPMODE & WIFI_ADHOC_STATE) && (priv->site_survey->bss_target[i].bsstype == WIFI_ADHOC_STATE)){
							if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK){
								if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK==1){
									if (!((priv->site_survey->bss_target[i].t_stamp[0] & (BIT(2)|BIT(4)|BIT(8)|BIT(10))))) {
										continue;
									}
								} 
								else if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK==2){
									if (!((priv->site_survey->bss_target[i].t_stamp[0] & (BIT(18)|BIT(20)|BIT(24)|BIT(26))))){
										continue;
									}
								}
							}
						}
						
			#ifdef CONFIG_IEEE80211W_CLI
						if(!priv->support_pmf 
							&& (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W==MGMT_FRAME_PROTECTION_REQUIRED)
							&& (priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA2)){
								PMFDEBUG("(%s)line=%d, AP NOT SUPPORT PMF, but CLI is PMF requested\n", __FUNCTION__, __LINE__);
								PMFDEBUG("(%s)line=%d, dot11IEEE80211W = %d\n", __FUNCTION__, __LINE__,priv->pmib->dot1180211AuthEntry.dot11IEEE80211W);
							continue ;
						}
			#endif
						//check encryption ; if security setting no match with mine  use next site_survey->bss_target
						{
							// check network type
							if (check_bss_networktype(priv, &(priv->site_survey->bss_target[i])))
							{
                                #ifdef MBSSID
                                //under multi-repeater case when some STA has connect , the other one don't connect to diff channel AP ; skip this
	                            if(IS_VAP_INTERFACE(priv) && multiRepeater_startlookup_chk(priv,i) ){
                                    STADEBUG("RP1 rdy connected and RP2's target ch/2nd ch not match\n");
                                    continue;
                                }
                                #endif						
								memcpy(tmpbuf, SSID2SCAN, SSID2SCAN_LEN);
								tmpbuf[SSID2SCAN_LEN] = '\0';
								DEBUG_INFO("found desired bss [%s], start to join\n", tmpbuf);								
								STADEBUG("(1)found desired bss [%s] start to join,\n ch=%d,2ndch=%d , i=%d\n",tmpbuf ,priv->pshare->switch_chan_rp ,priv->pshare->switch_2ndchoff_rp,i );


								memcpy(&pmib->dot11Bss, &(priv->site_survey->bss_target[i]), sizeof(struct bss_desc));
								break;
							}
						}
					}
				}
			}
		}
		priv->join_index = i;
	}
	else
	{
		for (i=priv->join_index+1; i<priv->site_survey->count_target; i++)
		{
			if((priv->pmib->miscEntry.stage == 0) ||
				((priv->site_survey->bss_target[i].stage + 1) == priv->pmib->miscEntry.stage))
			// check BSSID
			if (!memcmp(pmib->dot11StationConfigEntry.dot11DesiredBssid, null_mac, MACADDRLEN) ||
				!memcmp(priv->site_survey->bss_target[i].bssid, pmib->dot11StationConfigEntry.dot11DesiredBssid, MACADDRLEN))
			{
				// check BSS type
				if (((OPMODE & WIFI_STATION_STATE) && (priv->site_survey->bss_target[i].bsstype == WIFI_AP_STATE)) ||
					((OPMODE & WIFI_ADHOC_STATE) && (priv->site_survey->bss_target[i].bsstype == WIFI_ADHOC_STATE)))
				{
					// check encryption
					if (((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm) && (priv->site_survey->bss_target[i].capability&BIT(4))) ||
						((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==0) && ((priv->site_survey->bss_target[i].capability&BIT(4))==0)))
					{
						if ((OPMODE & WIFI_ADHOC_STATE) && (priv->site_survey->bss_target[i].bsstype == WIFI_ADHOC_STATE)){
							if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK){
								if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK==1){
									if (!((priv->site_survey->bss_target[i].t_stamp[0] & (BIT(2)|BIT(4)|BIT(8)|BIT(10))))) {
										continue;
									}
								} 
								else if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK==2){
									if (!((priv->site_survey->bss_target[i].t_stamp[0] & (BIT(18)|BIT(20)|BIT(24)|BIT(26))))){
										continue;
									}
								}
							}
						}
						// check network type
						if (check_bss_networktype(priv, &(priv->site_survey->bss_target[i])))
						{
                            #if 0   //def UNIVERSAL_REPEATER
							// if this is vxd interface, and chan of found AP is
							if ((GET_ROOT_PRIV(priv)->pmib->dot11RFEntry.dot11channel!= priv->site_survey->bss_target[i].channel)){
                                SDEBUG("if this is vxd interface, and chan of found AP is different with root interface AP, skip it\n");
								continue;
                            }
                            #endif
							memcpy(tmpbuf, priv->site_survey->bss_target[i].ssid, priv->site_survey->bss_target[i].ssidlen);
							tmpbuf[priv->site_survey->bss_target[i].ssidlen] = '\0';
							DEBUG_INFO("found desired bss [%s], start to join\n", tmpbuf);							
							STADEBUG("(2)found desired bss [%s], start to join\n\n", tmpbuf);

							memcpy(&pmib->dot11Bss, &(priv->site_survey->bss_target[i]), sizeof(struct bss_desc));
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
							if(priv->simple_config_status == 3)
							{					
								rtk_sc_check_security(priv, &(priv->site_survey->bss_target[i]));
							}
#endif

							break;
						}
					}
				}
			}
		}
		priv->join_index = i;
	}

#ifdef WIFI_WPAS
	if(priv->wpas_manual_assoc == 0)
#endif

//#ifdef SMART_REPEATER_MODE;20130725 remove
//	if (priv->ss_req_ongoing != 3)
//#endif
		start_clnt_join(priv);
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
	if((priv->simple_config_could_fix == 0) && (priv->pmib->dot11StationConfigEntry.sc_enabled == 1))
	{
		priv->simple_config_could_fix = 1;
	}
#endif
}


void calculate_rx_beacon(struct rtl8192cd_priv *priv)
{
	int window_top;
	unsigned int rx_beacon_delta, expect_num, decision_period, rx_data_delta;

	if ((((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) == (WIFI_STATION_STATE | WIFI_ASOC_STATE)) ||
		((OPMODE & WIFI_ADHOC_STATE) &&
				((JOIN_RES == STATE_Sta_Ibss_Active) || (JOIN_RES == STATE_Sta_Ibss_Idle)))) &&
		!priv->ss_req_ongoing)
	{
		if (OPMODE & WIFI_ADHOC_STATE)
			decision_period = ROAMING_DECISION_PERIOD_ADHOC;
		else
			decision_period = ROAMING_DECISION_PERIOD_INFRA;

		priv->rxBeaconCntArray[priv->rxBeaconCntArrayIdx] = priv->rxBeaconNumInPeriod;
		priv->rxDataCntArray[priv->rxBeaconCntArrayIdx] = priv->rxDataNumInPeriod;
		if (priv->rxBeaconCntArrayWindow < decision_period)
			priv->rxBeaconCntArrayWindow++;
		else
		{
			window_top = priv->rxBeaconCntArrayIdx + 1;
			if (window_top == decision_period)
				window_top = 0;

			rx_beacon_delta = UINT32_DIFF(priv->rxBeaconCntArray[priv->rxBeaconCntArrayIdx],
				priv->rxBeaconCntArray[window_top]);

			rx_data_delta = UINT32_DIFF(priv->rxDataCntArray[priv->rxBeaconCntArrayIdx],
				priv->rxDataCntArray[window_top]);

			expect_num = (decision_period * 1000) / priv->beacon_period;
			priv->rxBeaconPercentage = (rx_beacon_delta * 100) / expect_num;

			//DEBUG_INFO("Rx beacon percentage=%d%%, delta=%d, cnt=%d\n", priv->rxBeaconPercentage,
			//	rx_beacon_delta, priv->rxBeaconCntArray[priv->rxBeaconCntArrayIdx]);
#ifdef CLIENT_MODE
			if (OPMODE & WIFI_STATION_STATE)
			{
				// when fast-roaming is enabled, trigger roaming while (david+2006-01-25):
				//	- no any beacon frame received in last one sec (under beacon interval is <= 200ms)
				//  - rx beacon is less than FAST_ROAMING_THRESHOLD
				int offset, fast_roaming_triggered=0;
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)
				int if_idx = -1;
#endif
				if (priv->pmib->dot11StationConfigEntry.fastRoaming) {
					if (priv->beacon_period <= 200) {
						if (priv->rxBeaconCntArrayIdx == 0)
							offset = priv->rxBeaconNumInPeriod - priv->rxBeaconCntArray[decision_period];
						else
							offset = priv->rxBeaconNumInPeriod - priv->rxBeaconCntArray[priv->rxBeaconCntArrayIdx-1];
						if (offset == 0)
							fast_roaming_triggered = 1;
					}
					if (!fast_roaming_triggered && priv->rxBeaconPercentage < FAST_ROAMING_THRESHOLD)
						fast_roaming_triggered = 1;
				}
				if ((priv->rxBeaconPercentage < ROAMING_THRESHOLD || fast_roaming_triggered) && !rx_data_delta) {
					DEBUG_INFO("Roaming...\n");
					LOG_MSG("Roaming...\n");
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
					LOG_MSG_NOTICE("Roaming...;note:\n");
#endif
					OPMODE_VAL(OPMODE & ~(WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE));
#ifdef UNIVERSAL_REPEATER
					disable_vxd_ap(GET_VXD_PRIV(priv));
#endif

					JOIN_RES_VAL(STATE_Sta_No_Bss);
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)
					if((if_idx = get_wlan_if_type(priv))>=0){
						set_if_status(if_idx,IF_DOWN);
						set_if_change_status(if_idx,IF_STATUS_CHANGE);
					}
#endif

#ifdef RTK_NL80211
					event_indicate_cfg80211(priv, NULL, CFG80211_DISCONNECTED, NULL);
#endif


#ifndef RTK_NL80211//do not auto site survey when disconnected //eric-sync ??
                    STADEBUG("start_clnt_lookup(RESCAN_ROAMING)\n");
                    start_clnt_lookup(priv, RESCAN_ROAMING);
#endif
					#ifdef CONFIG_RTL_WLAN_STATUS
					//panic_printk("%s:%d\n",__FUNCTION__,__LINE__);
					priv->wlan_status_flag=1;
					#endif

				}
			}
			else
			{
				if ((rx_beacon_delta == 0) && (rx_data_delta == 0)) {
					if (JOIN_RES == STATE_Sta_Ibss_Active)
					{
						DEBUG_INFO("Searching IBSS...\n");
						LOG_MSG("Searching IBSS...\n");
						RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
						JOIN_RES_VAL(STATE_Sta_Ibss_Idle);
						start_clnt_lookup(priv, RESCAN);
					}
				}
			}
#endif // CLIENT_MODE
		}

		if (priv->rxBeaconCntArrayIdx++ == decision_period)
			priv->rxBeaconCntArrayIdx = 0;
	}
}


void rtl8192cd_reauth_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
#ifndef SMP_SYNC
	unsigned long flags = 0;
#endif
	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);

#ifdef MULTI_MAC_CLONE
	ACTIVE_ID = 0;
#endif

	REAUTH_COUNT_VAL(REAUTH_COUNT+1);
	if (REAUTH_COUNT > REAUTH_LIMIT)
	{
		DEBUG_WARN("Client Auth time-out!\n");
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
		start_clnt_lookup(priv, DONTRESCAN);
		return;
	}

	if (OPMODE & WIFI_AUTH_SUCCESS)
	{
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
		return;
	}

	AUTH_SEQ_VAL(1);
	OPMODE_VAL(OPMODE & ~(WIFI_AUTH_STATE1));
	OPMODE_VAL(OPMODE | WIFI_AUTH_NULL);

	DEBUG_INFO("auth timeout, sending auth req again\n");
	issue_auth(priv, NULL, 0);

	mod_timer(&priv->reauth_timer, jiffies + REAUTH_TO);

	RESTORE_INT(flags);
	SMP_UNLOCK(flags);
}


void rtl8192cd_reassoc_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
#ifndef SMP_SYNC    
	unsigned long flags = 0;
#endif
	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);

#ifdef MULTI_MAC_CLONE
	ACTIVE_ID = 0;
#endif

	REASSOC_COUNT_VAL(REASSOC_COUNT+1);
	if (REASSOC_COUNT > REASSOC_LIMIT)
	{
		DEBUG_WARN("Client Assoc time-out!\n");
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
		STADEBUG("Client Assoc time-out!; start_clnt_lookup(DONTRESCAN)\n");                
		start_clnt_lookup(priv, DONTRESCAN);
		return;
	}

	if (OPMODE & WIFI_ASOC_STATE)
	{
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
		return;
	}

	DEBUG_INFO("assoc timeout, sending assoc req again\n");
	issue_assocreq(priv);

	MOD_REASSOC_TIMER(REASSOC_TO);

	RESTORE_INT(flags);
	SMP_UNLOCK(flags);
}


void rtl8192cd_idle_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	SMP_LOCK(flags);
	if (!(priv->drv_state & DRV_STATE_OPEN)) {
		SMP_UNLOCK(flags);
		return;
	}

	RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
	LOG_MSG("Searching IBSS...\n");
	start_clnt_lookup(priv, RESCAN);
	SMP_UNLOCK(flags);
}

#ifdef MULTI_MAC_CLONE
void rtl8192cd_mclone_reauth_timer(unsigned long data)
{
	struct mclone_timer_data *timer_data = (struct mclone_timer_data *)data;
	struct rtl8192cd_priv *priv;
	unsigned long active_id = timer_data->active_id;
	unsigned long flags;

	ASSERT(active_id > 0 && active_id <= MAX_MAC_CLONE_NUM);
	priv = timer_data->priv->pshare->mclone_sta[active_id-1].priv;

	if ((NULL == priv) || (!(priv->drv_state & DRV_STATE_OPEN)))
		return;

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);
	if (!((GET_MIB(priv))->dot11OperationEntry.opmode & WIFI_ASOC_STATE)) {
		ACTIVE_ID = active_id;
		MOD_REAUTH_TIMER(REAUTH_TO);
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
		return;
	}

	panic_printk("%s enter %s [%d]\n", priv->dev->name, __FUNCTION__, active_id-1); // for debug

	ACTIVE_ID = active_id;	

	REAUTH_COUNT_VAL(REAUTH_COUNT+1);
	if (REAUTH_COUNT > REAUTH_LIMIT) {
		DEBUG_WARN("Client Auth time-out!\n");
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);

		start_clnt_auth(priv);		
		return;
	}

	if (OPMODE & WIFI_AUTH_SUCCESS) {	
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
		return;
	}

	
	AUTH_SEQ_VAL(1);
	OPMODE_VAL(OPMODE & ~(WIFI_AUTH_STATE1));
	OPMODE_VAL(OPMODE | WIFI_AUTH_NULL);

	DEBUG_INFO("auth timeout, sending auth req again\n");
	
	if ((GET_MIB(priv))->dot11OperationEntry.opmode & WIFI_ASOC_STATE) 
		issue_auth(priv, NULL, 0);

	MOD_REAUTH_TIMER(REAUTH_TO);

	RESTORE_INT(flags);
	SMP_UNLOCK(flags);
}

void rtl8192cd_mclone_reassoc_timer(unsigned long data)
{
	struct mclone_timer_data *timer_data = (struct mclone_timer_data *)data;
	struct rtl8192cd_priv *priv;
	unsigned long active_id = timer_data->active_id;
	unsigned long flags;

	ASSERT(active_id > 0 && active_id <= MAX_MAC_CLONE_NUM);
	priv = timer_data->priv->pshare->mclone_sta[active_id-1].priv;

	if ((NULL == priv) || (!(priv->drv_state & DRV_STATE_OPEN)))
		return;

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);

	if (!((GET_MIB(priv))->dot11OperationEntry.opmode & WIFI_ASOC_STATE)) {
		ACTIVE_ID = active_id;
		MOD_REASSOC_TIMER(REASSOC_TO);
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
		return;
	}

	panic_printk("%s enter %s [%d]\n", priv->dev->name, __FUNCTION__, active_id-1); // for debug

	ACTIVE_ID = active_id;

	REASSOC_COUNT_VAL(REASSOC_COUNT+1);
	if (REASSOC_COUNT > REASSOC_LIMIT) {	
		DEBUG_WARN("Client Assoc time-out!\n");
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);

		start_clnt_auth(priv);		
		return;
	}

	if (OPMODE & WIFI_ASOC_STATE) {	
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
		return;
	}

	DEBUG_INFO("assoc timeout, sending assoc req again\n");

	if ((GET_MIB(priv))->dot11OperationEntry.opmode & WIFI_ASOC_STATE) 
		issue_assocreq(priv);

	MOD_REASSOC_TIMER(REASSOC_TO);

	RESTORE_INT(flags);
	SMP_UNLOCK(flags);
}
#endif /* MULTI_MAC_CLONE */

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
unsigned int OnAssocRsp(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
#ifndef SMP_SYNC
	unsigned long	flags;
#endif
	struct wifi_mib	*pmib;
	struct stat_info *pstat;
	unsigned char	*pframe, *p;
#ifdef P2P_SUPPORT	    
   	unsigned char	*ptr;
#endif
	DOT11_ASSOCIATION_IND	Association_Ind;
	unsigned char	supportRate[32];
	int		supportRateNum;
	UINT16	val;
	int		len;
#ifdef P2P_SUPPORT
	unsigned char ReAssem_p2pie[MAX_REASSEM_P2P_IE];
	int IEfoundtimes=0;
	unsigned char *p2pIEPtr = ReAssem_p2pie ;
	int p2pIElen=0;
#endif
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)
	int if_idx = -1;
#endif



	if (!(OPMODE & WIFI_STATION_STATE))
		return SUCCESS;

	if (memcmp(GET_MY_HWADDR, pfrinfo->da, MACADDRLEN))
		return SUCCESS;

	if (OPMODE & WIFI_SITE_MONITOR)
		return SUCCESS;

	if (OPMODE & WIFI_ASOC_STATE)
		return SUCCESS;

	pmib = GET_MIB(priv);
	pframe = get_pframe(pfrinfo);
	DEBUG_INFO("got assoc response  (OPMODE %x seq %d)\n", OPMODE, GetSequence(pframe));

	// checking status
	val = cpu_to_le16(*(unsigned short*)((unsigned long)pframe + WLAN_HDR_A3_LEN + 2));

	if (val) {
		DEBUG_ERR("assoc reject, status: %d\n", val);
		goto assoc_rejected;
	}

	AID_VAL(cpu_to_le16(*(unsigned short*)((unsigned long)pframe + WLAN_HDR_A3_LEN + 4)) & 0x3fff);

	pstat = get_stainfo(priv, pfrinfo->sa);
	if (pstat == NULL) {
		pstat = alloc_stainfo(priv, pfrinfo->sa, -1);
		if (pstat == NULL) {
			DEBUG_ERR("Exceed the upper limit of supported clients...\n");
			goto assoc_rejected;
		}
	}
	else {
		release_stainfo(priv, pstat);
		cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
		init_stainfo(priv, pstat);
	}
	pstat->tpcache_mgt = GetTupleCache(pframe);

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		update_remapAid(priv,pstat); 
		if (pstat && pstat->sta_in_firmware == 1) {
			RTL8188E_MACID_NOLINK(priv, 0, REMAP_AID(pstat));
			RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
            pstat->txpause_flag = 0;            
		}
	}
#endif

#ifdef CONFIG_WLAN_HAL
            if(IS_HAL_CHIP(priv))
            {
                if(pstat && (REMAP_AID(pstat) < 128))
                {
                    DEBUG_WARN("%s %d OnAssocRsp, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
                    GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstat));                
                    pstat->txpause_flag = 0;
                }
                else
                {
                    DEBUG_WARN(" MACID sleep only support 128 STA \n");
                }
            }
#endif


	// Realtek proprietary IE
	pstat->is_realtek_sta = FALSE;
	pstat->IOTPeer = HT_IOT_PEER_UNKNOWN;	
	p = pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_; len = 0;
	for (;;) {
		p = get_ie(p, _RSN_IE_1_, &len,
		pfrinfo->pktlen - (p - pframe));
		if (p != NULL) {
			if (!memcmp(p+2, Realtek_OUI, 3) && *(p+2+3) == 2) { /*found realtek out and type == 2*/
				pstat->is_realtek_sta = TRUE;
				pstat->IOTPeer = HT_IOT_PEER_REALTEK;

				if(*(p+2+3+2) & RTK_CAP_IE_WLAN_8192SE)
					pstat->IOTPeer = HT_IOT_PEER_REALTEK_92SE;

				if(*(p+2+3+2) & RTK_CAP_IE_WLAN_88C92C)						
					pstat->IOTPeer = HT_IOT_PEER_REALTEK_81XX;				
				
				if (*(p+2+3+3) & ( RTK_CAP_IE_8812_BCUT | RTK_CAP_IE_8812_CCUT))
					pstat->IOTPeer = HT_IOT_PEER_REALTEK_8812;						
				break;
			}
		}
		else
			break;
		p = p + len + 2;
	}

	// identify if this is Broadcom sta
	p = pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_; len = 0;

	for (;;)
	{
		unsigned char Broadcom_OUI1[]={0x00, 0x05, 0xb5};
		unsigned char Broadcom_OUI2[]={0x00, 0x0a, 0xf7};
		unsigned char Broadcom_OUI3[]={0x00, 0x10, 0x18};

		p = get_ie(p, _RSN_IE_1_, &len,
				pfrinfo->pktlen - (p - pframe));
		if (p != NULL) {
			if (!memcmp(p+2, Broadcom_OUI1, 3) ||
					!memcmp(p+2, Broadcom_OUI2, 3) ||
					!memcmp(p+2, Broadcom_OUI3, 3)) {

				pstat->IOTPeer = HT_IOT_PEER_BROADCOM;

				break;
			}
		}
		else
			break;

		p = p + len + 2;
	}

	// identify if this is ralink sta
	p = pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_; len = 0;

	for (;;)
	{
		unsigned char Ralink_OUI1[]={0x00, 0x0c, 0x43};

		p = get_ie(p, _RSN_IE_1_, &len,
			pfrinfo->pktlen - (p - pframe));
		if (p != NULL) {
			if (!memcmp(p+2, Ralink_OUI1, 3)) {

				pstat->IOTPeer= HT_IOT_PEER_RALINK;

				break;
			}	
		}
		else
			break;
		p = p + len + 2;
	}

	if(!pstat->is_realtek_sta && pstat->IOTPeer != HT_IOT_PEER_BROADCOM && pstat->IOTPeer != HT_IOT_PEER_RALINK) 

	{
		unsigned int z = 0;
		for (z = 0; z < INTEL_OUI_NUM; z++) {
			if ((pstat->hwaddr[0] == INTEL_OUI[z][0]) &&
				(pstat->hwaddr[1] == INTEL_OUI[z][1]) &&
				(pstat->hwaddr[2] == INTEL_OUI[z][2])) {

				pstat->IOTPeer= HT_IOT_PEER_INTEL;

				pstat->no_rts = 1;
				break;
			}
		}

	}

#ifdef A4_STA
    if(priv->pshare->rf_ft_var.a4_enable == 2) {
        if(0 < parse_a4_ie(priv, pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, 
            pfrinfo->pktlen - (WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_))) {
            pstat->state |= WIFI_A4_STA;
        }
    }
    else if(priv->pshare->rf_ft_var.a4_enable == 1) {
        pstat->state |= WIFI_A4_STA;
    }
#endif    

	// get rates
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _SUPPORTEDRATES_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);
	if ((p == NULL) || (len > 32)){
		free_stainfo(priv, pstat);
		return FAIL;
	}
	memcpy(supportRate, p+2, len);
	supportRateNum = len;
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);
	if ((p !=  NULL) && (len <= 8)) {
		memcpy(supportRate+supportRateNum, p+2, len);
		supportRateNum += len;
	}

	// other capabilities
	memcpy(&val, (pframe + WLAN_HDR_A3_LEN), 2);
	val = le16_to_cpu(val);
	if (val & BIT(5)) {
		// set preamble according to AP
		RTL_W8(RRSR+2, RTL_R8(RRSR+2) | BIT(7));
		pstat->useShortPreamble = 1;
	}
	else {
		// set preamble according to AP
		RTL_W8(RRSR+2, RTL_R8(RRSR+2) & ~BIT(7));
		pstat->useShortPreamble = 0;
	}

	if ((priv->pshare->curr_band == BAND_2G) && (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G))
	{
		if (val & BIT(10)) {
			priv->pmib->dot11ErpInfo.shortSlot = 1;
			set_slot_time(priv, priv->pmib->dot11ErpInfo.shortSlot);
		}
		else {
			priv->pmib->dot11ErpInfo.shortSlot = 0;
			set_slot_time(priv, priv->pmib->dot11ErpInfo.shortSlot);
		}

		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _ERPINFO_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

		if (p && (*(p+2) & BIT(1)))	// use Protection
			priv->pmib->dot11ErpInfo.protection = 1;
		else
			priv->pmib->dot11ErpInfo.protection = 0;

		if (p && (*(p+2) & BIT(2)))	// use long preamble
			priv->pmib->dot11ErpInfo.longPreambleStaNum = 1;
		else
			priv->pmib->dot11ErpInfo.longPreambleStaNum = 0;
	}

	// set associated and add to association list
	pstat->state |= (WIFI_ASOC_STATE | WIFI_AUTH_SUCCESS);

#ifdef WIFI_WMM  //  WMM STA
	if (QOS_ENABLE) {
		int i;
		p = pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_;
		for (;;) {
			p = get_ie(p, _RSN_IE_1_, &len,
				pfrinfo->pktlen - (p - pframe));
			if (p != NULL) {
				if (!memcmp(p+2, WMM_PARA_IE, 6)) {
					pstat->QosEnabled = 1;
//capture the EDCA para
					p += 10;  // start of EDCA parameters
					for (i = 0; i <4; i++) {
						process_WMM_para_ie(priv, p);  //get the info
						p += 4;
					}
					DEBUG_INFO("BE: ACM %d, AIFSN %d, ECWmin %d, ECWmax %d, TXOP %d\n",
						GET_STA_AC_BE_PARA.ACM, GET_STA_AC_BE_PARA.AIFSN,
						GET_STA_AC_BE_PARA.ECWmin, GET_STA_AC_BE_PARA.ECWmax,
						GET_STA_AC_BE_PARA.TXOPlimit);
					DEBUG_INFO("VO: ACM %d, AIFSN %d, ECWmin %d, ECWmax %d, TXOP %d\n",
						GET_STA_AC_VO_PARA.ACM, GET_STA_AC_VO_PARA.AIFSN,
						GET_STA_AC_VO_PARA.ECWmin, GET_STA_AC_VO_PARA.ECWmax,
						GET_STA_AC_VO_PARA.TXOPlimit);
					DEBUG_INFO("VI: ACM %d, AIFSN %d, ECWmin %d, ECWmax %d, TXOP %d\n",
						GET_STA_AC_VI_PARA.ACM, GET_STA_AC_VI_PARA.AIFSN,
						GET_STA_AC_VI_PARA.ECWmin, GET_STA_AC_VI_PARA.ECWmax,
						GET_STA_AC_VI_PARA.TXOPlimit);
					DEBUG_INFO("BK: ACM %d, AIFSN %d, ECWmin %d, ECWmax %d, TXOP %d\n",
						GET_STA_AC_BK_PARA.ACM, GET_STA_AC_BK_PARA.AIFSN,
						GET_STA_AC_BK_PARA.ECWmin, GET_STA_AC_BK_PARA.ECWmax,
						GET_STA_AC_BK_PARA.TXOPlimit);

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
					if (IS_ROOT_INTERFACE(priv))
#endif
					{
						SAVE_INT_AND_CLI(flags);
						sta_config_EDCA_para(priv);
						RESTORE_INT(flags);
					}
					break;
				}
			}
			else {
				pstat->QosEnabled = 0;
				break;
			}
			p = p + len + 2;
		}
	} 
	else {
		pstat->QosEnabled = 0;
	}
#endif


#ifdef P2P_SUPPORT				/*cfg p2p cfg p2p */
    if((rtk_p2p_is_enabled(priv)) && (rtk_p2p_chk_role(priv,P2P_CLIENT)||(rtk_p2p_chk_role(priv,P2P_PRE_CLIENT))))
    {


	ptr = pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_ ;
	/*support ReAssemble*/		
	for (;;)
	{
		/*get P2P_IE*/ 
		ptr = get_ie(ptr, _P2P_IE_, &len,	
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_ - len);			
	
		if (ptr) {
			if (!memcmp(ptr+2, WFA_OUI_PLUS_TYPE, 4)) {
					memcpy(p2pIEPtr , ptr+6 ,len - 4);
					p2pIEPtr += (len - 4);
					IEfoundtimes ++;
			}
		}
		else{
			break;
		}
		ptr = ptr + len + 2;
		
	}

	if(IEfoundtimes){
		if(IEfoundtimes>1){
			P2P_DEBUG("ReAssembly p2p IE\n");
		}
		p2pIElen = (int)(((unsigned long)p2pIEPtr)-((unsigned long)ReAssem_p2pie));		
		
		if(p2pIElen > MAX_REASSEM_P2P_IE){
			P2P_DEBUG("\n\n	reassemble P2P IE exceed MAX_REASSEM_P2P_IE , chk!!!\n\n");
		}else{
			P2P_on_assoc_rsp(priv,pfrinfo->sa);
			pstat->is_p2p_client = 1;
		}
	}
}
#endif


	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && priv->ht_cap_len) {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _HT_CAP_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);
		if ((p !=  NULL) && (len <= sizeof(struct ht_cap_elmt))) {
			pstat->ht_cap_len = len;
			memcpy((unsigned char *)&pstat->ht_cap_buf, p+2, len);
		} else {
			pstat->ht_cap_len = 0;
			memset((unsigned char *)&pstat->ht_cap_buf, 0, sizeof(struct ht_cap_elmt));
		}

		p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _HT_IE_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);
		if ((p !=  NULL) && (len <= sizeof(struct ht_info_elmt))) {
			pstat->ht_ie_len = len;
			memcpy((unsigned char *)&pstat->ht_ie_buf, p+2, len);

			priv->ht_protection = 0;
			if (!priv->pmib->dot11StationConfigEntry.protectionDisabled && pstat->ht_ie_len) {
				unsigned int prot_mode =  (cpu_to_le16(pstat->ht_ie_buf.info1) & 0x03);
				if (prot_mode == _HTIE_OP_MODE1_ || prot_mode == _HTIE_OP_MODE3_)
					priv->ht_protection = 1;
			}
		} else {
			pstat->ht_ie_len = 0;
		}

		if (pstat->ht_cap_len) {
			if (cpu_to_le16(pstat->ht_cap_buf.ht_cap_info) & _HTCAP_AMSDU_LEN_8K_) {
				pstat->is_8k_amsdu = 1;
				pstat->amsdu_level = 7935 - sizeof(struct wlan_hdr);
			} else {
				pstat->is_8k_amsdu = 0;
				pstat->amsdu_level = 3839 - sizeof(struct wlan_hdr);
			}

#ifdef WIFI_11N_2040_COEXIST
			priv->coexist_connection = 0;

			if (priv->pmib->dot11nConfigEntry.dot11nCoexist &&
				(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G)) {
				p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _EXTENDED_CAP_IE_, &len,
					pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);

				if (p != NULL) {
					if (*(p+2) & _2040_COEXIST_SUPPORT_)
						priv->coexist_connection = 1;
				}
			}
#endif
		}
	}


//8812_client add pstat vht ie
#ifdef RTK_AC_SUPPORT
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
	{
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, EID_VHTCapability, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);

		if ((p !=  NULL) && (len <= sizeof(struct vht_cap_elmt))) {
			pstat->vht_cap_len = len;
			memcpy((unsigned char *)&pstat->vht_cap_buf, p+2, len);
			//printk("receive vht_cap len = %d \n", len);		
		}

		p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, EID_VHTOperation, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);

		if ((p !=  NULL) && (len <= sizeof(struct vht_oper_elmt))) {
			pstat->vht_oper_len = len;
			memcpy((unsigned char *)&pstat->vht_oper_buf, p+2, len);
			//printk("receive vht_oper len = %d \n", len);
		}
	}
#endif

#ifdef WIFI_WMM  //  WMM STA
	if (QOS_ENABLE) {
		if ((pstat->QosEnabled == 0) && pstat->ht_cap_len) {
			DEBUG_INFO("AP supports HT but doesn't support WMM, use default WMM value\n");
			pstat->QosEnabled = 1;
			default_WMM_para(priv);
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (IS_ROOT_INTERFACE(priv))
#endif
			{
				SAVE_INT_AND_CLI(flags);
				sta_config_EDCA_para(priv);
				RESTORE_INT(flags);
			}
		}
	}
#endif

//Client mode IOT issue, Button 2009.07.17
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _NO_PRIVACY_)
		#if defined(CONFIG_RTL_WAPI_SUPPORT)
			&&(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WAPI_SMS4_)
		#endif
			)
		{
			pstat->is_legacy_encrpt = 0;
			if ((pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)  ||
				(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_ ))
				pstat->is_legacy_encrpt = 2;
			else if (pmib->dot11RsnIE.rsnielen) {
				if (pmib->dot11RsnIE.rsnie[0] == _RSN_IE_1_) {
					if (is_support_wpa_aes(priv, pmib->dot11RsnIE.rsnie, pmib->dot11RsnIE.rsnielen) != 1)
						pstat->is_legacy_encrpt = 1;
				}
				else {
					if (is_support_wpa2_aes(priv, pmib->dot11RsnIE.rsnie, pmib->dot11RsnIE.rsnielen) != 1)
						pstat->is_legacy_encrpt = 1;
				}
			}
		}

        priv->pshare->AP_BW = -1;
#ifdef UNIVERSAL_REPEATER
        if (IS_VXD_INTERFACE(priv)) {
                if(GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11nUse40M) {
                        if((pstat->ht_cap_len > 0) && (pstat->ht_ie_len > 0) &&
                        (pstat->ht_ie_buf.info0 & _HTIE_STA_CH_WDTH_) &&
                        (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))) {
                                priv->pshare->is_40m_bw = 1;
                        }
                }
        }
#endif

        if (pstat->ht_cap_len) {
                if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))
                        pstat->tx_bw = HT_CHANNEL_WIDTH_20_40;
                else
                        pstat->tx_bw = HT_CHANNEL_WIDTH_20;
        }

	get_matched_rate(priv, supportRate, &supportRateNum, 1);
	update_support_rate(pstat, supportRate, supportRateNum);
	assign_tx_rate(priv, pstat, pfrinfo);
	assign_aggre_mthod(priv, pstat);
	assign_aggre_size(priv, pstat);

#ifdef INCLUDE_WPA_PSK	
	if (IEEE8021X_FUN && priv->pmib->dot1180211AuthEntry.dot11EnablePSK) {		
		if (psk_indicate_evt(priv, DOT11_EVENT_ASSOCIATION_IND, GetAddr2Ptr(pframe), NULL, 0) < 0){
            STADEBUG("assoc_rejected\n");
			goto assoc_rejected;	
          }
	}
#endif

	SAVE_INT_AND_CLI(flags);

	pstat->expire_to = priv->expire_to;
	asoc_list_add(priv, pstat);
	cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);

	if (!IEEE8021X_FUN &&
			!(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
			 priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_)) {
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
		LOG_MSG_NOTICE("Connected to AP;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
				*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
				*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)
		if((if_idx = get_wlan_if_type(priv))>=0){
			set_if_status(if_idx,IF_UP);
			set_if_change_status(if_idx,IF_STATUS_CHANGE);
		}
#endif
#else
	LOG_MSG("Associate to AP successfully - %02X:%02X:%02X:%02X:%02X:%02X\n",
		*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
		*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)
		if((if_idx = get_wlan_if_type(priv))>=0){
			set_if_status(if_idx,IF_UP);
			set_if_change_status(if_idx,IF_STATUS_CHANGE);
		}
#endif
#endif
	}

	// now we have successfully join the give bss...
	if (PENDING_REAUTH_TIMER)
		DELETE_REAUTH_TIMER;
	if (PENDING_REASSOC_TIMER)
		DELETE_REASSOC_TIMER;

	// clear cached Dev	
#if defined(BR_SHORTCUT) && defined(CLIENT_MODE)
	{
		extern unsigned char cached_sta_mac[6];
		extern struct net_device *cached_sta_dev;
		memset(cached_sta_mac, 0, MACADDRLEN);
		cached_sta_dev = NULL;
	}
#endif
	
	RESTORE_INT(flags);

	OPMODE_VAL(OPMODE | WIFI_ASOC_STATE);
	update_bss(&priv->pmib->dot11StationConfigEntry, &priv->pmib->dot11Bss);
	priv->pmib->dot11RFEntry.dot11channel = priv->pmib->dot11Bss.channel;
	join_bss(priv);
	
	JOIN_RES_VAL(STATE_Sta_Bss);
	JOIN_REQ_ONGOING_VAL(0);

#ifndef WITHOUT_ENQUEUE
	if (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm
#ifdef WIFI_SIMPLE_CONFIG
		&& !(priv->pmib->wscEntry.wsc_enable)
#endif
		)
	{
		memcpy((void *)Association_Ind.MACAddr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
		Association_Ind.EventId = DOT11_EVENT_ASSOCIATION_IND;
		Association_Ind.IsMoreEvent = 0;
		Association_Ind.RSNIELen = 0;
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Association_Ind,
					sizeof(DOT11_ASSOCIATION_IND));

#ifdef RTK_NL80211
		//event_indicate_cfg80211(priv, GetAddr2Ptr(pframe), CFG80211_NEW_STA, NULL);
#else //RTK_NL80211
#ifdef WIFI_HAPD
		event_indicate_hapd(priv, GetAddr2Ptr(pframe), HAPD_REGISTERED, NULL);
#ifdef HAPD_DRV_PSK_WPS
		event_indicate(priv, GetAddr2Ptr(pframe), 1);
#endif
#else
		event_indicate(priv, GetAddr2Ptr(pframe), 1);
#endif
#endif //RTK_NL80211
	}
#endif // WITHOUT_ENQUEUE

#ifdef WIFI_SIMPLE_CONFIG
	if (priv->pmib->wscEntry.wsc_enable) {
		DOT11_WSC_ASSOC_IND wsc_Association_Ind;

		memset(&wsc_Association_Ind, 0, sizeof(DOT11_WSC_ASSOC_IND));
		wsc_Association_Ind.EventId = DOT11_EVENT_WSC_ASSOC_REQ_IE_IND;
		memcpy((void *)wsc_Association_Ind.MACAddr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
#ifdef INCLUDE_WPS
		wps_NonQueue_indicate_evt(priv ,(UINT8 *)&wsc_Association_Ind,
			sizeof(DOT11_WSC_ASSOC_IND));		
#else
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&wsc_Association_Ind,
			sizeof(DOT11_WSC_ASSOC_IND));

#ifdef RTK_NL80211
		//event_indicate_cfg80211(priv, GetAddr2Ptr(pframe), CFG80211_NEW_STA, NULL);

#else //RTK_NL80211

#ifdef WIFI_HAPD
		event_indicate_hapd(priv, GetAddr2Ptr(pframe), HAPD_REGISTERED, NULL);
#ifdef HAPD_DRV_PSK_WPS
		event_indicate(priv, GetAddr2Ptr(pframe), 1);
#endif
#else
		event_indicate(priv, GetAddr2Ptr(pframe), 1);
#endif
#endif //RTK_NL80211

#endif
		pstat->state |= WIFI_WPS_JOIN;
	}
#endif

#if 0
	// Get operating bands
	//    |  B |  G | BG  <= AP
	//  B |  B |  x |  B
	//  G |  x |  G |  G
	// BG |  B |  G | BG
	if ((priv->pshare->curr_band == WIRELESS_11A) ||
		(priv->pshare->curr_band == WIRELESS_11B))
		priv->oper_band = priv->pshare->curr_band;
	else {			// curr_band == WIRELESS_11G
		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B) ||
			!is_CCK_rate(pstat->bssrateset[0] & 0x7f))
			priv->oper_band = WIRELESS_11G;
		else if (is_CCK_rate(pstat->bssrateset[pstat->bssratelen-1] & 0x7f))
			priv->oper_band = WIRELESS_11B;
		else
			priv->oper_band = WIRELESS_11B | WIRELESS_11G;
	}
#endif

	DEBUG_INFO("assoc successful!\n");
	STADEBUG("Assoc successful!\n");


#ifdef DFS
#ifdef UNIVERSAL_REPEATER
	//if(under_apmode_repeater(priv))
	{
		RTL_W8(TXPAUSE, 0x0);
	}
#endif
#endif

	if ((OPMODE & WIFI_STATION_STATE)
#ifdef UNIVERSAL_REPEATER
		&& IS_ROOT_INTERFACE(priv)
#endif
	)
		priv->up_flag = 1;

//#ifdef BR_SHORTCUT
#if 0
	clear_shortcut_cache();
#if defined(CONFIG_RTL_FASTBRIDGE)
	rtl_fb_flush();
#endif
#endif


#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		if ((pstat->ht_cap_len > 0) && (pstat->ht_ie_len > 0) &&				
			(pstat->ht_ie_buf.info0 & _HTIE_STA_CH_WDTH_) &&					
			(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))) {
			priv->pshare->is_40m_bw = 1;
			if ((pstat->ht_ie_buf.info0 & _HTIE_2NDCH_OFFSET_BL_) == _HTIE_2NDCH_OFFSET_BL_)
				priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
			else
				priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;

			if (priv->pshare->is_40m_bw == 1) {
				if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_ABOVE) {
					int i, channel = priv->pmib->dot11Bss.channel + 4;
					for (i=0; i<priv->available_chnl_num; i++) {
						if (channel == priv->available_chnl[i])
							break;
					}
					if (i == priv->available_chnl_num) {
						priv->pshare->is_40m_bw = 0;
						priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
						DEBUG_INFO("AP is 40M (ch%d-ch%d) but not fit region domain, sw back to 20M\n", priv->pmib->dot11Bss.channel, channel);
					}
				}
			}

#ifdef CONFIG_RTL_92D_SUPPORT
			if ((GET_CHIP_VER(priv) == VERSION_8192D)&&(priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY)) {
				clnt_ss_check_band(priv, priv->pmib->dot11Bss.channel); 
			}
#endif			

#ifdef RTK_AC_SUPPORT
//8812_client , check ap support 80m ??
			if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)) {
				if (pstat->vht_cap_len && (pstat->vht_oper_buf.vht_oper_info[0] == 1)) {
					pstat->tx_bw = HT_CHANNEL_WIDTH_80;
					priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_80; 
				}
			}

			//printk("vht_oper_info[0] = 0x%x\n", pstat->vht_oper_buf.vht_oper_info[0]);
			//printk("vht_cap_len=%d, is_40m_bw=%d\n", pstat->vht_cap_len, priv->pshare->is_40m_bw);
#endif
			priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
			SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
			SwChnl(priv, priv->pmib->dot11Bss.channel, priv->pshare->offset_2nd_chan);
			
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8192D)
				PHY_IQCalibrate(priv);

#ifdef DPK_92D		
			if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G && priv->pshare->rf_ft_var.dpk_on)
				PHY_DPCalibrate(priv);
#endif
#endif

			DEBUG_INFO("%s: set chan=%d, 40M=%d, offset_2nd_chan=%d\n",
				__FUNCTION__,
				priv->pmib->dot11Bss.channel,
				priv->pshare->is_40m_bw,  priv->pshare->offset_2nd_chan);
		}
		else {
			priv->pshare->is_40m_bw = 0;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;

//_TXPWR_REDEFINE
#ifdef CONFIG_RTL_92D_SUPPORT
			if ((GET_CHIP_VER(priv) == VERSION_8192D)&&(priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY)) {
				clnt_ss_check_band(priv, priv->pmib->dot11Bss.channel); 
			}
#endif				
			if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_5)
				priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_5;
			else if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_10)
				priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_10;
			else
				priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
			SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
			SwChnl(priv, priv->pmib->dot11Bss.channel, priv->pshare->offset_2nd_chan);
			
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8192D)
				PHY_IQCalibrate(priv);
#endif

		}
	}

//8812_client , check ap support 80m ??
#ifdef RTK_AC_SUPPORT
	if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)) {
		if (pstat->vht_cap_len && (pstat->vht_oper_buf.vht_oper_info[0] == 1)) {
			pstat->tx_bw = HT_CHANNEL_WIDTH_80;
			priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_80; 
		}
	}
#endif
#ifdef BEAMFORMING_SUPPORT
		
		//panic_printk("%s, %x\n", __FUNCTION__, cpu_to_le32(pstat->vht_cap_buf.vht_cap_info));
			if(((GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8814A))
				&& (priv->pmib->dot11RFEntry.txbf == 1)&&
				((pstat->ht_cap_len && (pstat->ht_cap_buf.txbf_cap)) 
#ifdef RTK_AC_SUPPORT
				|| (pstat->vht_cap_len && (cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & (BIT(SU_BFEE_S)|BIT(SU_BFER_S))))
//				BeamformingControl(priv, pstat->hwaddr, pstat->aid, 0, pstat->tx_bw);		
#endif
			)) {
					Beamforming_Enter(priv,pstat);
	}
#endif

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv)) {
#ifdef RTK_BR_EXT
		if (!(priv->pmib->ethBrExtInfo.macclone_enable && !priv->macclone_completed))
#endif
		{
			if (
#ifdef __ECOS
				GET_VXD_PRIV(priv) &&
#endif
				netif_running(GET_VXD_PRIV(priv)->dev))
				enable_vxd_ap(GET_VXD_PRIV(priv));
		}
	}
#endif

#ifndef USE_WEP_DEFAULT_KEY
	set_keymapping_wep(priv, pstat);
#endif

#if defined(CONFIG_PCI_HCI)
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
	} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		UpdateHalRAMask8812(priv, pstat, 3);
	} else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
		add_RATid(priv, pstat);
#endif
	} else
#endif
	{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
		add_update_RATid(priv, pstat);
#endif
	}
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	update_STA_RATid(priv, pstat);
#endif

#if defined(WIFI_11N_2040_COEXIST_EXT)
	update_40m_staMap(priv, pstat, 0);
	checkBandwidth(priv);
#endif

#ifdef WIFI_WPAS
	//printk("_Eric WPAS_REGISTERED at %s %d\n", __FUNCTION__, __LINE__);
	event_indicate_wpas(priv, GetAddr2Ptr(pframe), WPAS_REGISTERED, NULL);
#endif
#ifdef RTK_NL80211
#if 1 //wrt_clnt
	{
		unsigned char *assocrsp_ie = pfrinfo->pskb->data + (WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_);
		int assocrsp_ie_len = pfrinfo->pktlen - (WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_);

		if(assocrsp_ie_len > 0)
		{
			//printk("AssocRsp Len = %d\n", assocrsp_ie_len);
			if(assocrsp_ie_len > MAX_ASSOC_RSP_LEN)
			{
				printk("AssocRsp Len too LONG !!\n");
				memcpy(priv->rtk->clnt_info.assoc_rsp, assocrsp_ie, MAX_ASSOC_RSP_LEN);
				priv->rtk->clnt_info.assoc_rsp_len = MAX_ASSOC_RSP_LEN;
			}
			else
			{
				memcpy(priv->rtk->clnt_info.assoc_rsp, assocrsp_ie, assocrsp_ie_len);
				priv->rtk->clnt_info.assoc_rsp_len = assocrsp_ie_len;
			}			

		}
		else
			printk(" !! Error AssocRsp Len = %d\n", assocrsp_ie_len);
	}
#endif
	event_indicate_cfg80211(priv, GetAddr2Ptr(pframe), CFG80211_CONNECT_RESULT, NULL);
#endif


#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (priv->pmib->wapiInfo.wapiType==wapiTypeCert)
	{
		wapiAssert(pstat->wapiInfo->wapiState==ST_WAPI_AE_IDLE);
	}
	else if (priv->pmib->wapiInfo.wapiType==wapiTypePSK)
	{
		wapiAssert(pstat->wapiInfo->wapiState==ST_WAPI_AE_IDLE);
		wapiSetBK(pstat);
	}
#endif


#ifdef CLIENT_MODE

	if ((OPMODE & WIFI_STATION_STATE) && pstat->IOTPeer == HT_IOT_PEER_BROADCOM) 
	{
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) { 
			RTL_W8(0x51a, 0x0f);
		}
	}
#endif


	return SUCCESS;

assoc_rejected:

	JOIN_RES_VAL(STATE_Sta_No_Bss);
	JOIN_REQ_ONGOING_VAL(0);

	if (PENDING_REASSOC_TIMER)
		DELETE_REASSOC_TIMER;

	STADEBUG("assoc_rejected ; start_clnt_lookup(DONTRESCAN)\n");
	start_clnt_lookup(priv, DONTRESCAN);

#ifdef UNIVERSAL_REPEATER
	disable_vxd_ap(GET_VXD_PRIV(priv));
#endif

	return FAIL;
}


/**
 *	@brief	STA in Infra-structure mode Beacon process.
 */
unsigned int OnBeaconClnt_Bss(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char *bssid;
	struct stat_info *pstat;
	unsigned char *p, *pframe;
	int len;
	unsigned short val16;
#ifdef WIFI_11N_2040_COEXIST
	unsigned int channel=0;
#endif
#ifdef WIFI_WMM
	unsigned int i, vo_txop=0, vi_txop=0, be_txop=0, bk_txop=0;
#ifndef SMP_SYNC
	unsigned long flags;
#endif
#endif
#ifdef P2P_SUPPORT
	unsigned char *ptr;
	static	unsigned char ReAssem_p2pie[MAX_REASSEM_P2P_IE];
	int IEfoundtimes=0;
	unsigned char *p2pIEPtr = ReAssem_p2pie ;
	int p2pIElen=0;
#endif
#ifdef MULTI_MAC_CLONE
	int firstSta=0;
#endif
	int htcap_chwd_cur = 0;

	pframe = get_pframe(pfrinfo);
	bssid = GetAddr3Ptr(pframe);

	memcpy(&val16, (pframe + WLAN_HDR_A3_LEN + 8 + 2), 2);
	val16 = le16_to_cpu(val16);
	if (!(val16 & BIT(0)) || (val16 & BIT(1)))
		return SUCCESS;

#ifdef WIFI_11N_2040_COEXIST
	if ((!IS_BSSID(priv, bssid)) && 
		priv->pmib->dot11nConfigEntry.dot11nCoexist && 
		priv->coexist_connection &&
		(priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N|WIRELESS_11G))) {
		/*
		 *	check if there is any bg AP around
		 */
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_,
			&len, pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p == NULL) {
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _DSSET_IE_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if (p != NULL)
				channel = *(p+2);
			if (channel && (channel <= 14)) {
#if 0
//#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
				if(!priv->bg_ap_timeout) {
					priv->bg_ap_timeout = 180;
					priv->bg_ap_timeout_ch[channel-1] = 180;
					update_RAMask_to_FW(priv, 1);
				}
#endif				
				priv->bg_ap_timeout = 180;
				priv->bg_ap_timeout_ch[channel-1] = 180;
				channel = 0;
			}
		} else {
			/*
			 *	check if there is any 40M intolerant field set by other 11n AP
			 */
			struct ht_cap_elmt *ht_cap=(struct ht_cap_elmt *)(p+2);
			if (cpu_to_le16(ht_cap->ht_cap_info) & _HTCAP_40M_INTOLERANT_)
				priv->intolerant_timeout = 180;
		}
	}
#endif

	if (!IS_BSSID(priv, bssid))
		return SUCCESS;

	// this is our AP
	pstat = get_stainfo(priv, bssid);
	if (pstat == NULL) {
		DEBUG_ERR("Can't find our AP\n");
		return FAIL;
	}

#ifdef UNIVERSAL_REPEATER
	if(priv->pmib->dot11StationConfigEntry.bcastSSID_inherit && IS_VXD_INTERFACE(priv))
		isHiddenAP(pframe,pfrinfo,pstat,priv);
#endif

#ifdef RTK_5G_SUPPORT
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
		priv->rxBeaconNumInPeriod++;
	} else
#endif
	{
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _DSSET_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p != NULL) {
			if (priv->pmib->dot11Bss.channel == *(p+2)) {
				p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SSID_IE_, &len,				
					pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);			
				if (!(p && (len > 0) && *(p+2) &&				
					memcmp(priv->pmib->dot11Bss.ssid, p+2, priv->pmib->dot11Bss.ssidlen))) 			
				priv->rxBeaconNumInPeriod++;
			}
		}
	}

	if (priv->ps_state) {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _TIM_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p != NULL) {
			if (isOurFrameBuffred(p, _AID) == TRUE) {
#if defined(WIFI_WMM) && defined(WMM_APSD)
				if (QOS_ENABLE && APSD_ENABLE && priv->uapsd_assoc) {
					if (!(priv->pmib->dot11QosEntry.UAPSD_AC_BE && 
						priv->pmib->dot11QosEntry.UAPSD_AC_BK && 
						priv->pmib->dot11QosEntry.UAPSD_AC_VI && 
						priv->pmib->dot11QosEntry.UAPSD_AC_VO))
						issue_PsPoll(priv);
				} else
#endif
				{
					issue_PsPoll(priv);
				}
			}

#ifdef MULTI_MAC_CLONE
			{
				int k;
				if (MCLONE_NUM > 0) {
					for (k=0; k<MCLONE_NUM; k++) {
						ACTIVE_ID = k+1;
#if defined(WIFI_WMM) && defined(WMM_APSD)
						if (QOS_ENABLE && APSD_ENABLE && priv->uapsd_assoc) {
							if (!(priv->pmib->dot11QosEntry.UAPSD_AC_BE && 
								priv->pmib->dot11QosEntry.UAPSD_AC_BK && 
								priv->pmib->dot11QosEntry.UAPSD_AC_VI && 
								priv->pmib->dot11QosEntry.UAPSD_AC_VO))
								issue_PsPoll(priv);
							} else
#endif
								issue_PsPoll(priv);
						
					}
					ACTIVE_ID = 0;
				}
			}
#endif	
		}
	}

	if (val16 & BIT(5))
		pstat->useShortPreamble = 1;
	else
		pstat->useShortPreamble = 0;

	if ((priv->pshare->curr_band == BAND_2G) && (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G))
	{
		if (val16 & BIT(10)) {
			if (priv->pmib->dot11ErpInfo.shortSlot == 0) {
				priv->pmib->dot11ErpInfo.shortSlot = 1;
				set_slot_time(priv, priv->pmib->dot11ErpInfo.shortSlot);
			}
		}
		else {
			if (priv->pmib->dot11ErpInfo.shortSlot == 1) {
				priv->pmib->dot11ErpInfo.shortSlot = 0;
				set_slot_time(priv, priv->pmib->dot11ErpInfo.shortSlot);
			}
		}

		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _ERPINFO_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

		if (p && (*(p+2) & BIT(1)))	// use Protection
			priv->pmib->dot11ErpInfo.protection = 1;
		else
			priv->pmib->dot11ErpInfo.protection = 0;

		if (p && (*(p+2) & BIT(2)))	// use long preamble
			priv->pmib->dot11ErpInfo.longPreambleStaNum = 1;
		else
			priv->pmib->dot11ErpInfo.longPreambleStaNum = 0;
	}

	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if (p !=  NULL) {
		struct ht_cap_elmt *ht_cap = (struct ht_cap_elmt *)(p+2);
		if (OPMODE & WIFI_ASOC_STATE) { 		
			if ((ht_cap->ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_)))					
				htcap_chwd_cur = 1;
		}
	}
	/*
	 *	Update HT Operation IE of AP for protection and coexist infomation
	 */

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat->ht_cap_len) {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_IE_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p !=  NULL) {
			int htcap_chwd_offset = 0;
			if (OPMODE & WIFI_ASOC_STATE) {
				if ((p[3]& _HTIE_2NDCH_OFFSET_BL_) == _HTIE_2NDCH_OFFSET_BL_)
					htcap_chwd_offset = HT_2NDCH_OFFSET_BELOW;
				else if ((p[3] & _HTIE_2NDCH_OFFSET_BL_) == _HTIE_2NDCH_OFFSET_AB_)
					htcap_chwd_offset = HT_2NDCH_OFFSET_ABOVE;
				else
					htcap_chwd_offset = 0;
				if(htcap_chwd_cur ==0)
					htcap_chwd_offset = 0;

				if (priv->pmib->dot11nConfigEntry.dot11n2ndChOffset != htcap_chwd_offset) {
                    STADEBUG("my dot11n2ndChOffset[%d],AP's offset= [%d]\n\n",priv->pmib->dot11nConfigEntry.dot11n2ndChOffset, htcap_chwd_offset); 
					priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = htcap_chwd_offset;
                    #if 0   //def UNIVERSAL_REPEATER
					if (IS_VXD_INTERFACE(priv)) {
						OPMODE_VAL(WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE);
						JOIN_RES_VAL(STATE_Sta_No_Bss);
						DEBUG_INFO("%s: AP has changed 2nd ch offset, reconnect...\n", __FUNCTION__);
						return SUCCESS;
					}
					else
                    #endif
                    #if 0//defined(UNIVERSAL_REPEATER) || defined(MBSSID)
					if (IS_ROOT_INTERFACE(priv))
                    #endif
					{   //either root or non-root goto reconn
						DEBUG_INFO("%s: AP has changed 2nd ch offset, reconnect...\n", __FUNCTION__);
//						goto ReConn;
						priv->pmib->dot11Bss.t_stamp[1] &= ~(BIT(1) | BIT(2));
						if(htcap_chwd_offset == HT_2NDCH_OFFSET_BELOW) 
							priv->pmib->dot11Bss.t_stamp[1] |= (BIT(1) | BIT(2));
						else if(htcap_chwd_offset == HT_2NDCH_OFFSET_ABOVE) 
							priv->pmib->dot11Bss.t_stamp[1] |= (BIT(1));
						clnt_switch_chan_to_bss(priv);
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
						update_RAMask_to_FW(priv, 1);
#endif				
						SetTxPowerLevel(priv);
					}
				}
			}

			pstat->ht_ie_len = len;
			memcpy((unsigned char *)&pstat->ht_ie_buf, p+2, len);

			priv->ht_protection = 0;
			if (!priv->pmib->dot11StationConfigEntry.protectionDisabled && pstat->ht_ie_len) {
				unsigned int prot_mode =  (cpu_to_le16(pstat->ht_ie_buf.info1) & 0x03);
				if (prot_mode == _HTIE_OP_MODE1_ || prot_mode == _HTIE_OP_MODE3_)
					priv->ht_protection = 1;
			}
		}
#if 0

		//if ((priv->beacon_period > 200) || ((priv->rxBeaconNumInPeriod % 3) == 0)) {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p !=  NULL) {
			struct ht_cap_elmt *ht_cap = (struct ht_cap_elmt *)(p+2);
			int htcap_chwd_cur = 0;

			if (OPMODE & WIFI_ASOC_STATE) {			
				if ((ht_cap->ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))) 					
					htcap_chwd_cur = 1;
			
				if (priv->pshare->AP_BW < 0)
					priv->pshare->AP_BW = htcap_chwd_cur;
				else {
					if (priv->pshare->AP_BW != htcap_chwd_cur) {
						DEBUG_INFO("%s: AP has changed BW, reconnect...\n", __FUNCTION__);
						goto ReConn;
					}
				}
			}
		} else {
			DEBUG_INFO("%s: AP HT capability missing, reconnect...\n", __FUNCTION__);
			goto ReConn;
		}
		//}
#endif		
	}

	/*
	 *	Update TXOP from Beacon every 3 seconds
	 */
#ifdef WIFI_WMM
	if (QOS_ENABLE && pstat->QosEnabled && !(priv->up_time % 3) &&
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		IS_ROOT_INTERFACE(priv) &&
#endif
		priv->pmib->dot11OperationEntry.wifi_specific) {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _RSN_IE_1_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

		if (p != NULL) {
			if (!memcmp(p+2, WMM_PARA_IE, 6)) {
				/* save previous edca value */
				vo_txop = (unsigned short)(GET_STA_AC_VO_PARA.TXOPlimit);
				vi_txop = (unsigned short)(GET_STA_AC_VI_PARA.TXOPlimit);
				be_txop = (unsigned short)(GET_STA_AC_BE_PARA.TXOPlimit);
				bk_txop = (unsigned short)(GET_STA_AC_BK_PARA.TXOPlimit);
				
				/* capture the EDCA para */
				p += 10;  /* start of EDCA parameters at here*/
				for (i = 0; i <4; i++) {
					process_WMM_para_ie(priv, p);  /* get the info */
					p += 4;
				}

				/* check whether if TXOP is different from previous settings */
				if ((vo_txop != (unsigned short)(GET_STA_AC_VO_PARA.TXOPlimit)) ||
					(vi_txop != (unsigned short)(GET_STA_AC_VI_PARA.TXOPlimit)) ||
					(be_txop != (unsigned short)(GET_STA_AC_BE_PARA.TXOPlimit)) ||
					(bk_txop != (unsigned short)(GET_STA_AC_BK_PARA.TXOPlimit))) {
					SAVE_INT_AND_CLI(flags);
					sta_config_EDCA_para(priv);
					RESTORE_INT(flags);
					DEBUG_INFO("Client mode EDCA updated from beacon\n");
					DEBUG_INFO("BE: ACM %d, AIFSN %d, ECWmin %d, ECWmax %d, TXOP %d\n",
						GET_STA_AC_BE_PARA.ACM, GET_STA_AC_BE_PARA.AIFSN,
						GET_STA_AC_BE_PARA.ECWmin, GET_STA_AC_BE_PARA.ECWmax,
						GET_STA_AC_BE_PARA.TXOPlimit);
					DEBUG_INFO("VO: ACM %d, AIFSN %d, ECWmin %d, ECWmax %d, TXOP %d\n",
						GET_STA_AC_VO_PARA.ACM, GET_STA_AC_VO_PARA.AIFSN,
						GET_STA_AC_VO_PARA.ECWmin, GET_STA_AC_VO_PARA.ECWmax,
						GET_STA_AC_VO_PARA.TXOPlimit);
					DEBUG_INFO("VI: ACM %d, AIFSN %d, ECWmin %d, ECWmax %d, TXOP %d\n",
						GET_STA_AC_VI_PARA.ACM, GET_STA_AC_VI_PARA.AIFSN,
						GET_STA_AC_VI_PARA.ECWmin, GET_STA_AC_VI_PARA.ECWmax,
						GET_STA_AC_VI_PARA.TXOPlimit);
					DEBUG_INFO("BK: ACM %d, AIFSN %d, ECWmin %d, ECWmax %d, TXOP %d\n",
						GET_STA_AC_BK_PARA.ACM, GET_STA_AC_BK_PARA.AIFSN,
						GET_STA_AC_BK_PARA.ECWmin, GET_STA_AC_BK_PARA.ECWmax,
						GET_STA_AC_BK_PARA.TXOPlimit);
				}
			}
		}
	}
#endif

	// Realtek proprietary IE
    pstat->is_realtek_sta = FALSE;	
	p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_; len = 0;
	for (;;)
	{
		p = get_ie(p, _RSN_IE_1_, &len,
			pfrinfo->pktlen - (p - pframe));
		if (p != NULL) {
			if (!memcmp(p+2, Realtek_OUI, 3) && *(p+2+3) == 2) { /*found realtek out and type == 2*/
				pstat->is_realtek_sta = TRUE;
				break;
			}
		}
		else
			break;

		p = p + len + 2;
	}

	// Customer proprietary IE
	if (priv->pmib->miscEntry.private_ie_len) {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, priv->pmib->miscEntry.private_ie[0], &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p) {
			memcpy(pstat->private_ie, p, len + 2);
			pstat->private_ie_len = len + 2;
		}
	}

#ifdef DFS
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _CSA_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if (p!=NULL){
		DEBUG_INFO("Associated AP notified to do DFS\n");
		if(IS_ROOT_INTERFACE(priv)) {
			// channel switch mode
			priv->pshare->dfsSwitchChannel = (unsigned int)*(p+3);
			priv->pshare->dfsSwitchChCountDown =(unsigned int)*(p+4);
			DEBUG_INFO("CSA Detected mode=%d, channel=%d, countdown=%d\n",*(p+2), priv->pshare->dfsSwitchChannel, priv->pshare->dfsSwitchChCountDown);
			if (priv->pshare->dfsSwitchChCountDown <= 5) {
				if (timer_pending(&priv->dfs_cntdwn_timer))
					del_timer(&priv->dfs_cntdwn_timer);

				DFS_SwChnl_clnt(priv);
				priv->pshare->dfsSwCh_ongoing = 1;
				mod_timer(&priv->dfs_cntdwn_timer, jiffies + RTL_SECONDS_TO_JIFFIES(2));
			}
		} else {
			if(!GET_ROOT(priv)->pmib->dot11DFSEntry.DFS_detected) {
				GET_ROOT(priv)->pshare->dfsSwitchChannel = (unsigned int)*(p+3);
				GET_ROOT(priv)->pshare->dfsSwitchChCountDown =(unsigned int)*(p+4);
				GET_ROOT(priv)->pmib->dot11DFSEntry.DFS_detected = 1;
				DEBUG_INFO("Asscociated AP detected CSA , channel=%d, countdown=%d\n", GET_ROOT(priv)->pshare->dfsSwitchChannel, GET_ROOT(priv)->pshare->dfsSwitchChCountDown);
		}
	}
}
#endif

#ifdef P2P_SUPPORT
	if ((rtk_p2p_is_enabled(priv)) && rtk_p2p_chk_role(priv,P2P_CLIENT)) {			/*cfg p2p cfg p2p */
		/*just  take care beacon come from my BSSID*/
		if(!memcmp(BSSID,pfrinfo->sa,6)){
		ptr = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_;
		len = 0;

		/*support ReAssemble*/		
		for (;;)
		{
			/* get P2P_IE */
			ptr = get_ie(ptr, _P2P_IE_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_ - len);
			
			if (ptr) {
				if (!memcmp(ptr+2, WFA_OUI_PLUS_TYPE, 4)) {

					memcpy(p2pIEPtr , ptr+6 ,len - 4);
					p2pIEPtr += (len - 4);
					IEfoundtimes ++;
					
				}
			}
			else{
				break;
			}

			ptr = ptr + len + 2;
		}
		
		if(IEfoundtimes){
			p2pIElen = (int)(((unsigned long)p2pIEPtr)-((unsigned long)ReAssem_p2pie));		
			
			if(p2pIElen > MAX_REASSEM_P2P_IE){
				P2P_DEBUG("\n\n	reassemble P2P IE exceed MAX_REASSEM_P2P_IE , chk!!!\n\n");
			}else{
				P2P_client_on_beacon(priv, ReAssem_p2pie , p2pIElen , GetSequence(pframe));
			}
		}

		}
	}
#endif

	return SUCCESS;

#if 0
ReConn:

	OPMODE_VAL(OPMODE & ~(WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE));
#ifdef CONFIG_RTL_WLAN_STATUS
	priv->wlan_status_flag=1;
#endif
#ifdef UNIVERSAL_REPEATER
	disable_vxd_ap(GET_VXD_PRIV(priv));
#endif
	JOIN_RES_VAL(STATE_Sta_No_Bss);
	STADEBUG("start_clnt_lookup(RESCAN_ROAMING)\n");    
	start_clnt_lookup(priv, RESCAN_ROAMING);
	return SUCCESS;
#endif	
}


/**
 *	@brief	STA in ad hoc mode Beacon process.
 */
int OnBeaconClnt_Ibss(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char *bssid, *bdsa;
	struct stat_info *pstat;
	unsigned char *p, *pframe, channel;
	int len;
	unsigned char supportRate[32];
	int supportRateNum;
	unsigned short val16;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	pframe = get_pframe(pfrinfo);

	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _DSSET_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if(p == NULL)
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if (p != NULL)
		channel = *(p+2);
	else
		channel = priv->pmib->dot11RFEntry.dot11channel;

	/*
	 * check if OLBC exist
	 */
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
		(channel == priv->pmib->dot11RFEntry.dot11channel))
	{
		// look for ERP rate. if no ERP rate existed, thought it is a legacy AP
		unsigned char supportedRates[32];
		int supplen=0, legacy=1, i;

		pframe = get_pframe(pfrinfo);
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SUPPORTEDRATES_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p) {
			if (len>8)
				len=8;
			memcpy(&supportedRates[supplen], p+2, len);
			supplen += len;
		}

		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p) {
			if (len>8)
				len=8;
			memcpy(&supportedRates[supplen], p+2, len);
			supplen += len;
		}

		for (i=0; i<supplen; i++) {
			if (!is_CCK_rate(supportedRates[i]&0x7f)) {
				legacy = 0;
				break;
			}
		}

		// look for ERP IE and check non ERP present
		if (legacy == 0) {
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _ERPINFO_IE_, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if (p && (*(p+2) & BIT(0)))
				legacy = 1;
		}

		if (legacy) {
			if (!priv->pmib->dot11StationConfigEntry.olbcDetectDisabled &&
							priv->pmib->dot11ErpInfo.olbcDetected==0) {
				priv->pmib->dot11ErpInfo.olbcDetected = 1;
				check_protection_shortslot(priv);
				DEBUG_INFO("OLBC detected\n");
			}
			if (priv->pmib->dot11ErpInfo.olbcDetected)
				priv->pmib->dot11ErpInfo.olbcExpired = DEFAULT_OLBC_EXPIRE;
		}
	}


// mantis#2523
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SSID_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if ( p && (SSID_LEN == len) && !memcmp(SSID, p+2, len)) {
		memcpy(priv->rx_timestamp, pframe+WLAN_HDR_A3_LEN, 8);
	}	

	/*
	 * add into sta table and calculate beacon
	 */
	bssid = GetAddr3Ptr(pframe);
	bdsa = GetAddr2Ptr(pframe);

	if (!IS_BSSID(priv, bssid))
		return SUCCESS;

	memcpy(&val16, (pframe + WLAN_HDR_A3_LEN + 8 + 2), 2);
	val16 = le16_to_cpu(val16);
	if ((val16 & BIT(0)) || !(val16 & BIT(1)))
		return SUCCESS;

	// this is our peers
	pstat = get_stainfo(priv, bdsa);

	if (pstat == NULL) {
		DEBUG_INFO("Add IBSS sta, %02x:%02x:%02x:%02x:%02x:%02x!\n",
			bdsa[0],bdsa[1], bdsa[2],bdsa[3],bdsa[4],bdsa[5]);

		pstat = alloc_stainfo(priv, bdsa, -1);
		if (pstat == NULL)
			return SUCCESS;

		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SUPPORTEDRATES_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p == NULL) {
			free_stainfo(priv, pstat);
			return SUCCESS;
		}
		memcpy(supportRate, p+2, len);
		supportRateNum = len;
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, &len,
			pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		if (p !=  NULL) {
			memcpy(supportRate+supportRateNum, p+2, len);
			supportRateNum += len;
		}

#ifdef WIFI_WMM
		// check if there is WMM IE
		if (QOS_ENABLE) {
			p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_; len = 0;
			for (;;) {
				p = get_ie(p, _RSN_IE_1_, &len,
					pfrinfo->pktlen - (p - pframe));
				if (p != NULL) {
					if (!memcmp(p+2, WMM_IE, 6)) {
						pstat->QosEnabled = 1;
#ifdef WMM_APSD
						if (APSD_ENABLE)
							pstat->apsd_bitmap = *(p+8) & 0x0f;		// get QSTA APSD bitmap
#endif
						break;
					}
				}
				else {
					pstat->QosEnabled = 0;
#ifdef WMM_APSD
					pstat->apsd_bitmap = 0;
#endif
					break;
				}
				p = p + len + 2;
			}
		}
		else {
			pstat->QosEnabled = 0;
#ifdef WMM_APSD
			pstat->apsd_bitmap = 0;
#endif
		}
#endif

		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_, &len,
					pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if (p !=  NULL) {
				unsigned char mimo_ps;
				pstat->ht_cap_len = len;
				memcpy((unsigned char *)&pstat->ht_cap_buf, p+2, len);
				// below is the process to check HT MIMO power save
				mimo_ps = ((cpu_to_le16(pstat->ht_cap_buf.ht_cap_info)) >> 2)&0x0003;
				pstat->MIMO_ps = 0;
				if (!mimo_ps)
					pstat->MIMO_ps |= _HT_MIMO_PS_STATIC_;
				else if (mimo_ps == 1)
					pstat->MIMO_ps |= _HT_MIMO_PS_DYNAMIC_;

				check_NAV_prot_len(priv, pstat, 0);

				if (cpu_to_le16(pstat->ht_cap_buf.ht_cap_info) & _HTCAP_AMSDU_LEN_8K_) {
					pstat->is_8k_amsdu = 1;
					pstat->amsdu_level = 7935 - sizeof(struct wlan_hdr);
 				} else {
					pstat->is_8k_amsdu = 0;
					pstat->amsdu_level = 3839 - sizeof(struct wlan_hdr);
				}

				if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))
					pstat->tx_bw = HT_CHANNEL_WIDTH_20_40;
				else
					pstat->tx_bw = HT_CHANNEL_WIDTH_20;
			}
			else {
				pstat->ht_cap_len = 0;
				memset((unsigned char *)&pstat->ht_cap_buf, 0, sizeof(struct ht_cap_elmt));
			}

			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_IE_, &len,
					pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if (p !=  NULL) {
				pstat->ht_ie_len = len;
				memcpy((unsigned char *)&pstat->ht_ie_buf, p+2, len);
			}
			else
				pstat->ht_ie_len = 0;
		}
#ifdef RTK_AC_SUPPORT		//ADHOC-VHT support
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
		{
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTCapability, &len,
					pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if ((p !=  NULL) && (len <= sizeof(struct vht_cap_elmt))) {
				pstat->vht_cap_len = len;
				memcpy((unsigned char *)&pstat->vht_cap_buf, p+2, len);
			}
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTOperation, &len,
					pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if ((p !=  NULL) && (len <= sizeof(struct vht_oper_elmt))) {
				pstat->vht_oper_len = len;
				memcpy((unsigned char *)&pstat->vht_oper_buf, p+2, len);
			}
			if(pstat->vht_cap_len){
				switch(cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & 0x3) {
					default:				
					case 0: 
						pstat->is_8k_amsdu = 0;
						pstat->amsdu_level = 3895 - sizeof(struct wlan_hdr);
						break;
					case 1:
						pstat->is_8k_amsdu = 1;
						pstat->amsdu_level = 7991 - sizeof(struct wlan_hdr);
						break;
					case 2: 
						pstat->is_8k_amsdu = 1;
						pstat->amsdu_level = 11454 - sizeof(struct wlan_hdr);	
						break;
				}
				if (pstat->vht_oper_buf.vht_oper_info[0] == 1) {
					pstat->tx_bw = HT_CHANNEL_WIDTH_80;
					priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_80;	
				}
			}
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTOperatingMode, &len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
			if ((p !=  NULL) && (len == 1)) {
					if((p[2] &3) <= priv->pmib->dot11nConfigEntry.dot11nUse40M)
						pstat->tx_bw = p[2] &3;
					pstat->nss = ((p[2]>>4)&0x7)+1; 										
			}
#ifdef CONFIG_RTL_8812_SUPPORT			
			if(GET_CHIP_VER(priv)== VERSION_8812E){
				UpdateHalMSRRPT8812(priv, pstat, INCREASE);
			}
#endif
		}
#endif	

		// Realtek proprietary IE
		pstat->is_realtek_sta = FALSE;
		p = pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_; len = 0;
		for (;;)
		{
			p = get_ie(p, _RSN_IE_1_, &len,
								pfrinfo->pktlen - (p - pframe));

#ifdef WIFI_WPAS
			if (p != NULL) {
				if((*(unsigned char *)p == _RSN_IE_1_)&& (len >= 4))
				{
					WPAS_ASSOCIATION_INFO Assoc_Info;

					memset((void *)&Assoc_Info, 0, sizeof(struct _WPAS_ASSOCIATION_INFO));
					Assoc_Info.ReqIELen = p[1]+ 2;
					memcpy(Assoc_Info.ReqIE, p, Assoc_Info.ReqIELen);
					event_indicate_wpas(priv, NULL, WPAS_ASSOC_INFO, (UINT8 *)&Assoc_Info);
				}
			}	
#endif

			if (p != NULL) {
				if (!memcmp(p+2, Realtek_OUI, 3) && *(p+2+3) == 2) { /*found realtek out and type == 2*/
					pstat->is_realtek_sta = TRUE;
					break;
				}
			}
			else
				break;

			p = p + len + 2;
		}

		if ((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_) ||
			(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_) ||
			(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) ||
			(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_) ) {
			DOT11_SET_KEY Set_Key;
			memcpy(Set_Key.MACAddr, pstat->hwaddr, 6);
			Set_Key.KeyType = DOT11_KeyType_Pairwise;
			if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
					priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_) {
				Set_Key.EncType = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
				Set_Key.KeyIndex = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
				DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key,
				priv->pmib->dot11DefaultKeysTable.keytype[Set_Key.KeyIndex].skey);
			}
			else {
				Set_Key.EncType = (unsigned char)priv->pmib->dot11GroupKeysTable.dot11Privacy;
				Set_Key.KeyIndex = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
				DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key,
				priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKey1.skey);
			}
		}

		get_matched_rate(priv, supportRate, &supportRateNum, 0);
		update_support_rate(pstat, supportRate, supportRateNum);

		assign_tx_rate(priv, pstat, pfrinfo);
		assign_aggre_mthod(priv, pstat);
		assign_aggre_size(priv, pstat);

		val16 = cpu_to_le16(*(unsigned short*)((unsigned long)pframe + WLAN_HDR_A3_LEN + 8 + 2));
		if (!(val16 & BIT(5))) // NOT use short preamble
			pstat->useShortPreamble = 0;
		else
			pstat->useShortPreamble = 1;

		pstat->state |= (WIFI_ASOC_STATE | WIFI_AUTH_SUCCESS);

		SAVE_INT_AND_CLI(flags);
		pstat->expire_to = priv->expire_to;
		asoc_list_add(priv, pstat);
		cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);
		check_sta_characteristic(priv, pstat, INCREASE);
		RESTORE_INT(flags);

#ifdef RTK_NL80211 //wrt-adhoc
		event_indicate_cfg80211(priv, NULL, CFG80211_IBSS_JOINED, NULL);
#endif

		LOG_MSG("An IBSS client is detected - %02X:%02X:%02X:%02X:%02X:%02X\n",
			*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
			*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));

#if defined(CONFIG_PCI_HCI)
#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
		} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			UpdateHalRAMask8812(priv, pstat, 3);
		} else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
			add_RATid(priv, pstat);
#endif
		} else
#endif
		{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
			add_update_RATid(priv, pstat);
#endif
		}
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		update_STA_RATid(priv, pstat);
#endif
	}

	if (timer_pending(&priv->idle_timer))
		del_timer(&priv->idle_timer);

	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _DSSET_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if(p == NULL)
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_IE_, &len,
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	if (p != NULL) {
		if (priv->pmib->dot11Bss.channel	== *(p+2)) {
			pstat->beacon_num++;
			priv->rxBeaconNumInPeriod++;
			JOIN_RES_VAL(STATE_Sta_Ibss_Active);
		}
	}
	return SUCCESS;
}


/**
 *	@brief	STA recived Beacon process
 */
unsigned int OnBeaconClnt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
    int ret = SUCCESS;

    // Site survey and collect information
    if (OPMODE & WIFI_SITE_MONITOR) {
#ifdef P2P_SUPPORT			/*cfg p2p cfg p2p */
        if( (rtk_p2p_is_enabled(priv)==PROPERTY_P2P) && P2P_DISCOVERY){
            p2p_collect_bss_info(priv, pfrinfo, WIFI_BEACON);
        }          
        else
#endif
        {
            collect_bss_info(priv, pfrinfo);
        }
        return SUCCESS;
    }

	// Infra client mode, check beacon info
	if ((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) ==
		(WIFI_STATION_STATE | WIFI_ASOC_STATE))
		ret = OnBeaconClnt_Bss(priv, pfrinfo);
#if defined(WIFI_WMM) && defined(WMM_APSD)
	else if (QOS_ENABLE && APSD_ENABLE && (OPMODE & WIFI_STATION_STATE) && !(OPMODE & WIFI_ASOC_STATE))
		collect_bss_info(priv, pfrinfo);
#endif

	// Ad-hoc client mode, check peer's beacon
	if ((OPMODE & WIFI_ADHOC_STATE) &&
		((JOIN_RES == STATE_Sta_Ibss_Active) || (JOIN_RES == STATE_Sta_Ibss_Idle)))
		ret = OnBeaconClnt_Ibss(priv, pfrinfo);

	return ret;
}


/**
 *	@brief	STA recived ATIM
 *
 *	STA only.
 */
unsigned int OnATIM(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	return SUCCESS;
}


unsigned int OnDisassocClnt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned long link_time=0;
    struct stat_info *pstat=NULL;	
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned char *bssid = GetAddr3Ptr(pframe);
	unsigned short val16;
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)
	int if_idx = -1;
#endif
	if (!(OPMODE & WIFI_STATION_STATE))
		return SUCCESS;

	if (memcmp(GET_MY_HWADDR, pfrinfo->da, MACADDRLEN)
#ifdef CONFIG_IEEE80211W_CLI
		|| !IS_MCAST(pfrinfo->da) //BIP da: 0xFFFFFFFFFFFF
#endif
		)
		return SUCCESS;

	if (!memcmp(BSSID, bssid, MACADDRLEN)) {
		memcpy(&val16, (pframe + WLAN_HDR_A3_LEN), 2);
        val16 = le16_to_cpu(val16);
		DEBUG_INFO("recv Disassociation, reason: %d\n", val16);
        if(ACTIVE_ID == 0) {
            if(val16 == _RSON_MIC_FAILURE_ || val16 == _RSON_4WAY_HNDSHK_TIMEOUT_)
                priv->dot114WayStatus = val16;
        }
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			pstat = get_stainfo(priv, pfrinfo->da);

			if (pstat && pstat->sta_in_firmware == 1) {
				RTL8188E_MACID_NOLINK(priv, 1, REMAP_AID(pstat));
				RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
			}
		}
#endif

        pstat = get_stainfo(priv, bssid);
        if (pstat == NULL) {    
            link_time = 0;      
        }else{        
            link_time = pstat->link_time;
			
#ifdef RTK_NL80211
			event_indicate_cfg80211(priv, NULL, CFG80211_DISCONNECTED, NULL);
#endif
        }
                
		OPMODE_VAL(OPMODE & ~(WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE));
		JOIN_RES_VAL(STATE_Sta_No_Bss);

#ifdef SMART_REPEATER_MODE
        if(GET_MIB(priv)->ap_profile.enable_profile && GET_MIB(priv)->ap_profile.profile_num > 0)            
        {
            if((priv->site_survey->count_target > 0) && ((priv->join_index+1) > priv->site_survey->count_target)){
            	STADEBUG("start_clnt_lookup(RESCAN)\n");                            
                start_clnt_lookup(priv, RESCAN);        
            }else{
            	STADEBUG("start_clnt_lookup(DONTRESCAN)\n");                        
    		    start_clnt_lookup(priv, DONTRESCAN);        
            }
        }else
#endif
		if (link_time > priv->expire_to) {	// if link time exceeds timeout, site survey again
        	STADEBUG("start_clnt_lookup(RESCAN)\n");
			start_clnt_lookup(priv, RESCAN);
		} else{
            STADEBUG("start_clnt_lookup(DONTRESCAN)\n");    
			start_clnt_lookup(priv, DONTRESCAN);
		}
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
		LOG_MSG_NOTICE("Disassociated by AP;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
				*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
				*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)
		if((if_idx = get_wlan_if_type(priv))>=0){
			set_if_status(if_idx,IF_DOWN);
			set_if_change_status(if_idx,IF_STATUS_CHANGE);
		}
#endif
#else
		LOG_MSG("Disassociated by AP - %02X:%02X:%02X:%02X:%02X:%02X\n",
			*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
			*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)
		if((if_idx = get_wlan_if_type(priv))>=0){
			set_if_status(if_idx,IF_DOWN);
			set_if_change_status(if_idx,IF_STATUS_CHANGE);
		}
#endif
#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
		if (priv->link_status_cb_func)
			priv->link_status_cb_func("wlan0", WIFI_LINK_STATUS_DISASSOCIATED);
#endif
#endif

#ifdef UNIVERSAL_REPEATER
		disable_vxd_ap(GET_VXD_PRIV(priv));
#endif
#ifdef CONFIG_RTL_WLAN_STATUS
	priv->wlan_status_flag=1;
#endif
	}

	return SUCCESS;
}


/**
 *	@brief	STA recived authentication
 *	AP and STA authentication each other.
 */
unsigned int OnAuthClnt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned int	privacy, seq, len, status, algthm, offset, go2asoc=0;
#ifndef SMP_SYNC
	unsigned long	flags;
#endif
	struct wifi_mib	*pmib;
	unsigned char	*pframe, *p;

	if (!(OPMODE & WIFI_STATION_STATE))
		return SUCCESS;

	if (memcmp(GET_MY_HWADDR, pfrinfo->da, MACADDRLEN))
		return SUCCESS;

	if (OPMODE & WIFI_SITE_MONITOR)
		return SUCCESS;

#if defined(CLIENT_MODE) && defined(INCLUDE_WPA_PSK)
       if (priv->assoc_reject_on && !memcmp(priv->assoc_reject_mac, pfrinfo->sa, MACADDRLEN)) 
		return SUCCESS;
#endif

	DEBUG_INFO("got auth response\n");
	pmib = GET_MIB(priv);
	pframe = get_pframe(pfrinfo);

	privacy = priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm;

	if (GetPrivacy(pframe))
		offset = 4;
	else
		offset = 0;

	algthm 	= cpu_to_le16(*(unsigned short *)((unsigned long)pframe + WLAN_HDR_A3_LEN + offset));
	seq 	= cpu_to_le16(*(unsigned short *)((unsigned long)pframe + WLAN_HDR_A3_LEN + offset + 2));
	status 	= cpu_to_le16(*(unsigned short *)((unsigned long)pframe + WLAN_HDR_A3_LEN + offset + 4));

	if (status != 0)
	{
		DEBUG_ERR("clnt auth fail, status: %d\n", status);
		goto authclnt_err_end;
	}

	if (seq == 2)
	{
#ifdef WIFI_SIMPLE_CONFIG
		if (pmib->wscEntry.wsc_enable && algthm == 0)
			privacy = 0;
#endif

		if ((privacy == 1) || // legacy shared system
			((privacy == 2) && (AUTH_MODE_TOGGLE) && // auto and use shared-key currently
			 (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
			  priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)))
		{
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _AUTH_IE_OFFSET_, _CHLGETXT_IE_, (int *)&len,
				pfrinfo->pktlen - WLAN_HDR_A3_LEN - _AUTH_IE_OFFSET_);

			if (p == NULL) {
				DEBUG_ERR("no challenge text?\n");
				goto authclnt_fail;
			}

			DEBUG_INFO("auth chlgetxt len =%d\n", len);
			memcpy((void *)CHG_TXT, (void *)(p+2), len);
			SAVE_INT_AND_CLI(flags);
			AUTH_SEQ_VAL(3);
			OPMODE_VAL(OPMODE & (~ WIFI_AUTH_NULL));
			OPMODE_VAL(OPMODE | (WIFI_AUTH_STATE1));
			RESTORE_INT(flags);
			issue_auth(priv, NULL, 0);
			return SUCCESS;
		}
		else // open system
			go2asoc = 1;
	}
	else if (seq == 4)
	{
		if (privacy)
			go2asoc = 1;
		else
		{
			// this is illegal
			DEBUG_ERR("no privacy but auth seq=4?\n");
			goto authclnt_fail;
		}
	}
	else
	{
		// this is also illegal
		DEBUG_ERR("clnt auth failed due to illegal seq=%x\n", seq);
		goto authclnt_fail;
	}

	if (go2asoc)
	{
		DEBUG_INFO("auth successful!\n");
		start_clnt_assoc(priv);
		return SUCCESS;
	}

authclnt_fail:

	REAUTH_COUNT_VAL(REAUTH_COUNT+1);
	if (REAUTH_COUNT < REAUTH_LIMIT)
		return FAIL;

authclnt_err_end:

	if ((priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm == 2) &&
		((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) ||
		 (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)) &&
		(AUTH_MODE_RETRY == 0)) {
		// auto-auth mode, retry another auth method
		AUTH_MODE_RETRY_VAL(AUTH_MODE_RETRY+1);

		start_clnt_auth(priv);
		return SUCCESS;
	}
	else {
		JOIN_RES_VAL(STATE_Sta_No_Bss);
		JOIN_REQ_ONGOING_VAL(0);

		if (PENDING_REAUTH_TIMER)
			DELETE_REAUTH_TIMER;

		start_clnt_lookup(priv, DONTRESCAN);
		return FAIL;
	}
}


/**
 *	@brief	Client/STA De authentication
 *	First DeAuthClnt, Second OnDeAuthClnt
 */
unsigned int OnDeAuthClnt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	unsigned long link_time;
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned char *bssid = GetAddr3Ptr(pframe);
	struct stat_info *pstat;
	unsigned short val16;
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)
	int if_idx = -1;
#endif

#ifdef CONFIG_IEEE80211W_CLI
	unsigned int	status = _STATS_SUCCESSFUL_;
	unsigned short	frame_type;
#endif

	if (!(OPMODE & WIFI_STATION_STATE))
		return SUCCESS;


	if (memcmp(GET_MY_HWADDR, pfrinfo->da, MACADDRLEN)
#ifdef CONFIG_IEEE80211W_CLI
		|| !IS_MCAST(pfrinfo->da) //BIP da: 0xFFFFFFFFFFFF
#endif
	)
		return SUCCESS;

	if (memcmp(GetAddr2Ptr(pframe), bssid, MACADDRLEN))
		return SUCCESS;

	if (!memcmp(priv->pmib->dot11Bss.bssid, bssid, MACADDRLEN)) {
		memcpy(&val16, (pframe + WLAN_HDR_A3_LEN), 2);
		DEBUG_INFO("recv Deauthentication, reason: %d\n", le16_to_cpu(val16));

#ifdef CONFIG_IEEE80211W_CLI
		if( _RSON_CLS3_ == le16_to_cpu(val16))
		{
			PMFDEBUG("%s(%d), start SA initiation for received _RSON_CLS3_ \n", __FUNCTION__, __LINE__);
			if(((pstat->state) & WIFI_ASOC_STATE) &&
				pstat->isPMF &&
				!pstat->sa_query_timed_out &&
				pstat->sa_query_count)	{
				check_sa_query_timeout(pstat);
			}
			
			if(((pstat->state) & WIFI_ASOC_STATE) &&
				pstat->isPMF &&
				!pstat->sa_query_timed_out) {
				PMFDEBUG("%s(%d)\n", __FUNCTION__, __LINE__);
				status = _STATS_ASSOC_REJ_TEMP_;
				if(pstat->sa_query_count == 0) {			
					pstat->sa_query_start = jiffies;			
					pstat->sa_query_end = jiffies + RTL_MILISECONDS_TO_JIFFIES(SA_QUERY_MAX_TO);
				}
				
				frame_type = GetFrameSubType(pframe);
				if (frame_type == WIFI_ASSOCREQ)
					issue_asocrsp(priv, status, pstat, WIFI_ASSOCRSP);
				else
					issue_asocrsp(priv, status, pstat, WIFI_REASSOCRSP);
				
				if(pstat->sa_query_count == 0) {
					PMFDEBUG("sa_query_end=%lu, sa_query_start=%lu\n", pstat->sa_query_end, pstat->sa_query_start);
					pstat->sa_query_count++;
					issue_SA_Query_Req(priv->dev,pstat->hwaddr);
					PMFDEBUG("%s(%d), settimer, %x\n", __FUNCTION__, __LINE__, &pstat->SA_timer);
					
					if(timer_pending(&pstat->SA_timer))
						del_timer(&pstat->SA_timer);
		
					pstat->SA_timer.data = (unsigned long) pstat;
					pstat->SA_timer.function = rtl8192cd_sa_query_timer;
					mod_timer(&pstat->SA_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(SA_QUERY_RETRY_TO));				
					PMFDEBUG("%s(%d), settimer end\n", __FUNCTION__, __LINE__);
				}					
				return FAIL;
			}
			pstat->sa_query_timed_out = 0;
		}
#endif

		pstat = get_stainfo(priv, bssid);
		if (pstat == NULL) { // how come?
// Start scan again ----------------------
//			return FAIL;
			link_time = 0; // get next bss info
			goto do_scan;
//--------------------- david+2007-03-10
		}

#ifdef MULTI_MAC_CLONE
		if (ACTIVE_ID == 0)
#endif
		{
		link_time = pstat->link_time;

#ifdef RTK_NL80211
		event_indicate_cfg80211(priv, NULL, CFG80211_DISCONNECTED, NULL);
#endif

		SAVE_INT_AND_CLI(flags);
		if (asoc_list_del(priv, pstat)) {
			cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);

#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E && pstat->sta_in_firmware == 1) {
				RTL8188E_MACID_NOLINK(priv, 1, REMAP_AID(pstat));
				RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
			}
#endif
		}
		RESTORE_INT(flags);

		free_stainfo(priv, pstat);
		}
do_scan:
		OPMODE_VAL(OPMODE & ~(WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE));
		JOIN_RES_VAL(STATE_Sta_No_Bss);

// Delete timer --------------------------------------
		if (PENDING_REAUTH_TIMER)
			DELETE_REAUTH_TIMER;

		if (PENDING_REASSOC_TIMER)
			DELETE_REASSOC_TIMER;
//---------------------------------- david+2007-03-10

 #ifdef SMART_REPEATER_MODE
        if(GET_MIB(priv)->ap_profile.enable_profile && GET_MIB(priv)->ap_profile.profile_num > 0)            
        {
            if((priv->site_survey->count_target > 0) && ((priv->join_index+1) > priv->site_survey->count_target)){
    	    	STADEBUG("start_clnt_lookup(RESCAN)\n");                			
                start_clnt_lookup(priv, RESCAN);        
            }else{
	            STADEBUG("start_clnt_lookup(DONTRESCAN)\n");    			
                start_clnt_lookup(priv, DONTRESCAN);        
            }
        }else
#endif
		if (link_time > priv->expire_to) {	// if link time exceeds timeout, site survey again
        	STADEBUG("start_clnt_lookup(RESCAN)\n");                
			start_clnt_lookup(priv, RESCAN);
		} else{
            STADEBUG("start_clnt_lookup(DONTRESCAN)\n");    
			start_clnt_lookup(priv, DONTRESCAN);
		}
        
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
		LOG_MSG_NOTICE("Deauthenticated by AP;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
				*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
				*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)
		if((if_idx = get_wlan_if_type(priv))>=0){
			set_if_status(if_idx,IF_DOWN);
			set_if_change_status(if_idx,IF_STATUS_CHANGE);
		}
#endif
#else
		LOG_MSG("Deauthenticated by AP - %02X:%02X:%02X:%02X:%02X:%02X\n",
			*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
			*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)
		if((if_idx = get_wlan_if_type(priv))>=0){
			set_if_status(if_idx,IF_DOWN);
			set_if_change_status(if_idx,IF_STATUS_CHANGE);
		}
#endif
#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
		if (priv->link_status_cb_func)
			priv->link_status_cb_func("wlan0", WIFI_LINK_STATUS_DEAUTHENTICATED);
#endif
#endif

#ifdef UNIVERSAL_REPEATER
		disable_vxd_ap(GET_VXD_PRIV(priv));
#endif
#ifdef CONFIG_RTL_WLAN_STATUS
	priv->wlan_status_flag=1;
#endif
	}

	return SUCCESS;
}


void issue_PwrMgt_NullData(struct rtl8192cd_priv *priv)
{
	struct wifi_mib *pmib;
	unsigned char *hwaddr;
	DECLARE_TXINSN(txinsn);

	pmib = GET_MIB(priv);
	txinsn.retry = pmib->dot11OperationEntry.dot11ShortRetryLimit;
	hwaddr = pmib->dot11OperationEntry.hwaddr;

	txinsn.q_num = MANAGE_QUE_NUM;
#ifdef P2P_SUPPORT				/*cfg p2p cfg p2p */
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	txinsn.fr_type = _PRE_ALLOCHDR_;
	txinsn.phdr = get_wlanhdr_from_poll(priv);
	txinsn.pframe = NULL;

	if (txinsn.phdr == NULL)
		goto send_fail;

	memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

	SetFrameSubType(txinsn.phdr, WIFI_DATA_NULL);
	SetToDs(txinsn.phdr);
	if (priv->ps_state)
		SetPwrMgt(txinsn.phdr);
	else
		ClearPwrMgt(txinsn.phdr);

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)),  BSSID, MACADDRLEN);
	txinsn.hdr_len = WLAN_HDR_A3_LEN;

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return;

send_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
}


void issue_PsPoll(struct rtl8192cd_priv *priv)
{
	struct wifi_mib *pmib;
	unsigned char *hwaddr;
	DECLARE_TXINSN(txinsn);

	pmib = GET_MIB(priv);
	txinsn.retry = pmib->dot11OperationEntry.dot11ShortRetryLimit;
	hwaddr = GET_MY_HWADDR;

	txinsn.q_num = MANAGE_QUE_NUM;
#ifdef P2P_SUPPORT				/*cfg p2p cfg p2p */
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	txinsn.fr_type = _PRE_ALLOCHDR_;
	txinsn.phdr = get_wlanhdr_from_poll(priv);
	txinsn.pframe = NULL;

	if (txinsn.phdr == NULL)
		goto send_fail;

	memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

	SetFrameSubType(txinsn.phdr, WIFI_PSPOLL);
	SetPwrMgt(txinsn.phdr);
	SetPsPollAid(txinsn.phdr, _AID);

	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), hwaddr, MACADDRLEN);
	txinsn.hdr_len = WLAN_HDR_PSPOLL;

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return;

send_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
}

#ifdef MULTI_MAC_CLONE
void issue_NullData(struct rtl8192cd_priv *priv, unsigned char *da)
{
#ifdef WIFI_WMM
	unsigned char tempQosControl[2];
#endif
	unsigned char *hwaddr;
	DECLARE_TXINSN(txinsn);

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

	hwaddr = GET_MY_HWADDR;

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	txinsn.phdr = get_wlanhdr_from_poll(priv);
	txinsn.pframe = NULL;

	if (txinsn.phdr == NULL)
		goto send_qos_null_fail;

	memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

	SetToDs(txinsn.phdr);
#ifdef WIFI_WMM
	if ((QOS_ENABLE) && (priv->pmib->dot11Bss.t_stamp[1] & BIT(0)))
	{
		SetFrameSubType(txinsn.phdr, BIT(7) | WIFI_DATA_NULL);
		txinsn.hdr_len = WLAN_HDR_A3_QOS_LEN;
		memset(tempQosControl, 0, 2);
		tempQosControl[0] = 0x07;		//set priority to VO
		tempQosControl[0] |= BIT(4);	//set EOSP
		memcpy((void *)GetQosControl((txinsn.phdr)), tempQosControl, 2);

	}
	else // WIFI_DATA_NULL
#endif
	{
		SetFrameSubType(txinsn.phdr, WIFI_DATA_NULL);
		txinsn.hdr_len = WLAN_HDR_A3_LEN;
	}
	
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), da, MACADDRLEN);

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return;

send_qos_null_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
}
#endif


unsigned int isOurFrameBuffred(unsigned char* tim, unsigned int aid)
{
	unsigned int numSta;
	
	numSta = (*(tim + 4) & 0xFE) * 8;
	if (!((aid < numSta) || (aid >= (numSta + (*(tim + 1)-3)*8)))) {
		unsigned int offset;
		unsigned int offset_byte;
		unsigned int offset_bit;
		unsigned char *PartialBitmap = tim + 5;
		unsigned int result;

		offset = aid - numSta;
		offset_byte = offset / 8;
		offset_bit  = offset % 8;
		result = PartialBitmap[offset_byte] & (1 << offset_bit);

		return (result) ? TRUE : FALSE;
	}

	return FALSE;
}

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
void start_repeater_ss(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long flags = 0;
#endif
	int xtimeout = 0; //ecos have a "timeout" function. Rename the variable.
    int channel_tmp1;
    int i=0;
#ifdef MULTI_MAC_CLONE
	struct rtl8192cd_priv *vxd_priv=priv;
	struct rtl8192cd_priv *vap_priv;
	int num_sta=0;

    #ifdef MBSSID
	for (i=RTL8192CD_NUM_VWLAN-1; i>=0; i--)
	{
		vap_priv = GET_VAP_PRIV(priv, i);
		if (netif_running(vap_priv->dev) &&
			(GET_MIB(vap_priv)->dot11OperationEntry.opmode&WIFI_STATION_STATE))
		{
			num_sta++;
		}
	}
    #endif    
	if (netif_running(vxd_priv->dev) && (vxd_priv->drv_state&DRV_STATE_VXD_INIT))
	{
		num_sta++;
	}

	if (!num_sta)
		return;
#else
    if(!netif_running(priv->dev)){
        return;
    }
    		
#if defined( UNIVERSAL_REPEATER)		
    if(IS_VXD_INTERFACE(priv) && !(priv->drv_state & DRV_STATE_VXD_INIT) )
	  {
        return;
    }    
#endif
#endif

	//STADEBUG("count_target=%d;join_index=%d ;ongoing=%d\n",priv->site_survey->count_target,priv->join_index,priv->ss_req_ongoing);

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);

	if (  !(GET_MIB(priv)->dot11OperationEntry.opmode & WIFI_ASOC_STATE)) {			
		if (!netif_running(priv->dev) || priv->ss_req_ongoing) {
			xtimeout = RTL_SECONDS_TO_JIFFIES(1);
			goto out;
		}
		if (!netif_running(GET_ROOT(priv)->dev) || GET_ROOT(priv)->ss_req_ongoing) {

			xtimeout = RTL_SECONDS_TO_JIFFIES(1);
			goto out;
		}
		if (GET_MIB(priv)->wscEntry.wsc_enable) {
			goto out;
		}
        
#ifdef CONFIG_RTK_MESH
        if(GET_ROOT(priv)->pmib->dot1180211sInfo.mesh_enable && timer_pending(&(GET_ROOT(priv)->ss_timer)))
        {   /*if mesh is doing auto channel scanning, delay 1 sec*/        
			xtimeout = RTL_SECONDS_TO_JIFFIES(1);
			goto out;
        }
#endif
#ifdef DFS
		if(timer_pending(&GET_ROOT(priv)->ch_avail_chk_timer)){
			xtimeout = RTL_SECONDS_TO_JIFFIES(3);
			goto out;
		}
#endif
#ifdef SUPPORT_MULTI_PROFILE
		/*change because check_vxd_ap_timer hook on vxd now ; before it hook on root interface jw*/
		if (GET_MIB(priv)->ap_profile.enable_profile && GET_MIB(priv)->ap_profile.profile_num > 0) {
			priv->ss_ssidlen = strlen(GET_MIB(priv)->ap_profile.profile[priv->profile_idx].ssid);
			memcpy(priv->ss_ssid, GET_MIB(priv)->ap_profile.profile[priv->profile_idx].ssid, priv->ss_ssidlen);					
		}
		else
#endif			
		{
			priv->ss_ssidlen = GET_MIB(priv)->dot11StationConfigEntry.dot11SSIDtoScanLen;		
			memcpy(priv->ss_ssid, GET_MIB(priv)->dot11StationConfigEntry.dot11SSIDtoScan, priv->ss_ssidlen);
		}			


		priv->pshare->switch_chan_rp = 0;		
        //STADEBUG("  Call start_clnt_ss\n\n");

        #ifdef MBSSID
        /*if the other STA is connected just scan the same ch with it*/
        if(IS_VAP_INTERFACE(priv))
        {
            /*multiRepeater_connection_status return if other STA connected(return it's channel) or not*/
            channel_tmp1=multiRepeater_connection_status( priv );
            if(channel_tmp1==0){
                //STADEBUG("\n");                
                /*restore available_chnl_num,available_chnl*/                 
                if(priv->MultiSTA_available_backup==1){
                    //STADEBUG("\n");
                    for(i=0;i<priv->MultiSTA_available_chnl_num;i++)
                      priv->available_chnl[i] =  priv->MultiSTA_available_chnl[i];
                    
                    priv->available_chnl_num =  priv->MultiSTA_available_chnl_num;

                    priv->MultiSTA_available_backup = 0;
                }
                
            }else{
                //STADEBUG("channel_tmp1=[%d]\n",channel_tmp1);

                /*backup available_chnl_num,available_chnl*/ 
                if(priv->MultiSTA_available_backup==0){
                    STADEBUG("\n");                    
                    for(i=0;i<priv->available_chnl_num;i++)
                      priv->MultiSTA_available_chnl[i] =  priv->available_chnl[i];
                  
                      priv->MultiSTA_available_chnl_num=priv->available_chnl_num;
                      priv->MultiSTA_available_backup=1;                      
                }                

                /*only scan the channel the same with the other STA*/ 
                priv->available_chnl[0]=channel_tmp1;
                priv->available_chnl_num=1;                
            }
        }
        /*if the other STA is connected just scan the same ch with it*/
        #endif
        
 #ifdef SUPPORT_MULTI_PROFILE
	/* 1) AP disappear,2)AP's ch/2nd ch offset/band width be changed , under these cases do RESCAN instead of DONTRESCAN*/
	if(priv->rescantype==RESCAN_ROAMING){
		priv->ss_req_ongoing = SSFROM_REPEATER_VXD;     // 3 for vxd SiteSurvey case
		priv->rescantype=0;
		start_clnt_ss(priv);	              
	}else 
	if((priv->site_survey->count_target == 0) ||
		((priv->site_survey->count_target > 0) && ((priv->join_index+1) > priv->site_survey->count_target))){

    		priv->ss_req_ongoing = SSFROM_REPEATER_VXD;     // 3 for vxd SiteSurvey case
            STADEBUG("  start_clnt_ss()\n");
    		start_clnt_ss(priv);			
        }else{
            STADEBUG("  start_clnt_lookup(DONTRESCAN)\n");
            start_clnt_lookup(priv, DONTRESCAN);        
        }
#else
        priv->ss_req_ongoing = SSFROM_REPEATER_VXD;     // 3 for vxd SiteSurvey case
		start_clnt_ss(priv);			
#endif		
		xtimeout = 0;
	}
#if 0 // Dont run ss_timer when vxd is associated.	
    else{
        
        /*now vxd STA under WIFI_ASOC_STATE mode ; just hook a timer for check myself status*/
		xtimeout = CHECK_VXD_AP_TIMEOUT;        
    }
#endif	
out:
	if (xtimeout){	
        //STADEBUG("timeout=%d\n",timeout);
		mod_timer(&priv->ss_timer, jiffies + xtimeout);	        
     }	
	RESTORE_INT(flags);
	SMP_UNLOCK(flags);	
    
}

/*
 * reschedule STA( not root interface) to do Scan
 * SMP_LOCK before call set_vxd_rescan()
 */
void set_vxd_rescan(struct rtl8192cd_priv *priv,int rescantype)
{
    /*rescantype://
            RESCAN_BY_NEXTTIME by get_ss_level
            RESCAN_ROAMING:
            because            1) AP disappear,2)AP's ch/2nd ch offset/band width be changed , under these cases do RESCAN
        */
#ifndef SMP_SYNC        
	unsigned long flags = 0;
#endif
	if(IS_ROOT_INTERFACE(priv)){
		return;
	}

	if( OPMODE & WIFI_ASOC_STATE){
		return;
	}

#ifdef SMP_SYNC
	SMP_LOCK_ASSERT();
#endif

	SAVE_INT_AND_CLI(flags);
	//SMP_LOCK(flags);
	if (timer_pending(&priv->ss_timer)) {
		SMP_UNLOCK(flags);
		del_timer_sync(&priv->ss_timer);
		SMP_LOCK(flags);
	}

    if(rescantype==RESCAN_ROAMING){
        STADEBUG("1sec\n");        
        mod_timer(&priv->ss_timer, jiffies + RTL_SECONDS_TO_JIFFIES(1));        
    }else{
          /*    SS_LV_WSTA = 0,                has STA connect to root AP/VAP
                          SS_LV_WOSTA = 1,              no STA connect to root AP/VAP
                          SS_LV_ROOTFUNCOFF = 2,  only root AP and it's func_off=1    */
       	 switch(get_ss_level(priv)) {
	    	case SS_LV_WSTA:
                 /*SS_LV_WSTA = 0,                has STA connect to root AP/VAP*/                 
                STADEBUG("60secs\n");
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
			if((priv->pmib->dot11StationConfigEntry.sc_enabled == 1) && (priv->simple_config_status !=0))
			{
				mod_timer(&priv->ss_timer, jiffies + priv->pmib->dot11StationConfigEntry.sc_vxd_rescan_time);
			}
			else
	    		mod_timer(&priv->ss_timer, jiffies + CHECK_VXD_AP_TIMEOUT);
#else
	    		mod_timer(&priv->ss_timer, jiffies + CHECK_VXD_AP_TIMEOUT);
#endif
		    	break;
    		case SS_LV_ROOTFUNCOFF:
		        STADEBUG("1sec\n");
		    	mod_timer(&priv->ss_timer, jiffies + RTL_SECONDS_TO_JIFFIES(1));
			    break;            
    		default:
				// No STA connected to AP,and AP hasn't func_off,wait a while let DUT's AP can be connected.
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
			if((priv->pmib->dot11StationConfigEntry.sc_enabled == 1) && (priv->simple_config_status !=0))
			{
				mod_timer(&priv->ss_timer, jiffies + priv->pmib->dot11StationConfigEntry.sc_vxd_rescan_time);
			}
			else
                                mod_timer(&priv->ss_timer, jiffies + CHECK_VXD_RUN_DELAY);		
#else
                mod_timer(&priv->ss_timer, jiffies + CHECK_VXD_RUN_DELAY);		
#endif
         }
    }
        
	RESTORE_INT(flags);
	//SMP_UNLOCK(flags);
}

#if 0
void sync_channel_2ndch_bw(struct rtl8192cd_priv *priv)
{
    STADEBUG("\n");
    if(priv->pmib->dot11RFEntry.dot11channel != priv->pshare->switch_chan_rp){
        STADEBUG("ch;My[%d],RAP[%d]\n",priv->pmib->dot11RFEntry.dot11channel , priv->pshare->switch_chan_rp);        
        priv->pmib->dot11RFEntry.dot11channel = priv->pshare->switch_chan_rp;    
    }
    if(priv->pmib->dot11nConfigEntry.dot11n2ndChOffset != priv->pshare->switch_2ndchoff_rp){
        STADEBUG("2nd ch;My[%d],RAP[%d]\n",priv->pmib->dot11nConfigEntry.dot11n2ndChOffset , priv->pshare->switch_2ndchoff_rp);                
        priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = priv->pshare->switch_2ndchoff_rp;    
    }
    if(priv->pmib->dot11nConfigEntry.dot11nUse40M != priv->pshare->band_width_rp){
        STADEBUG("\n");        
        priv->pmib->dot11nConfigEntry.dot11nUse40M = priv->pshare->band_width_rp;
    }
}
#endif /* SMART_REPEATER_MODE */

#endif // defined(UNIVERSAL_REPEATER) || defined(MBSSID)
#endif // CLIENT_MODE

// A dedicated function to check link status
int chklink_wkstaQ(struct rtl8192cd_priv *priv)
{
	int link_status=0;
	if (OPMODE & WIFI_AP_STATE)
	{
		if (priv->assoc_num > 0)
			link_status = 1;
		else
			link_status = 0;
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE)
	{
		if (OPMODE & WIFI_ASOC_STATE) {
#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
			if (IEEE8021X_FUN) {
				struct stat_info *pstat;
				pstat = get_stainfo(priv, BSSID);
				if (pstat && pstat->ieee8021x_ctrlport)
					link_status = 1;
				else
					link_status = 0;
			}
			else {
				link_status = 1;
			}
#else
			link_status = 1;
#endif
		}
		else {
			link_status = 0;
		}
	}
	else if ((OPMODE & WIFI_ADHOC_STATE) &&
		((JOIN_RES == STATE_Sta_Ibss_Active) || (JOIN_RES == STATE_Sta_Ibss_Idle)))
	{
		if (priv->rxBeaconCntArrayWindow < ROAMING_DECISION_PERIOD_ADHOC) {
			if (priv->rxBeaconCntArrayWindow) {
				if (priv->rxBeaconCntArray[priv->rxBeaconCntArrayIdx-1] > 0) {
					link_status = 1;
				}
			}
		}
		else {
			if (priv->rxBeaconPercentage)
				link_status = 1;
			else
				link_status = 0;
		}
	}
#endif
	else
	{
		link_status = 0;
	}

	return link_status;
}


/*cfg p2p cfg p2p rm*/
#ifdef MBSSID
/*under multi-repeater case when some STA has connect , 
 the other STA don't connect to Remote AP that has diff channel or diff 2nd ch  skip this*/ 
int multiRepeater_startlookup_chk(struct rtl8192cd_priv *priv , int db_idx)
{

    struct rtl8192cd_priv *priv_root=NULL;
    struct rtl8192cd_priv *priv_other_va=NULL;
    int target_2ndch=0;
    if(!IS_VAP_INTERFACE(priv))
        return 0;
    if((OPMODE & WIFI_STATION_STATE)==0)
        return 0;
        
    priv_root = GET_ROOT(priv);

    if(priv_root==NULL)
        return 0;    

    /*now we default use wlan0-va1 and wlan0-va2 as STA under multiRepeater mode*/
    if(!strcmp(priv->dev->name,"wlan0-va1") || !strcmp(priv->dev->name,"wlan1-va1")){
        priv_other_va=priv_root->pvap_priv[2];
    }else if(!strcmp(priv->dev->name,"wlan0-va2") || !strcmp(priv->dev->name,"wlan1-va2")){
        priv_other_va=priv_root->pvap_priv[1];
    }
    
    if(priv_other_va && priv_other_va->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE){

         /*get target Remote AP's 2nd ch off set*/
    	if ((priv->site_survey->bss_target[db_idx].t_stamp[1] & (BIT(1) | BIT(2))) == (BIT(1) | BIT(2)))
    		target_2ndch = HT_2NDCH_OFFSET_BELOW;
    	else if ((priv->site_survey->bss_target[db_idx].t_stamp[1] & (BIT(1) | BIT(2))) == BIT(1))
    		target_2ndch = HT_2NDCH_OFFSET_ABOVE;
    	else { 
    		target_2ndch = HT_2NDCH_OFFSET_DONTCARE;
    	}

        /*check ch and 2nd ch off set*/                                
        if( (priv_other_va->pmib->dot11RFEntry.dot11channel != priv->site_survey->bss_target[db_idx].channel)||
            ( priv_other_va->pmib->dot11nConfigEntry.dot11n2ndChOffset != target_2ndch))
        {   
 		   /*                     
            if(   priv_other_va->pmib->dot11nConfigEntry.dot11n2ndChOffset != target_2ndch){
                
                STADEBUG("2nd ch no match other's[%d],target[%d]\n",priv_other_va->pmib->dot11nConfigEntry.dot11n2ndChOffset,target_2ndch);
            }
			*/
                return 1;                   
        }
    }
    else{
        return 0;                            
    }

    STADEBUG("\n");    
    return 0;                            

}

int multiRepeater_connection_status(struct rtl8192cd_priv *priv)
{
    struct rtl8192cd_priv *priv_root=NULL;
    struct rtl8192cd_priv *priv_other_va=NULL;

    if(!IS_VAP_INTERFACE(priv))
        return 0;
    if((OPMODE & WIFI_STATION_STATE)==0)
        return 0;
        
    priv_root = GET_ROOT(priv);

    if(priv_root==NULL)
        return 0;    

	//now we default use wlan0-va1 and wlan0-va2 as STA under multiRepeater mode
    if(!strcmp(priv->dev->name,"wlan0-va1") || !strcmp(priv->dev->name,"wlan1-va1")){
        priv_other_va=priv_root->pvap_priv[2];
    }else if(!strcmp(priv->dev->name,"wlan0-va2") || !strcmp(priv->dev->name,"wlan1-va2")){
        priv_other_va=priv_root->pvap_priv[1];
    }
    
    if(priv_other_va && priv_other_va->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE){
        return priv_other_va->pmib->dot11RFEntry.dot11channel;
    }
    else{
        return 0;                            
    }
}
#endif                      

#ifdef RTK_NL80211//openwrt_psd
extern u2Byte dB_Invert_Table[8][12];
static u4Byte rtl8192cd_convertto_db(u4Byte Value)
{
	u1Byte i;
	u1Byte j;
	u4Byte dB;

	Value = Value & 0xFFFF;
	
	for (i=0;i<8;i++)
	{
		if (Value <= dB_Invert_Table[i][11])
			break;
	}

	if (i >= 8)
		return (96);	// maximum 96 dB

	for (j=0;j<12;j++)
	{
		if (Value <= dB_Invert_Table[i][j])
			break;
	}
	dB = i*12 + j + 1;

	return (dB);
}

static int rtl8192cd_get_psd_data(struct rtl8192cd_priv *priv, unsigned int point)
{
	u4Byte psd_val;

	//2.4G
	if (GET_CHIP_VER(priv)==VERSION_8192E)
	{
		psd_val = RTL_R32(0x808);

		// FFT sample points will be adjusted in "acs_query_psd"
		// set which fft pts we calculate
		psd_val &= 0xFFFFFC00;
		psd_val |= point;
		RTL_W32(0x808, psd_val);

		// Reg 808[22] = 0 ( default )
		// set Reg08[22] = 1, 0->1, PSD activate
		psd_val |= BIT22;		
		RTL_W32(0x808, psd_val);

		delay_ms(1);
		psd_val &= ~(BIT22);
		// set Reg08[22] = 0
		RTL_W32(0x808, psd_val);
		
		psd_val = RTL_R32(0x8B4);
		psd_val &= 0x0000FFFF;
	}
	else if (GET_CHIP_VER(priv)==VERSION_8812E)
	{
		IN PDM_ODM_T pDM_Odm = ODMPTR;
		
		//Set DCO frequency index, offset=(40MHz/SamplePts)*point
		ODM_SetBBReg(pDM_Odm, 0x910, 0x3FF, point);

		//Start PSD calculation, Reg808[22]=0->1
		ODM_SetBBReg(pDM_Odm, 0x910, BIT22, 1);

		//Need to wait for HW PSD report
		delay_us(150);

		ODM_SetBBReg(pDM_Odm, 0x910, BIT22, 0);

		//Read PSD report, Reg8B4[15:0]
		psd_val = (int)ODM_GetBBReg(pDM_Odm,0xf44, bMaskDWord) & 0x0000FFFF;
		psd_val = (int)(rtl8192cd_convertto_db((u4Byte)psd_val));
	}
	
	return psd_val;
}

static int rtl8192cd_query_psd_2g(struct rtl8192cd_priv *priv, unsigned int * data, int fft_pts)
{	
	unsigned int 	regc70, regc7c, reg800, regc14, regc1c, reg522, reg88c, reg804, reg808, regc50;
	int 			regval;
	unsigned int 	psd_pts=0, psd_start=0, psd_stop=0, psd_data=0, i, j;

	//pre seeting for PSD start ---
	regc50 = RTL_R32(0xc50);	// initial gain
	
	//store init value	
	regc70 = RTL_R32(0xc70);	// AGC
	regc7c = RTL_R32(0xc7c);	// AAGC
	reg800 = RTL_R32(0x800);	// CCK
	regc14 = RTL_R32(0xc14);	// IQ matrix A
	
	// 2013-07-08 Jeffery modified
	regc1c = RTL_R32(0xc1c);	// IQ matrix B
	
	reg522 = RTL_R8(0x522);	// MAC queen
	reg88c = RTL_R32(0x88c);	// 3-wire
	reg804 = RTL_R32(0x804);	// PSD ant select
	reg808 = RTL_R32(0x808);	// PSD setting

	//CCK off 
	regval = RTL_R32(0x800);
	regval = regval & (~BIT24);//808[24]=0
	// regval = regval & 0xFEFFFFFF;//808[24]=0
	RTL_W32(0x800, regval);

	// CCA off path A & B, set IQ matrix = 0
	RTL_W32(0xc14, 0x0);
	RTL_W32(0xc1c, 0x0);
	
	//TX off, MAC queen
	RTL_W8(0x522, 0xFF);

	//2 set IGI before 3-wire off

	// caution ! mib uses decimal value
	RTL_W8(0xc50, 0x30);		// default IGI = -30 dBm
	// RTL_W8(0xc50, 0x32);

	//3-wire off
	regval = RTL_R32(0x88c);
	regval = regval | (BIT23|BIT22|BIT21|BIT20) ;
	RTL_W32(0x88c, regval) ;
	 
	//set PSD path  a, b = 0, 1
	//path = 0;
	regval = RTL_R32(0x804);
	regval = regval & (~(BIT4|BIT5));
	// regval = regval & 0xFFFFFFCF;
	RTL_W32(0x804, regval);

	
	regval = RTL_R32(0x808);

	//FFT pts 128, 256, 512, 1024 = 0, 1, 2, 3
#if 1
	if(fft_pts == 128)
		regval = regval & 0xFFFF3FFF;
	else if(fft_pts == 256)
		regval = regval & 0xFFFF7FFF;
	else if(fft_pts == 512)
		regval = regval & 0xFFFFBFFF;
	//else if(fft_pts == 1024)
	//	regval = regval & 0xFFFFFFFF;
	else//default 128
	{
		regval = regval & 0xFFFF3FFF;
		fft_pts = 128;
	}
	
	//set psd pts
	psd_start = fft_pts/2;
	psd_stop = psd_start+fft_pts;
	psd_pts = fft_pts;
#else
	//default set PSD pts 128
	printk("[2G] Now we only support PSD to scan 128 points, you set PSD pts:%d\n",fft_pts);
	regval = regval & 0xFFFF3FFF;
	psd_start = 64;
	psd_stop = 192;
	psd_pts = 128;
#endif

	RTL_W32(0x808, regval);

	
	regval = RTL_R32(0x808);
	regval = regval & ( (~BIT12)|(~BIT13) );
	RTL_W32(0x808, regval);	
	//pre setting for PSD end  ---

	//Get PSD Data
	i = psd_start;
	j = 0;
	while(i<psd_stop) 
	{	
		if( i>= psd_pts) 
			psd_data = rtl8192cd_get_psd_data(priv,(i-psd_pts));
		else
			psd_data = rtl8192cd_get_psd_data(priv,i);
		data[j] = psd_data;		
		i++;j++;
	}

	//rollback settings start ---
	RTL_W32(0xc70, regc70);
	RTL_W32(0xc7c, regc7c);
	RTL_W32(0x800, reg800);
	RTL_W32(0xc14, regc14);
	RTL_W32(0xc1c, regc1c);
	RTL_W8(0x522, reg522);
	RTL_W32(0x88c, reg88c);
	RTL_W32(0x804, reg804);
	RTL_W32(0x808, reg808);
	RTL_W32(0xc50, regc50);
	//rollback settings start ---
	
	return psd_pts;
}

static int rtl8192cd_query_psd_5g(struct rtl8192cd_priv *priv, unsigned int * data, int fft_pts)
{
	IN PDM_ODM_T pDM_Odm = ODMPTR;

	unsigned int 	psd_pts=0, psd_start=0, psd_stop=0, psd_data=0, i, j;
	int 			psd_pts_idx, initial_gain = 0x3e, initial_gain_org;
	

	//pre seeting for PSD start ---
	// Turn off CCK
	ODM_SetBBReg(pDM_Odm, 0x808, BIT28, 0);   //808[28]

	// Turn off TX
	// Pause TX Queue
	if (!priv->pmib->dot11DFSEntry.disable_tx)
		ODM_Write1Byte(pDM_Odm, 0x522, 0xFF); //REG_TXPAUSE set 0xff

	// Turn off CCA
	ODM_SetBBReg(pDM_Odm, 0x838, BIT3, 0x1); //838[3] set 1

	// PHYTXON while loop
	i = 0;
	while (ODM_GetBBReg(pDM_Odm, 0xfa0, BIT18)) {
		i++;
		if (i > 1000000) {
			panic_printk("Wait in %s() more than %d times!\n", __FUNCTION__, i);
			break;
		}
	}

	// backup IGI_origin , set IGI = 0x3e;
	pDM_Odm->bDMInitialGainEnable = FALSE; // setmib dig_enable 0;
	initial_gain_org = ODM_Read1Byte(pDM_Odm, 0xc50);
	ODM_Write_DIG(pDM_Odm, initial_gain);

	delay_us(100);

	// Turn off 3-wire
	ODM_SetBBReg(pDM_Odm, 0xC00, BIT1|BIT0, 0x0); //c00[1:0] set 0

	// pts value = 128=0, 256=1, 512=2, 1024=3
#if 1
	if(fft_pts == 128)
		psd_pts_idx = 0;
	else if(fft_pts == 256)
		psd_pts_idx = 1;
	else if(fft_pts == 512)
		psd_pts_idx = 2;
	//else if(fft_pts == 1024)
	//	psd_pts_idx = 3;
	else//default 128
	{
		psd_pts_idx = 0;
		fft_pts = 128;
	}
	ODM_SetBBReg(pDM_Odm, 0x910, BIT14|BIT15, psd_pts_idx); //910[15:14] set 0, 128 points
	//set psd pts
	psd_start = fft_pts/2;
	psd_stop = psd_start+fft_pts;
	psd_pts = fft_pts;
#else
	printk("[5G] Now we only support PSD to scan 128 points, you set PSD pts:%d\n",fft_pts);
	ODM_SetBBReg(pDM_Odm, 0x910, BIT14|BIT15, 0x0); //910[15:14] set 0, 128 points
	psd_start = 64;
	psd_stop = 192;
	psd_pts = 128;
#endif
	//pre seeting for PSD end ---


	//Get PDS DATA
	i = psd_start;
	j = 0;
	while(i<psd_stop) 
	{	
		if( i>= psd_pts) 
			psd_data = rtl8192cd_get_psd_data(priv,(i-psd_pts));
		else
			psd_data = rtl8192cd_get_psd_data(priv,i);
		data[j] = psd_data;		
		i++;j++;
	}


	//rollback settings start ---
	// CCK on
	ODM_SetBBReg(pDM_Odm, 0x808, BIT28, 1); //808[28]

	// Turn on TX
	// Resume TX Queue
	if (!priv->pmib->dot11DFSEntry.disable_tx)
		ODM_Write1Byte(pDM_Odm, 0x522, 0x00); //REG_TXPAUSE set 0x0

	// Turn on 3-wire
	ODM_SetBBReg(pDM_Odm, 0xc00, BIT1|BIT0, 0x3); //c00[1:0] set 3

	// Restore Current Settings
	// Resume DIG
	pDM_Odm->bDMInitialGainEnable = TRUE; // setmib dig_enable 1;
	ODM_Write_DIG(pDM_Odm, initial_gain_org); // set IGI=IGI_origin

	//Turn on CCA
	ODM_SetBBReg(pDM_Odm, 0x838, BIT3, 0); //838[3] set 0
	//rollback settings end ---
	
	return psd_pts;
}

static int rtl8192cd_query_psd_cfg80211(struct rtl8192cd_priv *priv, int chnl, int bw, int fft_pts)
{
	unsigned int psd_fft_info[1024];//128, 256, 512, 1024
	unsigned int backup_chnl = priv->pmib->dot11RFEntry.dot11channel;
	unsigned int backup_2ndch = priv->pshare->offset_2nd_chan;
	unsigned int backup_bw = priv->pshare->CurrentChannelBW;
	int i, scan_pts=0;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return -1;
	}

	//check channel
	if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
	{
		if(chnl>14 || chnl<1)
		{
			printk("\nFail: channel %d is not in 2.4G\n", chnl);
			return -1;
		}
	}
	else if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
	{
		if(chnl<14)
		{
			printk("\nFail: channel %d is not in 5G\n", chnl);
			return -1;
		}
	}

	memset(psd_fft_info, 0x0, sizeof(psd_fft_info));

	//set chnl
	SwChnl(priv, chnl, HT_2NDCH_OFFSET_BELOW);
	//set BW, PSD always scan 40M, is this necessary ???
	SwBWMode(priv, HT_CHANNEL_WIDTH_20_40, HT_2NDCH_OFFSET_BELOW);


	//query psd data
	if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
	{
		//printk("Scan 2G PSD in CH[%d] cur_CH[%d] fft_pts[%d]\n", chnl, backup_chnl,fft_pts);
		scan_pts = rtl8192cd_query_psd_2g(priv, psd_fft_info, fft_pts);
	}
	else if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
	{
		//printk("Sacn 5G PSD in CH[%d] cur_CH[%d] fft_pts[%d]\n", chnl, backup_chnl, fft_pts);
		scan_pts = rtl8192cd_query_psd_5g(priv, psd_fft_info, fft_pts);
	}

	//rollback chnl and bw
	priv->pmib->dot11RFEntry.dot11channel = backup_chnl;
	priv->pshare->offset_2nd_chan = backup_2ndch;
	priv->pshare->CurrentChannelBW = backup_bw;
	SwChnl(priv,priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
	SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);

	priv->rtk->psd_fft_info[0]=chnl;
	priv->rtk->psd_fft_info[1]=40;//bw;
	priv->rtk->psd_fft_info[2]=scan_pts;
	memcpy(priv->rtk->psd_fft_info+16, psd_fft_info, sizeof(psd_fft_info));

	//for debug
#if 0	
	printk("\n PDS Result:\n");
	for(i=0;i<128;i++)
	{
		if(i%16==0)
			printk("\n");
		printk("%3x ", psd_fft_info[i]);		
	}
	printk("\n");
#endif

	return 0;
}
#endif

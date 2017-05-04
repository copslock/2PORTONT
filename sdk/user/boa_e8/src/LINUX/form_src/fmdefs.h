/*
 *      Include file of form handler
 *      Authors:
 *
 */


#ifndef _INCLUDE_FMDEFS_H
#define _INCLUDE_FMDEFS_H


#include <string.h>
#include "webform.h"
#include "strde.h"		// Mason Yu. IGMP Proxy for e8b


#define _CGI_VER_TW10	1

#define _OPT				0
#define _NEED				1


#define	MAXIFNAMESIZE		30

//#define _CGI_DEBUG_

#define CHECK_INVALID(s)	if(!(s[0])) {lineno = __LINE__; goto check_err;}
#define CHECK_INVALID_OPT(s, opt)	if(opt && (s == NULL || !(s[0]))) {lineno = __LINE__; goto check_err;}
#ifdef _CGI_DEBUG_
#define FETCH_INVALID(s, n)	{s = boaGetVar(wp, n, ""); printf("%s:%s\n", n, s?s:"");CHECK_INVALID(s);}
#define FETCH_INVALID_OPT(s, n, opt){s = boaGetVar(wp, n, ""); printf("%s:%s\n", n, s?s:"");CHECK_INVALID_OPT(s, opt);}
#define _TRACE_CALL			{printf("entering %s....\n", __FUNCTION__);}
#define _TRACE_LEAVEL		{printf("leaving %s.... final line:%d\n", __FUNCTION__, lineno);}
#define _TRACE_POINT		{printf("line:%d\n",  __LINE__);}
#else
#define FETCH_INVALID(s, n)	{s = boaGetVar(wp, n, ""); CHECK_INVALID(s);}
#define FETCH_INVALID_OPT(s, n, opt){s = boaGetVar(wp, n, ""); printf("%s:%s\n", n, s?s:"");CHECK_INVALID_OPT(s, opt);}
#define _TRACE_CALL
#define _TRACE_LEAVEL
#define _TRACE_POINT
#endif

#define _GET_STR(name, opt)		{FETCH_INVALID_OPT(stemp, #name, opt); strncpy(name, stemp, sizeof(name));}
#define _GET_PSTR(name, opt)	{FETCH_INVALID_OPT(p##name, #name, opt);}
#define _GET_IP(name, opt)		{FETCH_INVALID_OPT(stemp, #name, opt); name = inet_addr(stemp);}
#define _GET_INT(name, opt)		{FETCH_INVALID_OPT(stemp, #name, opt); name = atoi(stemp);}
#define _GET_BOOL(name, opt)	{FETCH_INVALID_OPT(stemp, #name, _OPT); name = (strcmp(stemp, "on") == 0 )? 1 : 0;}

#define _PUT_STR(name)			{boaWrite(wp, "cgi."#name" = \"%s\";\n", name);}
#define _PUT_PSTR(name)			{boaWrite(wp, "cgi."#name" = \"%s\";\n", p##name);}
#define _PUT_IP(name)			{boaWrite(wp, "cgi."#name" = \"%s\";\n", inet_ntoa(*(struct in_addr*)&(name)));}
#define _PUT_INT(name)			{boaWrite(wp, "cgi."#name" = %d;\n", name);}
/*ql:20080805 START: if #name == -1, it should be false value, but it is true before.*/
#define _PUT_BOOL(name)			{boaWrite(wp, "cgi."#name" = %s;\n", ((name)==1) ? "true" : "false");}
/*ql:20080805 END*/

#define _PUT_ENTRY_STR(name)	{boaWrite(wp, "cgi."#name" = \"%s\";\n", entry.name);}
#define _PUT_ENTRY_IP(name)		{boaWrite(wp, "cgi."#name" = \"%s\";\n", inet_ntoa(*(struct in_addr*)&(entry.name)));}
#define _PUT_ENTRY_INT(name)	{boaWrite(wp, "cgi."#name" = %d;\n", entry.name);}
#define _PUT_ENTRY_BOOL(name)	{boaWrite(wp, "cgi."#name" = %d;\n", (entry.name) ? 1 : 0);}

#define _ENTRY_STR(name, opt)	{FETCH_INVALID_OPT(stemp, #name, opt); strncpy(entry.name, stemp, sizeof(entry.name));}
#define _ENTRY_IP(name, opt)	{FETCH_INVALID_OPT(stemp, #name, opt); *(unsigned long*)&(entry.name) = (*stemp?inet_addr(stemp):0);}
#define _ENTRY_INT(name, opt)	{FETCH_INVALID_OPT(stemp, #name, opt); entry.name = atoi(stemp);}
#define _ENTRY_BOOL(name, opt)	{FETCH_INVALID_OPT(stemp, #name, _OPT); entry.name = (strcmp(stemp, "on") == 0 )? 1 : 0;}

#define _COND_REDIRECT			{char *s = boaGetVar(wp, "submit-url", ""); if(s && *s) boaRedirect(wp, s);}

#define _ERR_MSG(msg) if(errcode){ \
	boaHeader(wp); \
	boaWrite(wp, "<body><blockquote><h4>%s</h4>\n", msg); \
	boaWrite(wp, "<form><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;%s&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body>",IDS_RESULT_OK); \
	boaFooter(wp); \
	boaDone(wp, 200); \
}
#define _ERR_MSG_WAN(msg,url) if(errcode){ \
	boaHeader(wp); \
	boaWrite(wp, "<body><blockquote><h4>%s</h4>\n", msg); \
	boaWrite(wp, "<form><input type=\"button\" onclick=window.location.replace(\"%s\"  value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body>"),url); \
	boaFooter(wp); \
	boaDone(wp, 200); \
}

#define _OK_MSG(url) if(errcode){ \
	boaHeader(wp); \
	boaWrite(wp, "<body><blockquote><h4>%s</h4>\n",IDS_RESULT_EFFECT); \
	if (url[0]) boaWrite(wp, "<form><input type=button value=\"  %s \" OnClick=window.location.replace(\"%s\"></form></blockquote></body>"), IDS_RESULT_OK,url);\
	else boaWrite(wp, "<form><input type=button value=\"  %s  \" OnClick=window.close()></form></blockquote></body>",IDS_RESULT_OK);\
	boaFooter(wp); \
	boaDone(wp, 200); \
}

#define _OK_MSG1(msg, url) if(errcode){ \
	boaHeader(wp); \
	boaWrite(wp, "<body><blockquote><h4>%s</h4>\n", msg); \
	if (url) boaWrite(wp, "<form><input type=button value=\"  %s \" OnClick=window.location.replace(\"%s\"></form></blockquote></body>"), IDS_RESULT_OK,url);\
	else boaWrite(wp, "<form><input type=button value=\"  %s  \" OnClick=window.close()></form></blockquote></body>",IDS_RESULT_OK);\
	boaFooter(wp); \
	boaDone(wp, 200); \
}

/*
 * for strde
 */
#define _BC_OPT				_OPT
#define _BC_NEED			_NEED

#define _BC_USE				_bcode* bc = NULL; int bcindex = 0; const char *sbc = NULL; float bcv = 0.0f;

#define _BC_INIT(vname)		{\
	FETCH_INVALID_OPT(sbc, vname, _NEED); \
	bc = bc_decode(sbc);\
	if(bc == NULL) {lineno = __LINE__; goto check_err;}\
	}
#define _BC_ISLIST()				(bc->flag & _BC_FLAG_LIST)
#define _BC_ITCNT()					(bc->itcnt)
#define _BC_NEXT()					(bc_seek(bc, bcindex++, 0) >= 0)
#define _BC_FREE()					{if(bc){bc_free(bc); bc = NULL;}}
#ifdef _CGI_DEBUG_
#define _BC_ENTRY_STR(name, opt)	{if((sbc = bc_gets(bc, #name)) == NULL){lineno = __LINE__; continue;} printf("%s:%s\n", #name, sbc?sbc:""); strncpy(entry.name, sbc, sizeof(entry.name));}
#define _BC_ENTRY_IP(name, opt)		{if((sbc = bc_gets(bc, #name)) == NULL){lineno = __LINE__; continue;} printf("%s:%s\n", #name, sbc?sbc:""); entry.name = inet_addr(sbc);}
#define _BC_ENTRY_IPV4(name, opt)	{if((sbc = bc_gets(bc, #name)) == NULL){lineno = __LINE__; continue;} printf("%s:%s\n", #name, sbc?sbc:""); inet_aton(sbc, (struct in_addr *)&entry.name);}
#define _BC_ENTRY_INT(name, opt)	{if(bc_getv(bc, #name, &bcv) == _BC_FALSE){lineno = __LINE__; continue;} printf("%s:%.2f\n", #name, bcv); entry.name = (int)bcv;}
#define _BC_ENTRY_INTX(name, opt)	{if(bc_getv(bc, #name, &bcv) == _BC_FALSE){lineno = __LINE__; continue;} printf("%s:%.2f\n", #name, bcv); name = (int)bcv;}
#else
#define _BC_ENTRY_STR(name, opt)	{if((sbc = bc_gets(bc, #name)) == NULL){lineno = __LINE__; continue;} strncpy(entry.name, sbc, sizeof(entry.name));}
#define _BC_ENTRY_IP(name, opt)		{if((sbc = bc_gets(bc, #name)) == NULL){lineno = __LINE__; continue;} *(unsigned long*)&(entry.name) = inet_addr(sbc);}
#define _BC_ENTRY_IPV4(name, opt)	{if((sbc = bc_gets(bc, #name)) == NULL){lineno = __LINE__; continue;} inet_aton(sbc, (struct in_addr *)&entry.name);}
#define _BC_ENTRY_INT(name, opt)	{if(bc_getv(bc, #name, &bcv) == _BC_FALSE){lineno = __LINE__; continue;} entry.name = (int)bcv;}
#define _BC_ENTRY_INTX(name, opt)	{if(bc_getv(bc, #name, &bcv) == _BC_FALSE){lineno = __LINE__; continue;} name = (int)bcv;}
#endif

typedef enum _itty{_itt_int8, _itt_int16, _itt_int32, _itt_string} _itty;
typedef struct _itval
{
	char* 	key;	/*key name*/
	_itty	ty;		/*value type*/
	int		len;	/*pval size if ty == _itt_string*/
	int*	pval;	/*zero based address*/
	char	opt;	/*is optional ?*/
}_itval;

#define _OFFSETT(type, field)		((unsigned long)(long *)&(((type *)0)->field))
#define _OFFSETO(o, field)			((unsigned long)(long *)&(o->field))
#define _ITDEF(k, t, l, addr, opt)	{k, t, l, addr, opt}

#define _IT_TRUE		1
#define _IT_FALSE		0

#define _PTS			", new it(\"%s\", \"%s\")"
#define _PTI			", new it(\"%s\", %d)"
//jim added to print unsigned long format..
#define _PTUL			", new it(\"%s\", %lu)"
#define _PTULL			", new it(\"%s\", %llu)"

#define _PME(name)		#name, entry.name
#define _PMEX(name)		#name, name
#define _PMEIP(name)	#name, strcpy(name, inet_ntoa(*(struct in_addr*)&(entry.name)))
#define _PMEXIP(name)	#name, strcpy(name, inet_ntoa(*(struct in_addr*)&(name)))

int	it_batfetch(request * wp, char *path, char *query, _itval* pitval);
#define MIB_CHAIN_DELETE(id,type,entry) \
			int cnt=mib_chain_total(id);\
			int mib_index=0;\
			type mibentry;\
			for(mib_index=0;mib_index<cnt;mib_index++)\
			{\
			mib_chain_get(id,mib_index,&mibentry);	\
			if(!memcmp(&mibentry,&entry,sizeof(type)))\
					{\
					mib_chain_delete(id, mib_index);\
					break;\
				}\
			}
/******************************************
**������Ϊ������Զ���ӵ���ʱ�Ľṹ���壬����web_cgi_interface
*/
typedef struct status_info {
	char servName[32];	//����
	char ifname[16];		//�ӿ�( Port/VPI/VCI)
	char encaps[8];		//��װ��ʽ
	char servType[4];		//�������
	char protocol[10];	//Э��
	char ipAddr[16];		//IP��ַ
	unsigned int vlanId;	// VLAN ID
	unsigned int connId;	// Con. ID (It seems constantly to be 1)
	int igmpEnbl;		// IGMP: 1-enable;  0-disable
	int qosEnbl;		// QoS: 1-enable;  0-disable
	int pvcEnbl;		// State: 1-enable;  0-disable
	char *strStatus;		//״̬
} status_info;

/*dhcp�豸��Ϣ*/
typedef struct dhcp_device_info {
	unsigned int uIpAddr;
	char devname[20];	//Computer; ��
} dhcp_device_info;

#if _CGI_VER_TW10 //��ʱ����
typedef struct atmvc_entryx {
	unsigned int ifIndex;	//nop
	unsigned char vpi;	// VPI
	//mapped to "qos" of MIB_CE_ATM_VC_T and recorded in "svtype" of MIB_CE_ATM_VC_T for web display
	/*
		 "UBR Without PCR" and UBR With PCR"  -->  ATMQOS_UBR
		"CBR"          -->       ATMQOS_CBR
		"Non Realtime VBR"  --->    ATMQOS_VBR_NRT
		"Realtime VBR"  --->    ATMQOS_VBR_RT
	*/
	unsigned char svtype;		//�������ͣ�0-"UBR Without PCR", 1-"UBR With PCR", 2-"CBR", 3-"Non Realtime VBR", 4-"Realtime VBR"
	unsigned short vci;	// VCI
	unsigned short pcr;	// ��ֵ��Ԫ����
	unsigned short scr;	// ������Ԫ����
	unsigned short mbs;	// ���ͻ����Ԫ��С
	//unsigned int cdvt;	// nop
	unsigned char encap;	// ��װ��ʽ: 1- LLC;  0- VC- Mux
	unsigned char napt;	// NAT
	unsigned char cmode;	//0- bridge; (1-2)- Route; 2- PPPoE
	unsigned char brmode;	// 0: transparent bridging, 1: PPPoE bridging(PPPoE·���Ż��ģʽ)
	unsigned char AddrMode;            // Bitmap, bit0: Slaac, bit1: Static, bit2: DS-Lite , bit3: 6rd, bit4: DHCP Client
	unsigned char pppUsername[MAX_NAME_LEN];	//�û���
	unsigned char pppPassword[MAX_NAME_LEN];	//����
	unsigned char pppAuth;	// 0:AUTO, 1:PAP, 2:CHAP
	unsigned char pppACName[MAX_NAME_LEN];	// nop
	unsigned char pppServiceName[MAX_NAME_LEN];	//��������
	unsigned char pppCtype;	//���ŷ�ʽ: 0- �Զ�����;  1- ������ʱ�Զ�����
	//unsigned short pppIdleTime;	// nop
	unsigned char ipDhcp;		//0- Static, 1- DHCP
	//unsigned char rip;		// nop
	unsigned char ipAddr[IP_ADDR_LEN];		//IP��ַ
	unsigned char remoteIpAddr[IP_ADDR_LEN];	//ȱʡ����
	unsigned char dgw;		//nop
	unsigned int mtu;			//nop
	//unsigned char enable;		//nop
	unsigned char netMask[IP_ADDR_LEN];	//��������
	//unsigned char ipunnumbered;	// nop
	unsigned char dnsMode;  // 1: enable, 0: disable
	unsigned char v4dns1[IP_ADDR_LEN];
	unsigned char v4dns2[IP_ADDR_LEN];
//#ifdef CONFIG_EXT_SWITCH
	// used for VLAN mapping
	unsigned char vlan;	// ����VLAN
	unsigned short vid;	// VLAN ID
	unsigned short vprio;	// 802.1p priority bits
	unsigned char vpass;	// vlan passthrough
	unsigned short itfGroup;			//bit0- LAN 1; bit1- LAN 2; ��; bit 4- WLAN ROOT; bit5- WLAN SSID1; ...
//#endif
//unsigned long cpePppIfIndex;   // Mason Yu. Remote Management
//unsigned long cpeIpIndex;      // Mason Yu. Remote Management

	unsigned char qos;	// QoS     mapped to "qosenable" of MIB_CE_ATM_VC_T

	unsigned char PPPoEProxyEnable;		//����PPPoE����
	unsigned int  PPPoEProxyMaxUser;	//�����û���
	unsigned int applicationtype;  		//BOTH,INTERNET, TR069, Other
#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
	unsigned char dslite_enable;
	unsigned char dslite_aftr_mode;	//0: auto, 1:manual
	unsigned char dslite_aftr_hostname[64];
#endif
} __PACK__ atmvc_entryx;
#endif

typedef struct dhcpd_entry {
	unsigned int dhcpRangeStart;		//��ʼIP��ַ
	unsigned int dhcpRangeEnd;		//��ֹIP��ַ
	unsigned int ipMask;			//��������
	unsigned int ulTime;			//����ʱ��(��λ: ��)
}dhcpd_entry;


#if _CGI_VER_TW10 //��ʱ����
typedef struct macBaseDhcp_entryx {
	unsigned char macAddr[20];	//MAC��ַ
	unsigned char ipAddr[20];	//IP��ַ
} __PACK__ macBaseDhcp_entryx;
#endif

typedef struct dhcp_client_entry {
	unsigned int pcRangeStart;		//PC��ʼ��ַ
	unsigned int pcRangeEnd;		//PC��ֹ��ַ
	unsigned int cmrRangeStart;		//����ͷ��ʼ��ַ
	unsigned int cmrRangeEnd;		//����ͷ��ֹ��ַ
	unsigned int stbRangeStart;		//��������ʼ��ַ
	unsigned int stbRangeEnd;		//��������ֹ��ַ
	unsigned int phoneRangeStart;
	unsigned int phoneRangeEnd;
}dhcp_client_entry;

#if _CGI_VER_TW10 //��ʱ����
typedef struct rip_entryx {
	unsigned char ifIndex;		//nop
	char ifname[32];			//�ӿ���
	unsigned char version;		//�汾
	unsigned char operation;	//����: 1- Active;  0- Passive
	unsigned char enable;		//����
} __PACK__ rip_entryx;
#endif

typedef struct ipfilter_blacklist_entry {
	char filterName[32];		//��������
	unsigned char protoType;	//Э��(0- ��;  1- TCP/UDP;  2- TCP;  3- UDP;  4- ICMP)
	unsigned int sipStart;		//Դ��ʼIP
	unsigned int sipEnd;		//Դ��ֹIP
	unsigned int smask;		//Դ��������
	unsigned short sportStart;	//Դ��ʼ�˿�
	unsigned short sportEnd;	//Դ��ֹ�˿�
	unsigned int dipStart;		//Ŀ����ʼIP
	unsigned int dipEnd;		//Ŀ����ֹIP
	unsigned int dmask;		//Ŀ����������
	unsigned short dportStart;	//Ŀ����ʼ�˿�
	unsigned short dportEnd;	//Ŀ����ֹ�˿�
} __PACK__ ipfilter_blacklist_entry;

typedef struct ipfilter_whitelist_entry {
	char filterName[32];		//��������
	unsigned char protoType;	//Э��(0- ��;  1- TCP/UDP;  2- TCP;  3- UDP;  4- ICMP)
	unsigned int sipStart;		//Դ��ʼIP
	unsigned int sipEnd;		//Դ��ֹIP
	unsigned int smask;		//Դ��������
	unsigned short sportStart;	//Դ��ʼ�˿�
	unsigned short sportEnd;	//Դ��ֹ�˿�
	unsigned int dipStart;		//Ŀ����ʼIP
	unsigned int dipEnd;		//Ŀ����ֹIP
	unsigned int dmask;		//Ŀ����������
	unsigned short dportStart;	//Ŀ����ʼ�˿�
	unsigned short dportEnd;	//Ŀ����ֹ�˿�
	unsigned char allport;		//ȫѡ
	unsigned char portnum;		//ѡ�ж˿���
	char **ifname;					//ѡ�ж˿���
} __PACK__ ipfilter_whitelist_entry;

#if _CGI_VER_TW10 //��ʱ����
typedef struct ddns_entryx {
	unsigned char provider[10];
	unsigned char hostname[35];
	//unsigned char interface[32]; //interface is a key for compiler
	unsigned char ifname[32];
	unsigned char username[35];
	unsigned char password[35];
} __PACK__ ddns_entryx;
#endif
typedef struct igmproxy_entry {
	unsigned char ifName[40];	//�ӿ���
	unsigned char enable;		//IGMP����
} __PACK__ igmproxy_entry;

typedef struct access_syslog_entry {
	char dateTime[32];		//����/ʱ��
	char severity[16];		//���س̶�
	char *msg;				//��Ϣ
} __PACK__ access_syslog_entry;

/*info*/
/*
extern int getvar(int eid, request * wp, int argc, char ** argv);
extern int cgivar(int eid, request * wp, int argc, char ** argv);
*/

/*status*/

extern int listWanConfig(int eid, request * wp, int argc, char ** argv);
#ifdef CONFIG_IPV6
extern int listWanConfigIpv6(int eid, request * wp, int argc, char ** argv);
#endif
#ifdef SUPPORT_WAN_BANDWIDTH_INFO
int listWanBandwidth(int eid, request * wp, int argc, char **argv);
#endif
extern int wlStatsList(int eid, request * wp, int argc, char **argv);
extern int wlStatus_parm(int eid, request * wp, int argc, char **argv);
extern int E8BPktStatsList(int eid, request * wp, int argc, char **argv);
extern int E8BDhcpClientList(int eid, request * wp, int argc, char **argv);
extern int getifstatus(int eid, request * wp, int argc, char **argv);
extern int addHttpNoCache(int eid, request * wp, int argc, char ** argv);
/*net*/
extern int listWanif(int eid, request * wp, int argc, char ** argv);
extern int initPageAdsl(int eid, request * wp, int argc, char ** argv);
extern int initPageAdsl2(int eid, request * wp, int argc, char ** argv);
#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN)
extern int initPageEth(int eid, request * wp, int argc, char ** argv);
extern int initPageEth2(int eid, request * wp, int argc, char ** argv);
extern void formEthernet(request * wp, char *path, char *query);
extern int initVlanRange(int eid, request * wp, int argc, char ** argv);
#endif
extern int initPageQoSAPP(int eid, request * wp, int argc, char ** argv);
extern int initdgwoption(int eid, request * wp, int argc, char ** argv);
extern void formAdsl(request * wp, char *path, char *query);
extern void formAdslClt(request * wp, char *path, char *query);	//�ͻ���
extern void formSetAdsl(request * wp, char *path, char *query);
extern int init_dsl_page(int eid, request * wp, int argc, char **argv);
extern void formDhcpd(request * wp, char *path, char *query);
extern int init_dhcpmain_page(int eid, request * wp, int argc, char **argv);
//ql_xu
#ifdef _PRMT_X_CT_COM_USERINFO_
extern int initE8clientUserRegPage(int eid, request * wp, int argc, char **argv);
extern int e8clientAccountRegResult(int eid, request * wp, int argc, char **argv);
extern int e8clientAutorunAccountRegResult(int eid, request * wp, int argc, char **argv);
extern void formAccountReg(request * wp, char *path, char *query);
extern int UserAccountRegResult(int eid, request * wp, int argc, char **argv);
extern void formUserReg(request * wp, char *path, char *query);
extern void formUserReg_inside_menu(request * wp, char *path, char *query);
extern int UserRegMsg(int eid, request * wp, int argc, char **argv);
#endif
extern void formIpRange(request * wp, char *path, char *query);
extern int init_dhcp_device_page(int eid, request * wp, int argc, char **argv);
extern void formMacAddrBase(request * wp, char *path, char *query);
extern int showMACBaseTable(int eid, request * wp, int argc, char **argv);
extern void formWlanSetup(request * wp, char *path, char *query);
extern int init_wlan_page(int eid, request * wp, int argc, char **argv);
#ifdef WLAN_MBSSID
extern void formMBSSID(request * wp, char *path, char *query);
extern int init_mbssid_page(int eid, request * wp, int argc, char **argv);
#endif
extern void formWlEncrypt(request * wp, char *path, char *query);
extern int init_wlanAdv_page(int eid, request * wp, int argc, char **argv);
extern void formTR069Config(request * wp, char *path, char *query);
extern void formCertcaInsert(request * wp, char *path, char *query);
extern int showCert(int eid, request * wp, int argc, char **argv);
extern void formTR069CertDel(request * wp, char *path, char *query);
extern void formTimezone(request * wp, char *path, char *query);
extern int init_sntp_page(int eid, request * wp, int argc, char **argv);
extern void formRoute(request * wp, char *path, char *query);
extern void formRip(request * wp, char *path, char *query);
extern int showRipIf(int eid, request * wp, int argc, char **argv);
extern int showStaticRoute(int eid, request * wp, int argc, char **argv);
#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
extern int initPagePBind(int eid, request * wp, int argc, char ** argv);
extern void formVlanMapping(request * wp, char *path, char *query);
#endif
#ifdef CONFIG_MCAST_VLAN
extern int listWanName(int eid, request * wp, int argc, char ** argv);
extern void formMcastVlanMapping(request * wp, char *path, char *query);
#endif

/*security*/
extern int initPageURL(int eid, request * wp, int argc, char ** argv);
extern void formURL(request * wp, char *path, char *query);
extern int initPageFirewall(int eid, request * wp, int argc, char ** argv);
extern void formFirewall(request * wp, char *path, char *query);
extern int initPageDos(int eid, request * wp, int argc, char ** argv);
extern void formDos(request * wp, char *path, char *query);
extern int brgMacFilterList(int eid, request * wp, int argc, char ** argv);
extern void formBrgMacFilter(request * wp, char *path, char *query);
extern int rteMacFilterList(int eid, request * wp, int argc, char ** argv);
extern void formRteMacFilter(request * wp, char *path, char *query);

#ifdef CONFIG_LED_INDICATOR_TIMER
extern void formLedTimer(request * wp, char *path, char *query);
#endif

#ifdef CONFIG_RG_SLEEPMODE_TIMER
extern int initPageSleepModeRule(int eid, request * wp, int argc, char ** argv);
extern void formSleepMode(request * wp, char *path, char *query);
#endif

#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
extern int initPageLanBandwidthMonitor(int eid, request * wp, int argc, char ** argv);
extern void formBandwidthMonitor(request * wp, char *path, char *query);
#endif

#ifdef CONFIG_USER_LAN_BANDWIDTH_CONTROL
extern int initPageBandwidthControl(int eid, request * wp, int argc, char ** argv);
extern void formBandWidth(request * wp, char *path, char *query);
#endif

#ifdef CONFIG_USER_LANNETINFO
extern int initPageLanNetInfo(int eid, request * wp, int argc, char ** argv);
#endif

#if defined (CONFIG_USER_LAN_BANDWIDTH_CONTROL)||defined (CONFIG_USER_LAN_BANDWIDTH_MONITOR)
extern void formBandwidthInterval(request * wp, char *path, char *query);
#endif
extern int ipPortFilterConfig(int eid, request * wp, int argc, char ** argv);
extern int ipPortFilterBlacklist (int eid, request * wp, int argc, char ** argv);
extern int ipPortFilterWhitelist (int eid, request * wp, int argc, char ** argv);
extern void formPortFilter(request * wp, char *path, char *query);
extern void formPortFilterWhite(request * wp, char *path, char *query);
extern void formPortFilterBlack(request * wp, char *path, char *query);


/*management*/
extern int initPageMgmUser(int eid, request * wp, int argc, char ** argv);
extern void formPasswordSetup(request * wp, char *path, char *query);
extern void formReboot(request * wp, char *path, char *query);
extern void formUSBbak(request * wp, char *path, char *query);
extern void formUSBUmount(request * wp, char *path, char *query);
#ifdef CONFIG_USER_RTK_SYSLOG
extern int initPageSysLog(int eid, request * wp, int argc, char ** argv);
extern void formSysLog(request * wp, char *path, char *query);
extern int sysLogList(int eid, request * wp, int argc, char ** argv);
extern int initPageSysLogConfig(int eid, request * wp, int argc, char ** argv);
extern void formSysLogConfig(request * wp, char *path, char *query);
#endif
extern int initPageMntMnt(int eid, request * wp, int argc, char ** argv);
extern int initPagePingResult(int eid, request * wp, int argc, char ** argv);
extern void formDiagTest(request * wp, char *path, char *query);
extern int initPageUSBbak(int eid, request * wp, int argc, char ** argv);



/*application*/
extern int showDNSTable(int eid, request * wp, int argc, char **argv);
extern void formDDNS(request * wp, char *path, char *query);
#ifdef CONFIG_IP_NF_ALG_ONOFF
extern int showAlgConfig(int eid, request * wp, int argc, char **argv);
extern void formALGOnOff(request * wp, char *path, char *query);
#endif
#ifdef DMZ_SUPPORT
extern int showDmzConfig(int eid, request * wp, int argc, char **argv);
extern void formDMZ(request * wp, char *path, char *query);
#endif
#ifdef VIRTUAL_SERVER_SUPPORT
extern int virtualSvrList (int eid, request * wp, int argc, char **argv);
extern int virtualSvrLeft(int eid, request * wp, int argc, char **argv);
extern void formVrtsrv(request * wp, char *path, char *query);
#endif
#ifdef PORT_TRIGGER_SUPPORT
extern int PortTriggerList(int eid, request * wp, int argc, char **argv);
extern void formPortTrigger(request * wp, char *path, char *query);
extern int portTriggerLeft(int eid, request * wp, int argc, char **argv);
#endif
extern int initPageUpnp(int eid, request * wp, int argc, char ** argv);
extern void formUpnp(request * wp, char *path, char *query);
extern void formMLDProxy(request * wp, char *path, char *query);	// Mason Yu. MLD Proxy
extern void formModify(request * wp, char *path, char *query);
extern void formFactory(request * wp, char *path, char *query);
extern void formVendorVersion(request * wp, char *path, char *query);
extern int initHidePage(int eid, request * wp, int argc, char ** argv);
extern int initFactory(int eid, request * wp, int argc, char ** argv);
extern int initVendorVersion(int eid, request * wp, int argc, char ** argv);
extern int initPageIgmpSnooping(int eid, request * wp, int argc, char ** argv);
extern void formMLDSnooping(request * wp, char *path, char *query);
extern int igmproxyList(int eid, request * wp, int argc, char ** argv);
extern void formIgmproxy (request * wp, char *path, char *query);
extern int listUsbDevices(int eid, request * wp, int argc, char ** argv);
extern int initPageStorage(int eid, request * wp, int argc, char ** argv);
extern void formApplication(request * wp, char *path, char *query);
extern void formServiceMgr(request * wp, char *path, char *query);
#ifdef CONFIG_PORT_MIRROR
extern void formPortMirror(request * wp, char *path, char *query);
extern int initPagePortMirror(int eid, request * wp, int argc, char ** argv);
#endif
#define UNDOWAIT 0
#define DOWAIT 1
#define MAX_SPACE_LEGNTH 1024
#define DOCMDINIT \
        char cmdargvs[MAX_SPACE_LEGNTH]={0};\
        int argvs_index=1;\
        char *_argvs[32];
#ifdef _CGI_DEBUG_
#define DOCMDARGVS(cmd,dowait,format,args...) \
		argvs_index=1;\
                 memset(cmdargvs,0,sizeof(cmdargvs));\
                 memset(_argvs,0,sizeof(_argvs));\
                 snprintf(cmdargvs,sizeof(cmdargvs),format , ##args);\
                 fprintf(stderr,"%s %s\n",cmd,cmdargvs);\
                 _argvs[argvs_index]=strtok(cmdargvs," ");\
                 while(_argvs[argvs_index]){\
			_argvs[++argvs_index]=strtok(NULL," ");\
                 	}\
                 do_cmd(cmd,_argvs,dowait);
#else
#define DOCMDARGVS(cmd,dowait,format,args...) \
		argvs_index=1;\
                 memset(cmdargvs,0,sizeof(cmdargvs));\
                 memset(_argvs,0,sizeof(_argvs));\
                 snprintf(cmdargvs,sizeof(cmdargvs),format , ##args);\
                 _argvs[argvs_index]=strtok(cmdargvs," ");\
                 while(_argvs[argvs_index]){\
			_argvs[++argvs_index]=strtok(NULL," ");\
                 	}\
                 do_cmd(cmd,_argvs,dowait);
#endif

#endif // _INCLUDE_FMDEFS_H

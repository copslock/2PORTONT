/*
 *      Web server handler routines for IP QoS
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"

#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../uClibc/include/linux/autoconf.h"
#endif

#undef QOS_DEBUG
static int function_print = 1;
static int debug_print    = 1;

#define QUEUE_RULE_NUM_MAX     256
#define TRAFFICTL_RULE_NUM_MAX 256
#define INF_NAME_MAX           16

#define DELIM      '&'
#define SUBDELIM   '|'
#define SUBDELIM1  "|"

//print debug information
#ifdef QOS_DEBUG
#define PRINT_FUNCTION do{ if(function_print) printf("%s: %s\n", __FILE__, __FUNCTION__);}while(0);
#define QOS_DEBUG(fmt, args...) do{if(debug_print) printf("QOS DEBUG: " fmt, ##args);}while(0)

#define  PRINT_QUEUE(pEntry)  \
    printf("[QUEUE]: ifIndex:%d, ifname:%s, desc:%s, prio:%d, queueKey:%d, enable:%d\n",  \
	    pEntry->ifIndex, pEntry->ifname, pEntry->desc, pEntry->prio, pEntry->queueKey, pEntry->enable);

#define PRINT_QUEUE_RULE(pEntry)  \
   printf("[QUEUE RULE]: ifidx:0x%x, name:%s, state:%d, prio:%d, mark dscp:0x%02x,\n"      \
	  "mark 8021p:%d, dscp:0x%02x, vlan 8021p:%d, phyPort:%d, prototype:%d, "    \
	  "srcip:%s, smaskbits:%d, dstip:%s, dmaskbits:%d, src port:%d-%d, dst port:%d-%d\n",               \
	   pEntry->outif, pEntry->RuleName, pEntry->enable, pEntry->prior, \
	   pEntry->m_dscp, pEntry->m_1p, pEntry->qosDscp, pEntry->vlan1p, pEntry->phyPort, pEntry->protoType,  \
	   inet_ntoa(*((struct in_addr*)&pEntry->sip)), pEntry->smaskbit, \
	   inet_ntoa(*((struct in_addr*)&pEntry->dip)), pEntry->dmaskbit, pEntry->sPort,          \
	   pEntry->sPortRangeMax, pEntry->dPort, pEntry->dPortRangeMax);

#define PRINT_TRAFFICTL_RULE(pEntry) \
    printf("[TRAFFIC CONTROL]: entryid:%d, ifIndex:%d, srcip:%s, smaskbits:%d, dstip:%s, dmaskbits:%d,"  \
	   "sport:%d, dport%d, protoType:%d, limitspeed:%d\n",                                                      \
	    pEntry->entryid, pEntry->ifIndex, inet_ntoa(*((struct in_addr*)&pEntry->srcip)),        \
	    pEntry->smaskbits, inet_ntoa(*((struct in_addr*)&pEntry->dstip)), pEntry->dmaskbits,                    \
	    pEntry->sport, pEntry->dport, pEntry->protoType, pEntry->limitSpeed);

#else

#define PRINT_FUNCTION do{}while(0);
#define QOS_DEBUG(fmt, args...) do{}while(0)
#define PRINT_QUEUE(pEntry)
#define PRINT_QUEUE_RULE(pEntry)
#define PRINT_TRAFFICTL_RULE(pEntry)

#endif

//show the wan interface list, using js code, must have waniflist array in js code
int ifWanList_tc(int eid, request * wp, int argc, char **argv)
{
	MIB_CE_ATM_VC_T entry;
	int entryNum = 0, i=0, nBytes = 0;
	char wanif[IFNAMSIZ] = {0};

	PRINT_FUNCTION

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	QOS_DEBUG("Total entry num:%d\n", entryNum);

	//default
	nBytes += boaWrite(wp, "waniflst.add(new it(\" \", \" \"));");

	for(i=0;i<entryNum;i++)
	{
		// Kaohj --- E8 don't care enableIpQos.
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, &entry)||!entry.enableIpQos)
		//if(!mib_chain_get(MIB_ATM_VC_TBL, i, &entry))
		    continue;

		//getWanName(&entry, wanif);
		ifGetName(entry.ifIndex, wanif, sizeof(wanif));
#ifndef BR_ROUTE_ONEPVC
		nBytes += boaWrite(wp, "waniflst.add(new it(\"%d|%s\", \"%s\"));",
			entry.ifIndex, wanif, wanif);
#else
		nBytes += boaWrite(wp, "waniflst.add(new it(\"%d|%d|%s\", \"%s\"));",
			entry.ifIndex, entry.cmode, wanif, wanif);
#endif
	}

	return nBytes;
}

int initQosLanif(int eid, request * wp, int argc, char **argv)
{
	int i;
	int nBytes = 0;
	
	for (i=0; i < SW_LAN_PORT_NUM; i++) {
		nBytes += boaWrite(wp, "iffs.add(new it(%d, \"LAN_%d\"));\n", i+1, i+1);
	}
	return nBytes;
}

int initOutif(int eid, request * wp, int argc, char **argv)
{
	MIB_CE_ATM_VC_T vcEntry;
	int entryNum, i, j, nBytes = 0;
	//char wanif[MAX_WAN_NAME_LEN]={0};
	char wanif[IFNAMSIZ]={0};
#ifdef BR_ROUTE_ONEPVC
	unsigned char pvcmode;
#endif
	PRINT_FUNCTION

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for(i=0; i<entryNum; i++)
	{
		// Kaohj --- E8 don't care enableIpQos.
		//if(!mib_chain_get(MIB_ATM_VC_TBL, i, &vcEntry)||!vcEntry.enableIpQos)
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, &vcEntry))
			continue;

		//getWanName(&vcEntry, wanif);
		ifGetName(vcEntry.ifIndex, wanif, sizeof(wanif));
#ifdef BR_ROUTE_ONEPVC
		pvcmode = vcEntry.cmode;
#endif

#ifdef BR_ROUTE_ONEPVC
		nBytes += boaWrite(wp, "oifkeys.add(new it(\"%d|%d\", \"%s\"));\n",
			vcEntry.ifIndex, pvcmode, wanif);
#else
		nBytes += boaWrite(wp, "oifkeys.add(new it(\"%d\", \"%s\"));\n",
			vcEntry.ifIndex, wanif);
#endif
	}
	return nBytes;
}

int initRulePriority(int eid, request * wp, int argc, char **argv)
{
	int j, nBytes = 0;

	PRINT_FUNCTION

    // only show priority. The entryNum +1 is priority.
	for (j=1; j<=4; j++)
		nBytes += boaWrite(wp, "quekeys.add(new it(\"%d\", \"Queue %d\"));\n", j , j);

	return nBytes;
}

char* netmaskbits2str(const int netmaskbit, char* netmaskstr, int strlen)
{
	unsigned int netmaskaddr = 0, i=0;

	if(!netmaskstr || !netmaskbit)
		return NULL;
	for(i=0;i<netmaskbit;i++)
		netmaskaddr = netmaskaddr|(0x80000000>>i);
	netmaskaddr = htonl(netmaskaddr);//host byte order to network byte order
	strncpy(netmaskstr, inet_ntoa(*(struct in_addr*)&netmaskaddr), strlen );

	return netmaskstr;
}

/******************************************************************
 * NAME:    intPageQosRule
 * DESC:    initialize the qos rules by reading mib setting and
 *          format them to send them to webs to dispaly
 * ARGS:
 * RETURN:
 ******************************************************************/
int initQosRulePage(int eid, request * wp, int argc, char **argv)
{
	MIB_CE_IP_QOS_T qEntry;
	MIB_CE_ATM_VC_T       vcEntry;
	char saddr[16]={0}, daddr[16]={0}, smask[16]={0}, dmask[16]={0};
	char smacaddr[20], dmacaddr[20];
	int i=0, qEntryNum = 0, vcEntryNum = 0, nBytes = 0;
	char wanifname[16]={0};
#ifdef CONFIG_IPV6
	unsigned char 	sip6Str[48]={0}, dip6Str[48]={0};
#endif

#ifdef CONFIG_BRIDGE_EBT_DHCP_OPT
	char duid_mac[20]={0};
#endif

	PRINT_FUNCTION

	//get number of  ip qos queue rules, if none, or cannot get, return
	if((qEntryNum=mib_chain_total(MIB_IP_QOS_TBL)) <=0)
		return;

	if((vcEntryNum=mib_chain_total(MIB_ATM_VC_TBL)) <=0)
		return;

	for(i=0;i<qEntryNum; i++)
	{
		char phyportName[8] = {0};
		if(!mib_chain_get(MIB_IP_QOS_TBL, i, (void*)&qEntry))
			continue;

		//src addr
		snprintf(saddr, 16, "%s", inet_ntoa(*((struct in_addr*)&(qEntry.sip))));

		//src subnet mask
		if(!netmaskbits2str(qEntry.smaskbit, smask, 16))
		{
			smask[0] = '\0';
		}

		//dst addr
		snprintf(daddr, 16, "%s", inet_ntoa(*((struct in_addr*)&(qEntry.dip))));
		//dst subnet mask
		if(!netmaskbits2str(qEntry.dmaskbit, dmask, 16))
		{
			dmask[0] = '\0';
		}

#ifdef CONFIG_IPV6
		inet_ntop(PF_INET6, (struct in6_addr *)qEntry.sip6, sip6Str, sizeof(sip6Str));
		inet_ntop(PF_INET6, (struct in6_addr *)qEntry.dip6, dip6Str, sizeof(dip6Str));
#endif

		// src mac
		snprintf(smacaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
			qEntry.smac[0], qEntry.smac[1],
			qEntry.smac[2], qEntry.smac[3],
			qEntry.smac[4], qEntry.smac[5]);

		// dst mac
		snprintf(dmacaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
			qEntry.dmac[0], qEntry.dmac[1],
			qEntry.dmac[2], qEntry.dmac[3],
			qEntry.dmac[4], qEntry.dmac[5]);

#ifdef CONFIG_BRIDGE_EBT_DHCP_OPT
		//ifdef DHCPOPT
		// duid mac
		snprintf(duid_mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
			qEntry.duid_mac[0], qEntry.duid_mac[1],
			qEntry.duid_mac[2], qEntry.duid_mac[3],
			qEntry.duid_mac[4], qEntry.duid_mac[5]);
#endif

		if(qEntry.outif==65535)
			strcpy(wanifname,"Any");
		else
			ifGetName(qEntry.outif,wanifname,sizeof(wanifname));

		//now write into webs using boaWrite function
		nBytes += boaWrite(wp, "rules.push(\n"
			"new it_nr(\"%s\",    \n"  //qos queue rule name
			"new it(\"index\",%d),\n"  //index of queue rule(identifier)
			"new it(\"state\",%d),\n"  //enable or disable
			"new it(\"prio\", \"%d\"),\n"  //queue priority, queueKey
#ifndef BR_ROUTE_ONEPVC
			"new it(\"outif\", \"%d\"),\n"  //queue priority, queueKey
#else
			"new it(\"outif\", \"%d|%d\"),\n"  //queue priority, queueKey
#endif
			"new it(\"wanifname\",  \"%s\"),\n" //source ip6
#ifdef CONFIG_IPV6
			//"new it(\"ipversion\",%d),\n"  //ipv4 or ipv6
			"new it(\"IpProtocolType\",%d),\n"  //ipv4 or ipv6
			"new it(\"sip6\",  \"%s\"),\n" //source ip6
			"new it(\"dip6\",  \"%s\"),\n" //dst ip6
			"new it(\"sip6PrefixLen\",%d),\n"  //source ip6 Prefix Len
			"new it(\"dip6PrefixLen\",%d),\n"  //dst ip6 Prefix Len
#endif
			"new it(\"mvid\",%d),\n"   //VLAN ID
			"new it(\"mdscp\",%d),\n"   //dscp mark
			"new it(\"m1p\",  %d),\n"     //802.1p mark for wan interface
			"new it(\"vlan1p\",%d),\n"    //802.1p match for packet
			"new it(\"ethType\",\"%04x\"),\n"    //Ethernet Type match for packet
			"new it(\"phypt\", %d),\n"    //Lan phy port number
			"new it(\"proto\",%d),\n"     //protocol index, reference to mib.h
			"new it(\"dscp\", %d),\n"     //dscp
			"new it(\"sip\",  \"%s\"),\n" //source ip, if stype=1, it is DHCP OPT 60
			"new it(\"smsk\", \"%s\"),\n" //source ip subnet mask
			"new it(\"dip\",  \"%s\"),\n"
			"new it(\"dmsk\", \"%s\"),\n"
			"new it(\"spts\", %d),\n"     //source port start
			"new it(\"spte\", %d),\n"     //source port end
			"new it(\"dpts\", %d),\n"
			"new it(\"dpte\", %d),\n"
			"new it(\"dhcpopt_type_select\", \"%d\"),\n"
			"new it(\"opt60_vendorclass\", \"%s\"),\n"
			"new it(\"opt61_iaid\", \"%d\"),\n"
			"new it(\"dhcpopt61_DUID_select\", \"%d\"),\n"
			"new it(\"duid_hw_type\", \"%d\"),\n"
			"new it(\"duid_mac\", \"%s\"),\n"
			"new it(\"duid_time\", \"%d\"),\n"
			"new it(\"duid_ent_num\", \"%d\"),\n"
			"new it(\"duid_ent_id\", \"%s\"),\n"
			"new it(\"opt125_ent_num\", \"%d\"),\n"
			"new it(\"opt125_manufacturer\", \"%s\"),\n"
			"new it(\"opt125_product_class\", \"%s\"),\n"
			"new it(\"opt125_model\", \"%s\"),\n"
			"new it(\"opt125_serial\", \"%s\"),\n"
			"new it(\"smac\", \"%s\"), \n"  //source mac address,now supported now, always 00:00:00:00:00:00
			"new it(\"smacw\",\"%s\"), \n"  //source mac address wildword
			"new it(\"dmac\", \"%s\"), \n"
			"new it(\"dmacw\",\"%s\")));\n",
			qEntry.RuleName,  i, !!(qEntry.enable),
			qEntry.prior,
#ifndef BR_ROUTE_ONEPVC
			qEntry.outif,
#else
			qEntry.outif, qEntry.cmode,
#endif
			wanifname,
#ifdef CONFIG_IPV6
			qEntry.IpProtocol, sip6Str, dip6Str, qEntry.sip6PrefixLen, qEntry.dip6PrefixLen,
#endif
			qEntry.m_vid,qEntry.m_dscp, qEntry.m_1p, qEntry.vlan1p, *(unsigned short *)&(qEntry.ethType), qEntry.phyPort, qEntry.protoType, qEntry.qosDscp,
			saddr, smask, daddr, dmask, qEntry.sPort,
			qEntry.sPortRangeMax, qEntry.dPort, qEntry.dPortRangeMax,
#ifdef CONFIG_BRIDGE_EBT_DHCP_OPT
			qEntry.dhcpopt_type,qEntry.opt60_vendorclass,qEntry.opt61_iaid,
			qEntry.opt61_duid_type,qEntry.duid_hw_type,duid_mac,
			qEntry.duid_time,qEntry.duid_ent_num,qEntry.duid_ent_id,
			qEntry.opt125_ent_num,qEntry.opt125_manufacturer,
			qEntry.opt125_product_class,qEntry.opt125_model,qEntry.opt125_serial,
#else
			0,0,0,
			0,0,"",
			0,0,"",
			0,"",
			"","","",
#endif
			smacaddr, "00:00:00:00:00:00", dmacaddr, "00:00:00:00:00:00");
	}

	return nBytes;
}

//Used to get subnet mask bit number
static int getNetMaskBit(char* netmask)
{
	unsigned int bits = 0, mask = 0;
	int i=0, flag = 0;

	if(!netmask||strlen(netmask)>15)
		return 0;
	mask = inet_network(netmask);
	for(;i<32;i++)
	{
		if(mask&(0x80000000>>i)) {
			if(flag)
				return 0;
			else
				bits++;
		}
		else {
	    		flag = 1;
		}
	}
	return bits;
}

/*
 * Return index of this rule
 * -1 on fail
 */
static int parseRuleArgs(char* args, MIB_CE_IP_QOS_Tp pEntry)
{
	char* p=NULL, *tmp=NULL, buff[32] = {0};
	int idx = 0;
	int ret;
#ifdef CONFIG_IPV6
	char buff2[48] = {0};
	struct in6_addr ip6Addr;
#endif

	//get index
	p = strstr(args, "index=");
	p +=strlen("index=");
	if(*p == DELIM) {
		//pEntry->index = 0;
		ret = 0;
	} else {
		//pEntry->index = strtol(p, &tmp, 0);
		ret = strtol(p, &tmp, 0);
		if(*tmp != DELIM) return -1;
	}

	//get RuleName
	p = strstr(args, "name=");
	for(p+=strlen("name="); *p!=DELIM&&idx<INF_NAME_MAX;p++)
		pEntry->RuleName[idx++] = *p;

	//state
#if 1 // always enabled
	pEntry->enable = 1;
#else
	p = strstr(args, "state=");
	p += strlen("state=");
	if(*p != DELIM) {
		pEntry->enable = strtol(p, &tmp, 0);
		if(*tmp != DELIM)
			pEntry->enable = 1;//default
	} else {
		pEntry->enable = 1;//default
	}
#endif

	//get prio
	p = strstr(args, "prio=");
	p += strlen("prio=");
	if(*p==DELIM) return 1;
	pEntry->prior = strtol(p, &tmp, 0);
	if(*tmp != DELIM) return -1;//invalid dscp value

/* Remove WANInterface
	//get ifIndex, cmode
	p = strstr(args, "outif=");
	p += strlen("outif=");
	if(*p==SUBDELIM||*p==DELIM) return 1;
	//ifIndex
	pEntry->outif = strtol(p, &tmp, 0);
	// Mason Yu. t123
	//if(*tmp != SUBDELIM) return -1;
#ifdef BR_ROUTE_ONEPVC
	if(*tmp != SUBDELIM) return -1;
	pEntry->cmode = strtol(++tmp, &p, 0);
	if (*p != DELIM) return -1;
#else
	if(*tmp != DELIM) return -1;
#endif
*/

	/*
	   printf("[%s:%d]\n",__func__,__LINE__);
	//get vid
	p = strstr(args, "markvid=");
	p += strlen("markvid=");
	if(*p == DELIM)//default
	pEntry->m_vid = 0;
	else {
	pEntry->m_vid = strtol(p, &tmp, 10);
	if(*tmp != DELIM) return -1;//invalid vid value
	}
	*/
	//mark 802.1p
	p = strstr(args, "mark1p=");
	p += strlen("mark1p=");
	if(*p == DELIM)//default
		pEntry->m_1p = 0;
	else {
		pEntry->m_1p = strtol(p, &tmp, 16);
		if(*tmp != DELIM) return -1;//invalid 802.1p value
	}

	//mark dscp
	p = strstr(args, "markdscp=");
	p += strlen("markdscp=");
	if(*p == DELIM)//default
		pEntry->m_dscp = 0;
	else {
		pEntry->m_dscp = strtol(p, &tmp, 0);
		if(*tmp != DELIM) return -1;//invalid dscp value
	}

	//protocol
	p = strstr(args, "proto=");
	p += strlen("proto=");
	if(*p == DELIM) {//default, none
		pEntry->protoType = 0;
	} else {
		pEntry->protoType = strtol(p, &tmp, 0);
		if(*tmp != DELIM) return -1;
	}

	//smac
	p = strstr(args, "smac=");
	p += strlen("smac=");
	if (*p==DELIM) {//default
		memset(pEntry->smac, 0, MAC_ADDR_LEN);
	} else {
		for(idx=0; *p != DELIM&&idx<32; ++p) {
			if (*p!=':')
				buff[idx++] = *p;
		}
		string_to_hex(buff, pEntry->smac, 12);
	}

	//dmac
	p = strstr(args, "dmac=");
	p += strlen("dmac=");
	if (*p==DELIM) {//default
		memset(pEntry->dmac, 0, MAC_ADDR_LEN);
	} else {
		for(idx=0; *p != DELIM&&idx<32; ++p) {
			if (*p!=':')
				buff[idx++] = *p;
		}
		string_to_hex(buff, pEntry->dmac, 12);
	}

	//vlan 802.1p match
	p = strstr(args, "vlan1p=");
	p += strlen("vlan1p=");
	if(*p=='\0') {//default
		pEntry->vlan1p =0;
	} else {
		pEntry->vlan1p = strtol(p, &tmp, 0);
		if(*tmp != '\0') return -1;
	}

#ifdef CONFIG_IPV6
	//IPVersion
	p = strstr(args, "IPversion=");
	p += strlen("IPversion=");
	if(*p == DELIM)//default
		pEntry->IpProtocol = IPVER_IPV4_IPV6;
	else {
		pEntry->IpProtocol = strtol(p, &tmp, 0);
		if(*tmp != DELIM) return -1;//invalid dscp value
	}

	// If this is a IPv4 rule
	if ( pEntry->IpProtocol == IPVER_IPV4) {
#endif
		//ip source address
		p = strstr(args, "sip=");
		p += strlen("sip=");
		if(*p==DELIM) {//default
			memset(pEntry->sip, 0, IP_ADDR_LEN);
		} else {
			for(idx=0; *p != DELIM&&idx<32; ++p)
				buff[idx++] = *p;
			buff[idx] = '\0';
			{//ip address
				inet_aton(buff, (struct in_addr *)&pEntry->sip);
			}
		}

		//source ip netmaskbit
		p = strstr(args, "smask=");
		p += strlen("smask=");
		if(*p==DELIM) {//default
			pEntry->smaskbit =0;
		} else {
			for(idx=0; *p != DELIM&&idx<32; ++p)
				buff[idx++] = *p;
			buff[idx] = '\0';
			pEntry->smaskbit = getNetMaskBit(buff);
		}

		//ip dest address
		p = strstr(args, "dip=");
		p += strlen("dip=");
		if(*p==DELIM) {//default
			memset(pEntry->dip, 0, IP_ADDR_LEN);
		} else {
			for(idx=0; *p != DELIM&&idx<32; ++p)
				buff[idx++] = *p;
			buff[idx] = '\0';
			{//ip address
				inet_aton(buff, (struct in_addr *)&pEntry->dip);
			}
		}

		//destination ip netmaskbit
		p = strstr(args, "dmask=");
		p += strlen("dmask=");
		if(*p==DELIM) {//default
			pEntry->dmaskbit =0;
		} else {
			for(idx=0; *p != DELIM&&idx<32; ++p)
				buff[idx++] = *p;
			buff[idx] = '\0';
			pEntry->dmaskbit = getNetMaskBit(buff);
		}
#ifdef CONFIG_IPV6
	}
#endif

#ifdef CONFIG_IPV6
	// If it is a IPv6 rule.
	if ( pEntry->IpProtocol == IPVER_IPV6 )
	{
		//ip6 source address
		p = strstr(args, "sip6=");
		p += strlen("sip6=");
		if(*p==DELIM) {//default
			memset(pEntry->sip6, 0, IP6_ADDR_LEN);
		} else {
			for(idx=0; *p != DELIM&&idx<48; ++p)
				buff2[idx++] = *p;
			buff2[idx] = '\0';
			{//ip address
				inet_pton(PF_INET6, buff2, &ip6Addr);
				memcpy(pEntry->sip6, &ip6Addr, sizeof(pEntry->sip6));
			}
		}

		//ip6 dest address
		p = strstr(args, "dip6=");
		p += strlen("dip6=");
		if(*p==DELIM) {//default
			memset(pEntry->dip6, 0, IP6_ADDR_LEN);
		} else {
			for(idx=0; *p != DELIM&&idx<48; ++p)
				buff2[idx++] = *p;
			buff2[idx] = '\0';
			{//ip address
				inet_pton(PF_INET6, buff2, &ip6Addr);
				memcpy(pEntry->dip6, &ip6Addr, sizeof(pEntry->dip6));
			}
		}

		// ip6 src IP prefix Len
		p = strstr(args, "sip6PrefixLen=");
		p += strlen("sip6PrefixLen=");
		if(*p == DELIM)//default
			pEntry->sip6PrefixLen = 0;
		else {
			pEntry->sip6PrefixLen = strtol(p, &tmp, 0);
			if(*tmp != DELIM) return -1;//invalid dscp value
		}

		// ip6 dst IP prefix Len
		p = strstr(args, "dip6PrefixLen=");
		p += strlen("dip6PrefixLen=");
		if(*p == DELIM)//default
			pEntry->dip6PrefixLen = 0;
		else {
			pEntry->dip6PrefixLen = strtol(p, &tmp, 0);
			if(*tmp != DELIM) return -1;//invalid dscp value
		}
	}
#endif
	//src port start
	p = strstr(args, "spts=");
	p += strlen("spts=");
	if(*p==DELIM) {//default
		pEntry->sPort =0;
	} else {
		pEntry->sPort = strtol(p, &tmp, 0);
		if(*tmp != DELIM) return -1;
	}

	//src port end
	p = strstr(args, "spte=");
	p += strlen("spte=");
	if(*p==DELIM) {//default
		pEntry->sPortRangeMax =0;
	} else {
		pEntry->sPortRangeMax = strtol(p, &tmp, 0);
		if(*tmp != DELIM) return -1;
	}


	//dst port start
	p = strstr(args, "dpts=");
	p += strlen("dpts=");
	if(*p==DELIM) {//default
		pEntry->dPort =0;
	} else {
		pEntry->dPort = strtol(p, &tmp, 0);
		if(*tmp != DELIM) return -1;
	}

	//dst port end
	p = strstr(args, "dpte=");
	p += strlen("dpte=");
	if(*p==DELIM) {//default
		pEntry->dPortRangeMax =0;
	} else {
		pEntry->dPortRangeMax = strtol(p, &tmp, 0);
		if(*tmp != DELIM) return -1;
	}	
	//dscp match
	p = strstr(args, "matchdscp=");
	p += strlen("matchdscp=");
	if(*p == DELIM) {//default
		pEntry->qosDscp = 0;
	} else {
		pEntry->qosDscp = strtol(p, &tmp, 0);
		if(*tmp != DELIM) return -1;
	}
	//phy port num
	p = strstr(args, "phyport=");
	p +=strlen("phyport=");
	if(*p == DELIM) {
		pEntry->phyPort = 0;//default phy port, none
	} else {
		pEntry->phyPort = strtol(p, &tmp, 0);
		if(*tmp != DELIM) return 1;
	}
	return ret;
}
//parse formated string recieved from web client:
//inf=VAL&proto=VAL&srcip=VAL&srcnetmask=VAL&dstip=VAL&dstnetmask=VAL&sport=VAL&dport=VAL&uprate=VAL(old).
//where VAL is the value of the corresponding item. if none, VAL is ignored but the item name cannot be
//The argument action is not modified by the function, so it can be a const string or non-const string.
static int parseArgs(char* action, MIB_CE_IP_TC_Tp pEntry)
{
	char* p = NULL, *tmp = NULL;
	int i = 0;
#ifdef CONFIG_IPV6
	int idx = 0;
	char buff2[48] = {0};
	struct in6_addr ip6Addr;
#endif

	PRINT_FUNCTION

	//ifIndex
	tmp = strstr(action, "inf=");
	tmp += strlen("inf=");
	pEntry->ifIndex = strtol(tmp, &p,0);	
	
	if(*p != SUBDELIM)
	//if(*p != DELIM)
		return 1;
#ifdef BR_ROUTE_ONEPVC
	pEntry->cmode = strtol(++p, &tmp, 0);
	if (*tmp != DELIM)
	//if(*p != DELIM)
	{
		return 1;
	}
#endif

	//protocol
	tmp =strstr(action, "proto=");
	tmp += strlen("proto=");
	if(!tmp||*tmp == DELIM)//not set protocol, set it to default,none
		pEntry->protoType = 0;
	else
	{
		pEntry->protoType = strtol(tmp, &p, 0);
		if(*p != DELIM)
			return 1;
	}

#ifdef CONFIG_IPV6
	//IPVersion
	tmp = strstr(action, "IPversion=");
	tmp += strlen("IPversion=");
	if(*tmp == DELIM)//default
		pEntry->IpProtocol = IPVER_IPV4_IPV6;
	else {
		pEntry->IpProtocol = strtol(tmp, &p, 0);
		if(*p != DELIM) return -1;//invalid dscp value
	}

	// If this is a IPv4 rule
	if ( pEntry->IpProtocol == IPVER_IPV4) {
#endif
		//source ip
		tmp = strstr(action, "srcip=");
		tmp += strlen("srcip=");
		if(!tmp||*tmp == DELIM)//noet set, set default
			memset(pEntry->srcip, 0, IP_ADDR_LEN);
		else
		{
			char sip[16]={0};
			p = strchr(tmp, DELIM);
			if(p&&p-tmp>15)
				return 1;
			strncpy(sip, tmp, p-tmp);
			inet_aton(sip, (struct in_addr *)&pEntry->srcip);
		}

		//source ip address netmask
		tmp = strstr(action, "srcnetmask=");
		tmp += strlen("srcnetmask=");
		if(!tmp||*tmp==DELIM)
			pEntry->smaskbits = 0;
		else
		{
			char smask[16]={0};
			p = strchr(tmp, DELIM);
			if(p&&p-tmp>15) return 1;
			strncpy(smask, tmp, p-tmp);
			pEntry->smaskbits = getNetMaskBit(smask);
		}

		//destination ip
		tmp = strstr(action, "dstip=");
		tmp += strlen("dstip=");
		if(!tmp||*tmp == DELIM)//noet set, set default
			memset(pEntry->dstip, 0, IP_ADDR_LEN);
		else
		{
			char dip[16]={0};
			p = strchr(tmp, DELIM);
			if(p&&p-tmp>15)
				return 1;
			strncpy(dip, tmp, p-tmp);
			inet_aton(dip, (struct in_addr *)&pEntry->dstip);
		}

		//destination ip address netmask
		tmp = strstr(action, "dstnetmask=");
		tmp += strlen("dstnetmask=");
		if(!tmp||*tmp==DELIM)
			pEntry->dmaskbits = 0;
		else
		{
			char dmask[16]={0};
			p = strchr(tmp, DELIM);
			if(p&&p-tmp>15)
				return 1;
			strncpy(dmask, tmp, p-tmp);
			pEntry->dmaskbits = getNetMaskBit(dmask);

		}
#ifdef CONFIG_IPV6
	}
#endif

#ifdef CONFIG_IPV6
	// If it is a IPv6 rule.
	if ( pEntry->IpProtocol == IPVER_IPV6 )
	{
		//ip6 source address
		tmp = strstr(action, "sip6=");
		tmp += strlen("sip6=");
		if(*tmp==DELIM) {//default
			memset(pEntry->sip6, 0, IP6_ADDR_LEN);
		} else {
			for(idx=0; *tmp != DELIM&&idx<48; ++tmp)
				buff2[idx++] = *tmp;
			buff2[idx] = '\0';
			{//ip address
				inet_pton(PF_INET6, buff2, &ip6Addr);
				memcpy(pEntry->sip6, &ip6Addr, sizeof(pEntry->sip6));
			}
		}

		// ip6 src IP prefix Len
		tmp = strstr(action, "sip6PrefixLen=");
		tmp += strlen("sip6PrefixLen=");
		if(*tmp == DELIM)//default
			pEntry->sip6PrefixLen = 0;
		else {
			pEntry->sip6PrefixLen = strtol(tmp, &p, 0);
			if(*p != DELIM) return -1;//invalid dscp value
		}

		//ip6 dest address
		tmp = strstr(action, "dip6=");
		tmp += strlen("dip6=");
		if(*tmp==DELIM) {//default
			memset(pEntry->dip6, 0, IP6_ADDR_LEN);
		} else {
			for(idx=0; *tmp != DELIM&&idx<48; ++tmp)
				buff2[idx++] = *tmp;
			buff2[idx] = '\0';
			{//ip address
				inet_pton(PF_INET6, buff2, &ip6Addr);
				memcpy(pEntry->dip6, &ip6Addr, sizeof(pEntry->dip6));
			}
		}

		// ip6 dst IP prefix Len
		tmp = strstr(action, "dip6PrefixLen=");
		tmp += strlen("dip6PrefixLen=");
		if(*tmp == DELIM)//default
			pEntry->dip6PrefixLen = 0;
		else {
			pEntry->dip6PrefixLen = strtol(tmp, &p, 0);
			if(*p != DELIM) return -1;//invalid dscp value
		}
	}
#endif

	//source port
	tmp = strstr(action, "sport=");
	tmp += strlen("sport=");
	if(!tmp||*tmp==DELIM)
		pEntry->sport = 0;
	else
	{
		pEntry->sport = strtol(tmp, &p, 0);
		if(*p != DELIM)
			return 1;
	}

	//destination port
	tmp = strstr(action, "dport=");
	tmp += strlen("dport=");
	if(!tmp||*tmp==DELIM)
		pEntry->dport = 0;
	else
	{
		pEntry->dport = strtol(tmp, &p, 0);
		if(*p != DELIM)
			return 1;
	}

	//rate limit
	tmp = strstr(action, "rate=");
	tmp += strlen("rate=");
	if(!tmp||*tmp=='\0')
		pEntry->limitSpeed = 0;
	else
	{
		pEntry->limitSpeed = strtol(tmp, &p, 0);
		if(*p != DELIM)
			return 1;
	}

	//direction limit
	tmp = strstr(action, "direction=");
	tmp += strlen("direction=");
	if(!tmp||*tmp=='\0')
		pEntry->direction = 0;
	else
	{
		pEntry->direction = strtol(tmp, &p, 0);
		if(*p != '\0')
			return 1;
	}
	return 0;
}

void formQosRuleEdit(request * wp, char* path, char* query)
{
	MIB_CE_IP_QOS_T entry;
	char* action = NULL, args[256]={0}, *p = NULL, *tmp=NULL;
	char* act1="addrule", *act2="editrule", *url = NULL;
	int entryNum = 0, index=0,i=0;
	int j;

	PRINT_FUNCTION

	action = boaGetVar(wp, "lst", "");
	if(action[0])
	{
		entryNum = mib_chain_total(MIB_IP_QOS_TBL);

		if(!strncmp(action, act1, strlen(act1)))
		{//add new one
			if( entryNum>=QUEUE_RULE_NUM_MAX)
			{
				ERR_MSG("伫列规则数目已达上限."); //You cannot add one new rule when queue is full.
				return;
			}

			//reset to zero
			bzero(&entry, sizeof(MIB_CE_IP_QOS_T));

			index = parseRuleArgs(action, &entry);
			if (index >= 0)
			{
				PRINT_QUEUE_RULE((&entry));

#ifdef _PRMT_X_CT_COM_QOS_
				// Mason Yu. 
				entry.modeTr69 = MODEOTHER;
				for(j=0;j<CT_TYPE_NUM;j++)
					entry.cttypemap[j]=0;
				updatecttypevalue(&entry);				
#endif

				if(!mib_chain_add(MIB_IP_QOS_TBL, &entry))
				{
					ERR_MSG("mib Error: qos规则加入失败"); //mib Error: Cannot add new one qos rule.
					return;
				}
			}
			else
			{
				ERR_MSG("AddQosRule:错误的格式!"); //AddQosRule:Wrong argument format!
				return;
			}

		} else if(!strncmp(action, act2, strlen(act2))) {//update old one
			MIB_CE_IP_QOS_T oldEntry;

			//reset to zero
			bzero(&entry, sizeof(MIB_CE_IP_QOS_T));

			index = parseRuleArgs(action, &entry);
			if(index<0)
			{
				ERR_MSG("UpdateQosRule: 错误的格式!"); //UpdateQosRule: Wrong argument format!
				return;
			}

			PRINT_QUEUE_RULE((&entry));

			mib_chain_get(MIB_IP_QOS_TBL, index, &oldEntry);
#ifdef _CWMP_MIB_
			entry.InstanceNum=oldEntry.InstanceNum;
#endif

#ifdef _PRMT_X_CT_COM_QOS_
			entry.modeTr69=oldEntry.modeTr69;
			entry.minphyPort=oldEntry.minphyPort;			
			for(j=0;j<CT_TYPE_NUM;j++)
				entry.cttypemap[j]=oldEntry.cttypemap[j];
			updatecttypevalue(&entry);
#endif
			if(!mib_chain_update(MIB_IP_QOS_TBL, &entry, index))
			{
				ERR_MSG("qos规则更新失败!"); //Updating qos rule into mib is wrong!
				return;
			}
		} else {//undefined operation
			ERR_MSG("错误的操作! 你只能新增或编辑qos规则!"); //Wrong operation happened! you only and add or edit qos rule in this page!
			return;
		}
	}
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	//redirect
	url = boaGetVar(wp, "submit-url","");
	if(url[0])
	{
		boaRedirect(wp, url);
	}

	return;
}

////////////////////////////////////////////////////////////////////////////////////////////
//Qos Traffic control code

//now the problem is that the ifIndex is used in c code, and ifname is used for displaying,
//how can i get the ifname or ifIndex
int initTraffictlPage(int eid, request * wp, int argc, char **argv)
{
	MIB_CE_IP_TC_T entry;
	int entryNum = 0, i=0, nBytes = 0;
	char sip[20], dip[20], wanname[IFNAMSIZ], *p = NULL;
	unsigned int total_bandwidth = 0;
	unsigned char totalBandWidthEn = 0;
#ifdef CONFIG_IPV6
	unsigned char 	sip6Str[55], dip6Str[55];
#endif

	PRINT_FUNCTION

	entryNum = mib_chain_total(MIB_IP_QOS_TC_TBL);

	if(mib_get(MIB_TOTAL_BANDWIDTH, &total_bandwidth))
	{
		nBytes += boaWrite(wp, "totalBandwidth=%u;\n", total_bandwidth);
	}
	else
	{
		nBytes += boaWrite(wp, "totalBandwidth=1024;\n");
	}

	if (mib_get(MIB_TOTAL_BANDWIDTH_LIMIT_EN, &totalBandWidthEn))
		nBytes += boaWrite(wp, "totalBandWidthEn=%d;\n", totalBandWidthEn);
	else
		nBytes += boaWrite(wp, "totalBandWidthEn=0;\n");

	for(;i<entryNum; i++)
	{
		MIB_CE_ATM_VC_T       vcEntry;

		if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry))
			continue;
		wanname[0]='\0';
		//if (!getWanEntrybyindex(&vcEntry, entry.ifIndex)) {
		//	getWanName(&vcEntry, wanname);
		//}
		ifGetName(entry.ifIndex, wanname, sizeof(wanname));
		strncpy(sip, inet_ntoa(*((struct in_addr*)&entry.srcip)), INET_ADDRSTRLEN);
		strncpy(dip, inet_ntoa(*((struct in_addr*)&entry.dstip)), INET_ADDRSTRLEN);
		if(entry.smaskbits)
		{
			p = sip + strlen(sip);
			snprintf(p,sizeof(sip)-strlen(sip), "/%d", entry.smaskbits );
		}

		if(entry.dmaskbits)
		{
			p = dip + strlen(dip);
			snprintf(p,sizeof(dip)-strlen(dip), "/%d", entry.dmaskbits );
		}

#ifdef CONFIG_IPV6
		inet_ntop(PF_INET6, (struct in6_addr *)entry.sip6, sip6Str, sizeof(sip6Str));
		if(entry.sip6PrefixLen)
		{
			p = sip6Str + strlen(sip6Str);
			snprintf(p,sizeof(sip6Str)-strlen(sip6Str), "/%d", entry.sip6PrefixLen );
		}

		inet_ntop(PF_INET6, (struct in6_addr *)entry.dip6, dip6Str, sizeof(dip6Str));
		if(entry.dip6PrefixLen)
		{
			p = dip6Str + strlen(dip6Str);
			snprintf(p,sizeof(dip6Str)-strlen(dip6Str), "/%d", entry.dip6PrefixLen );
		}
#endif

		nBytes += boaWrite(wp, "traffictlRules.push(new it_nr(\"\",\n"
#ifdef CONFIG_IPV6
			//"new it(\"ipversion\",%d),\n"  //ipv4 or ipv6
			"new it(\"IpProtocolType\",%d),\n"  //ipv4 or ipv6
			"new it(\"sip6\",  \"%s\"),\n" //source ip6
			"new it(\"dip6\",  \"%s\"),\n" //dst ip6
#endif
			"new it(\"id\",         %d),\n"
			"new it(\"inf\",    \"%s\"),\n"
			"new it(\"proto\",      %d),\n"
			"new it(\"sport\",      %d),\n"
			"new it(\"dport\",      %d),\n"
			"new it(\"srcip\",  \"%s\"),\n"
			"new it(\"dstip\",  \"%s\"),\n"
			"new it(\"rate\",  \"%d\"),\n"
			"new it(\"direction\",   %d)));\n",
#ifdef CONFIG_IPV6
			entry.IpProtocol, sip6Str, dip6Str,
#endif
			entry.entryid, wanname, entry.protoType, entry.sport,
			entry.dport, sip, dip, entry.limitSpeed,entry.direction);
	}

	return nBytes;
}

void formQosTraffictl(request * wp, char *path, char *query)
{
	char *action = NULL, *url = NULL;
	char *act1="applybandwidth", *act2 = "applysetting";
	char *act4="cancelbandwidth";
	int entryNum = 0;

	PRINT_FUNCTION

	action = boaGetVar(wp, "lst", "");

	entryNum = mib_chain_total(MIB_IP_QOS_TC_TBL);

	if(action[0])
	{
		if( !strncmp(action, act1, strlen(act1)) )
		{//set total bandwidth
			unsigned int totalbandwidth = 0;
			unsigned char totalbandwidthEn=0;
			char* strbandwidth = NULL;
			strbandwidth = strstr(action, "bandwidth=");
			strbandwidth += strlen("bandwidth=");
			if(strbandwidth)//found it
			{
				totalbandwidth = strtoul(strbandwidth, NULL, 0);

				totalbandwidthEn = 1;
				if (!mib_set(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&totalbandwidthEn)) {
					ERR_MSG("无法启用整体频宽限制!\n"); //Cannot set total bandwidth Enable into mib setting!
					return;
				}

				if(!mib_set(MIB_TOTAL_BANDWIDTH, (void*)&totalbandwidth))
				{
					ERR_MSG("无法设定整体频宽!\n"); //Cannot set total bandwidth into mib setting!
					return;
				}

				//take effect
				take_qos_effect_v3();
			}
			else {
				totalbandwidthEn = 0;
				if (!mib_set(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&totalbandwidthEn)) {
					ERR_MSG("无法关闭整体频宽限制!\n"); //Cannot set total bandwidth Disable into mib setting!
					return;
				}
			}
		}
		else if ( !strncmp(action, act4, strlen(act4)) )
		{//cancel total bandwidth restrict
			unsigned char totalbandwidthEn=0;
			if (!mib_set(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&totalbandwidthEn)) {
				ERR_MSG("无法关闭整体频宽限制!\n"); //Cannot disable total bandwidth restrict into mib setting!
				return;
			}

			//take effect
			take_qos_effect_v3();
		}
		else if( !strncmp(action, act2, strlen(act2)) )
		{//delete some
			int idlst[TRAFFICTL_RULE_NUM_MAX+1] = {0};
			int  i=0, j=0, index=1;
			char stridlst[256],err_msg[256], *p = NULL;
			MIB_CE_IP_TC_T entry;

			p = strstr(action, "id=");
			p += strlen("id=");
			if(*p == '\0') {//delete none
				goto done;
			}

			stridlst[0] = '\0';

			strncpy(stridlst, p, 256);

			//convert the id list, store them in idlst,
			//you can delete most 10 rules at one time
			p = strtok(stridlst, SUBDELIM1);
			if(p) index = atoi(p);
			if(index>0&&index<=TRAFFICTL_RULE_NUM_MAX) idlst[index]=1;
			while((p = strtok(NULL, SUBDELIM1)) != NULL)
			{
				index = atoi(p);
				idlst[index]=1;
				if(index > TRAFFICTL_RULE_NUM_MAX )
					break;
			}

			for(i=entryNum-1; i>=0; i--)
			{
				if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry))
					continue;

				if( 1 == idlst[entry.entryid]) //delete it
				{
					//delete rules of  tc and iptables
					if(1 != mib_chain_delete(MIB_IP_QOS_TC_TBL, i)) {
						snprintf(err_msg, 256, "删除规则%d错误", entry.entryid); //Error happened when deleting rule
						ERR_MSG(err_msg);
						return;
					}
				}
			}

done:
			take_qos_effect_v3();
		}
	}
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	//well, go back
	url = boaGetVar(wp, "submit-url", "");
	if(url[0])
		boaRedirect(wp, url);
	return;
}

//add traffic controlling rules's main function
void formQosTraffictlEdit(request * wp, char *path, char *query)
{
	MIB_CE_IP_TC_T entry;
	char* action=NULL, *url = NULL;
	int entryNum = 0, entryid = 1;
	unsigned char map[TRAFFICTL_RULE_NUM_MAX+1]={0};

	PRINT_FUNCTION

	entryNum = mib_chain_total(MIB_IP_QOS_TC_TBL);

	//You are allowed to have TRAFFICTL_RULE_NUM_MAX rules
	if(entryNum>=TRAFFICTL_RULE_NUM_MAX)
	{
		ERR_MSG("流量控制伫列已满,请删除规则!"); //Traffic controlling queue is full, you must delete some one!
		return;
	}

	action = boaGetVar(wp, "lst", "");

	if(action[0])
	{
		//allocate a free rule id for new entry
		{
			int i = 0;
			for(;i<entryNum;i++)
			{
				if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry))
					continue;

				map[entry.entryid] = 1;
			}
			for(i=1;i<=TRAFFICTL_RULE_NUM_MAX;i++)
			{
				if(!map[i])
				{
					entryid = i;
					break;
				}
				else if(i==TRAFFICTL_RULE_NUM_MAX)
				{
					ERR_MSG("流量控制伫列已满,请删除规则!"); //Traffic controlling queue is full, you must delete some one!
					return;
				}
			}
		}

		memset(&entry, 0, sizeof(MIB_CE_IP_TC_T));

		entry.entryid = entryid;

		if(parseArgs(action, &entry))
		{//some arguments are wrong
			ERR_MSG("设定错误!"); //Wrong setting is found!
			return;
		}

		PRINT_TRAFFICTL_RULE((&entry));

		#if 0
		/*ql:20080814 START: patch for TnW - while one acl contain other acl, we should change the rule
		* order to enable CAR function.
		*/
		if (entryNum!=0 && !isTraffictlRuleWithPort(&entry)) {
			if (!mib_chain_insert(MIB_IP_QOS_TC_TBL, 0, &entry))
			{
				ERR_MSG("Cannot insert setting into mib!");
				return;
			}
		} else
		#endif
		/*ql:20080814 END*/
		if(!mib_chain_add(MIB_IP_QOS_TC_TBL, &entry))
		{//adding mib setting is wrong
			ERR_MSG("加入失败!"); //Cannot add setting into mib
			return;
		}
	}
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	//redirect
	url = boaGetVar(wp, "submit-url", "");
	if(url[0])
	boaRedirect(wp, url);
	return;
}

/******************************************************************
 * NAME:     formQosRule
 * DESC:     main function deal with qos rules, delete and change
 * ARGS:
 * RETURN:
 ******************************************************************/
void formQosRule(request * wp, char* path, char* query)
{
	MIB_CE_IP_QOS_T entry;
	int entryNum=0, i=0, id = 0;
	unsigned char statuslist[QUEUE_RULE_NUM_MAX+1] = {0}, dellist[QUEUE_RULE_NUM_MAX+1]= {0};
	char* action = NULL, *p=NULL, *url = NULL;

	PRINT_FUNCTION

	//abstract args from string
	action = boaGetVar(wp, "lst", "");
	entryNum = mib_chain_total(MIB_IP_QOS_TBL);

	//printf("action=%s\n", action);
	p = strtok(action, ",&");
	if(p) id = atoi(p);
	if(id<0||id>QUEUE_RULE_NUM_MAX)
		goto END;
	p = strtok(NULL, ",&");
	if(p) statuslist[id] = !!atoi(p);
	p = strtok(NULL, ",&");
	if(p) dellist[id] = !!atoi(p);

	while( (p = strtok(NULL, ",&")) != NULL )
	{
		id = atoi(p);
		if(id<0||id>QUEUE_RULE_NUM_MAX) continue;
		p = strtok(NULL, ",&");
		if(p) statuslist[id] = !!atoi(p);
		p = strtok(NULL, ",&");
		if(p) dellist[id] = !!atoi(p);
	}

	//change status of rules if neccessary
	//printf("check statuslist\n");
	for(i=0;i<entryNum;i++)
	{
		if(!mib_chain_get(MIB_IP_QOS_TBL, i, &entry))
			continue;

		if(statuslist[i] != entry.enable)
		{
			//printf("entry %d status changed\n", i);
			entry.enable = statuslist[i];
			if(!mib_chain_update(MIB_IP_QOS_TBL, &entry, i))
			{
				ERR_MSG("更新规则失败!"); //updating rule error!
				return;
			}
		}
	}

	//printf("check deletelist!\n");
	//delete some one
	for(i=entryNum-1;i>=0;i--)
	{
		if(!mib_chain_get(MIB_IP_QOS_TBL, i, &entry))
			continue;
		if(i<=QUEUE_RULE_NUM_MAX&&dellist[i])
		{
			//printf("entry %d deleted\n", i);
			if(1 != mib_chain_delete(MIB_IP_QOS_TBL, i))
			{
				ERR_MSG("删除规则失败!"); //Delete rule error!
				return;
			}
		}
	}

	//take effect
	take_qos_effect_v3();

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

END:
	//redirect to show page
	url = boaGetVar(wp, "submit-url", "");
	if(url[0])
	{
		boaRedirect(wp, url);
	}
	return;
}

int initQueuePolicy(int eid, request * wp, int argc, char **argv)
{
	unsigned char policy = 0;
	char wanname[32];
	int nBytes=0;
	MIB_CE_IP_QOS_QUEUE_T qEntry;
	int qEntryNum, i;
	unsigned int total_bandwidth = 0;

	PRINT_FUNCTION

	if(mib_get(MIB_TOTAL_BANDWIDTH, &total_bandwidth))
	{
		nBytes += boaWrite(wp, "totalBandwidth=%u;\n", total_bandwidth);
	}
	else
	{
		nBytes += boaWrite(wp, "totalBandwidth=1024;\n");
	}

	if(!mib_get(MIB_QOS_ENABLE_QOS, &policy)|| (policy !=0&&policy!=1))
		policy = 0;
	nBytes = boaWrite(wp, "qosEnable=%d;\n", policy);
	if(!mib_get(MIB_QOS_POLICY, &policy)|| (policy !=0&&policy!=1))
		policy = 0;
	nBytes += boaWrite(wp, "policy=%d;\n", policy);

	if((qEntryNum=mib_chain_total(MIB_IP_QOS_QUEUE_TBL)) <=0)
		return nBytes;
	for(i=0;i<qEntryNum; i++)
	{
		if(!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, i, (void*)&qEntry))
			continue;
		nBytes += boaWrite(wp, "queues.push("
			"new it_nr(\"%d\","  //qos queue name
			"new it(\"qname\",\"Q%d\"),"  // name
			"new it(\"prio\",%d),"  // priority
			"new it(\"weight\", %d),"  // weight
			"new it(\"enable\", %d)));\n",
			i, i+1, i+1, qEntry.weight, qEntry.enable);
	}
	return nBytes;
}

void formQosPolicy(request * wp, char *path, char *query)
{
	char *action=NULL,*url=NULL, *p=NULL, *tmp=NULL;
	unsigned char policy = 0, modeflag = 0, old_modeflag = 0;
	int num = 0, ret;
	MIB_CE_IP_QOS_QUEUE_T qEntry;

	PRINT_FUNCTION

	action = boaGetVar(wp, "qosen", "");
	if (action[0]) {
		policy = (action[0]=='0') ? 0 : 1;
		mib_set(MIB_QOS_ENABLE_QOS, &policy);
	}

#if defined(CONFIG_USER_IP_QOS) && defined(_PRMT_X_CT_COM_QOS_)
	mib_get(CTQOS_MODE, &old_modeflag);
	action = boaGetVar(wp, "qosmode", "");
	if(action[0])
		modeflag = (unsigned char)atoi(action);
	else
		modeflag = OTHERMODE;			

/* TODO: UI should change setting according to the mode and template setting.
 *		 However, current UI is not ready for this. So, just read user setting
 *		 about force_weight/dscp_remark/1p_remark
 *
	if ( !((old_modeflag == OTHERMODE) && (modeflag == OTHERMODE))) {
		ret = setMIBforQosMode(modeflag);	
		if (ret < 0){
			ERR_MSG("Failed to setMIB for QoSMode.\n");
			return;
		}		
	}
*/
	action = boaGetVar(wp, "enable_force_weight", "");
	if (action[0]) {
		unsigned char value = atoi(action);
		if(!mib_set(MIB_QOS_ENABLE_FORCE_WEIGHT, (void *)&value)){
			ERR_MSG("qos force weight启用/关闭设定失败\n"); //Failed to set qos enable/disable force weight.
			return;
		}
	}

	action = boaGetVar(wp, "enable_dscp_remark", "");
	if (action[0]) {
		unsigned char value = atoi(action);
		if(!mib_set(MIB_QOS_ENABLE_DSCP_MARK, (void *)&value)){
			ERR_MSG("qos差分服务代码点重标记启用/关闭设定失败\n"); //Failed to set qos enable/disable dscp remark.
			return;
		}
	}

	action = boaGetVar(wp, "enable_1p_remark", "");
	if (action[0]) {
		unsigned char value = atoi(action);
		if(!mib_set(MIB_QOS_ENABLE_1P, (void *)&value)){
		ERR_MSG("qos 1p重标记启用/关闭设定失败\n"); //Failed to set qos enable/disable 1p remark.
		return;
		}
	}

#endif
	action = boaGetVar(wp, "lst", "");
	printf("policy: %s\n", action);
	if(action[0])
	{
		//policy
		p = strstr(action, "policy=");
		if(p)
		{
			p+=strlen("policy=");
			num = strtol(p, &tmp, 0);
			//if(tmp && tmp !=p && *tmp == DELIM)
			{
				printf("num=%d\n",num);
				policy = !!num;
				printf("policy=%d\n",policy);
				stopIPQ();
				if(!mib_set(MIB_QOS_POLICY, &policy)) {
					ERR_MSG("qos伫列法则设定失败\n"); //Failed to set qos queue policy.
					return;
				}
			}
		}
	}

	for (num=0; num<4; num++) {
		char wstr[]="w0";
		char qenstr[]="qen0";
		if(!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, num, (void*)&qEntry))
			continue;
		if (policy == 1) { // WRR
			wstr[1] += num;
			p = boaGetVar(wp, wstr, "");
			if (p && p[0])
				qEntry.weight = atoi(p);
		}
		qenstr[3] += num;
		p = boaGetVar(wp, qenstr, "");
		if (p && p[0])
			qEntry.enable = 1;
		else
			qEntry.enable = 0;
		mib_chain_update(MIB_IP_QOS_QUEUE_TBL, (void *)&qEntry, num);
	}

	action = boaGetVar(wp, "totalbandwidth", "");
	if (action[0]) {
		unsigned int totalbandwidth = 0;
		totalbandwidth = strtoul(action, NULL, 0);
		if(!mib_set(MIB_TOTAL_BANDWIDTH, (void*)&totalbandwidth))
		{
			ERR_MSG("formQosPolicy: 整体频宽设定失败!\n"); //formQosPolicy: Cannot set total bandwidth into mib setting!
			return;
		}
	}



done:

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	//take effect
	take_qos_effect_v3();

	//redirect web page
	url = boaGetVar(wp, "submit-url", "");
	if(url[0])
	{
		boaRedirect(wp, url);
	}
	return;
}



int ipqos_dhcpopt(int eid, request * wp, int argc, char **argv)
{
	int nBytes = 0;

#ifdef CONFIG_BRIDGE_EBT_DHCP_OPT
	nBytes += boaWrite(wp, " <td><font size=2><input type=\"radio\"  name=qos_rule_type value=4 onClick=ruleType_click();> DHCP Option</td>");	
#endif

	return nBytes;
}


int ipqos_dhcpopt_getoption60(int eid, request * wp, int argc, char **argv)
{
	int nBytes = 0, index=0;
	MIB_CE_IP_QOS_T entry;
	int start=0,end=0,len=0;
	char index_buf[3]={0};

#ifdef CONFIG_BRIDGE_EBT_DHCP_OPT
	if(wp->query_string)
	{
		start = strstr(wp->query_string,"rule_index=")+sizeof("rule_index=")-1;
		end = strstr(wp->query_string,"&rule");
		len=end-start;
		if(start&&end&&(len>0))
		{
			memcpy(index_buf,start,len);

			index= atoi(index_buf);
	if(!mib_chain_get(MIB_IP_QOS_TBL, index, &entry))
			return;
	nBytes += boaWrite(wp, "%s", entry.opt60_vendorclass);	
		}
	}
#endif

	return nBytes;

}

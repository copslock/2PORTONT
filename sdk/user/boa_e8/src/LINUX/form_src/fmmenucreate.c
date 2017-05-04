/*
*  fmmenucreate.c is used to create menu
*  added by xl_yue
*/
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../webs.h"
#include "boa.h"
#include "mib.h"
#include "webform.h"
#include "utility.h"
//add by ramen to include the autoconf.h created by kernel
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif
#include <rtk/options.h>

#define BEGIN_CATALOG(name)  	boaWrite (wp, "mncata = new menu(\"%s\");", name)
#define FLUSH_CATALOG()  		boaWrite (wp, "mnroot.add(mncata);")

#define BEGIN_MENU(name)  		boaWrite (wp, "mnpage = new menu(\"%s\");", name)
#define ADD_MENU(link, page)  		boaWrite (wp, "mnpage.add(\"%s\",\"%s\");", link, page)
#define END_MENU()  				boaWrite (wp, "mncata.add(mnpage);")

#ifdef E8B_NEW_DIAGNOSE
//Added by robin, for diagnosis pages
int createMenuDiag(int eid, request * wp, int argc, char **argv)
{
	boaWrite(wp, "var mncata = null;\n");
	boaWrite(wp, "var mnpage = null;\n");

	//״̬
	//modify by liuxiao 2008-01-23
	BEGIN_CATALOG("״̬");

	BEGIN_MENU("�豸��Ϣ");
	ADD_MENU("diag_dev_basic_info.asp", "�豸������Ϣ");
	END_MENU();

	BEGIN_MENU("�������Ϣ");
	ADD_MENU("diag_net_connect_info.asp", "������Ϣ");
	ADD_MENU("diag_net_dsl_info.asp", "DSL��Ϣ");
	END_MENU();

	BEGIN_MENU("�û�����Ϣ");
#ifdef WLAN_SUPPORT
	ADD_MENU("diag_wlan_info.asp", "WLan�ӿ���Ϣ");
#endif
	ADD_MENU("diag_ethernet_info.asp", "��̫������Ϣ");
	ADD_MENU("diag_usb_info.asp", "USB�ӿ���Ϣ");
	END_MENU();

	BEGIN_MENU("Զ�̹���״̬");
	ADD_MENU("status_tr069_info.asp", "Զ�����ӽ���״̬");
	ADD_MENU("status_tr069_config.asp", "ҵ�������·�״̬");
	END_MENU();

	FLUSH_CATALOG();

	BEGIN_CATALOG("���");

	BEGIN_MENU("��ϲ���");
	ADD_MENU("diag_ping.asp", "PING����");
	ADD_MENU("diag_tracert.asp", "Tracert����");
	ADD_MENU("diagnose_tr069.asp", "�ֶ��ϱ� Inform");
	END_MENU();

	FLUSH_CATALOG();

}
#endif
int createMenuEx(int eid, request * wp, int argc, char **argv)
{
#if defined(CONFIG_EPON_FEATURE) || defined(CONFIG_GPON_FEATURE)
	unsigned int pon_mode;

	mib_get(MIB_PON_MODE, &pon_mode);
#endif
	struct user_info *pUser_info;

	pUser_info = search_login_list(wp);

	if (!pUser_info)
		return -1;

	boaWrite(wp, "var mncata = null;\n");
	boaWrite(wp, "var mnpage = null;\n");

	//״̬
	//modify by liuxiao 2008-01-23
	BEGIN_CATALOG("״̬");	//user

	BEGIN_MENU("�豸��Ϣ");	//user
	ADD_MENU("status_device_basic_info.asp", "�豸������Ϣ");
	END_MENU();

	BEGIN_MENU("�������Ϣ");	//user
	if (pUser_info->priv) {	//admin
		ADD_MENU("status_net_connet_info.asp", "IPv4������Ϣ");
		ADD_MENU("status_net_connet_info_ipv6.asp", "IPv6������Ϣ");
#ifdef SUPPORT_WAN_BANDWIDTH_INFO
		ADD_MENU("status_wan_bandwidth.asp", "WAN������Ϣ");
#endif
	} else {
		ADD_MENU("status_user_net_connet_info.asp", "IPv4������Ϣ");
		ADD_MENU("status_user_net_connet_info_ipv6.asp",
			 "IPv6������Ϣ");
	}

#ifdef CONFIG_DEV_xDSL
	if (pUser_info->priv)	//admin
		ADD_MENU("status_net_dsl_info.asp", "DSL��Ϣ");	//user- new tech spec
	else
		ADD_MENU("status_user_net_dsl_info.asp", "DSL��Ϣ");	//user- new tech spec
#endif

#ifdef CONFIG_EPON_FEATURE
	if (pon_mode == EPON_MODE)
		ADD_MENU("status_epon.asp", "EPON ��Ϣ");
#endif

#ifdef CONFIG_GPON_FEATURE
	if (pon_mode == GPON_MODE)
		ADD_MENU("status_gpon.asp", "GPON ��Ϣ");
#endif

	END_MENU();

	BEGIN_MENU("�û�����Ϣ");	//user
#ifdef WLAN_SUPPORT
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)
	ADD_MENU("status_wlan_info_11n.asp", "WLAN�ӿ���Ϣ");
#else
	ADD_MENU("status_wlan_info.asp", "WLAN�ӿ���Ϣ");
#endif
#endif
	ADD_MENU("status_ethernet_info.asp", "��̫���ӿ���Ϣ");

#ifdef CONFIG_USER_LANNETINFO
		ADD_MENU("status_lan_net_info.asp", "�¹��豸��Ϣ");
#endif

#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
{
	unsigned char vChar=0;
	mib_get(MIB_LANHOST_BANDWIDTH_MONITOR_ENABLE, (void*)&vChar);
	if(vChar)
		ADD_MENU("status_lan_bandwidth_monitor.asp", "�¹��豸��������Ϣ");
}
#endif

#ifdef USB_SUPPORT
	ADD_MENU("status_usb_info.asp", "USB�ӿ���Ϣ");
#endif
	END_MENU();

#ifdef VOIP_SUPPORT
	//SD6-bohungwu, e8c voip
	BEGIN_MENU("���������Ϣ");
	ADD_MENU("status_voip_info.asp", "���������Ϣ");
	END_MENU();
#endif //#ifdef VOIP_SUPPORT

#ifdef E8B_NEW_DIAGNOSE
	if (pUser_info->priv)	//admin
	{
		BEGIN_MENU("Զ�̹���״̬");
		ADD_MENU("status_tr069_info_admin.asp", "Զ�����ӽ���״̬");
		ADD_MENU("status_tr069_config_admin.asp", "ҵ�������·�״̬");
		END_MENU();
	}
#endif

	FLUSH_CATALOG();
	//modify end by liuxiao 2008-01-23

	//��  ��
	BEGIN_CATALOG("��  ��");	//user

	if (pUser_info->priv)	//admin
	{
		BEGIN_MENU("�������");
#if defined(CONFIG_ETHWAN)
		ADD_MENU
		    ("boaform/formWanRedirect?redirect-url=/net_eth_links.asp&if=eth",
		     "Internet ����");
#endif
#if defined(CONFIG_PTMWAN)
		ADD_MENU
		    ("boaform/formWanRedirect?redirect-url=/net_eth_links.asp&if=ptm",
		     "Internet PTM ����");
#endif
#if defined(CONFIG_RTL8672_SAR)
		ADD_MENU("net_adsl_links.asp", "Internet PVC ����");
#endif
		END_MENU();
#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
		BEGIN_MENU("������");
		ADD_MENU("net_vlan_mapping.asp", "��ģʽ");
		END_MENU();
#endif
	}

	BEGIN_MENU("LAN���ַ����");
	ADD_MENU("net_dhcpd.asp", "IPv4����");
	ADD_MENU("ipv6.asp", "IPv6 ����");
	ADD_MENU("dhcpdv6.asp", "IPv6 DHCP Server����");
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("radvdconf.asp", "RA ����");
	}
	END_MENU();
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
	if (pUser_info->priv){

		BEGIN_MENU("VPN WAN"); //user
		ADD_MENU("pptp.asp", "PPTP");	
		ADD_MENU("l2tp.asp", "L2TP");	
		END_MENU();
	}
#endif

#ifdef WLAN_SUPPORT
	BEGIN_MENU("WLAN����");	//user
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)
	if (pUser_info->priv)	//admin
		ADD_MENU("net_wlan_basic_11n.asp", "WLAN����");
	else
		ADD_MENU("net_wlan_basic_user_11n.asp", "WLAN����");
#else
	if (pUser_info->priv)	//admin
		ADD_MENU("net_wlan_basic.asp", "WLAN����");
	else
		ADD_MENU("net_wlan_basic_user.asp", "WLAN����");
#endif
#ifdef WIFI_TIMER_SCHEDULE
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("net_wlan_sched.asp", "���ض�ʱ");
		ADD_MENU("net_wlan_timer.asp", "���ض�ʱ(����)");
	}
#endif
	END_MENU();
#endif
	if (pUser_info->priv)	//admin
	{
		BEGIN_MENU("Զ�̹���");
		ADD_MENU("net_tr069.asp", "ITMS������");
		ADD_MENU("net_certca.asp", "�ϴ�CA֤��");
#ifdef CONFIG_MIDDLEWARE
		ADD_MENU("net_midware.asp", "�м������");
#endif
		ADD_MENU("usereg_inside_menu.asp", "�߼�IDע��");
		END_MENU();

		BEGIN_MENU("QoS");
		//ADD_MENU("net_qos_queue.asp", "��������");
	/*
#ifndef QOS_SETUP_IMQ
		ADD_MENU("net_qos_policy.asp", "��������");
#else
		ADD_MENU("net_qos_imq_policy.asp", "��������");
#endif
*/
		ADD_MENU("net_qos_imq_policy.asp", "��������");
		ADD_MENU("net_qos_cls.asp", "QoS����");
//		ADD_MENU("net_qos_app.asp", "QoSҵ��");
		ADD_MENU("net_qos_traffictl.asp", "��������");
		END_MENU();

		BEGIN_MENU("ʱ�����");
		ADD_MENU("net_sntp.asp", "ʱ�������");
		END_MENU();

		BEGIN_MENU("·������");
		// Mason Yu. 2630-e8b
		ADD_MENU("rip.asp", "��̬·��");
		// Mason Yu. 2630-e8b
		ADD_MENU("routing.asp", "��̬·��");
		END_MENU();
	}

	FLUSH_CATALOG();

	//��  ȫ
	BEGIN_CATALOG("��  ȫ");	//user

	BEGIN_MENU("��������������");	//user
	ADD_MENU("secu_urlfilter_cfg.asp", "��������������");
	END_MENU();

	BEGIN_MENU("����ǽ");
	ADD_MENU("secu_firewall_level.asp", "��ȫ��");	//user
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("secu_firewall_dosprev.asp", "������������");
	}
	END_MENU();

	BEGIN_MENU("MAC����");	//user
#ifdef	MAC_FILTER_SRC_ONLY
	ADD_MENU("secu_macfilter_src.asp", "MAC����");
#else
	ADD_MENU("secu_macfilter_bridge.asp", "�Ž�MAC����");
	ADD_MENU("secu_macfilter_router.asp", "·��MAC����");
#endif
	END_MENU();

	if (pUser_info->priv)	//admin
	{
		BEGIN_MENU("�˿ڹ���");
		ADD_MENU("secu_portfilter_cfg.asp", "�˿ڹ���");
		END_MENU();
	}

	FLUSH_CATALOG();

	//Ӧ  ��
	BEGIN_CATALOG("Ӧ  ��");	//user

	if (pUser_info->priv)	//admin
	{
#ifdef CONFIG_RG_SLEEPMODE_TIMER
		BEGIN_MENU("������������");
		ADD_MENU("app_sleepmode_rule.asp", "������������");
		END_MENU();
#endif
#ifdef CONFIG_LED_INDICATOR_TIMER
		BEGIN_MENU("����LED����");
		ADD_MENU("app_led_sched.asp", "����LED����");
		END_MENU();
#endif
		BEGIN_MENU("DDNS����");
		ADD_MENU("app_ddns_show.asp", "DDNS����");
		END_MENU();

		BEGIN_MENU("�߼�NAT����");
		// Mason Yu. 2630-e8b
		ADD_MENU("algonoff.asp", "ALG����");
		ADD_MENU("fw-dmz.asp", "DMZ����");
		ADD_MENU("app_nat_vrtsvr_cfg.asp", "������������");
#if 0
		ADD_MENU("app_nat_porttrig_show.asp", "�˿ڴ���");
#endif
		END_MENU();

		BEGIN_MENU("UPNP����");
		ADD_MENU("app_upnp.asp", "UPNP����");
#ifdef CONFIG_USER_MINIDLNA
		ADD_MENU("dms.asp", "DLNA����");
#endif
		END_MENU();

		//SD6-bohungwu, e8c voip
#ifdef VOIP_SUPPORT
		BEGIN_MENU("����绰����");
		ADD_MENU("app_voip.asp", "����绰����");
		ADD_MENU("app_voip2.asp", "����绰�߼�����");
		END_MENU();
#endif //#ifdef VOIP_SUPPORT

		BEGIN_MENU("IGMP����");
		ADD_MENU("app_igmp_snooping.asp", "IGMP SNOOPING");
		ADD_MENU("app_igmp_proxy.asp", "IGMP Proxy ");
		END_MENU();

		// Mason Yu. MLD Proxy
		BEGIN_MENU("MLD����");
		ADD_MENU("app_mld_snooping.asp", "MLD SNOOPING����");	// Mason Yu. MLD snooping for e8b
		ADD_MENU("app_mldProxy.asp", "MLD Proxy����");
		END_MENU();
		
#if defined (CONFIG_USER_LAN_BANDWIDTH_MONITOR) || defined (CONFIG_USER_LAN_BANDWIDTH_CONTROL)
		BEGIN_MENU("�¹��ն�����");
#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
		ADD_MENU("app_bandwidth_monitor.asp", "�����д�����");
#endif
#ifdef CONFIG_USER_LAN_BANDWIDTH_CONTROL
		ADD_MENU("app_bandwidth_control.asp", "�����д�������");
#endif
		ADD_MENU("app_bandwidth_interval.asp", "�����������");
		END_MENU();
#endif	// end of (CONFIG_USER_LAN_BANDWIDTH_MONITOR) || defined (CONFIG_USER_LAN_BANDWIDTH_CONTROL)		
	}

	BEGIN_MENU("�ճ�Ӧ��");	//user
#ifdef USB_SUPPORT
	ADD_MENU("app_storage.asp", "��ͥ�洢");
#endif
#ifdef CONFIG_MCAST_VLAN
	if (pUser_info->priv)
		ADD_MENU("app_iptv.asp", "IPTV");
#endif
	END_MENU();

#ifndef USB_SUPPORT
if(pUser_info->priv)
#endif
	FLUSH_CATALOG();

	//��  ��
	BEGIN_CATALOG("��  ��");	//user

	BEGIN_MENU("�û�����");	//user
	ADD_MENU("mgm_usr_user.asp", "�û�����");
	END_MENU();

	BEGIN_MENU("�豸����");
	ADD_MENU("mgm_dev_reboot.asp", "�豸����");	//user
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("mgm_dev_reset.asp", "�ָ���������");
	}
#ifdef USB_SUPPORT
	ADD_MENU("mgm_dev_usbbak.asp", "USB��������");
	ADD_MENU("mgm_dev_usb_umount.asp", "USBж��");
#endif
	END_MENU();

	if (pUser_info->priv)	//admin
	{
		BEGIN_MENU("��־�ļ�����");
		ADD_MENU("mgm_log_cfg.asp", "д��ȼ�����");
		ADD_MENU("mgm_log_view.asp", "�豸��־");
		END_MENU();

		BEGIN_MENU("ά��");
		ADD_MENU("mgm_mnt_mnt.asp", "ά��");
		END_MENU();
	}

	FLUSH_CATALOG();

#ifdef E8B_NEW_DIAGNOSE
	if (pUser_info->priv)	//admin
	{
		//���
		BEGIN_CATALOG("���");

		BEGIN_MENU("�������");
#ifdef CONFIG_RTL8672_SAR
		ADD_MENU("diag_f5loop_admin.asp", "��·����");
#endif
		ADD_MENU("diag_ping_admin.asp", "PING����");
		ADD_MENU("diag_tracert_admin.asp", "Tracert����");
		ADD_MENU("diagnose_tr069_admin.asp", "�ֶ��ϱ� Inform");
		END_MENU();

#ifdef CONFIG_USER_RTK_LBD
		BEGIN_MENU("��·���");
		ADD_MENU("diag_loopback_detect.asp", "��·���");
		END_MENU();
#endif

		FLUSH_CATALOG();
	}
#endif

	//��  ��
	//modify by liuxiao 2008-01-23
	BEGIN_CATALOG("��  ��");	//user

	BEGIN_MENU("״̬����");
	ADD_MENU("/help/help_status_device.html", "�豸��Ϣ����");
	ADD_MENU("/help/help_status_net.asp", "�������Ϣ����");
	ADD_MENU("/help/help_status_user.html", "�û�����Ϣ����");
#ifdef VOIP_SUPPORT
	ADD_MENU("/help/help_status_voip.html", "���������Ϣ����");
#endif
	END_MENU();

	BEGIN_MENU("�������");
	if (pUser_info->priv)	//admin
	{
#ifdef CONFIG_DEV_xDSL
		ADD_MENU("/help/help_net_broadband.html", "������ð���");
#elif defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
		ADD_MENU("/help/help_net_pon.html", "������ð���");
#endif
		ADD_MENU("/help/help_net_dhcp.html", "DHCP���ð���");
	}
#ifdef WLAN_SUPPORT
	ADD_MENU("/help/help_net_wlan.html", "WLAN���ð���");
#endif
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("/help/help_net_remote.html", "Զ�̹������");
		ADD_MENU("/help/help_net_qos.html", "QoS����");
		ADD_MENU("/help/help_net_time.html", "ʱ��������");
		ADD_MENU("/help/help_net_route.html", "·�����ð���");
	}
	END_MENU();

	BEGIN_MENU("��ȫ����");
	ADD_MENU("/help/help_security_wanaccess.html", "�������������ð���");
	ADD_MENU("/help/help_security_firewall.html", "����ǽ����");
	ADD_MENU("/help/help_security_macfilter.html", "MAC���˰���");
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("/help/help_security_portfilter.html", "�˿ڹ��˰���");
	}
	END_MENU();

	BEGIN_MENU("Ӧ�ð���");
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("/help/help_apply_ddns.html", "DDNS���ð���");
		ADD_MENU("/help/help_apply_nat.html", "�߼�NAT���ð���");
		ADD_MENU("/help/help_apply_upnp.html", "UPNP���ð���");
#ifdef VOIP_SUPPORT
		ADD_MENU("/help/help_apply_voip.html", "����绰���ð���");
#endif
		ADD_MENU("/help/help_apply_igmp.html", "IGMP���ð���");
	}
#ifdef USB_SUPPORT
	ADD_MENU("/help/help_apply_familymemory.html", "��ͥ�洢����");
#endif
	END_MENU();

	BEGIN_MENU("�������");
	ADD_MENU("/help/help_manage_user.html", "�û��������");
	ADD_MENU("/help/help_manage_device.html", "�豸�������");
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("/help/help_manage_logfile.html", "��־�ļ��������");
		ADD_MENU("/help/help_manage_keep.html", "ά������");
	}
	END_MENU();

	FLUSH_CATALOG();
	//modify end by liuxiao 2008-01-23

	return 0;
}

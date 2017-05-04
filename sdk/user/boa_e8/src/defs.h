#ifndef WEB_LANGUAGE_DEFS_H
#define WEB_LANGUAGE_DEFS_H
#include "LINUX/options.h"
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../include/linux/autoconf.h"
#endif

#define Tfast_config	"Fast Configure"
#define Tfc_intro     "the fast configuration will help you finish the configuration step by step."
#define Tattention    "attention:"
#define Tatt_intro    "original configuration will be replaced by the fast configuration."
#define Tint_conn_conf  "Internet Connection Configure -- ATM PVC config"
#define Tint_conn_intro "please set VPI and VCI. Do not modify it, unless ISP need modify it."
#define Tnext_step    "next step >"
#define Tint_conn_type  "Internet Connection Configure -- connection type"
#define Tconn_type_intro    "please select WAN connection type provided by ISP."
#define Tconn_type    "WAN connection type:"
#define Tencap_type   "encapsulation:"
#define Tenter_vpi	"please Enter VPI (0~255)"
#define Tinvalid_vpi	"invalid VPI!"
#define Tenter_vci	"please enter VCI (0~65535)"
#define Tinvalid_vci	"invalid VCI!"
#define Tget_chain_err	"Get chain record error!\n"
#define Tconn_exist	"Connection already exists, delete existed connection?"
#define Tvc_exceed	"Error: Maximum number of VC exceeds !"
#define Tadd_err		"Error! Add chain record."
#define Tsecond_ip_err		"Set second IP-address error!"
#define Tsecond_mask_err	"Set second subnet-mask error!"
#define Tdhcp_err			"Set dhcp mode MIB error!"
#define Tget_ip_err		"Get IP-address error!"
#define Tget_mask_err	"Get Subnet Mask error!"
#define Tdhcp_range_err	"Invalid DHCP client range!"
#define Tset_start_ip_err		"Set DHCP client start range error!"
#define Tset_end_ip_err		"Set DHCP client end range error!"
#define Tset_lease_err		"Set DHCP lease time error!"
#define Tset_ip_err		"Set IP-address error!"
#define Tset_mask_err	"Set net mask error!"
#define Tinvalid_ip		"Invalid IP-address value!"
#define Tinvalid_mask	"Invalid subnet-mask value!"
#define Tinvalid_ip2		"Invalid second IP-address value!"
#define Tinvalid_mask2	"Invalid second subnet-mask value!"
#define Tip2vsip		"first Lan and second Lan must belong to different subNet!"
#define Tinvalid_start_ip	"Invalid DHCP start IP-address value!"
#define Tinvalid_end_ip	"Invalid DHCP end IP-address value!"
#define Tinvalid_dhcp_ip	"DHCP address range error!"
#define Tuser_long		"Error! User Name length too long."
#define Tpassw_long	"Error! Password length too long."
#define Tinvalid_ppp_type	"Invalid PPP type value!"
#define Tconn_forever	", connect forever"
#define Tconn_on_dem	", connect on demand, idle for %d minute"
#define Tconn_manual	", connect manually"
#define Tdd_hh_mm		"%dday %dhour %dmin"
#define Tdhcp_ip_range	"DHCP IP Range"
#define Tdhcp_lease_t	"DHCP Lease Time"
#define Tnapt			"NAPT"
#define Twan_ip		"WAN IP"
#define Tres_gw		"Reserved Gateway"
#define Tauto_assign	"auto assigned"
#define Tno_wan_ip		"none"
#define Tdnss			"DNS Server"
#define Tbytes			"data bytes"
#define Tping_recv		"%d bytes from %s: icmp_seq=%u"
#define Tping_stat		"--- ping statistics ---"
#define Ttrans_pkt		"%ld packets transmitted, "
#define Trecv_pkt		"%ld packets received."
#define Tno_conn		"Connection not exists!"
#define Trecv_cell_suc	"Loopback cell���ճɹ�!" //Loopback cell received successfully !
#define Trecv_cell_fail	"Loopback ʧ��!" //Loopback failed !
#define Tadsl_diag_suc	"ADSL Diagnostics successful !!"
#define Tadsl_diag_fail	"ADSL Diagnostics failed !!"
#define Tdownstream		"Downstream"
#define Tupstream		"Upstream"
#define Tloop_annu		"Loop Attenuation"
#define Tsig_annu		"Signal Attenuation"
#define Thlin_scale		"Hlin Scale"
#define Tsnr_marg		"SNR Margin"
#define Tattain_rate		"Attainable Rate"
#define Tout_power		"Output Power"
#define Ttone_num		"Tone Number"
#define Tlan_conn_chk	"LAN Connection Check"
#define Ttest_eth_conn	"Test Ethernet LAN Connection"
#define Ttest_adsl_syn	"Test ADSL Synchronization"
#define Ttest_oam_f5_seg	"Test ATM OAM F5 Segment Loopback"
#define Ttest_oam_f5_end 	"Test ATM OAM F5 End-to-end Loopback"
#define Ttest_oam_f4_seg	"Test ATM OAM F4 Segment Loopback"
#define Ttest_oam_f4_end	"Test ATM OAM F4 End-to-end Loopback"
#define Tadsl_conn_chk	"ADSL Connection Check"
#define Tint_conn_chk 	"Internet Connection Check"
#define Ttest_ppps_conn	"Test PPP Server Connection"
#define Ttest_auth		"Test Authentication with ISP"
#define Ttest_assigned_ip	"Test the assigned IP Address"
#define Tping_pri_dnss	"Ping Primary Domain Name Server"
#define Tback			"  Back  "
#define Tping_def_gw	"Ping Default Gateway"
#define Tadsl_diag_wait	"The test result will come out 3 minutes later and the page will refresh itself automatically. Please wait ...\n"
#define Tppp_conn_excd	"Maximum number of PPPoE connections exceeds in this vc!"
#define Tdef_gw_exist	"Default route already exists !"
#define Tinvalid_wan_dhcp	"Invalid WAN DHCP value!"
#define Tinvalid_gw		"Invalid reserved gateway IP-address value!"
#define Tinvalid_dns_mode	"Invalid DNS mode value!"
#define Tinvalid_dns	"Invalid DNS address!"
#define Treboot_wd0		"The System is Restarting ..."
#define Treboot_wd1		"The DSL bridge has been configured and is rebooting."
#define Treboot_wd2		"Close the DSL Router Configuration window and wait"\
			" for 2 minutes before reopening your web browser."\
			" If necessary, reconfigure your PC's IP address to match"\
			" your new configuration.";
#define Ttime_zone1		"System Date has been modified successfully."
#define Ttime_zone2		"Please reflesh your \"Status\" page."
#define Tset_tz			"set time zone error!"
#define Tset_date_err	"cannot set date"
#define Tset_ntp_ena	"Set enabled flag error!"
#define Tset_ntp_svr	"Set NTP server error!"
#define Tset_ntp_ip		"Set NTP server IP error!"
#define Tbrg_not_exist	"br0 interface not exists !!"
#define Tyes			"yes"
#define Tno				"no"
#define Tport_err		"port size error!"
#define Twan_port		"wan port set error!"
#define Tsvr_port		"server port set error!"
#define Tvrtsrv_rule	"execute virtual server rules failed!\n"
#define Tdmz_error		"Invalid IP address! It should be set within the current subnet."

#define strWrongIP "���Ϸ���IP��ַ!" //"Invalid IP-address value!"
#define strSetIPerror "Set IP-address error!"
#define strWrongMask "���Ϸ�����·����!" //"Invalid subnet-mask value!"
#define strSetMaskerror "Set subnet-mask error!"

#define strGetDhcpModeerror "Get DHCP MIB error!"
#define strGetIperror "Get IP-address error!"
#define strGetMaskerror "Get Subnet Mask error!"
#define strInvalidRange "���Ϸ���DHCP client��Χ" //"Invalid DHCP client range!"
#define strSetStarIperror "Set DHCP client start range error!"
#define strSetEndIperror "Set DHCP client end range error!"
#define strSetLeaseTimeerror "DHCP��Լ�趨����!" //"Set DHCP lease time error!"
#define strSetDomainNameerror "Set DHCP Domain Name error!"
#define strInvalidGatewayerror "���Ϸ���DHCP Server���ص�ַ!" //"Invalid DHCP Server Gateway Address value!"
#define strSetGatewayerror "Set DHCP Server Gateway Address error!"
#define strSetDhcpModeerror "DHCP�趨����!" //"Set DHCP error!"

#if defined(IP_BASED_CLIENT_TYPE) || defined(_PRMT_X_TELEFONICA_ES_DHCPOPTION_)
#define strInvalidRangepc "invalid pc IP pool range!"
#define strInvalidRangecmr "invalid camera IP pool range!"
#define strInvalidRangestb "invalid stb IP pool range!"
#define strInvalidRangephn "invalid phone IP pool range!"
#define strInvalidTypeRange "�豸��ַӦ��DHCP��ַ�صķ�Χ��"
#define strOverlapRange "�豸��ַ��Ӧ�ص�"
#define strInvalidNstbRange "address for every equipment that is not a STB: From .33 until .200"
#define strInvalidStbRage "For STB, it can use addresses from .200 on !"
#define strInvalidOpt60	"option 60 can not be NULL!"
#define strInvalidOpchaddr	"invalid opch address"
#define strInvalidOpchport	"invalid opch port"

#define strSetPcStartIperror "set pc IP pool start address failed!"
#define strSetPcEndIperror "set pc IP pool end address failed!"
#define strSetCmrStartIperror "set camera IP pool start address failed!"
#define strSetCmrEndIperror "set camera IP pool end address failed!"
#define strSetStbStartIperror "set stb IP pool start address failed!"
#define strSetStbEndIperror "set stb IP pool end address failed!"
#define strSetPhnStartIperror "set phone IP pool start address failed!"
#define strSetPhnEndIperror "set phone IP pool end address failed!"
#endif

#define strDelChainerror "chain recordɾ������!" //Delete chain record error!
#define strModChainerror "chain record�޸Ĵ���!" //Modify chain record error!

#define strInvalDhcpsAddress "Invalid DHCPS address value!"
#define strSetDhcpserror "Set DHCPS MIB error!"

#define strStaticipexist "This static ip configuration already exists!"

#define strConnectExist "Connection already exists!"
#define strMaxVc "Error: Maximum number of VC exceeds !"
#define strAddChainerror "Error! Add chain record."
#define strTableFull "Error! Table Full."

#define strUserNametoolong "Error! User Name length too long."
#define strUserNameempty "Error! User Name cannot be empty."
#define strPasstoolong "Error! Password length too long."
#define strPassempty "Error! Password cannot be empty."
#define strInvalPPPType "Invalid PPP type value!"
#define strDrouteExist  "Default route already exists !"
#define strACName "Error! AC-Name length too long."
#define strServerName "Error! Service-Name length too long."

#define strInvalDHCP "Invalid WAN DHCP value!"
#define strInvalIP "Invalid IP-address value!"
#define strIPAddresserror "Error! IP address cannot be empty."
#define strInvalGateway "Invalid remote IP-address value!"
#define strGatewayIpempty "Error! Remote IP address cannot be empty."
#define strInvalMask "Invalid subnet mask value!"
#define strMaskempty "Error! Subnet mask cannot be empty."
#define strMruErr	"Invalid MTU!"

#define strMaxNumPPPoE "Maximum number of PPPoE connections exceeds in this vc!"



#define strSelectvc "Please select one vc!"
#define strGetChainerror "Get chain record error!\n"
#define strInvalidQos "Invalid QoS!"
#define strInvalidPCR "Invalid PCR!"
#define strInvalidCDVT "Invalid CDVT!"
#define strInvalidSCR "Invalid SCR!"
#define strInvalidMBS "Invalid MBS!"

#define strSetcommunityROerror "Set snmpCommunityRO mib error!"
#define strSetcommunityRWerror "Set snmpCommunityRW mib error!"
#define strInvalTrapIp "Invalid Trap IP value!"
#define strSetTrapIperror "Set snmpTrapIpAddr mib error!"


#define strGetToneerror "ERROR: Get ADSL Tone MIB error!"
#define  WARNING_EMPTY_OLD_PASSWORD  "ERROR: Old Password cannot be empty."
#define  WARNING_EMPTY_NEW_PASSWORD  "ERROR: New Password cannot be empty."
#define WARNING_EMPTY_CONFIRMED_PASSWORD  "ERROR: Confirmed Password cannot be empty."
#define WARNING_UNMATCHED_PASSWORD  "ERROR: New Password is not the same as Confirmed Password."
#define WARNING_GET_PASSWORD  "ERROR,Get password failed!"
#define WARNING_SET_PASSWORD  "ERROR: Modify password failed!"
#define WARNING_WRONG_PASSWORD  "ERROR: wrong password!"
#define WARNING_WRONG_USER  "ERROR: wrong username!"


#define COMMIT_REBOOT_TITLE    "Commit and Reboot"
#define COMMIT_REBOOT_INFO " This page is used to commit changes to system memory and reboot your system."
#define COMMIT_REBOOT_BUTTON  "<input type=\"submit\" value=\"Commit and Reboot\" name=\"save\" onclick=\"return saveClick()\">"

#define SYSTEM_LOG_TITLE "System Log"

//for wireless setting
#define   strDisbWlanErr				"�رմ���!" //Set disabled flag error!
#define   strInvdTxPower			"���Ϸ���TxPowerֵ!" //Invalid TxPower value!
#define   strSetMIBTXPOWErr		"MIB_TX_POWER�趨����!" //Set MIB_TX_POWER error!
#define   strInvdMode				"�趨ģʽ����!" //Invalid mode value!
#define   strSetWPAWarn			"�޷�����clientģʽ��WPA-RADIUS֮��!<br><br>���ڰ�ȫҳ����ļ���ģʽ" //You cannot set client mode with WPA-RADIUS!<br><br>Please change the encryption method in security page first.
#define   strSetWEPWarn			"�޷�����clientģʽ��WEP-802.1x֮��!<br><br>���ڰ�ȫҳ����ļ���ģʽ" //You cannot set client mode with WEP-802.1x!<br><br>Please change the encryption method in security page first.
#define   strSetMIBWLANMODEErr		"MIB_WLAN_MODE�趨����!" //Set MIB_WLAN_MODE error!
#define   strSetSSIDErr				"SSID�趨����!" //Set SSID error!
#define   strInvdChanNum			"���Ϸ���Ƶ��!"  //Invalid channel number!
#define   strSetChanErr				"Ƶ���趨����!" //Set channel number error!
#define   strInvdNetType			"���Ϸ�����·����!" //Invalid network type value!
#define   strSetMIBWLANTYPEErr		"MIB_WLAN_NETWORK_TYPE�趨����!" //Set MIB_WLAN_NETWORK_TYPE failed!
#define   strInvdAuthType			"����! ���Ϸ�����֤ģʽ" //Error! Invalid authentication value.
#define   strSetAuthTypeErr			"��֤�趨����!" //Set authentication failed!
#define   strSetAdapRateErr			"rate adaptive�趨����!" //Set rate adaptive failed!
#define   strSetFixdRateErr			"fix rate�趨����!" //Set fix rate failed!
#define   strSetBaseRateErr			"Tx basic rate�趨����!" //Set Tx basic rate failed!
#define   strSetOperRateErr			"Tx operation rate�趨����!" //Set Tx operation rate failed!
#define   strInvdTxRate				"���Ϸ���tx rateֵ!" //invalid value of tx rate!
#define   strInvdBrodSSID			"����! ���Ϸ���hiddenSSIDֵ" //Error! Invalid hiddenSSID value.
#define   strSetBrodSSIDErr			"hidden ssid�趨����!" //Set hidden ssid failed!
#define   strInvdProtection             	"���Ϸ���protection�趨"      //add by yq_zhou 2.10 //Invalid protection setting
#define   strInvdAggregation	                "���Ϸ���aggregation�趨" //Invalid aggregation setting
#define   strInvdShortGI0 	                "���Ϸ���short GI�趨" //Invalid short GI setting
#define   strSetProtectionErr          	"protection�趨����" //Set protection error
#define   strSetAggregationErr	  	"aggregation�趨����" //Set aggregation  error
#define   strSetShortGI0Err			"guard interval�趨����" //Set guard interval error
#define   strSetChanWidthErr  		"channel width�趨����" //Set channel width error
#define   strSet11NCoexistErr  		"11N co-existence�趨����" //Set 11N co-existence error
#define   strSetCtlBandErr 			"control sideband�趨����" //Set control sideband error
#define   strSetBandErr				"band�趨����!" //Set band error!
#define   strEnabAccCtlErr			"����access control�趨����!" //Set enabled access control error!
#define   strInvdMACAddr			"���Ϸ���MACλַ" //Error! Invalid MAC address.
#define   strAddAcErrForFull			"��Ŀ���Ѵ�����!" //Cannot add new entry because table is full!
#define   strMACInList				"��Ŀ�Ѵ���!" //Entry already exists!
#define   strAddListErr				"����! ����chain record" //Error! Add chain record.
#define   strDelListErr				"ɾ��chain record����!" //Delete chain record error!
#define   strFragThreshold			"����! ���Ϸ���fragment threshold" //Error! Invalid value of fragment threshold.
#define   strSetFragThreErr			"�趨fragment threshold����!" //Set fragment threshold failed!
#define   strRTSThreshold			"����! ���Ϸ���RTS threshold" //Error! Invalid value of RTS threshold.
#define   strSetRTSThreErr			"RTS threshold�趨����!" //Set RTS threshold failed!
#define   strInvdBeaconIntv			"����! ���Ϸ���Beacon Interval" //Error! Invalid value of Beacon Interval.
#define   strSetBeaconIntvErr		"Beacon interval�趨����!" //Set Beacon interval failed!
#define   strInvdPreamble			"����! ���Ϸ���Preambleֵ" //Error! Invalid Preamble value.
#define   strSetPreambleErr			"Preamble�趨����!" //Set Preamble failed!
#define   strInvdDTIMPerd			"����! ���Ϸ���DTIM����" //Error! Invalid value of DTIM period.
#define   strSetDTIMErr				"DTIM�����趨ʧ��!" //Set DTIM period failed!
#define   strInvdIAPP				"����! ���Ϸ���IAPPֵ" //Error! Invalid IAPP value.
#define   strMIBIAPPDISBErr			"MIB_WLAN_IAPP_DISABLED�趨ʧ��!" //Set MIB_WLAN_IAPP_DISABLED failed!
#define   strSetRelayBlockErr			"block relay�趨ʧ��!" //Set block relay failed!
#define   strSetLanWlanBlokErr		"block ethernet to wireless�趨ʧ��!" //Set block ethernet to wireless failed!
#define   strSetWlanWMMErr			"WMM(QoS) to wireless�趨ʧ��!" //Set WMM(QoS) to wireless failed!
#define   strMACAddr				"MAC��ַ" //MAC Address
#define   strSelect					"ѡ��" //Select
#define   strGetMBSSIBTBLErr		"����! MIB_MBSSIB_TBL��ȡ����" //Error! Get MIB_MBSSIB_TBL error.
#define   strGetVAPMBSSIBTBLErr		"����! formWlanMBSSID: MIB_MBSSIB_TBL for VAP SSID ��ȡ����" //Error! formWlanMBSSID: Get MIB_MBSSIB_TBL for VAP SSID error.
#define   strGetMULTIAPTBLErr		"����! formWlanMultipleAP: MIB_MBSSIB_TBL for VAP ��ȡ����" //Error! formWlanMultipleAP: Get MIB_MBSSIB_TBL for VAP error.
#define   strNotSuptSSIDType		"����! �޷�֧Ԯ��SSID����" //Error! Not support this SSID TYPE.
#define   strNoSSIDTypeErr			"����! �޴�SSID����" //Error! no SSID TYPE.
#define   strNoEncryptionErr			"����! �޴˼���ģʽ" //Error! no encryption method.
#define   strInvdEncryptErr			"���Ϸ��ļ���ģʽ!" //Invalid encryption method!
#define   strSetWLANENCRYPTErr		"MIB_WLAN_ENCRYPT�趨����!" //Set MIB_WLAN_ENCRYPT mib error!
#define   strSet8021xWarning		"����! 802.1x��֤�޷�����clientģʽ" //Error! 802.1x authentication cannot be used when device is set to client mode.
#define   strSet1xEnabErr			"1x enable flag�趨����!" //Set 1x enable flag error!
#define   strGetWLANWEPErr			"MIB_WLAN_WEP MIB ��ȡ����!" //Get MIB_WLAN_WEP MIB error!
#define   strSetWLANWEPErr			"WEP MIB�趨����!" //Set WEP MIB error!
#define   strSetWPARADIUSWarn		"����! WPA-RADIUS��֤�޷�����clientģʽ" //Error! WPA-RADIUS cannot be used when device is set to client mode.
#define   strInvdWPAAuthValue		"����! ���Ϸ���wpa��ֵ֤" //Error! Invalid wpa authentication value.
#define   strSetAUTHTYPEErr			"MIB_WLAN_AUTH_TYPE�趨����!" //Set MIB_WLAN_AUTH_TYPE failed!
#define   strNoPSKFormat			"����! û��psk��ʽ" //Error! no psk format.
#define   strInvdPSKFormat			"����! ���Ϸ���psk��ʽ" //Error! invalid psk format.
#define   strSetWPAPSKFMATErr		"MIB_WLAN_WPA_PSK_FORMAT�趨����!" //Set MIB_WLAN_WPA_PSK_FORMAT failed!
#define   strInvdPSKValue			"���Ϸ���pskֵ" //Error! invalid psk value.
#define   strSetWPAPSKErr			"MIB_WLAN_WPA_PSK�趨����!" //Set MIB_WLAN_WPA_PSK error!
#define   strInvdRekeyDay			"����! ���Ϸ���rekey dayֵ" //Error! Invalid value of rekey day.
#define   strInvdRekeyHr			"����! ���Ϸ���rekey hrֵ" //Error! Invalid value of rekey hr.
#define   strInvdRekeyMin			"����! ���Ϸ���rekey minֵ" //Error! Invalid value of rekey min.
#define   strInvdRekeySec			"����! ���Ϸ���rekey secֵ" //Error! Invalid value of rekey sec.
#define   strSetREKEYTIMEErr		"MIB_WLAN_WPA_GROUP_REKEY_TIME�趨����!" //Set MIB_WLAN_WPA_GROUP_REKEY_TIME error!
#define   strInvdRSPortNum			"����! ���Ϸ���RS�˿���ֵ" //Error! Invalid value of RS port number.
#define   strSetRSPortErr			"RS�˿��趨����!" //Set RS port error!
#define   strNoIPAddr				"û��RS IPλַ!" //No RS IP address!
#define   strInvdRSIPValue			"���Ϸ���RS IPλַ!" //Invalid RS IP-address value!
#define   strSetIPAddrErr			"RS IPλַ�趨����!" //Set RS IP-address error!
#define   strRSPwdTooLong			"RS�������!" //RS password length too long!
#define   strSetRSPwdErr			"RS�����趨����!" //Set RS password error!
#define   strInvdRSRetry			"���Ϸ���RS retryֵ!" //Invalid RS retry value!
#define   strSetRSRETRYErr			"MIB_WLAN_RS_RETRY�趨����!" //Set MIB_WLAN_RS_RETRY error!
#define   strInvdRSTime				"���Ϸ���RSʱ��ֵ!" //Invalid RS time value!
#define   strSetRSINTVLTIMEErr		"MIB_WLAN_RS_INTERVAL_TIME�趨����!" //Set MIB_WLAN_RS_INTERVAL_TIME error!
#define   strInvdWepKeyLen			"����! ���Ϸ���wepkeylenֵ" //Error! Invalid wepkeylen value.
#define   strSetMIBWLANWEPErr		"MIB_WLAN_WEP�趨����!" //Set MIB_WLAN_WEP failed!
#define   strGetMBSSIBTBLUpdtErr	"����! MIB_MBSSIB_TBL ��ȡ����(Updated)." //Error! Get MIB_MBSSIB_TBL error(Updated).
#define   strGetMBSSIBWEPTBLErr	"����! MIB_MBSSIB_WEP_TBL ��ȡ����" //Error! Get MIB_MBSSIB_WEP_TBL error.
#define   strKeyLenMustExist			"Key length�������!" //Key length must exist!
#define   strInvdKeyLen				"���Ϸ���key lengthֵ!" //Invalid key length value
#define   strKeyTypeMustExist		"Key type�������!" //Key type must exist!
#define   strInvdKeyType			"���Ϸ���key typeֵ!" //Invalid key type value!
#define   strSetWepKeyTypeErr		"WEP key type�趨����!" //Set WEP key type error!
#define   strInvdDeftKey				"Invalid default tx key id!"
#define   strSetDeftKeyErr			"Set default tx key id error!"
#define   strInvdKey1Len			"���Ϸ���key 1����!" //Invalid key 1 length!
#define   strInvdWEPKey1			"���Ϸ���wep-key1ֵ!" //Invalid wep-key1 value!
#define   strSetWEPKey1Err			"wep-key1�趨����!" //Set wep-key1 error!
#define   strInvdKey2Len			"���Ϸ���key 2����!" //Invalid key 2 length!
#define   strInvdWEPKey2			"���Ϸ���wep-key2ֵ!" //Invalid wep-key2 value!
#define   strSetWEPKey2Err			"wep-key2�趨����!" //Set wep-key2 error!
#define   strInvdKey3Len			"���Ϸ���key 3����!" //Invalid key 3 length!
#define   strInvdWEPKey3			"���Ϸ���wep-key3ֵ!" //Invalid wep-key3 value!
#define   strSetWEPKey3Err			"wep-key3�趨����!" //Set wep-key3 error!
#define   strInvdKey4Len			"���Ϸ���key 4����!" //Invalid key 4 length!
#define   strInvdWEPKey4			"���Ϸ���wep-key4ֵ!" //Invalid wep-key4 value!
#define   strSetWEPKey4Err			"wep-key4�趨����!" //Set wep-key4 error!
#define   strGetMBSSIBWEPTLBUpdtErr   "����! MIB_MBSSIB_WEP_TBL ��ȡ����(Updated)." //Error! Get MIB_MBSSIB_WEP_TBL error(Updated).
#define   strWdsComment			"Comment"
#define   strSetEnableErr   			"enabled flag�趨����!" //Set enabled flag error!
#define   strCommentTooLong 		"����! Comment����" //Error! Comment length too long.
#define   strGetEntryNumErr 			"��ȡ��Ŀ������!" //Get entry number error!
#define   strErrForTablFull 			"��Ŀ���Ѵ�����!" //Cannot add new entry because table is full!
#define   strAddEntryErr    			"������Ŀʧ��!" //Add table entry error!
#define   strDelRecordErr   			"chain recordɾ��ʧ��!" //Delete chain record error!


#define   IDS_RESULT_EFFECT           "Setting successful!"
#define   IDS_RESULT_OK               "OK"



//for web upgrade
#define   FILEOPENFAILED "File open error!"
#define   FILEWITHWRONGSIG	"Invalid signature!"

#define  INFO_ENABLED 	"Enabled"
#define  INFO_DISABLED	"Disabled"
//for tr069
#if defined(_CWMP_MIB_)

#define strSetCerPasserror  "Set CPE Certificat's Password error!"
#define strACSURLWrong  "ACS's URL����Ϊ��!" //ACS's URL can't be empty!
#define strSSLWrong "CPE��֧ԮSSL! URL��Ӧ��'https://'��ͷ!" //CPE does not support SSL! URL should not start with 'https://'!
#define strSetACSURLerror "ACS's URL �趨����!" //Set ACS's URL error!
#define strSetUserNameerror "�û������趨����!" //Set User Name error!
#define strSetPasserror "�����趨����!" //Set Password error!
#define strSetInformEnableerror "�趨�����ϱ�����ʧ��!" //Set Inform Enable error!
#define strSetInformIntererror "�趨�����ϱ����ʱ��ʧ��!" //Set Inform Interval error!
#define strSetConReqUsererror "�趨���������û�����ʧ��!" //Set Connection Request UserName error!
#define strSetConReqPasserror "�趨������������ʧ��!" //Set Connection Request Password error!
#define strSetCWMPFlagerror "CWMP_FLAG�趨����!" //Set CWMP_FLAG error!
#define strGetCWMPFlagerror "CWMP_FLAG��ȡ����!" //Get CWMP_FLAG error!
#define strUploaderror "�ϴ�ʧ��!" //Upload error!
#define strMallocFail "malloc failure!"
#define strArgerror "Insufficient args\n"
#endif

#ifdef CONFIG_MIDDLEWARE
#define strSetMidwarePortError "�趨middleware�˿ڴ���! " //set middleware port error!
#define strSetMidwareAddrError "�趨middleware��ַ����! " //set middleware address error!
#define strSetTR069EnableError "�趨tr069���ô���" //set tr069 enabling error! 
#define strSwitchTR069EnableError "���úͻ��֮�䲻��ֱ���л�! "
#endif

//for dns web fmdns.c
#define Tinvalid_DNS_mode "Invalid DNS mode value!"
#define TDNS_mib_get_error      "Get DNS MIB error!"
#define Tinvalid_DNS_address  	"Invalid DNS address value!"
#define TDNS_mib_set_error		"Set DNS MIB error!"

//for ipqos  fmqos.c  ip/portfilter, fmfwall.
#define Texceed_max_rules				"Error: Maximum number of Qos rule exceeds !"	
#define Tinvalid_source_ip   				"Error! Source IP."
#define Tinvalid_source_netmask 			"Error! Source Netmask"
#define Tinvalid_source_port 				"Error! Invalid source port."
#ifdef QOS_SPEED_LIMIT_SUPPORT
#define Tinvalid_pvc_bandwidth				"Error! Invalid pvc  bandwidth."
#endif
#define Tinvalid_destination_ip			"Error! Destination IP."
#define Tinvalid_destination_netmask		"Error! Destination Netmask."
#define Tinvalid_destination_port			"Error! Invalid destination port."
#define Tinvalid_speed					"Error!Invalid speed"
#define Tadd_chain_error					"Error! Add chain record."
#define Tdelete_chain_error				"Error! Delete chain record."
#define Tinvalid_if_ip					"Error! invalid IP addr!"
#define Tinvalid_if_mask				"Error! invalid mask addr."


//for firewall ip/port filter, fmfwall.c

#define Tprotocol_empty							"Error! Protocol type cannot be empty."
#define Tinvalid_port_range							"Error! port range error"
#define Tinvalid_rule_action						"Invalid Rule Action value!"
#define Tinvalid_source_mac						"Error! Invalid src MAC address."
#define Tinvalid_dest_mac						"Error! Invalid dst MAC address."
#define Toutgoing_ippfilter   						"Outgoing"
#define Tincoming_ippfilter						"Incoming"
#define Tdeny_ippfilter							"Deny"
#define Tallow_ippfilter						"Allow"
#define Tinvalid_rule							"Invaild! This is a duplicate or conflicting rule !"

//for dos
#define strSetDosSYSSYNFLOODErr   "Set DoS SYSSYN_FLOOD error!"
#define strSetDosSYSFINFLOODErr   "Set DoS SYSFIN_FLOOD error!" 
#define strSetDosSYSUDPFLOODErr   "Set DoS SYSUDP_FLOOD error!" 
#define strSetDosSYSICMPFLOODErr  "Set DoS SYSICMP_FLOOD error!"
#define strSetDosPIPSYNFLOODErr   "Set DoS PIPSYN_FLOOD error!"
#define strSetDosPIPFINFLOODErr   "Set DoS PIPFIN_FLOOD error!"
#define strSetDosPIPUDPFLOODErr   "Set DoS PIPUDP_FLOOD error!"
#define strSetDosPIPICMPFLOODErr  "Set DoS PIPICMP_FLOOD error!"
#define strSetDosIPBlockTimeErr   "Set DoS IP Block Timer error!"
#define strSetDosEnableErr        "Set DoS enable error!"

//for wan config(static pppoe IP)
#define Tip_addr		"IP Address"
#define Tdynamic_ip		"Dynamic IP"
#define Tstatic_ip		"Static IP"

#define Tinvalid_ip_net		"Invalid IP address! It should be set within the current subnet.\n"

#define TstrUrlExist        " FQDN already exists!"
#define TstrKeyExist        " KEYword already exists!"
#define TMaxUrl             "Maximum number of FQDN records exceeds!"
#define TMaxKey             "Maximum number of KEYWORD records exceeds!" 


#define strIpExist   "The IP addr has been exist!"
#define strMacExist	 "The MAC addr has been exist!"
#define strSetInterfaceerror   "Set ACL's Interface error!"
#define strSetACLCAPerror  "Set ACL Capability error!"

#define strSetNatSessionError "Set Nat session limitation error!"

#endif

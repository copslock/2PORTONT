<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�й�����-WLAN����</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--ϵͳ����css-->
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<!--ϵͳ�����ű�-->
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<script type="text/javascript" src="share.js"></script>
<SCRIPT language="javascript" type="text/javascript">

var ssidIdx = "0";//SSID������
var wpa="tkip+aes";//�����жϺ���л���
var mode = "open";//������֤��ʽ
var bcntpsubcriber = "1";//������֤��ʽ���أ�0��ʾ���ɱ༭��1��ʾ�ɱ༭
var wlssidlist="China_kkk";//�б���ʾ
var wep = "enabled";//sep���ֱܷ�Ϊ��enabled��disabled
var wlCorerev = "3";//WPA����:���ڵ���3ʱ,WPA �����б�ֵ�ֱ�Ϊ��"TKIP", "AES", "TKIP+AES"������WPA �����б�ֵ"TKIP"
var bit = "0";//��Կ����:WEP128=0;WEP64=1
var wpapskValue = "00005544";//WPAԤ������Կ
var wepkValue = "11111";//WEPԤ������Կ
var wpapskUpInter = "55";//WPA���»Ự��Կ���
var keyIdx = "1";//������Կ�����ŷֱ��Ӧ��1,2,3,4
var keys = new Array( "key1", "key2","key3", "key4" ); //������Կֵ�б�

var defPskLen, defPskFormat;
var wps20;
var oldMethod;
var wlanMode;
var encrypt_value;
var cipher_value;
var wpa2cipher_value;
var _wlan_mode=new Array();
var _encrypt=new Array();
var _enable1X=new Array();
var _wpaAuth=new Array();
var _wpaPSKFormat=new Array();
var _wpaPSK=new Array();
var _rsPort=new Array();
var _rsIpAddr=new Array();
var _rsPassword=new Array();
var _uCipher=new Array();
var _wpa2uCipher=new Array();
var _wepAuth=new Array();
var _wepLen=new Array();
var _wepKeyFormat=new Array();
var _wlan_isNmode=new Array();

function pin_window(pin)
{
	with (document.forms[0]) {
		var w = window.open("", "", "toolbar=no,width=500,height=100");
		w.document.write('<font size=5><b><center>' + pin + '</center></b></font>');
		w.document.close();
	}
}

function help_window()
{
	with(document.forms[0]) {
		var w = window.open("", "", "toolbar=no,width=500,height=100");
		w.document.write('If left blank, a new PIN will be generated when you click Configure.<br>');
		w.document.write('Use the link next to PIN textbox to show the generated PIN.');
		w.document.close();
	}
}

function wpapsk_window() 
{
	var psk_window = window.open("", "", "toolbar=no,width=500,height=100");
	psk_window.document.write(" WPA ��Ԥ������Կ�� " + wpapskValue);
	psk_window.document.close();
}

//cathy
function isHexaDigit(val)
{
	if(val>='a' && 'f'>=val)
		return true;
	else if(val>='A' && 'F'>=val)
		return true;
	else if(val>='0' && '9'>=val)
		return true;
	else
		return false;
}

function isValidwpapskValue(val) 
{
	var ret = false;
	var len = val.length;
	var maxSize = 64;
	var minSize = 8;

	if (len >= minSize && len < maxSize)
		ret = true;
	else if (len == maxSize) {
		for (i = 0; i < maxSize; i++)
			if (isHexaDigit(val.charAt(i)) == false)
				break;
		if (i == maxSize)
			ret = true;
	}
	else
		ret = false;

	return ret;
}

function isValidIpAddress(address) 
{
	var i = 0;

	if (address == '0.0.0.0' ||address == '255.255.255.255')
		return false;

	addrParts = address.split('.');
	if (addrParts.length != 4)
		return false;
	for (i = 0; i < 4; i++) {
		if (isNaN(addrParts[i]) || addrParts[i] =="")
			return false;
		num = parseInt(addrParts[i]);
		if (num < 0 || num > 255)
			return false;
	}
	return true;
}

//cathy
function isAllZero(val)
{
	var len = val.length;
	for (i=0; i<len; i++)
		if (i != '0')
			break;
	if (i == len)
		return true;
	else
		return false;
}

function isValidKey(val, size) 
{
	var ret = false;
	var len = val.length;
	var dbSize = size * 2;

	if (isAllZero(val))		//cathy
		return false;
	
	if (len == size)
		ret = true;
	else if (len == dbSize) {
		for (i = 0; i < dbSize; i++)
			if (isHexaDigit(val.charAt(i)) == false)
				break;
		if (i == dbSize)
			ret = true;
	}
	else
		ret = false;

	return ret;
}

/********************************************************************
**          on document submit
********************************************************************/

/*function on_submit() 
{
	var swep = getSelect(document.forms[0].wlWep);
	var swpa = getSelect(document.forms[0].wlWpa);

/*   if ( enblWireless == '0' ) 
   {
         alert('��Ϊ��ǰ���߱����ã���������.');
         return;
   }*/

/*	with (document.forms[0]) {
		var authMode = getSelect(wlAuthMode);
		var value;
		if (authMode.indexOf("psk")!= -1) {
			value = wlWpaPsk.value;
			if (isValidwpapskValue(value) == false) {
				alert('WPAԤ���õ���ԿӦ����8����63��ASCII�ַ���64��ʮ����������֮��.');
				return;
			}
		}

		if (authMode.indexOf("wpa")!= -1 || authMode.indexOf("psk")!= -1) {
			value = parseInt(wlwpapskUpInter.value);
			if (isNaN(value) == true || value < 0 || value > 0xffffffff) {
				alert('WPA Ⱥ�������Կ���ʱ�� "' + wlwpapskUpInter.value + '" Ӧ���� 0 �� 4294967295 ֮��.');
				return;
			}

/*		value = parseInt(wlNetReauth.value);*/
		
/*		if (  isNaN(value) == true || value < 0 || value > 0xffffffff ) 
		{
			alert('WPA ����������֤���ʱ��"' + wlNetReauth.value + '" Ӧ���� 0 �� 4294967295 ֮��.');
			return;
		}
	} */
      
/*	if (authMode.indexOf("wpa")!= -1 || authMode == 'radius') 
	{
	
		if ( isValidIpAddress(wlRadiusIPAddr.value) == false ) 
		{
			alert('RADIUS ������ IP ��ַ "' + wlRadiusIPAddr.value + '" ����Ч IP ��ַ.');
			return;
		}
	}*/

/*		if (getSelect(wlWep) == "enabled") {
			var i, val;
			var cbit = getSelect(wlKeyBit);
			var num = parseInt(getSelect(wlKeyIndex))-1;
			val = wlKeys[num].value;
			if (val == '' && !(swep == 'enabled' && authMode == 'radius')) {
				alert('����ѡ���ֵ��Կ.');
				return;
			}
			if (val != '') {
				if (cbit == '1') {
					if (isValidKey(val, 13) == false) {
						alert('Key "' + val + '" ��Ч. ������13��ASCII�ַ���26��ʮ����������Ϊ128λWEP��Կ.');
						return;
					}
				}
				else {
					if (isValidKey(val, 5) == false) {
						alert('Key "' + val + '" ��Ч. ������5��ASCII�ַ���10��ʮ����������Ϊһ��64λWEP��Կ.');
						return;
					}
				}
			}

			var tmplst = "";
			for (i=0; i<4; i++) {
				val = wlKeys[i].value;
				if (val != '') {
					if (cbit == '1') {
						if (isValidKey(val, 13) == false) {
							alert('Key "' + val + '" ��Ч. ������13��ASCII�ַ���26��ʮ����������Ϊ128λWEP��Կ.');
							return;
						}
					}
					else {
						if (isValidKey(val, 5) == false) {
							alert('Key "' + val + '" ��Ч. ������5��ASCII�ַ���10��ʮ����������Ϊһ��64λWEP��Կ.');
							return;
						}
					}
				}
				//if(tmplst.length != 0)
				if (i>0)
					tmplst += ";";
				tmplst += val;
			} 
			lst.value = tmplst;
		}
		submit();
	}
}*/

function get_by_id(id)
{
	with(document) {
		return getElementById(id);
	}
}

function show_wpa_settings()
{
	get_by_id("show_wpa_psk2").style.display = "";		
}

function show_authentication()
{	
	var security = get_by_id("security_method");
	var form1 = document.formEncrypt;

	if (wlanMode==1 && security.value == 6) {	/* client and WIFI_SEC_WPA2_MIXED */
		alert("Not allowed for the Client mode.");
		security.value = oldMethod;
		return false;
	}
	oldMethod = security.value;
	get_by_id("show_wep_auth").style.display = "none";
	get_by_id("setting_wep").style.display = "none";
	get_by_id("setting_wpa").style.display = "none";
	get_by_id("show_wpa_cipher").style.display = "none";
	get_by_id("show_wpa2_cipher").style.display = "none";

	if (security.value == 1) {	/* WIFI_SEC_WEP */
		get_by_id("show_wep_auth").style.display = "";
		if (wlanMode == 1)
			get_by_id("setting_wep").style.display = "";
		else {
			get_by_id("setting_wep").style.display = "";
		}
	}else if (security.value == 2 || security.value == 4 || security.value == 6){	/* WIFI_SEC_WPA/WIFI_SEC_WPA2/WIFI_SEC_WPA2_MIXED */
		form1.ciphersuite_t.checked = false;
		form1.ciphersuite_a.checked = false;
		form1.wpa2ciphersuite_t.checked = false;
		form1.wpa2ciphersuite_a.checked = false;
		get_by_id("setting_wpa").style.display = "";
		if (security.value == 2) {	/* WIFI_SEC_WPA */
			get_by_id("show_wpa_cipher").style.display = "";
			if(encrypt_value != security.value){
				form1.ciphersuite_t.checked = true;
			}
			else{
				if(cipher_value & 1)
					form1.ciphersuite_t.checked = true;
				if(cipher_value & 2)
					form1.ciphersuite_a.checked = true;
			}
		}
		if(security.value == 4) {	/* WIFI_SEC_WPA2 */
			get_by_id("show_wpa2_cipher").style.display = "";
			if(encrypt_value != security.value){
				form1.wpa2ciphersuite_t.checked = false;
				form1.wpa2ciphersuite_a.checked = true;
			}
			else{
				if(wpa2cipher_value & 1)
					form1.wpa2ciphersuite_t.checked = true;
				if(wpa2cipher_value & 2)
					form1.wpa2ciphersuite_a.checked = true;
			}
		}
		if(security.value == 6){	/* WIFI_SEC_WPA2_MIXED */
			get_by_id("show_wpa_cipher").style.display = "";
			get_by_id("show_wpa2_cipher").style.display = "";
			if(encrypt_value != security.value){
				form1.ciphersuite_t.checked = true;
				form1.ciphersuite_a.checked = false;
				form1.wpa2ciphersuite_t.checked = false;
				form1.wpa2ciphersuite_a.checked = true;
			}
			else{
				if(cipher_value & 1)
					form1.ciphersuite_t.checked = true;
				if(cipher_value & 2)
					form1.ciphersuite_a.checked = true;
				if(wpa2cipher_value & 1)
					form1.wpa2ciphersuite_t.checked = true;
				if(wpa2cipher_value & 2)
					form1.wpa2ciphersuite_a.checked = true;
			}
		}
		show_wpa_settings();
	}
}

function setDefaultKeyValue(form, wlan_id)
{
	if (form.elements["length"+wlan_id].selectedIndex == 0) {
		if (form.elements["format"+wlan_id].selectedIndex == 0) {
			form.elements["key"+wlan_id].maxLength = 5;
			form.elements["key"+wlan_id].value = "*****";	
/*		
			form.elements["key1"+wlan_id].maxLength = 5;
			form.elements["key2"+wlan_id].maxLength = 5;
			form.elements["key3"+wlan_id].maxLength = 5;
			form.elements["key4"+wlan_id].maxLength = 5;
			form.elements["key1"+wlan_id].value = "*****";
			form.elements["key2"+wlan_id].value = "*****";
			form.elements["key3"+wlan_id].value = "*****";
			form.elements["key4"+wlan_id].value = "*****";
*/		
		}
		else {
			form.elements["key"+wlan_id].maxLength = 10;
			form.elements["key"+wlan_id].value = "**********";
/*		
			form.elements["key1"+wlan_id].maxLength = 10;
			form.elements["key2"+wlan_id].maxLength = 10;
			form.elements["key3"+wlan_id].maxLength = 10;
			form.elements["key4"+wlan_id].maxLength = 10;
			form.elements["key1"+wlan_id].value = "**********";
			form.elements["key2"+wlan_id].value = "**********";
			form.elements["key3"+wlan_id].value = "**********";
			form.elements["key4"+wlan_id].value = "**********";
*/
		}
	}
	else {
		if (form.elements["format"+wlan_id].selectedIndex == 0) {
			form.elements["key"+wlan_id].maxLength = 13;		
			form.elements["key"+wlan_id].value = "*************";		
/*		
			form.elements["key1"+wlan_id].maxLength = 13;
			form.elements["key2"+wlan_id].maxLength = 13;
			form.elements["key3"+wlan_id].maxLength = 13;
			form.elements["key4"+wlan_id].maxLength = 13;
			form.elements["key1"+wlan_id].value = "*************";
			form.elements["key2"+wlan_id].value = "*************";
			form.elements["key3"+wlan_id].value = "*************";
			form.elements["key4"+wlan_id].value = "*************";
*/		
		}
		else {
			form.elements["key"+wlan_id].maxLength = 26;
			form.elements["key"+wlan_id].value ="**************************";		
/*		
			form.elements["key1"+wlan_id].maxLength = 26;
			form.elements["key2"+wlan_id].maxLength = 26;
			form.elements["key3"+wlan_id].maxLength = 26;
			form.elements["key4"+wlan_id].maxLength = 26;
			form.elements["key1"+wlan_id].value ="**************************";
			form.elements["key2"+wlan_id].value ="**************************";
			form.elements["key3"+wlan_id].value ="**************************";
			form.elements["key4"+wlan_id].value ="**************************";
*/		
		}
	}
	//form.elements["key"+wlan_id].value = wepkValue;
}

function updateWepFormat(form, wlan_id)
{
	if (form.elements["length" + wlan_id].selectedIndex == 0) {
		form.elements["format" + wlan_id].options[0].text = 'ASCII (5 characters)';
		form.elements["format" + wlan_id].options[1].text = 'Hex (10 characters)';
	}
	else {
		form.elements["format" + wlan_id].options[0].text = 'ASCII (13 characters)';
		form.elements["format" + wlan_id].options[1].text = 'Hex (26 characters)';
	}
	setDefaultKeyValue(form, wlan_id);
}

function check_wepkey()
{
	form = document.formEncrypt;
	var keyLen;
	if (form.length0.selectedIndex == 0) {
  		if (form.format0.selectedIndex == 0)
			keyLen = 5;
		else
			keyLen = 10;
	}
	else {
		if (form.format0.selectedIndex == 0)
			keyLen = 13;
		else
			keyLen = 26;
	}
	if (form.key0.value.length != keyLen) {
		alert('���Ϸ���key lengthֵ, ӦΪ' + keyLen + '��Ԫ');
		form.key0.focus();
		return 0;
	}
	if (form.key0.value == "*****" ||
		form.key0.value == "**********" ||
		form.key0.value == "*************" ||
		form.key0.value == "**************************" )
		return 1;

	if (form.format0.selectedIndex==0)
		return 1;

	for (var i=0; i<form.key0.value.length; i++) {
		if ( (form.key0.value.charAt(i) >= '0' && form.key0.value.charAt(i) <= '9') ||
			(form.key0.value.charAt(i) >= 'a' && form.key0.value.charAt(i) <= 'f') ||
			(form.key0.value.charAt(i) >= 'A' && form.key0.value.charAt(i) <= 'F') )
			continue;

		alert("���Ϸ���keyֵ, ӦΪʮ������ (0-9 or a-f).");
		form.key0.focus();
		return 0;
	}
	
	return 1;
}

function on_submit()
{
	form = document.formEncrypt;
	if (form.security_method.value == 0) {	/* WIFI_SEC_NONE */
		alert("����: δ�趨����! ��·���ܲ���ȫ!"); //Warning : security is not set!this may be dangerous!
	}
	if (form.security_method.value == 1) {	/* WIFI_SEC_WEP */
		if (check_wepkey() == false)
			return false;
		alert("֪ͨ: ��ʹ��WEPʱ, WPS�ᱻ�ر�!"); //Info : WPS will be disabled when using WEP!
	}
  else if (form.security_method.value == 2 || form.security_method.value == 4 || form.security_method.value == 6) {	/* WPA or WPA2 or WPA2 mixed */
		if (form.security_method.value == 2) {	/* WIFI_SEC_WPA */
			if (form.ciphersuite_t.checked == false && form.ciphersuite_a.checked == false) {
				alert("WPA����ѡ���Ϊ��"); //WPA Cipher Suite Can't be empty.
				return false;
			}
			if (form.ciphersuite_t.checked == true && form.ciphersuite_a.checked == true) {
				alert("����ͬʱѡ��TKIP and AES"); //Can't select TKIP and AES in the same time.
				return false;
			}
			alert("֪ͨ: ��ʹ��WPA onlyʱ, WPS�ᱻ�ر�!"); //Info : WPS will be disabled when using WPA only!
		}

    if (form.security_method.value == 4) {	/* WIFI_SEC_WPA2 */
			if (form.wpa2ciphersuite_t.checked == false && form.wpa2ciphersuite_a.checked == false) {
				alert("WPA2����ѡ���Ϊ��"); //WPA2 Cipher Suite Can't be empty.
				return false;
			}
			if (form.wpa2ciphersuite_t.checked == true && form.wpa2ciphersuite_a.checked == true) {
				alert("����ͬʱѡ��TKIP and AES"); //Can't select TKIP and AES in the same time.
				return false;
			}
			if (form.wpa2ciphersuite_t.checked == true) {
				if (wps20)
					alert("֪ͨ: ��ʹ��TKIP onlyʱ, WPS�ᱻ�ر�!"); //Info : WPS will be disabled when using TKIP only!
			}
    }
	if (form.security_method.value == 6) {	/* WIFI_SEC_WPA2_MIXED */
			if (wlanMode == 1 && ((form.ciphersuite_t.checked == true && form.ciphersuite_a.checked == true)
				|| (form.wpa2ciphersuite_t.checked == true && form.wpa2ciphersuite_a.checked == true))) {
				alert("��clientģʽ, ����ͬʱѡ��TKIP and AES"); //In the Client mode, you can't select TKIP and AES in the same time.
				return false;				
			}
			if (form.ciphersuite_t.checked == false && form.ciphersuite_a.checked == false) {
				alert("WPA����ѡ���Ϊ��"); //WPA Cipher Suite Can't be empty.
				return false;
			}
			if (form.wpa2ciphersuite_t.checked == false && form.wpa2ciphersuite_a.checked == false) {
				alert("WPA2����ѡ���Ϊ��"); //WPA2 Cipher Suite Can't be empty.
				return false;
			}

			if (wps20 && form.ciphersuite_t.checked == true && form.wpa2ciphersuite_t.checked == true
				&& form.ciphersuite_a.checked == false && form.wpa2ciphersuite_a.checked == false)
				alert("֪ͨ: ��ʹ��TKIP onlyʱ, WPS�ᱻ�ر�!"); //Info : WPS will be disabled when using TKIP only!
		}

		var str = form.pskValue.value;
		if (form.security_method.value > 1) {
			if (str.length < 8) {
				alert('�A������Կ����8����Ԫ'); //Pre-Shared Key value should be set at least 8 characters.
				form.pskValue.focus();
				return false;
			}
			if (str.length > 63) {
				alert('�A������Կ����64����Ԫ'); //Pre-Shared Key value should be less than 64 characters.
				form.pskValue.focus();
				return false;
			}
			if (checkString(form.pskValue.value) == 0) {
				alert('��Ч���A������Կ'); //Invalid Pre-Shared Key !
				form.pskValue.focus();
				return false;
			}
		}
	}

	with (document.forms[0]) {
		submit();
	}
}

function postSecurity(encrypt, enable1X, wpaAuth, wpaPSKFormat, wpaPSK, rsPort, rsIpAddr, rsPassword, uCipher, wpa2uCipher, wepAuth, wepLen, wepKeyFormat) 
{	
	document.formEncrypt.security_method.value = encrypt;
	encrypt_value = encrypt;
	cipher_value = uCipher;
	wpa2cipher_value = wpa2uCipher;

//	if (encrypt == 2 || encrypt == 3)	/* ENCRYPT_WPA_TKIP or ENCRYPT_WPA_AES */
//		document.formEncrypt.security_method.value = 2;	/* WIFI_SEC_WPA */
//	else if (encrypt == 4 || encrypt == 5)	/* ENCRYPT_WPA2_TKIP or ENCRYPT_WPA2_AES */
//		document.formEncrypt.security_method.value = 3;	/* WIFI_SEC_WPA2 */
//	else if (encrypt == 6)	/* ENCRYPT_WPA2_MIXED */
//		document.formEncrypt.security_method.value = 4;	/* WIFI_SEC_WPA2_MIXED */
//	else if (encrypt == 7)	/* ENCRYPT_WAPI */
//		document.formEncrypt.security_method.value = 5;	/* WIFI_SEC_WAPI */
//	else	/* ENCRYPT_DISABLED or ENCRYPT_WEP */
//		document.formEncrypt.security_method.value = encrypt;	/* WIFI_SEC_NONE or WIFI_SEC_WEP */

	document.formEncrypt.pskValue.value = wpaPSK;				

	document.formEncrypt.ciphersuite_t.checked = false;
	document.formEncrypt.ciphersuite_a.checked = false;
	if ( uCipher == 1 )
		document.formEncrypt.ciphersuite_t.checked = true;
	if ( uCipher == 2 )
		document.formEncrypt.ciphersuite_a.checked = true;
	if ( uCipher == 3 ) {
		document.formEncrypt.ciphersuite_t.checked = true;
		document.formEncrypt.ciphersuite_a.checked = true;
	}

	document.formEncrypt.wpa2ciphersuite_t.checked = false;
	document.formEncrypt.wpa2ciphersuite_a.checked = false;	
	if ( wpa2uCipher == 1 )
		document.formEncrypt.wpa2ciphersuite_t.checked = true;
	if ( wpa2uCipher == 2 )
		document.formEncrypt.wpa2ciphersuite_a.checked = true;
	if ( wpa2uCipher == 3 ) {
		document.formEncrypt.wpa2ciphersuite_t.checked = true;
		document.formEncrypt.wpa2ciphersuite_a.checked = true;
	}	

	document.formEncrypt.auth_type[wepAuth].checked = true;
	
	if ( wepLen == 0 )
		document.formEncrypt.length0.value = 1;
	else
		document.formEncrypt.length0.value = wepLen;
	
	document.formEncrypt.format0.value = wepKeyFormat+1;			
	show_authentication();

	defPskLen = document.formEncrypt.pskValue.value.length;
	updateWepFormat(document.formEncrypt, 0);
}

function SSIDSelected(index)
{
	wlanMode = _wlan_mode[index];
	document.formEncrypt.isNmode.value = _wlan_isNmode[index];
	postSecurity(_encrypt[index], _enable1X[index],
		_wpaAuth[index], _wpaPSKFormat[index], _wpaPSK[index],
		_rsPort[index], _rsIpAddr[index], _rsPassword[index],
		_uCipher[index], _wpa2uCipher[index], _wepAuth[index],
		_wepLen[index], _wepKeyFormat[index]);
}

function getUrlVars() {
    var vars = {};
    var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
        vars[key] = value;
    });
    return vars;
}
function returnUrl(){
	//var url = window.location.pathname+window.location.search;
	document.formEncrypt.elements["submit-url"].value = window.location.pathname+window.location.search;
	document.formEncrypt.elements["wlan_idx"].value = getUrlVars()['wlan_idx'];
	//alert("returnUrl");
	//return url;
}

function backToBasic() 
{
	var loc;
//	if(document.formEncrypt.elements["wlan_idx"].value!="")
//		loc = 'net_wlan_basic_11n.asp?wlan_idx='+wlan_idx;
//	else
		loc = 'net_wlan_basic_11n.asp';
	var code = 'location.assign("' + loc + '")';

	eval(code);
}
</SCRIPT>
</HEAD>
<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onload="returnUrl(); SSIDSelected(0)">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:10px">
<form action=/boaform/admin/formWlEncrypt method=POST name="formEncrypt">
<b>�������� -- ��ȫ</b><br>
<br>��ҳ��������LAN �ڵİ�ȫ���ԡ� ����������֤��ʽ�����ݼ��ܷ�ʽ������������֤��Կ����Կ���ȡ� <br>

<table width="540"  border="0" cellpadding="0" cellspacing="4">
	<input type=hidden name="isNmode" value=0>
	<tr>
	<tr>
		<td width="35%"> SSID����: </td>
		<td><select name="wpaSSID">
			<option value="0">SSID1</option>
			<!--% SSID_select %-->
		</select></td>
	</tr>
</table>
<table width="540" border="0" cellpadding="0" cellspacing="4">
	<tr><hr size=1 noshade align=top></tr>
	<tr><td width="35%">������֤��ʽ:</b>
		<select size="1" id="security_method" name="security_method" onChange="show_authentication()">
	  		<% checkWrite("wifiSecurity"); %> 
		</select>
	<!--	<select style="display:none" size="1" id="method" name="method" onChange="show_authentication()">
			<% checkWrite("wpaEncrypt"); %>
		</select> -->
	</td></tr>
	<tr id="show_wep_auth" style="display:none">
		<td colspan="2" width="100%">
			<table width="100%" border="0" cellpadding="0" cellspacing="4">
				<tr>
					<td width="30%">��֤:</td>
					<td width="70%">
						<input name="auth_type" type=radio value="open">����
						<input name="auth_type" type=radio value="shared">����
						<input name="auth_type" type=radio value="both">Auto
					</td>
				</tr>
			</table>
		</td></tr>

	<tr id="setting_wep" style="display:none">
		<td colspan="2" width="100%">
			<table width="100%" border="0" cellpadding="0" cellspacing="4">
				<input type="hidden" name="wepEnabled" value="ON" checked>
				<tr >
					<td width="30%">��Կ����:</td>
					<td width="70%"><select size="1" name="length0" id="length" onChange="updateWepFormat(document.formEncrypt, 0)">
						<option value=1> 64-bit</option>
						<option value=2>128-bit</option>
					</select></td>
				</tr>
				<tr>
					<td width="30%">��Կ:</td>
					<td width="70%"><select id="format" name="format0" onChange="setDefaultKeyValue(document.formEncrypt, 0)">
						<option value="1">ASCII</option>
						<option value="2">Hex</option>
					</select></td>
				</tr>
				<tr>
					<td width="30%">������Կ:</td>
					<td width="70%"><input type="text" id="key" name="key0" maxlength="26" size="26" value=""></td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td>128λ��ԿҪ������13��ASCII�ַ���26��ʮ����������64λ��ԿҪ������5��ASCII�ַ���10��ʮ����������</td>
				</tr>
			</table>
		</td>
	</tr>

	<tr id="setting_wpa" style="display:none">
		<td colspan="2">
			<table width="100%" border="0" cellpadding="0" cellspacing="4">			
				<input type="hidden" name="wpaAuth" value="psk" checked>
				<tr id="show_wpa_cipher" style="display:none">
					<td width="30%">WPA����:</td>
					<td width="70%">
						<input type="checkbox" name="ciphersuite_t" value=1>TKIP&nbsp;
						<input type="checkbox" name="ciphersuite_a" value=1>AES
					</td>
				</tr> 
				<tr id="show_wpa2_cipher" style="display:none">
					<td width="30%">WPA2����:</td>
					<td width="70%">
						<input type="checkbox" name="wpa2ciphersuite_t" value=1>TKIP&nbsp;
						<input type="checkbox" name="wpa2ciphersuite_a" value=1>AES
					</td>
				</tr> 

				<tr id="show_wpa_psk2" style="display:none">
					<td width="30%">WPA Ԥ������Կ:</td>
					<td width="70%"><input type="text" name="pskValue" id="wpapsk" size="32" maxlength="64" value=""></td>
					<!--td width="50%"><A HREF="javascript:wpapsk_window()">���������ʾ</A></td-->
				</tr>
			</table>
		</td>
	</tr>
</table>
<br><br>
<table>
	<tr>
		<td width='180'><input type='button' onClick='on_submit()' value='����/Ӧ��'></td>
		<td><input name="button" type='button' onClick ='history.back()' value='����'></td>
		<td>&nbsp;</td>
	</tr>
</table>

<input type='hidden' name="lst" >
<input type="hidden" name="submit-url" value="">
<input type="hidden" name="wlan_idx" value="">
<input type="hidden" name="wlan_idx2" value="222">
<script>
	//alert(getUrlVars()['wlan_idx']);
	//alert(window.location.pathname+window.location.search);
	
	//alert(document.getElementsByName("submit-url").value);
	<% initPage("wlwpa_mbssid"); %>
	<% checkWrite("wpsVer"); %>
	show_authentication();
	defPskLen = document.formEncrypt.pskValue.value.length;
	updateWepFormat(document.formEncrypt, 0);
	SSIDSelected(0);
</script>
</form>
</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>

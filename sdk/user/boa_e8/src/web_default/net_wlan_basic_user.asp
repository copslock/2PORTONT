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

var cgi_wlCurrentChannel = "11"; // ��ǰ�ŵ�
var cgi_wlBssid = "wlBssid"; // wlBssidֵ
var enbl = true; // ȡwlEnblֵ��true ��ʾ��������, false �����á�
var ssid = "ChinaNet-one"; // SSIDֵ
var txPower = "100"; // ���͹���
var hiddenSSID = true; // ȡ���㲥�Ŀ���, false ��ʾ���ù㲥��true ��ʾ�����ù㲥
var wlanMode = 1; // WLANģʽ
var regDomain, defaultChan;

<% init_wlan_page(); %>
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	with (document.wlanSetup) {
		/* Tsai: the initial value */
		wlEnbl.checked = enbl;
		wlSsid.value = ssid;
		wlHide.checked = hiddenSSID;

		/* Tsai: show or hide elements */
		wlSecCbClick(wlEnbl);
	}
}

/********************************************************************
**          on document update
********************************************************************/
function wlSecCbClick(cb) 
{
	var status = cb.checked;

	with (document.wlanSetup) {
		wlSsid.disabled = !status;
		wlHide.disabled = !status;

		wlSecInfo.style.display = status ? "block" : "none";
		adminWlinfo.style.display = status ? "block" : "none";
		advanced.style.display = status ? "block" : "none";
  	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	var str;

	with (document.wlanSetup) {
		if (wlEnbl.checked) {
			str = wlSsid.value;

			if (str.length == 0) {
				alert("SSID����Ϊ��.");
				return;
			}

			if (str.length > 32) {
				alert('SSID "' + str + '" ���ܴ���32���ַ���');
				return;
			}

			if (str.indexOf("ChinaNet-") != 0) {
				alert('SSID "' + str + '" ������ChinaNet-��ͷ�����������롣');
				return;
			}

			if (isIncludeInvalidChar(str)) {
				alert("SSID ���зǷ��ַ�������������!");
				return;
			}	
		}
		submit();
	}
}

function on_adv() 
{
	var loc = 'net_wlan_adv.asp';
	var code = 'location.assign("' + loc + '")';

	eval(code);
}

</script>
</head>
   
<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000"><!--onLoad='on_init()'-->
	<blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:10px">
	<form action=/boaform/formWlanSetup method="post" name="wlanSetup">
	<b>�������� -- ����</b><br>
	<br>
    ��ҳ��������LAN �ڵĻ������ԡ� �������û��������LAN�ڡ��ӹ���վ��APɨ�������� ����SSID�� ��������������(��SSID)��<br>
	<br>
    ���"����/Ӧ��"���������õĻ���������Ч��<br>
	<br>
	<table border="0" cellpadding="4" cellspacing="0">
		<tr>
			<td valign="middle" align="center" width="30" height="30">
				<input type='checkbox' name='wlEnbl' onClick='wlSecCbClick(this);' value="ON"></td>
			<td>��������</td>
		</tr>
	</table>
	<div id='wlSecInfo'>
	<table border="0" cellpadding="5" cellspacing="0">
		<tr>
			<td width="26%">SSID:</td>
			<td>
				<input type='text' name='wlSsid' maxlength="32" size="32" value="<% getInfo("ssid"); %>"></td>
		</tr>
		<tr>
			<td width="26%">BSSID:</td>
			<td>
				<script language="javascript">
					document.writeln(cgi_wlBssid);
				</script>
			</td>
		</tr>
	</table>
	</div>
	<div id="adminWlinfo"  style="display:none">
	<table width="337">
		<tr>
			<td width="26%">ȡ���㲥:</td>
			<td valign="middle" width="30" height="30">
				<input type='checkbox' name='wlHide' value="ON"></td>
		</tr>
		<tr> 
			<td width="26%">&nbsp;</td>
			<td colspan="2">&nbsp;</td>
		</tr>
	</table>          
	</div>                  
	<table width="295" border="0" cellpadding="4" cellspacing="0">
		<tr>
			<td><input type="hidden" value="/net_wlan_basic_user.asp" name="submit-url"></td>    
			<td width="162"><input type='button' onClick='on_submit()' value='����/Ӧ��'></td>
			<td><input type='button' onClick='on_adv()' name="advanced" value='�߼�'></td>
			<script>
				on_init();
			</script>
		</tr>
	</table>
	</form>
	</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>

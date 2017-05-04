<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�й�����-DHCP</TITLE>
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
<SCRIPT language="javascript" type="text/javascript">

var popUpWin=0;

var cgi = new Object();
<%init_dhcpmain_page();%>

var old_lan_ip = null;

/********************************************************************
**          on document load
********************************************************************/

function on_init()
{
	sji_docinit(document, cgi);
	typeClick();
	old_lan_ip = document.forms[0].uIp.value;
}

/********************************************************************
**          on document update
********************************************************************/
function typeClick(obj)
{
	with ( document.forms[0])
	{
		if(uDhcpType[0].checked  == true)
		{
			dhcpRangeStart.disabled = true;
			dhcpRangeEnd.disabled = true;
			//ipMask.disabled = true;//LGD_FOR_TR098
			ulTime.disabled = true;
			uServerIp.disabled = true;
		}
		else if(uDhcpType[1].checked  == true)
		{
			dhcpRangeStart.disabled = false;
			dhcpRangeEnd.disabled = false;
			//ipMask.disabled = false;//LGD_FOR_TR098
			ulTime.disabled = false;
			uServerIp.disabled = true;
		}
		else
		{
			dhcpRangeStart.disabled = true;
			dhcpRangeEnd.disabled = true;
			//ipMask.disabled = true;//LGD_FOR_TR098
			ulTime.disabled = true;
			uServerIp.disabled = false;
		}
	}
}

function popUpWindow(URLStr, left, top, width, height)
{
	if(popUpWin)
	{
		if(!popUpWin.closed) popUpWin.close();
	}
	popUpWin = open(URLStr, "popUpWin", "toolbar=yes,location=no,directories=no,status=no,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=yes,width="+width+",height="+height+",left="+left+", top="+top+",screenX="+left+",screenY="+top+"");
}

function dhcpDevice()
{
	var loc = "net_dhcpdevice.asp";
	var code = "location=\"" + loc + "\"";
	eval(code);
}

function isSameSubNet(lan1Ip, lan1Mask, lan2Ip, lan2Mask)
{
	var count = 0;

	lan1a = lan1Ip.split(".");
	lan1m = lan1Mask.split(".");
	lan2a = lan2Ip.split(".");
	lan2m = lan2Mask.split(".");

	for (i = 0; i < 4; i++)
	{
		l1a_n = parseInt(lan1a[i]);
		l1m_n = parseInt(lan1m[i]);
		l2a_n = parseInt(lan2a[i]);
		l2m_n = parseInt(lan2m[i]);
		if ((l1a_n & l1m_n) == (l2a_n & l2m_n))
			count++;
	}
	if (count == 4)
		return true;
	else
		return false;
}

/********************************************************************
**          on document submit
********************************************************************/

function on_submit(reboot)
{
	if(reboot)
	{
		var loc = "mgm_dev_reboot.asp";
		var code = "location.assign(\"" + loc + "\")";
		eval(code);
	}
	else
	{
		with ( document.forms[0] )
		{
			if ( sji_checkvip(uIp.value) == false )
			{
				uIp.focus();
				alert("IP��ַ \"" + uIp.value + "\" ����Ч��IP��ַ.");
				return;
			}
			if ( sji_checkmask(uMask.value) == false )
			{
				uMask.focus();
				alert("�������� \"" + uMask.value + "\" ����Ч����������.");
				return;
			}
			if ( uDhcpType[1].checked == true )
			{
				/*
				//LGD_FOR_TR098
				if ( sji_checkmask(ipMask.value) == false )
				{
					ipMask.focus();
					alert("DHCP �������� \"" + ipMask.value + "\" ����Ч����������.");
					return;
				}
				*/

				if (sji_checkvip(dhcpRangeStart.value) == false || !(isSameSubNet(uIp.value, uMask.value, dhcpRangeStart.value, uMask.value)))
				{
					dhcpRangeStart.focus();
					alert("��ʼIP��ַ\"" + dhcpRangeStart.value + "\"����ЧIP��ַ.");
					return;
				}
				if ( sji_checkvip(dhcpRangeEnd.value) == false || !(isSameSubNet(uIp.value, uMask.value, dhcpRangeEnd.value, uMask.value)))
				{
					dhcpRangeEnd.focus();
					alert("��ֹIP��ַ\"" + dhcpRangeEnd.value + "\"����ЧIP��ַ.");
					return;
				}
				if (sji_ipcmp(dhcpRangeStart.value, dhcpRangeEnd.value) > 0)
				{
					alert("��ֹIP��ַ������ڻ���ڳ�ʼIP��ַ.");
					return;
				}
			}
			else if ( uDhcpType[2].checked == true )
			{
				if ( sji_checkvip(uServerIp.value) == false )
				{
					uServerIp.focus();
					alert("DHCP������IP��ַ \"" + uServerIp.value + "\" ����Ч��IP��ַ.");
					return;
				}
			}
			if(old_lan_ip != uIp.value)
				alert("���Ѿ��� IP ��ַ�޸ĳ� \"" + uIp.value + "\"��֮�����ɴ� IP ��ַ����·������Ҳ��ǵ��޸�װ�õ�DHCP��ַ���䣬ȷ������װ�ÿ���˳��������");
			submit();
		}
	}
}
</script>
</head>
<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
	<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
		<blockquote>
			<DIV align="left" style="padding-left:20px; padding-top:10px">
				<form action=/boaform/formDhcpServer method=POST name="dhcpd">
					<b>������(LAN)&nbsp;����</b><br>
					<br>
					Ϊ���������ü�ͥ����IP��ַ����������ӿڡ�&nbsp;���"Ӧ��"��ťʹLAN������Ч��<!--&nbsp; "����"��ť����������������ݺ�����·������ʹ�µ����ó־���Ч��--><br>
					<div id="rstip" style="display:none;"><font color="red">��ʾ����ҳ������ã���Ҫ����·����������Ч��</font><br></div>
					<hr align="left" class="sep" size="1" width="90%">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td width="150">IP��ַ:</td>
						<td><input type="text" name="uIp" value=<% getInfo("dhcplan-ip"); %>></td>
						</tr>
						<tr>
						<td>��������:</td>
						<td><input type="text" name="uMask" value=<% getInfo("dhcplan-subnet"); %>></td>
						</tr>
					</table>
					<br>
					<div id="dhcpInfo">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td colspan="2"><input type="radio" name="uDhcpType" value = "0" onClick="typeClick(this);">&nbsp;&nbsp;����DHCP������</td>
						</tr>
						<tr>
						<td colspan="2"><input type="radio" name="uDhcpType" value = "1" onClick="typeClick(this);">&nbsp;&nbsp;����DHCP������</td>
						</tr>
						<tr>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;��ʼIP��ַ:</td>
						<td><input type="text" name="dhcpRangeStart" value=<% getInfo("lan-dhcpRangeStart") %>></td>
						</tr>
						<tr>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;��ֹIP��ַ:</td>
						<td><input type="text" name="dhcpRangeEnd" value=<% getInfo("lan-dhcpRangeEnd") %>></td>
						</tr>
<!--
						<tr>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;��������:</td>
						<td><input type="text" name="ipMask"></td>
						<td><input type="hidden" name="ipMask"></td>
						</tr>
-->
						<tr>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;����ʱ��:</td>
						<td><select size="1" name="ulTime">
						<option value="60">һ����</option>
						<option value="3600">һСʱ</option>
						<option  value="86400" >һ��</option>
						<option value="604800">һ��</option>
						</select>
						</td>
						</tr>
					</table>
					</div>

					<div id="relayInfo">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td colspan="2"><input type="radio" name="uDhcpType" value = "2" onClick="typeClick(this)">&nbsp;&nbsp;����DHCP�������м�</td>
						</tr>
						<tr>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;DHCP������IP��ַ:&nbsp;&nbsp;</td>
						<td><input type="text" name="uServerIp" value=<% getInfo("wan-dhcps"); %>></td>
						</tr>
					</table>
					</div>
					<br>
<!--
					<b>&nbsp;&nbsp;Ԥ��IP��ַ</b><br>
					<br>
					&nbsp;&nbsp;ѡ��"�༭Ԥ��IP��ַ�б�" ������Ԥ��IP��ַ�б�<br>
					&nbsp;&nbsp;ע��1:���Ԥ��10��ip��ַ�������mac�� <br>
					&nbsp;&nbsp;ע��2:����������һ��Ԥ��IP�����������豸ʹ������Ч��<br>
					<br>
-->
					<table border="0" width="520">
						<tr>
						<td>
						<INPUT type="button" class="button" onClick="popUpWindow('net_mopreipaddr.asp',350,350,500,300);" value="�༭Ԥ��IP��ַ�б�">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
						<INPUT type="button" class="button" onClick="dhcpDevice()" value="�༭DHCP��ַ����">
						</td>
						</tr>
					</table>
					<br>

					<hr align="left" class="sep" size="1" width="90%">
					<br>
					<input type="button" class="button" onClick="on_submit(0);" value="Ӧ��">&nbsp; &nbsp; &nbsp; &nbsp;
					<!--<input type="button" class="button" onClick="on_submit(1);" value="����">-->
					<input type="hidden" name="submit-url" value="">
				</form>
			</div>
		</blockquote>
	</body>
<%addHttpNoCache();%>
<script>
	<% initPage("dhcp-mode"); %>

	var lease_time = <% getInfo("lan-dhcpLTime"); %>;

	if(lease_time == 60)
		document.dhcpd.ulTime.selectedIndex = 0;
	else if(lease_time == 3600)
		document.dhcpd.ulTime.selectedIndex = 1;
	else if(lease_time == 86400)
		document.dhcpd.ulTime.selectedIndex = 2;
	else if(lease_time == 604800)
		document.dhcpd.ulTime.selectedIndex = 3;
	else
		document.dhcpd.ulTime.selectedIndex = -1;
</script>
</html>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>д��ȼ�����</TITLE>
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

var cgi = new Object();
<%init_dhcp_device_page();%>

/********************************************************************
**          on document load
********************************************************************/

function on_init()
{
	sji_docinit(document, cgi);
}

/********************************************************************
**          on document submit
********************************************************************/

function on_submit()
{
	with ( document.forms[0] )
	{
		if ( sji_checkvip(pcRangeStart.value) == false )
		{
			alert("��ַ \"" + pcRangeStart.value + "\" ����ЧIP��ַ.");
			return;
		}
		if ( sji_checkvip(pcRangeEnd.value) == false )
		{
			alert("��ַ \"" + pcRangeEnd.value + "\" ����ЧIP��ַ.");
			return;
		}
		if ( sji_checkvip(cmrRangeStart.value) == false )
		{
			alert("��ַ \"" + cmrRangeStart.value + "\" ����ЧIP��ַ.");
			return;
		}
		if ( sji_checkvip(cmrRangeEnd.value) == false )
		{
			alert("��ַ \"" + cmrRangeEnd.value + "\" ����ЧIP��ַ.");
			return;
		}
		if ( sji_checkvip(stbRangeStart.value) == false )
		{
			alert("��ַ \"" + stbRangeStart.value + "\" ����ЧIP��ַ.");
			return;
		}
		if ( sji_checkvip(stbRangeEnd.value) == false )
		{
			alert("��ַ \"" + stbRangeEnd.value + "\" ����ЧIP��ַ.");
			return;
		}
		if ( sji_checkvip(phoneRangeStart.value) == false )
		{
			alert("��ַ \"" + phoneRangeStart.value + "\" ����ЧIP��ַ.");
			return;
		}
		if ( sji_checkvip(phoneRangeEnd.value) == false )
		{
			alert("��ַ \"" + phoneRangeEnd.value + "\" ����ЧIP��ַ.");
			return;
		}

		submit();
	}
}

function on_back()
{
	var loc = "net_dhcpd.asp";
	var code = "location=\"" + loc + "\"";
	eval(code);
}

</SCRIPT>
</HEAD>
	<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init()">
		<blockquote>
			<DIV align="left" style="padding-left:20px; padding-top:10px">
				<form action=/boaform/formIpRange method= "post">
					<b>DHCP��ַ�������ü����豸���͵�ӳ��</b><br>
					<div id="rstip" style="display:none;"><font color="red">��ʾ����ҳ������ã���Ҫ����·����������Ч��</font><br></div>
					<table class="flat" border="1" cellpadding="0" cellspacing="0">
						<tr>
						<td class="hd" width="150">PC��ʼ��ַ:</td>
						<td><input type="text" name="pcRangeStart" ></td>
						</tr>
						<tr>
						<td class="hd">PC��ֹ��ַ:</td>
						<td><input type="text" name="pcRangeEnd"></td>
						</tr>
						<tr>
						<td colspan="2">&nbsp;</td>
						</tr>
						<tr>
						<td class="hd">����ͷ��ʼ��ַ:</td>
						<td><input type="text" name="cmrRangeStart" ></td>
						</tr>
						<tr>
						<td class="hd">����ͷ��ֹ��ַ:</td>
						<td><input type="text" name="cmrRangeEnd"></td>
						</tr>
						<tr>
						<td colspan="2">&nbsp;</td>
						</tr>
						<tr>
						<td class="hd">��������ʼ��ַ:</td>
						<td><input type="text" name="stbRangeStart" ></td>
						</tr>
						<tr>
						<td class="hd">��������ֹ��ַ:</td>
						<td><input type="text" name="stbRangeEnd"></td>
						</tr>
						<tr>
						<td colspan="2">&nbsp;</td>
						</tr>
						<tr>
						<td class="hd">Phone��ʼ��ַ:</td>
						<td><input type="text" name="phoneRangeStart" ></td>
						</tr>
						<tr>
						<td class="hd">Phone��ֹ��ַ:</td>
						<td><input type="text" name="phoneRangeEnd"></td>
						</tr>
					</table>
					<br>
					&nbsp;
					<input type="button" class="button" onClick="on_back()" value="����">
					&nbsp; &nbsp; &nbsp;
					<input type="button" class="button" onClick="on_submit()" value="Ӧ��">
					<input type="hidden" name="submit-url" value="">
				</form>
			</DIV>
		</blockquote>
	</body>
<%addHttpNoCache();%>
</html>

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
<SCRIPT language="javascript" src="share.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">

var popUpWin=0;

//var cgi = new Object();

/********************************************************************
**          on document load
********************************************************************/

function on_init()
{
//	sji_docinit(document, cgi);
}

/********************************************************************
**          on document update
********************************************************************/

function popUpWindow(URLStr, left, top, width, height)
{
	if(popUpWin)
	{
		if(!popUpWin.closed) popUpWin.close();
	}
	popUpWin = open(URLStr, "popUpWin", "toolbar=yes,location=no,directories=no,status=no,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=yes,width="+width+",height="+height+",left="+left+", top="+top+",screenX="+left+",screenY="+top+"");
}


/********************************************************************
**          on document submit
********************************************************************/

function on_lanipv6addrform_submit(reboot)
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
			if ( document.lanipv6addrform.lanIpv6addr.value == "" )
			{
				document.lanipv6addrform.lanIpv6addr.focus();
				alert("IP��ַ \"" + document.lanipv6addrform.lanIpv6addr.value + "\" ����Ч��IP��ַ.");
				return false;
			}
			else
			{
				if ( isLinkLocalIpv6Address(document.lanipv6addrform.lanIpv6addr.value) == 0) {
					alert("��Ч��LAN IPv6��ַ!");	//Invalid LAN IPv6 address!
					document.lanipv6addrform.lanIpv6addr.focus();
					document.lanipv6addrform.lanIpv6addr.value ="<% checkWrite("lanipv6addr"); %>";
					return false;
				}

			}
			submit();
		}
	}
}




function prefixModeChange()
{
	with ( document.lanipv6prefixmodeform )
	{
		var prefix_mode =ipv6lanprefixmode.value;
		
		v6delegated_WANConnection.style.display = 'none';
		staticipv6prefix.style.display = 'none';
		switch(prefix_mode){
			case '0': //WANDelegated
					v6delegated_WANConnection.style.display = 'block';
					break;
			case '1': //Static
					staticipv6prefix.style.display = 'block';
					break;
					
		}
	}
}

function dnsModeChange()
{
	with ( document.lanipv6dnsform )
	{
		var dns_mode =ipv6landnsmode.value;
		
		v6dns_WANConnection.style.display = 'none';	
		v6dns_Staic.style.display = 'none';
		switch(dns_mode){
			case '0': //HGWProxy
					break;
			case '1': //WANConnection
					v6dns_WANConnection.style.display = 'block';
					break;
			case '2': //Static
					v6dns_Staic.style.display = 'block';
					if(Ipv6Dns1.value == "::") //clear the value
						Ipv6Dns1.value ="";
					if(Ipv6Dns2.value == "::")
						Ipv6Dns2.value ="";

					break;
					
		}
	}

}

//Handle DNSv6 mode
function on_lanipv6dnsform_submit(reboot)
{

	with ( document.lanipv6dnsform )
	{
	
		if ( ipv6landnsmode.value==2 ){  //static
				if(Ipv6Dns1.value == "" && Ipv6Dns2.value == "" )  //Both DNS setting is NULL
				{
					Ipv6Dns1.focus();
					alert("IPv6��DNS ��ַ " + Ipv6Dns1.value + "\" ����Ч��IPv6 Prefix ��ַ.");
					return false;
				}
				else if (Ipv6Dns1.value != "" || Ipv6Dns2.value != ""){
					if(Ipv6Dns1.value != "" ){
						if (! isUnicastIpv6Address( Ipv6Dns1.value) ){
								alert("��ѡ IPv6 DNS ��ַ\"" + Ipv6Dns1.value + "\"Ϊ��Ч��ַ�����������룡");
								Ipv6Dns1.focus();
								return false;
						}
					}
					if(Ipv6Dns2.value != "" ){
						if (! isUnicastIpv6Address( Ipv6Dns2.value) ){
								alert("��ѡ IPv6 DNS ��ַ\"" + Ipv6Dns2.value + "\"Ϊ��Ч��ַ�����������룡");
								Ipv6Dns2.focus();
								return false;
						}
					}
				}
		}	
		submit();
	}	
}

//Handle Prefix v6 mode
function on_lanipv6prefixmodeform_submit(reboot)
{
	with ( document.lanipv6prefixmodeform )
	{
			if ( ipv6lanprefixmode.value==1 ){
				if(document.lanipv6prefixmodeform.lanIpv6prefix.value == "" )
				{
					document.lanipv6prefixmodeform.lanIpv6prefix.focus();
					alert("IP��ַ \"" + document.lanipv6prefixmodeform.lanIpv6prefix.value + "\" ����Ч��IPv6 Prefix ��ַ.");
					return false;
				}
				else if ( validateKeyV6Prefix(document.lanipv6prefixmodeform.lanIpv6prefix.value) == 0) { //check if is valid ipv6 address
					alert("��Ч��LAN IPv6 ǰ׺����!");	
					document.lanipv6prefixmodeform.lanIpv6prefix.focus();
					return false;
				}
			}
			submit();
	}
}

/*
function on_lanipv6prefixform_submit(reboot)
{
	if(reboot)
	{
		var loc = "mgm_dev_reboot.asp";
		var code = "location.assign(\"" + loc + "\")";
		eval(code);
	}
	else
	{
		with ( document.lanipv6prefixform )
		{
			if ( document.lanipv6prefixform.lanIpv6prefix.value == "" )
			{
				document.lanipv6addrform.lanIpv6prefix.focus();
				alert("IP��ַ \"" + document.lanipv6prefixform.lanIpv6prefix.value + "\" ����Ч��IPv6 Prefix ��ַ.");
				return false;
			}
		
			else
			{
				if ( validateKeyV6Prefix(document.lanipv6prefixform.lanIpv6prefix.value) == 0) {
					alert("��Ч��LAN IPv6 ǰ׺����!");	
					document.formlanipv6prefix.lanIpv6prefix.focus();
					document.formlanipv6prefix.lanIpv6prefix.value ="<% checkWrite("lanIpv6prefix"); %>";
					return false;
				}
			}
					
			submit();
		}
	}
}
*/

</script>
</head>
<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
	<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
		<blockquote>
			<DIV align="left" style="padding-left:20px; padding-top:10px">
				<form action=/boaform/formlanipv6 method=POST name="lanipv6addrform">
					<b>������(LAN) IPv6��ַ&nbsp;����</b><br>
					<br>
					Ϊ���������ü�ͥ����IPv6��ַ��&nbsp;���"Ӧ��"��ťʹLAN IPv6��ַ������Ч��<!--&nbsp; "����"��ť����������������ݺ�����·������ʹ�µ����ó־���Ч��--><br>
					<div id="rstip" style="display:none;"><br></div>
					<hr align="left" class="sep" size="1" width="90%">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td width="150">IPv6��ַ:</td>
						<td><input type="text" name="lanIpv6addr" value=<% checkWrite("lanipv6addr"); %>></td>
						</tr>
					</table>

					<hr align="left" class="sep" size="1" width="90%">
					<br>
					<input type="button" class="button" onClick="on_lanipv6addrform_submit(0);" value="����/Ӧ��">&nbsp; &nbsp; &nbsp; &nbsp;
					<input type="hidden" value="/ipv6.asp" name="submit-url">
				</form>
			</div>
			
			<DIV align="left" style="padding-left:20px; padding-top:10px">
				<form action=/boaform/formlanipv6dns method=POST name="lanipv6dnsform">
					<b>������(LAN) DNS Server&nbsp;����</b><br>
					<br>
					Ϊ���������ü�ͥ���� LAN IPv6 DNS Server��&nbsp;���"Ӧ��"��ťʹLAN IPv6 DNS Server������Ч��<br><!--&nbsp; "����"��ť����������������ݺ�����·������ʹ�µ����ó־���Ч��--><br>
					<div id="rstip" style="display:none;"><br></div>
					<hr align="left" class="sep" size="1" width="90%">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td width="150">LAN��DNS��ȡ��ʽ</td>
						<td><select name="ipv6landnsmode"  onChange="dnsModeChange()">
  							<option value="0">HGWProxy</option>
							<option value="1">WANConnection</option>
  							<option value="2">Static</option>
							</select></td> 
						</tr>
						<tr><td><div id='v6dns_WANConnection' style="display:none;"> �ӿ�: <select name="ext_if" > <% if_wan_list("rtv6"); %> </select></div> </td><tr>
						
						
					</table>
					
					<div id='v6dns_Staic' style="display:none;"> 
					<table border="0" cellpadding="0" cellspacing="0">
						<tr nowrap>		
					<tr>
						<td width="150px">��ѡ IPv6 DNS:</td>
						<td><input type="text" name="Ipv6Dns1" size="36" maxlength="39" value=<% getInfo("wan-dnsv61"); %> style="width:150px"></td>
					</tr>
					<tr>
						<td width="150px">��ѡ IPv6 DNS:</td>
						<td><input type=text name="Ipv6Dns2" size="36" maxlength="39" value=<% getInfo("wan-dnsv62"); %> style="width:150px"></td>
					</tr>
					</table>
						</div>

					<hr align="left" class="sep" size="1" width="90%">
					<br>
					<input type="button" class="button" onClick="on_lanipv6dnsform_submit(0);" value="����/Ӧ��">&nbsp; &nbsp; &nbsp; &nbsp;
					<input type="hidden" value="/ipv6.asp" name="submit-url">
				</form>
			</div>
			
			
									<DIV align="left" style="padding-left:20px; padding-top:10px">
				<form action=/boaform/formlanipv6prefix method=POST name="lanipv6prefixmodeform">
					<b>ǰ���Դ����</b><br>
					<br>
					Ϊ���������ü�ͥ���� LAN IPv6 ǰ���Դ;���"Ӧ��"��ťʹLAN IPv6 ǰ���Դ������Ч��<br>
					<div id="rstip" style="display:block;"><br></div>
					<hr align="left" class="sep" size="1" width="90%">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td width="150">LAN��ǰ꡻�ȡ��ʽ</td>
						<td><select name="ipv6lanprefixmode"  onChange="prefixModeChange()">
  							<option value="0">WANDelegated </option>
							<option value="1">Static</option>
							</select></td> 
						</tr>
						<tr><td><div id='v6delegated_WANConnection' style="display:none;"> �ӿ�: <select name="ext_if" > <% if_wan_list("rtv6"); %> </select></div> </td><tr>
						
						
					</table>
					
					<div id="staticipv6prefix" style="display:none;">
					<table border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td width="150">IPv6 ǰ׺ : (��: 2001::/64)</td>
						<td><input type="text" name="lanIpv6prefix" value=<% checkWrite("lanipv6prefix"); %>></td>
						</tr>
					</table>
					</div>

					<hr align="left" class="sep" size="1" width="90%">
					<br>
					<input type="button" class="button" onClick="on_lanipv6prefixmodeform_submit(0);" value="����/Ӧ��">&nbsp; &nbsp; &nbsp; &nbsp;
					<input type="hidden" value="/ipv6.asp" name="submit-url">
				</form>
			</div>
		</blockquote>
		
		
<script>

	ifIdx = <% getInfo("prefix-delegation-wan-conn"); %>;
	if (ifIdx != 65535)
		document.lanipv6prefixmodeform.ext_if.value = ifIdx;
	else
		document.lanipv6prefixmodeform.ext_if.selectedIndex = 0;

	document.lanipv6prefixmodeform.ipv6lanprefixmode.value = <% getInfo("prefix-mode"); %>;
	
		ifIdx = <% getInfo("dns-wan-conn"); %>;
	if (ifIdx != 65535)
		document.lanipv6dnsform.ext_if.value = ifIdx;
	else
		document.lanipv6dnsform.ext_if.selectedIndex = 0;
	
	document.lanipv6dnsform.ipv6landnsmode.value=<% getInfo("dns-mode"); %>;
	
	prefixModeChange();
	dnsModeChange();
</script>
	</body>
</html>

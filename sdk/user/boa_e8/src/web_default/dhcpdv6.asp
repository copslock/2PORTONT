<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>DHCP Server Setup</TITLE>
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
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<% language=javascript %>
<SCRIPT language="javascript" type="text/javascript">

function showDhcpv6Svr(ip)
{
	var html;
	
	if (document.dhcpd.dhcpdenable[0].checked == true)
		document.getElementById('displayDhcpSvr').innerHTML=
			'<input type="submit" value="����/Ӧ��" name="save">&nbsp;&nbsp;';
	else if (document.dhcpd.dhcpdenable[1].checked == true)
		document.getElementById('displayDhcpSvr').innerHTML=
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			'<tr><b>DHCPv6 Relay ����</b></tr>'+			
			'<tr><hr size=1 noshade align=top></tr>'+
			'</table>'+			
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			   '<tr>'+
      			       '<td width="30%">Upper Interface:</td>'+
                               '<td width="35%">'+
      	                           '<select name="upper_if">'+
      	                           '<% if_wan_list("all2");  %>'+    	                           
      	                           '</select>'+     	
                               '</td>'+
                           '</tr>'+			   
			'</table>'+			
			'<input type="submit" value="����/Ӧ��" name="save">&nbsp;&nbsp;';
		
	else if (document.dhcpd.dhcpdenable[2].checked == true) {
		html=
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			'<tr><b>�ֶ�����DHCPv6 Server</b></tr>'+					
		 	'</table>'+
			'<table border=0 width="500" cellspacing=4 cellpadding=0>';
		html+='<tr>'+
				'<td width="30%">IP Pool Range start:</td>'+
				'<td width="70%"><input type="text" name="dhcpRangeStart" size=40 maxlength=39 value="<% getInfo("dhcpv6s_range_start"); %>">&nbsp;</td>'+
				'</tr>';		
		html+='<tr>'+
				'<td width="30%">IP Pool Range end:</td>'+
				'<td width="70%"><input type="text" name="dhcpRangeEnd" size=40 maxlength=39 value="<% getInfo("dhcpv6s_range_end"); %>">&nbsp;</td>'+
				'</tr>';		
		html += '<tr>'+
			'<td width="30%">ǰꡳ���:</td>'+
			'<td width="70%">'+
			'<input type="text" name="prefix_len" size=10 maxlength=3 value="<% getInfo("dhcpv6s_prefix_length"); %>">'+
			'</td>'+
			'</tr>';
		html += '<tr>'+
			'<td width="30%">��Ч����:</td>'+
			'<td width="70%">'+
			'<input type="text" name="Dltime" size=10 maxlength=9 value="<% getInfo("dhcpv6s_default_LTime"); %>"> seconds'+
			'</td>'+
			'</tr>'+
			'<tr>'+
			'<td width="30%">��ѡ����:</td>'+
			'<td width="70%">'+
			'<input type="text" name="PFtime" size=10 maxlength=9 value="<% getInfo("dhcpv6s_preferred_LTime"); %>"> seconds'+
			'</td>'+
			'</tr>'+
			'<tr>'+
			'<td width="30%">Renew Time:</td>'+
			'<td width="70%">'+
			'<input type="text" name="RNtime" size=10 maxlength=9 value="<% getInfo("dhcpv6s_renew_time"); %>"> seconds'+
			'</td>'+
			'</tr>'+
			'<tr>'+
			'<td width="30%">Rebind Time:</td>'+
			'<td width="70%">'+
			'<input type="text" name="RBtime" size=10 maxlength=9 value="<% getInfo("dhcpv6s_rebind_time"); %>"> seconds'+
			'</td>'+
			'</tr>'+
			'<tr>'+
			'<td width="30%">Client DUID:</td>'+
			'<td width="70%">'+
			'<input type="text" name="clientID" size=42 maxlength=41 value="<% getInfo("dhcpv6s_clientID"); %>">'+
			'</td>'+
			'</tr>'+							
			'</table>'+
			'<input type="submit" value="����/Ӧ��" name="save" onClick="return saveChanges()">&nbsp;&nbsp;'+
			
			'<tr><hr size=1 noshade align=top></tr>'+
			'<tr>'+
				'<td><b>domain:</b></td>'+
				'<td><input type="text" name="domainStr" size="15" maxlength="50">&nbsp;&nbsp;</td>'+
				'<td><input type="submit" value="���" name="addDomain">&nbsp;&nbsp;</td>'+
			'</tr>'+ 
			'<br>'+
			'<br>'+
			'<table border=0 width="300" cellspacing=4 cellpadding=0>'+
			'<tr>Domain Search Table:</tr>'+
			<% showDhcpv6SDOMAINTable(); %>
			'</table>'+
			'<br>'+	
			'<input type="submit" value="ɾ��" name="delDomain">&nbsp;&nbsp;'+
			'<input type="submit" value="ɾ��ȫ��" name="delAllDomain">&nbsp;&nbsp;&nbsp;'+
			 
			'<br>'+
			'<br>'+	
			'<tr><hr size=1 noshade align=top></tr>'+
			
			'<tr>'+
				'<td><b>DNS��������ַ:</b></td>'+
				'<td><input type="text" name="nameServerIP" size="15" maxlength="40">&nbsp;&nbsp;</td>'+
				'<td><input type="submit" value="���" name="addNameServer">&nbsp;&nbsp;</td>'+
			'</tr>'+ 
			'<br>'+
			'<br>'+
			'<table border=0 width="300" cellspacing=4 cellpadding=0>'+
			'<tr>Name Server Table:</tr>'+
			<% showDhcpv6SNameServerTable(); %>
			'</table>'+
			'<br>'+	
			'<input type="submit" value="ɾ��" name="delNameServer">&nbsp;&nbsp;'+
			'<input type="submit" value="ɾ��ȫ��" name="delAllNameServer">&nbsp;&nbsp;&nbsp;';
		
		document.getElementById('displayDhcpSvr').innerHTML=html;		
	}
	else if (document.dhcpd.dhcpdenable[3].checked == true){
/*		document.getElementById('displayDhcpSvr').innerHTML=
			'<tr>'+
			    '<b>�Զ�����DHCPv6 Server.'+
			'</tr><br>'+
			'<input type="submit" value="����/Ӧ��" name="save">&nbsp;&nbsp;';
*/

	html=
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			'<tr><b>����DHCPv6 Server</b></tr>'+					
		 	'</table>'+
			'<table border=0 width="500" cellspacing=4 cellpadding=0>';
		html+='<tr>'+
				'<td width="30%">��ַ������ʼ��ַ:</td>'+
				'<td width="70%"><input type="text" name="dhcpRangeStart" size=20 maxlength=19 value="<% getInfo("dhcpv6s_min_address"); %>">&nbsp;(IP��ַ�ĺ�64λ)</td>'+
				'</tr>';		
		html+='<tr>'+
				'<td width="30%">��ַ���������ַ:</td>'+
				'<td width="70%"><input type="text" name="dhcpRangeEnd" size=20 maxlength=19 value="<% getInfo("dhcpv6s_max_address"); %>">&nbsp;(IP��ַ�ĺ�64λ)</td>'+
				'</tr>'+					
			'</table>'+
			'<input type="submit" value="����/Ӧ��" name="save" onClick="return saveChanges()">&nbsp;&nbsp;';
			
		
		document.getElementById('displayDhcpSvr').innerHTML=html;		
    }
}

function checkDigitRange_leaseTime(str, min)
{
  d = parseInt(str, 10);
  if ( d < min || d == 0)
      	return false;
  return true;
}

function validateKey_leasetime(str)
{
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
    		(str.charAt(i) == '-' ) )
			continue;
	return 0;
  }
  return 1;
}


function saveChanges()
{
	if (document.dhcpd.dhcpRangeStart.value =="") {
		alert(document.dhcpd.dhcpRangeStart.value + "Ϊ��Ч��ַ������ʼ��ַ(IP��ַ�ĺ�64λ)����ʽΪ����xxxx:xxxx:xxxx:xxxx����x Ϊ16��λ, EX:1:1:a:a ���������룡");
		document.dhcpd.dhcpRangeStart.value = document.dhcpd.dhcpRangeStart.defaultValue;
		document.dhcpd.dhcpRangeStart.focus();
		return false;
	} else {
		if (! isUnicastIpv6AddressForDHCPv6( '0::'+document.dhcpd.dhcpRangeStart.value) ){
			alert(document.dhcpd.dhcpRangeStart.value + "Ϊ��Ч��ַ������ʼ��ַ(IP��ַ�ĺ�64λ)����ʽΪ����xxxx:xxxx:xxxx:xxxx����x Ϊ16��λ, EX:1:1:a:a ���������룡");
			document.dhcpd.dhcpRangeStart.focus();
			return false;
		}
	}
	
	
	if (document.dhcpd.dhcpRangeEnd.value =="") {
		alert(document.dhcpd.dhcpRangeEnd.value + "Ϊ��Ч��ַ���������ַ(IP��ַ�ĺ�64λ)����ʽΪ����xxxx:xxxx:xxxx:xxxx����x Ϊ16��λ, EX:1:1:a:a ���������룡");
		document.dhcpd.dhcpRangeEnd.value = document.dhcpd.dhcpRangeEnd.defaultValue;
		document.dhcpd.dhcpRangeEnd.focus();
		return false;
	} else {
		if (! isUnicastIpv6AddressForDHCPv6( '0::'+document.dhcpd.dhcpRangeEnd.value) ){
			alert(document.dhcpd.dhcpRangeEnd.value + "Ϊ��Ч��ַ���������ַ(IP��ַ�ĺ�64λ)����ʽΪ����xxxx:xxxx:xxxx:xxxx����x Ϊ16��λ, EX:1:1:a:a ���������룡");
			document.dhcpd.dhcpRangeEnd.focus();
			return false;
		}
	}
	
/*
	if ( document.dhcpd.prefix_len.value=="") {
		alert("������IPǰ׺����"); //Please input IP prefix length.
		document.dhcpd.prefix_len.focus();
		return false;
	}
	if ( document.dhcpd.Dltime.value=="") {
		alert("������dhcp��Ч����"); //Please input dhcp default lease time.
		document.dhcpd.Dltime.focus();
		return false;
	}
	if ( validateKey_leasetime( document.dhcpd.Dltime.value ) == 0 ) {
		alert("��Ч��DHCP Server��Ч������ֵ"); //Invalid DHCP Server default lease time number.
		document.dhcpd.Dltime.value = document.dhcpd.Dltime.defaultValue;
		document.dhcpd.Dltime.focus();
		return false;
	}
	if ( !checkDigitRange_leaseTime(document.dhcpd.Dltime.value, 0) ) {
	  	alert('��Ч��DHCP Server��Ч����'); //Invalid DHCP Server default lease time.
		document.dhcpd.Dltime.value = document.dhcpd.Dltime.defaultValue;
		document.dhcpd.Dltime.focus();
		return false;
	}	 	
	
	if ( document.dhcpd.PFtime.value=="") {
		alert("������dhcp��ѡ����"); //Please input dhcp Prefered-lifetime.
		document.dhcpd.PFtime.focus();
		return false;
	}
	if ( validateKey_leasetime( document.dhcpd.PFtime.value ) == 0 ) {
		alert("��Ч��DHCP Server��ѡ������ֵ"); //Invalid DHCP Server Prefered-lifetime number.
		document.dhcpd.PFtime.value = document.dhcpd.PFtime.defaultValue;
		document.dhcpd.PFtime.focus();
		return false;
	}
	if ( !checkDigitRange_leaseTime(document.dhcpd.PFtime.value, 0) ) {
	  	alert('��Ч��DHCP Server��ѡ����'); //Invalid DHCP Server Prefered-lifetime.
		document.dhcpd.PFtime.value = document.dhcpd.PFtime.defaultValue;
		document.dhcpd.PFtime.focus();
		return false;
	}
	if ( document.dhcpd.RNtime.value=="") {
		alert("������dhcp��������ʱ��(Renew Time)"); //Please input dhcp Renew Time.
		document.dhcpd.RNtime.focus();
		return false;
	}
	if ( document.dhcpd.RBtime.value=="") {
		alert("������dhcp���°�ʱ��(Rebind Time)"); //Please input dhcp Rebind Time.
		document.dhcpd.RBtime.focus();
		return false;
	}
	if ( document.dhcpd.clientID.value=="") {
		alert("������dhcp Client OUID"); //Please input dhcp Client OUID.
		document.dhcpd.clientID.focus();
		return false;
	}
*/	
	return true;
}

function enabledhcpd()
{
	document.dhcpd.dhcpdenable[2].checked = true;
	//ip = ShowIP(document.dhcpd.lan_ip.value);
	showDhcpv6Svr();
}

function disabledhcpd()
{
	document.dhcpd.dhcpdenable[0].checked = true;
	showDhcpv6Svr();
}

function enabledhcprelay()
{
	document.dhcpd.dhcpdenable[1].checked = true;
	showDhcpv6Svr();
}

function autodhcpd()
{
	document.dhcpd.dhcpdenable[3].checked = true;
	showDhcpv6Svr();
}

</SCRIPT>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">

<form action=/boaform/formDhcpv6Server method=POST name="dhcpd">
<div class="tip" style="width:90% ">
	<b>DHCPV6 ����</b><br><br>	
</div>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr>
  <td>DHCPv6 Mode:   
  <% checkWrite("dhcpV6Mode"); %>

  </td>
  </tr>
</table>

<table border="0" width="500" cellpadding="0" cellspacing="0">
<hr size=2 noshade align=top>
  <tr><td ID="displayDhcpSvr"></td></tr>
</table>

   <br>
      <input type="hidden" value="/dhcpdv6.asp" name="submit-url">

<script>
	<% initPage("dhcpv6-mode"); %>	
	showDhcpv6Svr();
</script>


 </form>
 </DIV>
</blockquote>
</body>

</html>

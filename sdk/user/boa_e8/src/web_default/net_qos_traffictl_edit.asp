<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<html>
<head>
<title>QoS �����������</title>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--ϵͳ����css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<style>
SELECT {width:200px;}
</style>
<!--ϵͳ�����ű�-->
<script language="javascript" src="common.js"></script>
<script language="javascript" type="text/javascript">
var oifkeys = new it_nr("outifkey");
<% initOutif(); %>

function on_init_page() {
	with(document.forms[0]) {
	for(var i in oifkeys){
		if(i == "name"||i=="undefined" ||(typeof oifkeys[i] != "string" && typeof oifkeys[i] != "number")) continue;
		inflist.options.add(new Option(oifkeys[i],i));
	}
	}
	
	if ( <%checkWrite("IPv6Show");%> )
	{
		if (document.getElementById)  // DOM3 = IE5, NS6
		{
			document.getElementById('ipprotbl').style.display = 'block';						
		}
		else {
			if (document.layers == false) // IE4
			{
				document.all.ipprotbl.style.display = 'block';						
			}
		}
	}
}

// Mason Yu:20110524 ipv6 setting. START


function on_apply() {
	with(document.forms[0]) {
		if (inflist.value == " ")
		{
			inflist.focus();
			alert("δָ���ӿ�!");
			return;
		}
		if(srcip.value != "" && sji_checkip(srcip.value) == false)
		{
			srcip.focus();
			alert("ԴIP��ַ"+srcip.value+"��Ч��");
			return;
		}
		
		if(dstip.value != "" && sji_checkip(dstip.value) == false)
		{
			dstip.focus();
			alert("Ŀ��IP��ַ"+dstip.value+"��Ч��");
			return;
		}
		
		if(srcnetmask.value != "" && sji_checkip(srcnetmask.value) == false)
		{
			srcnetmask.focus();
			alert("ԴIP��������"+srcnetmask.value+"��Ч��");
			return;
		}
		
		if(dstnetmask.value != "" && sji_checkip(dstnetmask.value) == false)
		{
			dstnetmask.focus();
			alert("Ŀ��IP��������"+dstnetmask.value+"��Ч��");
			return;
		}
		if(sport.value <0 || sport.value > 65536)
		{
			sport.focus();
			alert("Դ�˿ں� "+ sport.value + "��Ч��");
			return;
		}
		if (sport.value > 0 && sport.value < 65535)
		{
			if (protolist.value == 3) {  //ICMP should not specify src port
				sport.focus();
				alert("��ָ��TCP/UDPЭ��!");
				return;
			}
		}
		if(dport.value <0 || dport.value > 65536)
		{
			dport.focus();
			alert("Ŀ�Ķ˿ں� "+ dport.value + "��Ч��");
			return;
		}
		if (dport.value > 0 && dport.value<65535)
		{
			if (protolist.value ==3) { //ICMP should not specify dst port
				dport.focus();
				alert("��ָ��TCP/UDPЭ��!");
				return;
			}
		}
		if(rate.value<=0)
		{
			rate.focus();
			alert("���� "+ rate.value + " kbs, ��Ч��");
			return;
		}		
		
		if((rate.value%512)!=0)
		{
			rate.focus();
			alert("���� "+ rate.value + " kbs, ��Ч��������512�ı���!");
			return;
		}			

		
		if ( <%checkWrite("IPv6Show");%> ) {
			// For IPv6
			if(document.forms[0].IpProtocolType.value == 2) {
				if(sip6.value != ""){
					if (! isGlobalIpv6Address(sip6.value) ){
						alert("Invalid Source IPv6 address!");					
						return;
					}
					if ( sip6PrefixLen.value != "" ) {
						var prefixlen= getDigit(sip6PrefixLen.value, 1);
						if (prefixlen > 128 || prefixlen <= 0) {
							alert("Invalid Source IPv6 prefix length!");							
							return;
						}
					}
				}
				
				if(dip6.value != ""){
					if (! isGlobalIpv6Address(dip6.value) ){
						alert("Invalid Destination IPv6 address!");					
						return;
					}
					if ( dip6PrefixLen.value != "" ) {
						var prefixlen= getDigit(dip6PrefixLen.value, 1);
						if (prefixlen > 128 || prefixlen <= 0) {
							alert("Invalid Destination IPv6 prefix length!");							
							return;
						}
					}
				}
			}
		}
		
		// For IPv4 and IPv6
		if ( <%checkWrite("IPv6Show");%> ) {
			// For IPv4
			if(document.forms[0].IpProtocolType.value == 1){				
				lst.value = "inf="+inflist.value+"&proto="+protolist.value+"&IPversion="+IpProtocolType.value+"&srcip="+srcip.value+"&srcnetmask="+srcnetmask.value+
					"&dstip="+dstip.value+"&dstnetmask="+dstnetmask.value+"&sport="+sport.value+"&dport="+dport.value+"&rate="+rate.value+"&direction="+direction.value;
			}
			// For IPv6
			else if (document.forms[0].IpProtocolType.value == 2) {				
				lst.value = "inf="+inflist.value+"&proto="+protolist.value+"&IPversion="+IpProtocolType.value+"&sip6="+sip6.value+"&sip6PrefixLen="+sip6PrefixLen.value+
					"&dip6="+dip6.value+"&dip6PrefixLen="+dip6PrefixLen.value+"&sport="+sport.value+"&dport="+dport.value+"&rate="+rate.value+rate.value+"&direction="+direction.value;
			}
		}
		// For IPv4 only
		else
		{
				lst.value = "inf="+inflist.value+"&proto="+protolist.value+"&srcip="+srcip.value+"&srcnetmask="+srcnetmask.value+
					"&dstip="+dstip.value+"&dstnetmask="+dstnetmask.value+"&sport="+sport.value+"&dport="+dport.value+"&rate="+rate.value+"&direction="+direction.value;
		}		
		submit();				
	}
}

function protocolChange()
{	
	// If protocol is IPv4 only.
	if(document.forms[0].IpProtocolType.value == 1){			
		if (document.getElementById)  // DOM3 = IE5, NS6
		{
			document.getElementById('ip4tbl').style.display = 'block';
			document.getElementById('ip6tbl').style.display = 'none';						
		}
		else {
			if (document.layers == false) // IE4
			{
				document.all.ip4tbl.style.display = 'block';
				document.all.ip6tbl.style.display = 'none';						
			}
		}			
	}
	// If protocol is IPv6 only.
	else if(document.forms[0].IpProtocolType.value == 2){			
		if (document.getElementById)  // DOM3 = IE5, NS6
		{
			document.getElementById('ip4tbl').style.display = 'none';
			document.getElementById('ip6tbl').style.display = 'block';						
		}
		else {
			if (document.layers == false) // IE4
			{
				document.all.ip4tbl.style.display = 'none';
				document.all.ip6tbl.style.display = 'block';						
			}
		}
	}	
}

</script>
</head>
<body  topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init_page();">
<blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px;">
		<form id="form" action="/boaform/admin/formQosTraffictlEdit" method="post">
			<p><b>�������������������͹���</b></p>			
			
			<div id='ipprotbl' style="display:none">
			<table cellSpacing="1" cellPadding="0" border="0">								
				<tr><td width="150px">IPЭ��汾:</td>
					<td><select id="IpProtocolType" onChange="protocolChange()" name="IpProtocolType">						
						<option value="1" > Ipv4</option>
						<option value="2" > Ipv6</option>
						</select>
					</td>
				</tr>					
			</table>
			</div>
						<div id='ip4tbl' style="display:none;">
						<table cellSpacing="1" cellPadding="0" border="0">	
				<tr>
					<td width="150px">�ӿڣ�</td>
					<td><select id="inflist"></select></td>
				</tr>
				</table>
				</div>
			<table cellSpacing="1" cellPadding="0" border="0">		
	
				<tr>
					<td width="150px">����</td>
					<td>
			  			<select name="direction">
							<option value="0">����</option>
							<option value="1">���� </option>
		      			</select>
					</td>
				</tr>				
				<tr>
					<td width="150px">Э�飺</td>
					<td>
			  			<select name="protolist">
							<option value="0">NONE</option>
							<option value="1">TCP</option>
							<option value="2">UDP </option>
							<option value="3">ICMP </option>
		      			</select>
					</td>
				</tr>
			</table>
			
			<div id='ip4tbl' style="display:block;">
			<table cellSpacing="1" cellPadding="0" border="0">
				<tr>
					<td width="150px">ԴIP��</td>
					<td><input type="text" name="srcip" size="15" maxlength="15" style="width:150px"></td>
				</tr>
				<tr>
			  		<td width="150px">Դ�������룺</td>
					<td><input type="text" name="srcnetmask" size="15" maxlength="15" style="width:150px"></td>
				</tr>
				<tr>
					<td width="150px">Ŀ��IP��</td>
					<td><input type="text" name="dstip" size="15" maxlength="15" style="width:150px"></td>
				</tr>
				<tr>
			  		<td width="150px">Ŀ���������룺</td>
					<td><input type="text" name="dstnetmask" size="15" maxlength="15" style="width:150px"></td>
				</tr>
			</table>
			</div>
			
			<div id='ip6tbl' style="display:none;">
			<table cellSpacing="1" cellPadding="0" border="0">
				<tr>
					<td width="150px">ԴIP��</td>
					<td><input type="text" name="sip6" size="26" maxlength="39" style="width:150px"></td>
				</tr>
				<tr>
			  		<td width="150px">Դǰꡳ���:</td>
					<td><input type="text" name="sip6PrefixLen" size="15" maxlength="15" style="width:150px"></td>
				</tr>
				<tr>
					 <td width="150px">Ŀ��IP��</td>
					<td><input type="text" name="dip6" size="26" maxlength="39" style="width:150px"></td>
				</tr>
				<tr>
			  		<td width="150px">Ŀ��ǰꡳ��ȣ�</td>
					<td><input type="text" name="dip6PrefixLen" size="15" maxlength="15" style="width:150px"></td>
				</tr>
			</table>
			</div>
			
			<table cellSpacing="1" cellPadding="0" border="0">
				<tr>
					<td width="150px">Դ�˿ڣ�</td>
					<td><input type="text" name="sport" size="6" style="width:80px"></td>
				</tr>
				<tr>
			  		<td width="150px">Ŀ�Ķ˿ڣ�</td>
					<td><input type="text" name="dport" size="6" style="width:80px"></td>
				</tr>
				<tr>
					 <td width="150px">���ʣ�</td>
					<td><input type="text" name="rate" size="6" style="width:80px">kb/s </td>
				</tr>
			</table><br><br>
   			<input type="button" name="return" value="����" onClick="location.href='/net_qos_traffictl.asp';" style="width:80px">
         	<input type="button" name="apply" value="����" onClick="on_apply();" style="width:80px">
			<input type="hidden" name="lst" id="lst" value="">
			<input type="hidden" name="submit-url" value="/net_qos_traffictl.asp">
		</form>
	</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>

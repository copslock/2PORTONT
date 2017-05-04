<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>DNS����</TITLE>
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
var showIPv6 = 0;

function autoDNSclicked()
{
	document.dns.dns1.disabled = true;
	document.dns.dns2.disabled = true;
	document.dns.dns3.disabled = true;
	if (document.getElementById)  // DOM3 = IE5, NS6
      {
      	document.getElementById('dnsInfo').style.display = 'none';
      	document.getElementById('dnsenable').style.display = 'none';
      }
   else {
      if (document.layers == false) // IE4
         {
         	document.all.dnsInfo.style.display = 'none';
         	document.all.dnsenable.style.display = 'none';
        }
   }
	// Mason Yu for IPv6
	if ( showIPv6 == 1 )
		autoDNSv6clicked();
}

function manualDNSclicked()
{
	document.dns.dns1.disabled = false;
	document.dns.dns2.disabled = false;
	document.dns.dns3.disabled = false;
		if (document.getElementById)  // DOM3 = IE5, NS6
      {
      	document.getElementById('dnsInfo').style.display = 'block';
      	document.getElementById('dnsenable').style.display = 'block';
      }
   else {
      if (document.layers == false) // IE4
         {
         	document.all.dnsInfo.style.display = 'block';
         	document.all.dnsenable.style.display = 'block';
        }
   }
   DnsBindPvcClicked();
   
	// Mason Yu for IPv6
	if ( showIPv6 == 1 )
	{
		manualDNSv6clicked();
		Dnsv6BindPvcClicked();
	}
}

// Mason Yu for IPv6
function autoDNSv6clicked()
{
	document.dns.dnsv61.disabled = true;
	document.dns.dnsv62.disabled = true;
	document.dns.dnsv63.disabled = true;
	if (document.getElementById)  // DOM3 = IE5, NS6
      {
      	document.getElementById('dnsv6Info').style.display = 'none';
      	document.getElementById('dnsv6enable').style.display = 'none';
      }
   else {
      if (document.layers == false) // IE4
         {
         	document.all.dnsv6Info.style.display = 'none';
         	document.all.dnsv6enable.style.display = 'none';
        }
   }
}

// Mason Yu for IPv6
function manualDNSv6clicked()
{
	document.dns.dnsv61.disabled = false;
	document.dns.dnsv62.disabled = false;
	document.dns.dnsv63.disabled = false;
		if (document.getElementById)  // DOM3 = IE5, NS6
      {
      	document.getElementById('dnsv6Info').style.display = 'block';
      	document.getElementById('dnsv6enable').style.display = 'block';
      }
   else {
      if (document.layers == false) // IE4
         {
         	document.all.dnsv6Info.style.display = 'block';
         	document.all.dnsv6enable.style.display = 'block';
        }
   }
}


function saveClick()
{	
	if (document.dns.dnsMode[0].checked) {
		return true;
	}

	if(sji_checkip(document.dns.dns1.value) == false){
		document.dns.dns1.focus();
		alert("IP��ַ\"" + document.dns.dns1.value + "\"Ϊ��Ч��ַ�����������룡");
		return;
	}
	
	
//	if (document.dns.dns2.value=="") {
	if (document.dns.dns2.value=="") {
		if (document.dns.dns3.value=="") {	// Jenny,  buglist B059, dns2 couldn't be empty if dns3 is not empty
			// Mason Yu for IPv6
			if ( showIPv6 == 1 )						
				return (saveClickIpv6());
			else
				return true;
		}
		else {
			alert("�����뱸ѡ������1��IP��ַ !");
			document.dns.dns2.focus();
			return false;
		}
	}
	if(sji_checkip(document.dns.dns2.value) == false){
		document.dns.dns2.focus();
		alert("IP��ַ\"" + document.dns.dns2.value + "\"Ϊ��Ч��ַ�����������룡");
		return;
	}
	
	if (document.dns.dns3.value=="") {
		// Mason Yu for IPv6
		if ( showIPv6 == 1 )						
			return (saveClickIpv6());
		else
			return true;				
	}
	if(sji_checkip(document.dns.dns3.value) == false){
		document.dns.dns3.focus();
		alert("IP��ַ\"" + document.dns.dns3.value + "\"Ϊ��Ч��ַ�����������룡");
		return;
	}
	
	// Mason Yu for IPv6
	if ( showIPv6 == 1 )						
		return (saveClickIpv6());
	else
		return true;		
	
}

// Mason Yu for IPv6
function saveClickIpv6()
{
	if (document.dns.dnsv61.value=="") {
		document.dns.dnsv61.focus();
		alert("������V6��ѡ��������IP��ַ !");
		return false;
	}
	
	if(document.dns.dnsv61.value != "" ){
		if (! isUnicastIpv6Address( document.dns.dnsv61.value) ){
			alert("V6��ѡ������Ϊ��Ч��ַ������������ !");
			document.dns.dnsv61.focus();
			return false;
		}
	}
	
	if (document.dns.dnsv62.value=="") {
		if (document.dns.dnsv63.value=="")	// Jenny,  buglist B059, dns2 couldn't be empty if dns3 is not empty
			return true;
		else {
			alert("������V6��ѡ������1��IP��ַ !");
			document.dns.dnsv62.focus();
			return false;
		}
	}
	
	if(document.dns.dnsv62.value != "" ){
		if (! isUnicastIpv6Address( document.dns.dnsv62.value) ){
			alert("V6��ѡ������1Ϊ��Ч��ַ������������ !");
			document.dns.dnsv62.focus();
			return false;
		}
	}
	
	if(document.dns.dnsv63.value != "" ){
		if (! isUnicastIpv6Address( document.dns.dnsv63.value) ){
			alert("V6��ѡ������2Ϊ��Ч��ַ������������ !");
			document.dns.dnsv63.focus();
			return false;
		}
	}
	
	return true;
}

function DnsBindPvcClicked()
{
	if(document.dns.enableDnsBind.checked)
		{
			if(<%checkWrite("WanPvcRouter");%>) {
				alert("�Բ���,Ҫ����DNS�󶨷���,��������Ҫ����һ��·��ģʽ��PVC!");
				document.dns.enableDnsBind.checked=false;
				return false;
			}
			if (document.getElementById)  // DOM3 = IE5, NS6
		     {
		     document.getElementById('wanlist1').style.display = 'block';
			 document.getElementById('wanlist2').style.display = 'block';
			 document.getElementById('wanlist3').style.display = 'block';
						}
		   else {
		      if (document.layers == false) // IE4
		         {
		         document.all.wanlist1.style.display = 'block';
				  document.all.wanlist2.style.display = 'block';
				   document.all.wanlist3.style.display = 'block';
		      	}
		}		
		}
		else{
				if (document.getElementById)  // DOM3 = IE5, NS6
		      				{
		      				document.getElementById('wanlist1').style.display = 'none';
						document.getElementById('wanlist2').style.display = 'none';
						document.getElementById('wanlist3').style.display = 'none';
					}
		   else {
		      if (document.layers == false) // IE4
		         		{
		         		document.all.wanlist1.style.display = 'none';
						document.all.wanlist2.style.display = 'none';
						document.all.wanlist3.style.display = 'none';
		      	}
		}		
}
return true;
}

// Mason Yu for IPv6
function Dnsv6BindPvcClicked()
{
	if(document.dns.enableDnsv6Bind.checked)
		{
			if(<%checkWrite("WanPvcRouter");%>) {
				alert("�Բ���,Ҫ����DNS�󶨷���,��������Ҫ����һ��·��ģʽ��PVC!");
				document.dns.enableDnsv6Bind.checked=false;
				return false;
			}
			if (document.getElementById)  // DOM3 = IE5, NS6
		     {
		     document.getElementById('v6wanlist1').style.display = 'block';
			 document.getElementById('v6wanlist2').style.display = 'block';
			 document.getElementById('v6wanlist3').style.display = 'block';
						}
		   else {
		      if (document.layers == false) // IE4
		         {
		         document.all.v6wanlist1.style.display = 'block';
				  document.all.v6wanlist2.style.display = 'block';
				   document.all.v6wanlist3.style.display = 'block';
		      	}
		}		
		}
		else{
				if (document.getElementById)  // DOM3 = IE5, NS6
		      				{
		      				document.getElementById('v6wanlist1').style.display = 'none';
						document.getElementById('v6wanlist2').style.display = 'none';
						document.getElementById('v6wanlist3').style.display = 'none';
					}
		   else {
		      if (document.layers == false) // IE4
		         		{
		         		document.all.v6wanlist1.style.display = 'none';
						document.all.v6wanlist2.style.display = 'none';
						document.all.v6wanlist3.style.display = 'none';
		      	}
		}		
}
return true;
}

function DnsBindSelectdInit(selectname,ifIndex)
{
	var i,num,op;
 num=document.getElementById(selectname).length
 for(i=0;i<num;i++){
 op=document.getElementById(selectname).options[i]
 if(op.value==ifIndex) op.selected=true;
 }

}

function on_init()
{
/*
	with ( document.forms[0] ) 
	{
		for(var i = 0; i < links.length; i++)
		{
			dnspvc1.options.add(new Option(links[i].displayname(1), links[i].ifIndex));
			dnspvc2.options.add(new Option(links[i].displayname(1), links[i].ifIndex));
			dnspvc3.options.add(new Option(links[i].displayname(1), links[i].ifIndex));
		}
	}*/
	
	// Mason Yu for IPv6
	if (<%checkWrite("IPv6Show");%>) {
		showIPv6 = 1;
		if (document.getElementById)  // DOM3 = IE5, NS6
		{
			document.getElementById('dnsv6Tbl').style.display = 'block';						
		}
		else {
			if (document.layers == false) // IE4
			{
				document.all.dnsv6Tbl.style.display = 'block';						
			}
		}
	}
	
	return true;
}

</SCRIPT>
</head>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/formDNS method=POST name="dns">
			<b>DNS����</b><br><br>
			������� DNS �Զ���ȡ����, ��·��������ȡ�õĵ�һ�� DNS IP ��ַ���趨Ϊϵͳ DNS. ��������� DNS �Զ���ȡ����, ���ֶ�ָ����ѡ������ DNS ������.<br>
			<hr align="left" class="sep" size="1" width="90%">
			
			<table border=0 width="600" cellspacing=4 cellpadding=0>
		  <tr>
		    <tr>
		      <td width="40%" colspan="3">
		        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="radio" value="dnsAuto" name="dnsMode" <%checkWrite("dns0"); %> onClick="autoDNSclicked()">�Զ���ȡDNS������
		      </td>
		      <td></td>
		    </tr>
		    <tr>
		      <td width="40%" colspan="3">
		        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="radio" value="dnsManual" name="dnsMode" <%checkWrite("dns1"); %> onClick="manualDNSclicked()">�ֶ�ָ��DNS������
		      </td>
		      <td><div id='dnsenable' style="display:none"><%checkWrite("DnsBindPvc");%></div></td>
		    </tr>
		    </table>
		    
		     <div id='dnsInfo' style="display:none">
		     <table border=0 width="600" cellspacing=4 cellpadding=0>
		    <tr>
		       <td width="30%">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;��ѡDNSv4������:</td>
		       <td width="30%"><input type="text" name="dns1" size="18" maxlength="15" value=<% getInfo("wan-dns1"); %>></td>
		       <td width="40%" align=left>       
		      	<select name="dnspvc1" id='wanlist1' style="display:none">
		      	<option value=255>����PVC</option>
				<!-- // Mason Yu for IPv6 -->				
		      	<%  if_wan_list("rt");  %>
		      	</select>
		      </td>
		    </tr>
		    <tr>
		       <td width="30%">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;��ѡDNSv4������1:</td>
		       <td width="30%"><input type="text" name="dns2" size="18" maxlength="15" value=<% getInfo("wan-dns2"); %>></td>
		      <td width="40%" align=left>         
		      	<select name="dnspvc2" id='wanlist2' style="display:none">
		      	<option value=255>����PVC</option>
				<!-- // Mason Yu for IPv6 -->
		      	<%  if_wan_list("rt");  %>
		      	</select>
		      </td>
		    </tr>
		    <tr>
		       <td width="30%">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;��ѡDNSv4������2:</td>
		       <td width="30%"><input type="text" name="dns3" size="18" maxlength="15" value=<% getInfo("wan-dns3"); %>></td>
		      <td width="40%" align=left>       
		      	<select name="dnspvc3" id='wanlist3' style="display:none">
		      	<option value=255>����PVC</option>
				<!-- // Mason Yu for IPv6 -->
		      	<%  if_wan_list("rt");  %>
		      	</select>
		      </td>
		    </tr>
		    </table>
		    </div>
			
			<!-- // Mason Yu for IPv6. Start-->
			<div id='dnsv6Tbl' style="display:none">
			<table border=0 width="600" cellspacing=4 cellpadding=0>		    
		    <tr>
		      <td width="40%" colspan="3">		        
		      </td>
		      <td><div id='dnsv6enable' style="display:none"><%checkWrite("Dnsv6BindPvc");%></div></td>
		    </tr>
		    </table>		
			
			 <div id='dnsv6Info' style="display:none">
		     <table border=0 width="600" cellspacing=4 cellpadding=0>
		    <tr>
		       <td width="30%">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;��ѡDNSv6������:</td>
		       <td width="30%"><input type="text" name="dnsv61" size="18" maxlength="39" value=<% getInfo("wan-dnsv61"); %>></td>
		       <td width="40%" align=left>       
		      	<select name="dnsv6pvc1" id='v6wanlist1' style="display:none">
		      	<option value=255>����PVC</option>
		      	<%  if_wan_list("rtv6");  %>
		      	</select>
		      </td>
		    </tr>
		    <tr>
		       <td width="30%">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;��ѡDNSv6������1:</td>
		       <td width="30%"><input type="text" name="dnsv62" size="18" maxlength="39" value=<% getInfo("wan-dnsv62"); %>></td>
		      <td width="40%" align=left>         
		      	<select name="dnsv6pvc2" id='v6wanlist2' style="display:none">
		      	<option value=255>����PVC</option>
		      	<%  if_wan_list("rtv6");  %>
		      	</select>
		      </td>
		    </tr>
		    <tr>
		       <td width="30%">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;��ѡDNSv6������2:</td>
		       <td width="30%"><input type="text" name="dnsv63" size="18" maxlength="39" value=<% getInfo("wan-dnsv63"); %>></td>
		      <td width="40%" align=left>       
		      	<select name="dnsv6pvc3" id='v6wanlist3' style="display:none">
		      	<option value=255>����PVC</option>
		      	<%  if_wan_list("rtv6");  %>
		      	</select>
		      </td>
		    </tr>
		    </table>
			</div>
			</div>
			<!-- // Mason Yu for IPv6. End-->
			
		     <input type="hidden" value="/net_dns.asp" name="submit-url">
	 		  <input type="submit" value="Ӧ��" name="save" onClick="return saveClick()">&nbsp;&nbsp;&nbsp;
	 		  <input type="button" value="����" name="save" onClick="history.back()">
		    </form>
		</DIV>
<script>
	initAutoDns = document.dns.dnsMode[0].checked;
	if (document.dns.dnsMode[0].checked)
	{
		autoDNSclicked();
		
		// Mason Yu for IPv6
		if ( <%checkWrite("IPv6Show");%>)
			autoDNSv6clicked();
	}
	else
	{
		manualDNSclicked();
		<%checkWrite("dnsBindPvcInit");%>
		
		// Mason Yu for IPv6
		if ( <%checkWrite("IPv6Show");%> )
		{
			manualDNSv6clicked();
			<%checkWrite("dnsv6BindPvcInit");%>
		}
	}
	if(document.dns.dns1.value=='')
		document.dns.dns1.value='0.0.0.0';
	if(document.dns.dns2.value=='0.0.0.0')
		document.dns.dns2.value='';
	if(document.dns.dns3.value=='0.0.0.0')
		document.dns.dns3.value='';	

    if(document.dns.dnsv61.value=='::')
        document.dns.dnsv61.value='';
    if(document.dns.dnsv62.value=='::')
        document.dns.dnsv62.value='';
    if(document.dns.dnsv63.value=='::')
        document.dns.dnsv63.value='';
	
</script>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>

<!-- add by liuxiao 2008-01-16 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>�й�����</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv="refresh" content="5">
<meta http-equiv=content-script-type content=text/javascript>
<style type=text/css>
@import url(/style/default.css);
</style>
<script language="javascript" src="common.js"></script>
<SCRIPT language="javascript" type="text/javascript">
var links = new Array();
<% listWanConfig(); %>
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if(lstrc.rows){while(lstrc.rows.length > 1) lstrc.deleteRow(1);}
	for(var i = 0; i < links.length; i++)
	{
		var row = lstrc.insertRow(i + 1);
		
		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";
		var cell = row.insertCell(0);
		cell.innerHTML = i+1;
		cell = row.insertCell(1);
		cell.innerHTML = links[i].encaps;
		cell = row.insertCell(2);
		cell.innerHTML = links[i].servName;
		cell = row.insertCell(3);
		cell.innerHTML = links[i].strStatus;
		cell = row.insertCell(4);
		cell.innerHTML = links[i].ipAddr;

	}
}

</SCRIPT>

</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>WAN��Ϣ</b></div>
			<table class="flat" id="lstrc" border="1" cellpadding="1" cellspacing="1" width="90%">
				<tr class="hdb" align="center" nowrap>
				<td>�ӿ����</td><td>��װ��ʽ</td><td>����ӿ�</td><td>״̬</td><td>IP��ַ</td>
				</tr>
			</table>
			<br><br>
			<b>������Ϣ</b>
			<br>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td width="20%" class="hdb">ȱʡ����</td>
					<td><% getDefaultGW(); %></td>
				</tr>
				<tr>
					<td class="hdb">��������</td>
					<td><% getDefaultGWMask(); %></td>
				</tr>
				<tr>
					<td class="hdb">��ѡDNS������</td>
					<td><% getInfo("primary-dns"); %></td>
				</tr>
				<tr>
					<td class="hdb">����DNS������</td>
					<td><% getInfo("secondary-dns"); %></td>
				</tr>
			</table>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

<!-- add by liuxiao 2008-01-21 -->
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

var cgi = new Object();

var lanNetInfo = new Array();
with(lanNetInfo){<% initPageLanNetInfo(); %>}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if (lstLanHost.rows) {
		while (lstLanHost.rows.length > 1)
			lstLanHost.deleteRow(1);
	}

	for (var i = 0; i < lanNetInfo.length; i++) {
		var row = lstLanHost.insertRow(i + 1);

		row.nowrap = true;
		row.style.verticalAlign = "center";
		row.style.textAlign = "center";

		var cell = row.insertCell(0);
		cell.innerHTML = lanNetInfo[i].devName;
		cell = row.insertCell(1);
		cell.innerHTML = lanNetInfo[i].devType;
		cell = row.insertCell(2);
		cell.innerHTML = lanNetInfo[i].brand;
		cell = row.insertCell(3);
		cell.innerHTML = lanNetInfo[i].OS;
		cell = row.insertCell(4);
		cell.innerHTML = lanNetInfo[i].port;
		cell = row.insertCell(5);
		cell.innerHTML = lanNetInfo[i].mac;
		cell = row.insertCell(6);
		cell.innerHTML = lanNetInfo[i].ip;
		cell = row.insertCell(7);
		cell.innerHTML = lanNetInfo[i].connectionType;
		cell = row.insertCell(8);
		cell.innerHTML = lanNetInfo[i].onlineTime;
	}
}
</SCRIPT>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px;">
		<br>
			<div align="left"><b>��ͥ���������¹��豸��Ϣ��</b></div>
			<br>
				<table id="lstLanHost" class="flat" border="1" cellpadding="1" cellspacing="1">
					<tr class="hdb" align="center" nowrap>
						<td width="160px">�豸����</td>
						<td width="60px">�豸����</td>
						<td width="100px">Ʒ����</td>
						<td width="140px">����ϵͳ</td>
						<td width="30px">�˿�</td>
						<td width="120px">MAC��ַ</td>
						<td width="100px">IP��ַ</td>
						<td width="80px">��������</td>
						<td width="100px">����ʱ��(sec)</td>
					</tr>
				</table>
		</form>	
		</div>
	</blockquote>
</body>
<!-- add end by martin ZHU 2016-01-13 -->
<%addHttpNoCache();%>
</html>

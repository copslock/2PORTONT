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

var dmacips = new Array();
<% showMACBaseTable(); %>

/********************************************************************
**          on document load
********************************************************************/

function on_init()
{
	sji_docinit(document);

	for(var i = 0; i < dmacips.length; i++)
	{
		var row = lstmacip.insertRow(i + 1);

		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";

		var cell = row.insertCell(0);
		cell.innerHTML = dmacips[i].macAddr_Dhcp;
		cell = row.insertCell(1);
		cell.innerHTML = dmacips[i].ipAddr_Dhcp;
		cell = row.insertCell(2);
		cell.innerHTML = "<a href=\"javascript:removeClick('" + dmacips[i].macAddr_Dhcp + "', '" + dmacips[i].ipAddr_Dhcp + "');\"><img border=\"0\" src=\"image/delete.gif\" alt=\"Delete\"></a>";
	}
}

/********************************************************************
**          on document update
********************************************************************/

function removeClick(mac,ip)
{
	with(document.forms[0])
	{
		action.value = "rm";
		macAddr_Dhcp.value=mac;
		ipAddr_Dhcp.value=ip;
		submit();
	}
}

/********************************************************************
**          on document submit
********************************************************************/

function on_submit(act)
{
	with ( document.forms[0] )
	{
		action.value = act;

		if(act == "add")
		{
			var loc = "net_addbindmac.asp";
			var code = "location.assign(\"" + loc + "\")";
			eval(code);
		}
	}
}
	</script>
</head>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onload="on_init()">
	<DIV align="left" style="padding-left:20px; padding-top:10px">
		<form action=/boaform/formMacAddrBase method=POST>
			<b>Ԥ��IP��ַ�б�:</b><br><br>
			ͨ����MAC��ַ��IP��ַ֮������ӳ����Ŀ��ΪPCԤ��һ�������IP��ַ������Ԥ��10��ip��ַ�������mac��<br><br>
			<table id = "lstmacip" class="flat" border="1" cellpadding="2" cellspacing="0">
				<tr class="hdb" align="center">
				<td width="150px">MAC��ַ</td>
				<td width="150px">IP��ַ</td>
				<td width="50px">ɾ��</td>
				</tr>
			</table>
			<br><br>
			<table border="0" width="350px" cellpadding="0" cellspacing="0">
				<tr>
				<td align="right">
				<input type="button" class="button" onClick="on_submit('add')" value="���">
				<input type="hidden" name="submit-url" value="" >
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="macAddr_Dhcp">
				<input type="hidden" name="ipAddr_Dhcp">
				<input type="button" class="button" onClick="window.close()" value="�ر�"></td>
				</tr>
			</table>
		</form>
	</div>
</body>
<%addHttpNoCache();%>
</html>

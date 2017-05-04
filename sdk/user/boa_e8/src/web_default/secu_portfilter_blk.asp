<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�������������</TITLE>
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

var protos = new Array("(��)", "TCP/UDP", "TCP", "UDP", "ICMP");
var ipv6_protos = new Array("(��)", "TCP/UDP", "TCP", "UDP", "ICMPV6");
var rcs = new Array();
with(rcs){<% ipPortFilterBlacklist(); %>}

function on_chkclick(index)
{
	if(index < 0 || index >= rcs.length)
		return;
	rcs[index].select = !rcs[index].select;
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document);

	if(lstrc.rows){while(lstrc.rows.length > 1) lstrc.deleteRow(1);}
	for(var i = 0; i < rcs.length; i++)
	{
		var row = lstrc.insertRow(i + 1);

		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";
		var cell = row.insertCell(0);
		cell.innerHTML = rcs[i].filterName;

		cell = row.insertCell(1);
		// For IPv4 and IPv6
		if ( <%checkWrite("IPv6Show");%> ) {
			// For IPv4
			if ( rcs[i].IpProtocolType == "1" ) {
				cell.innerHTML = protos[rcs[i].protoType];
			}
			// For IPv6
			else if ( rcs[i].IpProtocolType == "2" ) {
				cell.innerHTML = ipv6_protos[rcs[i].protoType];
			}
		}
		// For IPv4 only
		else {
			cell.innerHTML = protos[rcs[i].protoType];
		}

		cell = row.insertCell(2);
		// For IPv4 and IPv6
		if ( <%checkWrite("IPv6Show");%> ) {
			// For IPv4
			if ( rcs[i].IpProtocolType == "1" ) {
				if(rcs[i].sipStart != "0.0.0.0")cell.innerHTML = rcs[i].sipStart + ((rcs[i].sipEnd == "0.0.0.0") ? "" : ("-" + rcs[i].sipEnd));
			}
			// For IPv6
			else if ( rcs[i].IpProtocolType == "2" ) {
				if(rcs[i].sip6Start != "::")cell.innerHTML = rcs[i].sip6Start + ((rcs[i].sip6End == "::") ? "" : ("-" + rcs[i].sip6End));
			}
		}
		// For IPv4 only
		else {
			if(rcs[i].sipStart != "0.0.0.0")cell.innerHTML = rcs[i].sipStart + ((rcs[i].sipEnd == "0.0.0.0") ? "" : ("-" + rcs[i].sipEnd));
		}

		cell = row.insertCell(3);
		// For IPv4 and IPv6
		if ( <%checkWrite("IPv6Show");%> ) {
			// For IPv4
			if ( rcs[i].IpProtocolType == "1" ) {
				if(rcs[i].smask != "0.0.0.0")cell.innerHTML = rcs[i].smask;
			}
			// For IPv6
			else if ( rcs[i].IpProtocolType == "2" ) {
				if(rcs[i].sip6PrefixLen != "")cell.innerHTML = rcs[i].sip6PrefixLen;
			}
		}
		// For IPv4 only
		else {
			if(rcs[i].smask != "0.0.0.0")cell.innerHTML = rcs[i].smask;
		}

		cell = row.insertCell(4);
		if(rcs[i].sportStart != 0) cell.innerHTML = rcs[i].sportStart + ((rcs[i].sportEnd == rcs[i].sportStart) ? "" : ("-" + rcs[i].sportEnd));

		cell = row.insertCell(5);
		// For IPv4 and IPv6
		if ( <%checkWrite("IPv6Show");%> ) {
			// For IPv4
			if ( rcs[i].IpProtocolType == "1" ) {
				if(rcs[i].dipStart != "0.0.0.0")cell.innerHTML = rcs[i].dipStart + ((rcs[i].dipEnd == "0.0.0.0") ? "" : ("-" + rcs[i].dipEnd));
			}
			// For IPv6
			else if ( rcs[i].IpProtocolType == "2" ) {
				if(rcs[i].dip6Start != "::")cell.innerHTML = rcs[i].dip6Start + ((rcs[i].dip6End == "::") ? "" : ("-" + rcs[i].dip6End));
			}
		}
		// For IPv4 only
		else {
			if(rcs[i].dipStart != "0.0.0.0")cell.innerHTML = rcs[i].dipStart + ((rcs[i].dipEnd == "0.0.0.0") ? "" : ("-" + rcs[i].dipEnd));
		}

		cell = row.insertCell(6);
		// For IPv4 and IPv6
		if ( <%checkWrite("IPv6Show");%> ) {
			// For IPv4
			if ( rcs[i].IpProtocolType == "1" ) {
				if(rcs[i].dmask != "0.0.0.0")cell.innerHTML = rcs[i].dmask;
			}
			// For IPv6
			else if ( rcs[i].IpProtocolType == "2" ) {
				if(rcs[i].dip6PrefixLen != "")cell.innerHTML = rcs[i].dip6PrefixLen;
			}
		}
		// For IPv4 only
		else {
			if(rcs[i].dmask != "0.0.0.0")cell.innerHTML = rcs[i].dmask;
		}

		cell = row.insertCell(7);
		if(rcs[i].dportStart != 0) cell.innerHTML = rcs[i].dportStart + ((rcs[i].dportEnd == rcs[i].dportStart) ? "" : ("-" + rcs[i].dportEnd));

		cell = row.insertCell(8);
		cell.align = "center";
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";

		cell = row.insertCell(9);
		if ( <%checkWrite("IPv6Show");%> ) {
			if (rcs[i].IpProtocolType == "1")
				cell.innerHTML = "IPv4";
			else if (rcs[i].IpProtocolType == "2")
				cell.innerHTML = "IPv6";
		}
		else {
			cell.innerHTML = "IPv4";
		}
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit()
{
	with ( document.forms[0] )
	{
		//form.bcdata.value = sji_encode(rcs, "select");
		form.bcdata.value = sji_varencode(rcs, "select", "filterName");
		if(!(form.bcdata.value == null || form.bcdata.value.length==2))
			submit();
	}
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<DIV align="left" style="padding-left:5px; padding-top:0px">
		<form id="form" action="/boaform/admin/formPortFilterBlack" method="post">
			<b>����IP����������</b>
			<br><br>
			<div class="tip" style="width:90% ">
				Ĭ�������, ��������IPͨ�Ŵ�LAN����, ���ǿ���ͨ����������IP��������<font color="red"><b>��ֹ</b></font>ĳЩIPͨ��.<br>
				ѡ����ӻ�ɾ����ť������LAN������IP������.<br>
			</div>
			<br>
			<table id="lstrc" class="flat" border="1" cellpadding="1" cellspacing="0">
				<tr class="hdb" align="center" nowrap>
          <td width="60px">��������</td>
					<td width="60px">Э��</td>
					<td width="160px">ԴIP��ַ(��Χ)</td>
					<td width="120px">Դ��������</td>
					<td width="60px">Դ�˿�</td>
					<td width="160px">Ŀ��IP��ַ(��Χ)</td>
					<td width="120px">Ŀ����������</td>
					<td width="60px">Ŀ�Ķ˿�</td>
					<td width="60px">ɾ��</td>
					<td width="60px">IP�汾</td>
				</tr>
			</table>
			<br>
			<input type="button" class="button" onClick="location.href='secu_portfilter_blkadd.asp';" value="���">
			<input type="button" class="button" onClick="on_submit();" value="ɾ��">
			<input type="hidden" id="action" name="action" value="rm">
			<input type="hidden" name="bcdata" value="le">
			<input type="hidden" name="submit-url" value="">
		</form>
	</DIV>
</body>
<%addHttpNoCache();%>
</html>

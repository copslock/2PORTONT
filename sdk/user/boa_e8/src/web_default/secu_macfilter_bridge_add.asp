<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�û�����</TITLE>
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

var links = new Array();
with(links){<%listWanif("br");%>}

function on_chkclick(index)
{
	if(index < 0 || index >= links.length)
		return;
	links[index].select = !links[index].select;
	if(links[index].select)form.portNum.value = parseInt(form.portNum.value) + 1;
	else form.portNum.value = parseInt(form.portNum.value) - 1;
}

function on_sel()
{
	with(form)
	{
		for(var i = 0; i <lstrc.rows.length; i++)
		{
			lstrc.rows[i].cells[0].children[0].click();
		}
	}
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document);

	if(lstrc.rows){while(lstrc.rows.length > 0) lstrc.deleteRow(0);}
	for(var i = 0; i < links.length; i++)
	{
		var row = lstrc.insertRow(i);

		row.nowrap = true;
		row.vAlign = "top";
		row.align = "left";

		var cell = row.insertCell(0);
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";
		cell = row.insertCell(1);
		//cell.innerHTML = links[i].displayname(1);
		cell.innerHTML = links[i].name;
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit()
{
	with ( document.forms[0] )
	{
		//////////
		if(dmac.value.length != 0 && sji_checkmac(dmac.value) == false)
		{
			dmac.focus();
			alert("Ŀ��MAC��ַ\"" + dmac.value + "\"Ϊ��ЧMAC��ַ�����������룡");
			return;
		}
		if(smac.value.length != 0 && sji_checkmac(smac.value) == false)
		{
			smac.focus();
			alert("ԴMAC��ַ\"" + smac.value + "\"Ϊ��ЧMAC��ַ�����������룡");
			return;
		}
		if(smac.value.length == 0&&dmac.value.length == 0)
					{
					alert("����Ҫ��ԴMAC��ַ��Ŀ��MAC��ַ��һ��ָ��MAC��ַ");
					return;
					}
		var sif = "";
		for(var i in links)
		{
			if(!links[i].select)continue;
			//if(sif.length != 0) sif += ";" + links[i].displayname();
			//else sif += links[i].displayname();
			if(sif.length != 0) sif += ";" + links[i].ifIndex;
			else sif += links[i].ifIndex;
		}
		if(sif=="")
		{
		alert("����Ҫָ������һ��WAN�ӿ�");
		return ;
		}
		ifname.value = sif;

		submit();
	}
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formBrgMacFilter" method="post">
				<div align="left">
					<b>���MAC��ַ���˹���</b><br>
					<br>
					<table border="0" cellpadding="0" cellspacing="0">
					   <tr>
						  <td width="100px">Э������:</td>
						  <td><select name="protoType" size="1" style="width:150px">
								<option value="0" selected>ALL</option>
								<option value="1">PPPoE</option>
								<option value="2">IPv4</option>
								<option value="3">IPv6</option>
								<option value="4">AppleTalk</option>
								<option value="5">IPX</option>
								<option value="6">NetBEUI</option>
							 </select></td>
					   </tr>
					   <tr>
						  <td>Ŀ��MAC��ַ:</td>
						  <td><input type="text" size="17" name="dmac" style="width:150px"></td>
					   </tr>

					   <tr>
						  <td>ԴMAC��ַ:</td>
						  <td><input type="text" size="17" name="smac" style="width:150px"></td>
					   </tr>
					   <tr>
						  <td>֡����:</td>
						  <td><select name="direction" size="1" style="width:150px">
								<option value="0" selected>LAN=&gt;WAN</option>
								<option value="1">WAN=&gt;LAN</option>
								<option value="2">LAN&lt;=&gt;WAN</option>
							 </select></td>
					   </tr>
					</table>
					<br><br>
					<b>��ѡ��MAC���˵�WAN�ӿ�(MAC���˽����Žӿ���Ч)</b><br>
					<!--<input type="checkbox" name="allport" onClick="on_sel();">ȫѡ<br>-->
					<table id="lstrc" border="0" cellpadding="0" cellspacing="0"></table>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<INPUT type="button" class="button" value="����/Ӧ��" onClick="on_submit();">
				<input type="hidden" name="action" value="ad">
				<input type="hidden" name="portNum" value="0">
				<input type="hidden" name="ifname" value="">
				<input type="hidden" name="submit-url" value="/secu_macfilter_bridge.asp">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

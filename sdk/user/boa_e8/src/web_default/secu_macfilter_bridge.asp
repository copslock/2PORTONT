<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>��������������</TITLE>
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

//var protos = new Array("","PPPoE", "IPv4", "IPv6", "AppleTalk", "IPX", "NetBEUI", "IGMP");
var protos = new Array("ALL","PPPoE", "IPv4", "IPv6", "AppleTalk", "IPX", "NetBEUI");
var dirs = new Array("LAN -> WAN","WAN -> LAN", "LAN <-> WAN");
var cgi = new Object();
var rules = new Array();
with(rules){<% brgMacFilterList(); %>}

function on_chkclick(index)
{
	if(index < 0 || index >= rules.length)
		return;
	rules[index].select = !rules[index].select;
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);

	if(cgi.macFilterEnble == false)
	{
		form.macFilterMode[0].disabled = true;
		form.macFilterMode[1].disabled = true;
		form.add.disabled = true;
		form.remove.disabled = true;
	}

	if(rulelst.rows)
	{
		while(rulelst.rows.length > 1)
			rulelst.deleteRow(1);
	}

	for(var i = 0; i < rules.length; i++)
	{
		var row = rulelst.insertRow(i + 1);

		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";

		var cell = row.insertCell(0);
		cell.innerHTML = rules[i].ifname;
		cell = row.insertCell(1);
		cell.innerHTML = protos[rules[i].protoType];
		cell = row.insertCell(2);
		cell.innerHTML = rules[i].dmac;
		cell = row.insertCell(3);
		cell.innerHTML = rules[i].smac;
		cell = row.insertCell(4);
		cell.innerHTML = dirs[rules[i].direction];
		cell = row.insertCell(5);
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";
	}
}

function addClick()
{
   var loc = "secu_macfilter_bridge_add.asp";
   var code = "window.location.href=\"" + loc + "\"";
   eval(code);
}

function on_action(act)
{
	form.action.value = act;

	if(act == "rm" && rules.length > 0)
	{
		form.bcdata.value = sji_encode(rules, "select");
	}

	with(form)
	{
		submit();
	}
}

function onmode()
{
	var val = ((form.macFilterMode[0].checked == true) ? false : true);
	if(cgi.macFilterMode == val)return;

	form.action.value = "modesw";
	if(form.macFilterMode[0].checked)
	{
		cfg.style.display = "none";
		w2b.style.display = "block";
		b2w.style.display = "none";
		cfm.style.display = "block";
	}
	else
	{
		cfg.style.display = "none";
		w2b.style.display = "none";
		b2w.style.display = "block";
		cfm.style.display = "block";
	}
}

function btnNo()
{
	w2b.style.display = "none";
	b2w.style.display = "none";
	cfm.style.display = "none";
	form.macFilterMode[sji_int(cgi.macFilterMode)].checked = true;
	cfg.style.display = "block";
}

function btnYes()
{
	form.submit();
}

</SCRIPT>
</HEAD>
<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV id="cfg" align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/formBrgMacFilter method=POST>
				<b>�Ž�ģʽ�µ�MAC��ַ���� -- ���������� 16 ������.</b><br><br>
				<div id="rstip" style="display:none;"><font color="red">��ʾ����ҳ������ã���Ҫ����·����������Ч��</font><br></div>
				<hr align="left" class="sep" size="1" width="90%">
				<table border="0" cellpadding="2" cellspacing="0">
					<tr>
						<td>MAC��ַ����:</td>
						<td><input type="radio" name="macFilterEnble" value="off" onClick="on_action('sw')">&nbsp;&nbsp;����</td>
						<td><input type="radio" name="macFilterEnble" value="on" onClick="on_action('sw')">&nbsp;&nbsp;����</td>
					</tr>
					<tr>
						<td>����ģʽ:</td>
						<td><input type="radio" name="macFilterMode" value="off" onClick="onmode();">&nbsp;&nbsp;������</td>
						<td><input type="radio" name="macFilterMode" value="on" onClick="onmode();">&nbsp;&nbsp;������</td>
					</tr>
				</table>
				<br><br>
				<table id="rulelst" class="flat" border="1" cellpadding="2" cellspacing="0">
					<tr align="center" class="hd">
						<td>WAN�ӿ�</td>
						<td>Э��</td>
						<td>Ŀ��MAC��ַ</td>
						<td>ԴMAC��ַ</td>
						<td>֡����</td>
						<td>ɾ��</td>
					</tr>
				</table>
				<br>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" name="add" class="button" onClick="addClick()" value="���">
				<input type="button" name="remove" class="button" onClick="on_action('rm')" value="ɾ��">
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="bcdata" value="le">
				<input type="hidden" name="submit-url" value="" >
			</form>
		</div>
		<DIV id="w2b" align="left" style="padding-left:20px; padding-top:5px; display:none;">
			<b>
			ת��MAC��ַ����ģʽ<br><br>
			ע��: �ڰ�������ת�����Զ�ɾ�������Ѷ���Ĺ��������¶�����Ҫ�Ĺ���.<br><br>
			��ȷ��Ҫ��MAC��ַ����ģʽ��<font color="green">������</font>ת��Ϊ<font color="red">������</font>��?<br><br>
			</b>
		</div>
		<DIV id="b2w" align="left" style="padding-left:20px; padding-top:5px; display:none;">
			<b>
			ת��MAC��ַ����ģʽ<br><br>
			ע��: �ڰ�������ת�����Զ�ɾ�������Ѷ���Ĺ��������¶�����Ҫ�Ĺ���.<br><br>
			��ȷ��Ҫ��MAC��ַ����ģʽ��<font color="red">������</font>ת��Ϊ<font color="green">������</font>��?<br><br>
			</b>
		</div>
		<DIV id="cfm" align="left" style="padding-left:20px; padding-top:5px; display:none;">
			<input type="button" class="button" onClick="btnNo()" value="ȡ��"> <input type="button" class="button" onClick="btnYes()" value="ȷ��">
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

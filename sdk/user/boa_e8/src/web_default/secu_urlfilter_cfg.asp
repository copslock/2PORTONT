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

var cgi = new Object();
var rules = new Array();
with(rules){<% initPageURL(); %>}

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

	if(cgi.urlfilterEnble == false)
	{
		form.urlFilterMode[0].disabled = true;
		form.urlFilterMode[1].disabled = true;
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
		cell.innerHTML = rules[i].url;
		cell.align = "left";
		cell = row.insertCell(1);
		//cell.innerHTML = rules[i].port;
		//cell = row.insertCell(2);
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";
	}
}

function addClick()
{
   var loc = "secu_urlfilter_add.asp";
   var code = "window.location.href=\"" + loc + "\"";
   eval(code);
}

function on_action(act)
{
	form.action.value = act;

	if(act == "rm" && rules.length > 0)
	{
		//form.bcdata.value = sji_encode(rules, "select");
		form.bcdata.value = sji_idxencode(rules, "select", "idx");
	}

	with(form)
	{
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
			<form id="form" action=/boaform/admin/formURL method=POST name="form">
				<b>URL���� -- ������������ 100������.</b><br><br>
				<div id="rstip" style="display:none;"><font color="red">��ʾ����ҳ������ã���Ҫ����·����������Ч��</font><br></div>
				<hr align="left" class="sep" size="1" width="90%">
				<table border="0" cellpadding="2" cellspacing="0">
					<tr>
						<td>URL����:</td>
						<td><input type="radio" name="urlfilterEnble" value="off" onClick="on_action('sw')">&nbsp;&nbsp;����</td>
						<td><input type="radio" name="urlfilterEnble" value="on" onClick="on_action('sw')">&nbsp;&nbsp;����</td>
					</tr>
					<tr>
						<td>����ģʽ:</td>
						<td><input type="radio" name="urlFilterMode" value="off" onClick="on_action('md')">&nbsp;&nbsp;������</td>
						<td><input type="radio" name="urlFilterMode" value="on" onClick="on_action('md')">&nbsp;&nbsp;������</td>
					</tr>
				</table>
				<br><br><br>
				<table id="rulelst" class="flat" border="1" cellpadding="2" cellspacing="0">
					<tr align="center" class="hd">
						<td width="240px">URL��ַ</td>
						<!--<td>�˿�</td>-->
						<td>ɾ��</td>
					</tr>
				</table>
				<br>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" name="add" onClick="addClick()" value="���">
				<input type="button" class="button" name="remove" onClick="on_action('rm')" value="ɾ��">
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="bcdata" value="le">
				<input type="hidden" name="submit-url" value="" >
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

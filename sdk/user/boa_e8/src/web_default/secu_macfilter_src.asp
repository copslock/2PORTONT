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
with(rules){<% rteMacFilterList(); %>}

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
		form.add.disabled = true;
		form.remove.disabled = true;
		document.getElementById("macfilter_list").style.display="none";
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

		var cell; // = row.insertCell(0);
		//cell.innerHTML = rules[i].devname;
		cell = row.insertCell(0);
		cell.innerHTML = rules[i].mac;
		cell = row.insertCell(1);
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";
	}

	if(rules.length == 0)
	{
		form.remove.disabled = true;
	}
}

function addClick()
{
   var loc = "secu_macfilter_src_add.asp";
   var code = "window.location.href=\"" + loc + "\"";
   eval(code);
}

function removeClick()
{
	with ( document.forms[0] )
	{
		form.bcdata.value = sji_encode(rules, "select");
		submit();
	}
}

function on_action(act)
{
	form.action.value = act;

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
		<form id="form" action=/boaform/admin/formRteMacFilter method=POST name="form">
			<b>MAC��ַ���� -- ���������� 16 ������.</b><br>
			<hr align="left" class="sep" size="1" width="90%">
			<table border="0" cellpadding="2" cellspacing="0">
					<tr>
						<td>MAC��ַ����:</td>
						<td><input type="radio" name="macFilterEnble" value="off" onClick="on_action('sw')">&nbsp;&nbsp;����</td>
						<td><input type="radio" name="macFilterEnble" value="on" onClick="on_action('sw')">&nbsp;&nbsp;����</td>
					</tr>
			</table>
			<br>
			<div id="macfilter_list">
			<table id="rulelst" class="flat" border="1" cellpadding="2" cellspacing="0">
			   <tr class="hd" align="center">
				  <!--<td>�������豸��</td>-->
				  <td>SRC MAC</td>
				  <td>�Ƴ�</td>
			   </tr>
			</table>
			<br>
			<hr align="left" class="sep" size="1" width="90%">
			<input type="button" name="add" class="button" onClick="addClick()" value="���">
			<input type="button" name="remove" class="button" onClick="removeClick()" value="ɾ��">
			</div>
			<input type="hidden" name="action" value="rm">
			<input type="hidden" name="bcdata" value="le">
			<input type="hidden" name="submit-url" value="">
		</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

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
var rules = new Array();
with(rules){<% rteMacFilterList(); %>}

/********************************************************************
**          on document apply
********************************************************************/
function btnApply()
{
	/*
	if(form.devname.value == "")
	{
		alert("�������豸������Ϊ�գ�");
		return false;
	}
	*/
	if(form.mac.value == "")
	{
		alert("mac ��ַ����Ϊ�գ�");
		return false;
	}
	if(!sji_checkmac(form.mac.value))
	{
		alert("mac ��ַ����");
		return false;
	}
	for(var i = 0; i < rules.length; i++)
	{
		if(/*rules[i].name == form.devname.value ||*/ rules[i].mac == form.mac.value)
		{
			alert( "�ù����Ѵ���");//alert("That rule already exists");
			return false;
		}
	}
	form.submit();
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
  <blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px">
		<form id="form" action=/boaform/admin/formRteMacFilter method=POST name="form">
			<b>��Ӿܾ���ȡ�� MAC ��ַ���˹���</b><br><br><br>
			<!--��ҳ���������˵�������·������ָ���������豸��MAC��ַ��<br>
			��"�������豸��"һ�������������Ƶľ������豸������"MAC��ַ"һ��������豸��MAC��ַ��<br>
			�����������������"ipconfig /all"���鿴����PC��MAC��ַ��<br> -->
			<hr align="left" class="sep" size="1" width="90%">
			<table border="0" cellpadding="0" cellspacing="0">
				<tr style="display:none">
					<td width="180">�������豸��</td>
					<td><input type="text" name="devname" size="18" maxlength="31"></td>
				</tr>
				<tr>
					<td width="180">MAC��ַ(xx-xx-xx-xx-xx-xx)&nbsp;</td>
					<td><input type="text" name="mac" size="18"></td>
				</tr>
			</table>
			<hr align="left" class="sep" size="1" width="90%">
			<input type="button" class="button" value="����/Ӧ��" onClick="btnApply()">
			<input type="hidden" name="action" value="ad">
			<input type="hidden" name="submit-url" value="/secu_macfilter_src.asp">
		</form>
	</div>
  </blockquote>
</body>
<%addHttpNoCache();%></html>

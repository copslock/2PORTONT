<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<html>
<head>
<title>��־</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv=content-script-type content=text/javascript>
<!--ϵͳ����css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<!--ϵͳ�����ű�-->
<script language="javascript" src="common.js"></script>
<script language="javascript" type="text/javascript">

var cgi = new Object();
<% initPageSysLog(); %>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);
	if (cgi.issu == false) {
		form.newrec.style.display = "none";
		//form.clrrec.style.display = "none";
	}
}

function btnView()
{
	var options =
	    "menubar=yes,resizable=yes,scrollbars=yes,titlebar=yes,toolbar=no,width=640,height=600";
	window.open("mgm_log_view_access.asp", "ϵͳ��־", options);
}

function btnSecView()
{
	var options =
	    "menubar=yes,resizable=yes,scrollbars=yes,titlebar=yes,toolbar=no,width=640,height=600";

	window.open("mgm_log_view_sec.asp", "ϵͳ��־", options);
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(act)
{
	with (document.forms[0]) {
		action.value = act;
		submit();
	}
}

</script>
</head>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formSysLog" method="post">
				<b>ϵͳ��¼</b><br>
				<br>
				�����Բ鿴ϵͳ��¼.<br>
				<br>
				��������ʼ�¼���鿴ϵͳ���ʼ�¼.<br>
				<br>
				�������ȫ��¼���鿴ϵͳ��ȫ��¼.<br>
				<br>
				������������־��¼������������ϵͳ��¼ .<br>
				<br>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" name="sysrec" onClick="btnView();" value="���ʼ�¼">
				<!--<input type="button" class="button" name="secrec" onClick="btnSecView();" value="��ȫ��¼">-->
				<!--<input type="button" class="button" name="newrec" onClick="on_submit('new');" value="������־�ļ�">-->
				<input type="button" class="button" name="clrrec" onClick="on_submit('clr');" value="�����¼">
				<input type="button" class="button" name="saverec" onClick="on_submit('saveLog');" value="��־����">
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="submit-url" value="">
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

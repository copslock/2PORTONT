<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<html>
<head>
<title>�û�����</title>
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
<% initPageMgmUser(); %>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);
	if (cgi.issu)
		lstin.deleteRow(1);
	document.getElementById("sutip").style.display = cgi.issu ? "block" : "none";
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit()
{
	with (document.forms[0]) {
		if (!cgi.issu) {
			if (oldPasswd.value.length <= 0) {
				oldPasswd.focus();
				alert("���벻��Ϊ�գ�����������!");
				return;
			}
			if (sji_checkusername(oldPasswd.value, 1, 16) == false) {
				oldPasswd.focus();
				alert("���������������������!");
				return;
			}
		}

		if (newPasswd.value.length <= 0) {
			newPasswd.focus();
			alert("�����벻��Ϊ�գ�������������!");
			return;
		}
		if (sji_checkusername(newPasswd.value, 1, 16) == false) {
			newPasswd.focus();
			alert("�������������������������!");
			return;
		}
		if (affirmPasswd.value.length <= 0) {
			affirmPasswd.focus();
			alert("ȷ�����벻��Ϊ�գ�������ȷ������!");
			return;
		}
		if (sji_checkusername(affirmPasswd.value, 1, 16) == false) {
			affirmPasswd.focus();
			alert("ȷ�������������������ȷ������!");
			return;
		}
		if (newPasswd.value != affirmPasswd.value) {
			affirmPasswd.focus();
			alert("�������ȷ�����벻ƥ�䣬����������ȷ������!");
			return;
		}
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
			<form id="form" action="/boaform/admin/formPasswordSetup" method="post">
				<div align="left">
					<b>���ʿ��� -- ����</b><br><br>
					<div id="sutip" align="left" style="display: none">
						����·������ͨ�������û���������: telecomadmin��useradmin��<br>
						<br>
						�û���"telecomadmin"���Բ������Ƶ�������޸����ļ�ͥ���ء�<br>
						<br>
						�û���"useradmin"���Է��ʼ�ͥ���أ�������ú�ͳ�Ʊ�<br>
						<br>
						ʹ������������������16���ַ���Ȼ����"����/Ӧ��"���ı�򴴽����롣 ע��:���벻�ܺ��ո�<br>
					</div>
					<hr align="left" class="sep" size="1" width="90%">
					<br>
					<table id="lstin" border="0" cellpadding="0" cellspacing="0">
						<tr>
							<td width="120" height="23">ԭ�û���:</td>
							<td><input name="oldUserName" type="text" size="20" maxlength="16" style="width:200px" disabled="true" value="<% getInfo("normal-user"); %>"></td>
						</tr>
						<tr>
							<td>������:</td>
							<td><input name="oldPasswd" type="password" size="20" maxlength="16" style="width:200px"></td>
						</tr>
						<tr>
							<td>������:</td>
							<td><input name="newPasswd" type="password" size="20" maxlength="16" style="width:200px"></td>
						</tr>
						<tr>
							<td>ȷ������:</td>
							<td><input name="affirmPasswd" type="password" size="20" maxlength="16" style="width:200px"></td>
						</tr>
					</table>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" value="����/Ӧ��" onclick="on_submit();">
				<input type="hidden" name="submit-url" value="">
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<html>
<head>
<title>д��ȼ�����</title>
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
<script type="text/javascript" src="share.js"></script>
<script language="javascript" type="text/javascript">
var addr = '<% getInfo("syslog-server-ip"); %>';
var port = <% getInfo("syslog-server-port"); %>;

function getLogPort()
{
	if (isNaN(port) || port == 0)
		port = 514; // default system log server port is 514

	return port;
}

/********************************************************************
**          on document update
********************************************************************/
function on_updatectrl() 
{
	with (document.forms[0]) {
		if (syslogEnable[0].checked) {
			recordLevel.disabled = true;
			dispLevel.disabled = true;
			sysMode.disabled = true;
			logAddr.disabled = true;
			logPort.disabled = true;
			logAddr.value = '';
			logPort.value = '';
		} else {
			recordLevel.disabled = false;
			dispLevel.disabled = false;
			sysMode.disabled = false;
			logAddr.disabled = false;
			logPort.disabled = false;
			logAddr.value = addr;
			logPort.value = getLogPort();
		}
		srvInfo.style.display = (sysMode.value & 2) ? "block" : "none";
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit()
{
	with (document.forms[0]) {
		if (syslogEnable[1].checked == true && sysMode.value & 2) {
			if (sji_checkvip(logAddr.value) == false) {
				logAddr.focus();
				alert("������IP��ַ\"" + logAddr.value +
				      "\"Ϊ��Ч��ַ�����������룡");
				return;
			}
			if (sji_checkdigitrange(logPort.value, 1, 65535) ==
			    false) {
				logPort.focus();
				alert("������UDP�˿�\"" + logPort.value +
				      "\"Ϊ��Ч�˿ڣ����������룡");
				return;
			}
		}
		submit();
	}
}
</script>
</head>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<div align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formSysLogConfig" method="post">
				<b>ϵͳ��¼ -- ����<br></b>
				<br>
				�����¼ģʽ�����ã�ϵͳ����ʼ��¼������ѡ����¼������ڼ�¼�ĵȼ�����������ѡ�ȼ���Ȼ���ߵȼ����¼�������¼��<br>
				������ʾ�ĵȼ�����������ѡ�ȼ���Ȼ���ߵȼ����¼�������ʾ��<br>
				���ѡ���ģʽ��"Remote"��"Both���¼������͵�Զ��ϵͳ��¼���������ض�IP��ַ��UDP�˿ڡ�<br>
				���ѡ���ģʽ��"Local"��"Both"���¼�������¼�ڱ��ش洢����<br>
				<br>
					ѡ����Ҫ��ֵ��Ȼ����"����/Ӧ��"������ϵͳ��¼ѡ�<br>
				<hr align="left" class="sep" size="1" width="90%">
				<table border="0" cellpadding="0" cellspacing="0">
				   <tr>
					  <td width="80">��¼:</td>
					  <td><input name="syslogEnable" type="radio" value="0" onClick="on_updatectrl();" <% checkWrite("log-cap0"); %>>����</td>
					  <td><input name="syslogEnable" type="radio" value="1" onClick="on_updatectrl();"<% checkWrite("log-cap1"); %>>����</td>
				   </tr>
				</table>
				<br>
				<table border="0" cellpadding="0" cellspacing="0">
				   <tr>
					  <td>��¼�ȼ�:</td>
					  <td colspan="2">
				  		<select name="recordLevel" size="1" style="width:120px ">
						<% checkWrite("syslog-log"); %>
						</select>
					 </td>
				   </tr>
				   <tr>
					  <td>��ʾ�ȼ�:</td>
					  <td colspan="2">
					  	<select name="dispLevel" size="1" style="width:120px ">
						<% checkWrite("syslog-display"); %>
						</select>
					  </td>
				   </tr>
				   <tr>
					  <td width="120">ģʽ:</td>
					  <td colspan="2">
				  		<select name="sysMode" size="1" onChange="if (this.selectedIndex == 0) { srvInfo.style.display = 'none'; } else { srvInfo.style.display = 'block'; }" style="width:120px ">
						<% checkWrite("syslog-mode"); %>
						</select>
					 </td>
				   </tr>
				</table>
				<div id="srvInfo">
				   <table border="0" cellpadding="0" cellspacing="0">
					  <tr>
						 <td width="120">������IP��ַ:</td>
						 <td><input type="text" name="logAddr"></td>
					  </tr>
					  <tr>
						 <td>������UDP�˿�:</td>
						 <td><input type="text" name="logPort"></td>
					  </tr>
				   </table>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="button" class="button" onClick="on_submit()" value="����/Ӧ��">
				<input type="hidden" name="submit-url" value="/mgm_log_cfg.asp">
<script>
	on_updatectrl();
</script>			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

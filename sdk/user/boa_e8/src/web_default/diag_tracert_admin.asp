<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>Tracert����</TITLE>
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

/********************************************************************
**          on document load
********************************************************************/
function on_Apply(){
	if( !sji_checkvip(document.forms[0].target_addr.value) && !sji_checkhostname(document.forms[0].target_addr.value)
		&& !isIPv6(document.forms[0].target_addr.value)){
		alert("Ŀ���ַ����");
		document.forms[0].target_addr.focus();
		return false;
	}

	return true;
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
	<form id="form" action=/boaform/admin/formTracert method=POST>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>Tracert����</b>
			<br><br>
			</div>
			<table width="50%" align="left" valign="middle">
			<tr><td>��ַ��<input type="text" name="target_addr" maxlength="15" /></td></tr>
			<tr><td>WAN �ӿڣ�<select name="waninf"><% checkWrite("wan-interface-name"); %></select></td></tr>
			<tr><td><br><input type="submit" value="Trace" width="100px" onClick="return on_Apply();" /></td></tr>
			</table>
		</div>
	</form>
	</blockquote>
</body>
</html>


<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<html>
<head>
<title>USBж��</title>
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
var rcs = new Array();
with (rcs) {
	<% listUsbDevices(); %>
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);

	for (var i = 0; i < rcs.length; i++) {
		form.usbdev.options.add(new Option(rcs[i].path, rcs[i].path));
	}
	if (rcs.length > 0)
		form.usbdev.selectedIndex = 0;
	else
		form.usbdev.disabled = true;
}
</script>
</head>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<div align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formUSBUmount" method="post">
				<table align="left"> 
					<tr>
						<td width="110px" height="31px">USB����ѡ��</td>
						<td><select size="1" name="usbdev" style="width:140px"></select></td>
					</tr>
				</table>
				<br><br>
				<input type="submit" class="button" value="ж��">
				<input type="hidden" name="submit-url" value="">
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

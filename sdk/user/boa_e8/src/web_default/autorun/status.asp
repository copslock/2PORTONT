<!-- add by liuxiao 2008-02-15 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>�й�����</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv="refresh" content="5">
<meta http-equiv=content-script-type content=text/javascript>
<style>
BODY{font-family:"��������";font-weight:bolder;}
</style>
<!--ϵͳ�����ű�-->
<script language="javascript" type="text/javascript">
document.onmousedown = function(e)
{
	var tar = null;
	if (!e)e = window.event;
	if(e.button != 1)return true;
	if (e.target) tar = e.target;
	else if (e.srcElement) tar = e.srcElement;
	if(tar.tagName == "INPUT" || tar.tagName == "IMG") return true;
	document.location.href = "app:mouse:onmousedown";
}

var cgi = new Object();
<%getifstatus();%>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	for(var name in cgi)
	{
		var obj = document.getElementById(name);
		if(typeof obj != "object" || obj == null)continue;
		obj.innerHTML = ((cgi[name]) ? "<font color='green'><b>PASS</b></font>" : "<font color='red'><b>FAIL</b></font>");
	}
}

</script>
</head>
<!--��ҳ����-->
<body bgcolor="E0E0E0" onLoad="on_init();">
	<p align="center"><font size="+2">ϵ ͳ �� ��</font><b></b></p>
	<table cellspacing="3" cellpadding="3" width="256px" align="center" border="0">
	  <tr><td align="left" colspan="2">���Ծ���������</tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;ENNET1���Ӳ���:</td><td id="lan1"></td></tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;ENNET2���Ӳ���:</td><td id="iTV"></td></tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;ENNET3���Ӳ���:</td><td id="lan3"></td></tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;ENNET4���Ӳ���:</td><td id="lan4"></td></tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;�������Ӳ���:</td><td id="wlan"></td></tr>
	  <tr height="6px"><td align="left" colspan="2">&nbsp;</tr>
	  <tr><td align="left" colspan="2">����ADSL����</tr>
	  <tr><td align="left">&nbsp;&nbsp;&nbsp;&nbsp;ADSLͬ������:</td><td id="dsl"></td></tr>
	</table>
</body>
</html>
<!-- add end by liuxiao 2008-02-15 -->

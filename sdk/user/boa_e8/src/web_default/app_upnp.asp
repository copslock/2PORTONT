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
/********************************************************************
**          on document load
********************************************************************/
function proxySelection()
{
	if(document.upnp.daemon[0].checked)
	{
		document.upnp.ext_if.disabled = true;
	}
	else
	{
		document.upnp.ext_if.disabled = false;
	}
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="">
  <blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px">
		<form id="form" action=/boaform/admin/formUpnp method=POST name="upnp">
			<b>UPnP ����</b><br><br>
			<table border="0" cellpadding="0" cellspacing="0">
			  <tr>
			  	<td>UPnP:</td>
				<td>
					<input type="radio" value="0" name="daemon" <%checkWrite("upnp0"); %> onClick="proxySelection()">Disable&nbsp;&nbsp;
					<input type="radio" value="1" name="daemon" <%checkWrite("upnp1"); %> onClick="proxySelection()">Enable </td>
			  </tr>
			  <tr>
			  	<td>WAN Interface:&nbsp;</td>
				<td> <select name="ext_if" <%checkWrite("upnp0d"); %>> <% if_wan_list("rt"); %> </select> </td>
			  </tr>
			</table>
			<br><br>
			<input type="submit" class="button" value="����" name="save">
			<input type="hidden" value="/app_upnp.asp" name="submit-url">
		</form>
	</DIV>
  </blockquote>

<script>
	initUpnpDisable = document.upnp.daemon[0].checked;

	ifIdx = <% getInfo("upnp-ext-itf"); %>;
	if (ifIdx != 65535)
		document.upnp.ext_if.value = ifIdx;
	else
		document.upnp.ext_if.selectedIndex = 0;

	proxySelection();
</script>
</body>
<%addHttpNoCache();%>
</html>

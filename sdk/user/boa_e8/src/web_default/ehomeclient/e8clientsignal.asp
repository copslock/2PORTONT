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
<!--ϵͳ����css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<!--ϵͳ�����ű�-->
<script language="javascript" src="/common.js"></script>
<script language="javascript" type="text/javascript">
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
		if(typeof obj != "object")continue;
		obj.src = ((cgi[name]) ? "/image/signal_up.gif" : "/image/signal_down.gif");
	}
}

</script>
</head>
<!--��ҳ����-->
<body onLoad="on_init();">
<table cellspacing="0" cellpadding="0" width="256" align="center" border="0">
  <tr valign="top">
    <td height="1">
	  <table height="200" cellspacing="0" cellpadding="0" width="100%" align="center" border="0">
	    <tr>
	      <td vAlign=top>
	      <!--��ϵͳ�ϵ�ʱ����ʾ�̵ƣ�ͼƬΪsignal_up.gif��ϵͳ���� flag = 1����ϵͳ���ϵ�ʱ��������ʾͼƬΪsignal_down.gif��ϵͳ���� flag =0 -->
	        <table cellspacing="0" cellpadding="0" border="0" width="100%">
	          <tr>
	            <td width="20%" align="center"><img id="power" src="/image/signal_up.gif" border="0"></td>
	            <td width="20%" align="center"><img id="dsl" src="/image/signal_down.gif" border="0"></td>
	            <td width="20%" align="center"><img id="wan" src="/image/signal_down.gif" border="0"></td>
	            <td width="20%" align="center"><img id="lan4" src="/image/signal_up.gif" border="0"></td>
	            <td width="20%" align="center"><img id="lan3" src="/image/signal_down.gif" border="0"></td>
	          </tr>
	          <tr>
	            <td align="center">��Դ</td>
	            <td align="center">DSL</td>
	            <td align="center">WAN</td>
	            <td align="center">LAN4</td>
	            <td align="center">LAN3</td>
	          </tr>
	          <tr>
	            <td align="center"><img id="iTV" src="/image/signal_down.gif" border="0"></td>
	            <td align="center"><img id="lan1" src="/image/signal_down.gif" border="0"></td>
	            <td align="center"><img id="wlan" src="/image/signal_up.gif" border="0"></td>
	            <td align="center"><img id="wps" src="/image/signal_down.gif" border="0"></td>
	            <td align="center"><img id="usb" src="/image/signal_down.gif" border="0"></td>
	          </tr>
	          <tr>
	            <td align="center">iTV</td>
	            <td align="center">LAN1</td>
	            <td align="center">WLAN</td>
	            <td align="center">WPS</td>
	            <td align="center">USB</td>
	          </tr>       
	        </table>
		  </td>
	    </tr>
	    <tr>
	      <td height="1">
	        <table height="35" cellspacing="0" cellpadding="0" width="100%" border="0">
	          <tr>
	            <td bgcolor="#427594">&nbsp;</td>
			  </tr>
			</table>
		  </td>
		</tr>
      </table>
	</td>
  </tr>
</table>
</body>
<%addHttpNoCache();%>
</html>

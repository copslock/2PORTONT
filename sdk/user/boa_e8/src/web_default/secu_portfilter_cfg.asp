<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�˿ڹ���</TITLE>
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
<%ipPortFilterConfig();%>
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);

	if(cgi.ipfilterEnable == false)
	{
		form.ipFilterMode[0].disabled = true;
		form.ipFilterMode[1].disabled = true;
		policy_frame.location.href = "about:blank";
	}
	else
	{
		on_mode();
	}
}

function on_action()
{
	with(form)
	{
		submit();
	}
}

function on_mode()
{
	var surl = ( (form.ipFilterMode[0].checked == true)? "secu_portfilter_blk.asp" : "secu_portfilter_wht.asp");
	if(policy_frame.location)policy_frame.location.href = surl;
	else policy_frame.src = surl;
}

</SCRIPT>
</HEAD>
<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/formPortFilter method=POST name="form">
				<b>�˿ڹ��� -- ������������ 16������.</b><br><br>
				<!--
				<div id="rstip" style="display:none;"><font color="red">��ʾ����ҳ������ã���Ҫ����·����������Ч��</font><br></div>
				-->
				<hr align="left" class="sep" size="1" width="90%">
				<table border="0" cellpadding="2" cellspacing="0">
					<tr>
						<td>IP��ַ����:</td>
						<td><input type="radio" name="ipfilterEnable" value="off" onClick="on_action()">&nbsp;&nbsp;����</td>
						<td><input type="radio" name="ipfilterEnable" value="on" onClick="on_action()">&nbsp;&nbsp;����</td>
						<!--
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font color="red"><b>ע����Internet���Ӳ������ã�</b></font></td>
						-->
					</tr>
					<tr>
						<td>����ģʽ:</td>
						<td><input type="radio" name="ipFilterMode" onClick="on_mode();" checked>&nbsp;&nbsp;������</td>
						<td><input type="radio" name="ipFilterMode" onClick="on_mode();">&nbsp;&nbsp;������</td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font color="red"><b>ע���ڰ�������ͬʱ�����ģ�</b></font></td>
					</tr>
				</table>
				<br>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="hidden" id="action" name="action" value="sw">
				<input type="hidden" name="submit-url" value="" >
			</form>
		</div>
	</blockquote>
	<iframe src="about:blank" id="policy_frame" width="90%" frameborder="0" style="border-style:none; height:80%"></iframe>
</body>
<%addHttpNoCache();%>
</html>

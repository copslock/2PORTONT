<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�˿ھ�������</TITLE>
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
<%initPagePortMirror();%>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	with (document.forms[0]) 
	{
		mirrorEnable[mirrornum].checked = true;
		vpi.value = mirrorvpi;
		vci.value = mirrorvci;
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(act) 
{
	with (document.forms[0]) 
	{
		submit();
	}
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formPortMirror" method="post">
				<div class="tip" style="width:90% ">
					<b>Port Mirror</b><br><br>
					<div id="rstip"><font color="red" size=1>��ʾ��ͨ����ҳ������ã��ɽ�WAN�˿ڵ�����������̫LAN�˿ڡ�VPI/VCI�ɲ�ָ�������񵥲�����Ŀ��MAC�ĵ�3��4�ֽڵ�ֵ���1������PPPoA��IPoA���ģ�PPPoE��MACͷ(Ŀ��MAC: 00-06-68-89-90-06, ԴMAC: 00-66-88-99-00-66)���Զ���ӡ�</font><br></div>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<table id="tb_mirror" cellpadding="0px" cellspacing="2px">
						<tr><td>�˿ھ���:</td></tr>
						<tr><td>&nbsp;&nbsp;<input type="radio" name="mirrorEnable" value="0">&nbsp;����</td></tr>
						<tr><td>&nbsp;&nbsp;<input type="radio" name="mirrorEnable" value="1">&nbsp;����</td></tr>
						<tr><td>&nbsp;&nbsp;<input type="radio" name="mirrorEnable" value="2">&nbsp;����</td></tr>
						<tr><td>&nbsp;&nbsp;<input type="radio" name="mirrorEnable" value="3">&nbsp;˫��</td></tr>
				</table>
				<br>
				<table id="tb_vpi" cellpadding="0px" cellspacing="2px">
					<tr>
						<td>VPI: <input type="text" name="vpi" value="0" maxlength="3" size="2" style="width:80px"></td>
					</tr>
				</table>
				<br>
				<table id="tb_vci" cellpadding="0px" cellspacing="2px">
					<tr>
						<td>VCI: <input type="text" name="vci" value="0" maxlength="5" size="2" style="width:80px"></td>
					</tr>
				</table>
				<br>
				<input type="button" class="button" style="width:80px" onClick="on_submit();;" value="ȷ��">
				<input type="hidden" name="submit-url" value="/port_mirror.asp">
			</form>
		</DIV>
	</blockquote>
</body>
</HTML>

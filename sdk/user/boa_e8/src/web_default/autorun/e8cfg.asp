<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�й�����-E8����ָ��</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--ϵͳ����css-->
<style>
BODY{font-family:"��������";}
TR{height:16px;}
SELECT {width:140px;}
</style>
<!--ϵͳ�����ű�-->
<script language="javascript" src="/common.js"></script>
<SCRIPT language="javascript" type="text/javascript">
document.onmousedown = function(e)
{
	var tar = null;
	if (!e)e = window.event;
	if(e.button != 1)return true;
	if (e.target) tar = e.target;
	else if (e.srcElement) tar = e.srcElement;
	if(tar.tagName == "INPUT" || tar.tagName == "IMG" || tar.tagName == "A") return true;
	document.location.href = "app:mouse:onmousedown";
}

/********************************************************************
**          on document load
********************************************************************/
function on_init() 
{
}

function on_goto(page)
{
	var rtval = external.Navigate(top.mainFrame, page, 1);
	if(rtval < 1)
	{
		if(rtval == 0)external.ShowDlg("��ȡMODEM�豸��Ϣ������˶��豸�Ƿ�������������");
		else external.ShowDlg("�޷�����MODEM�豸����˶����������Ƿ���ȷ��");
		return false;
	}
	return true;
}

</SCRIPT>
</HEAD>
<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" bgcolor="E0E0E0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init()">
	<DIV align="center" style="padding-left:5px; padding-top:5px">
			<table width="80%"><tr><td><p align="left"><font size="+2"><b>����ָ��</b></font></p></td></tr></table><br><br>
			<table border="0" cellpadding="3" cellspacing="0">
				<tr>
				<td><a href="javascript:on_goto('autorun/accreg.asp');">�����֤ע��</a></td>
				</tr>
				<tr> <td>&nbsp;</td></tr>
				<tr>
				<td><a href="javascript:on_goto('autorun/acccfg.asp');">��������޸�</a></td>
				</tr>
				<tr> <td>&nbsp;</td></tr>
				<tr>
				<td><a href="javascript:on_goto('autorun/wlancfg.asp');">������������</a></td>
				</tr>
			</table>
	</DIV>
</body>
</html>


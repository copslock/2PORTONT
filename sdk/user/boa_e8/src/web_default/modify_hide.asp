<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>����ҳ������</TITLE>
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
<%initHidePage();%>
/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
//	form.devmodel.disabled=true;
	sji_docinit(document, cgi);
}

function btnApply(act)
{
	with(form)
	{
		if(act=="modifyhard"){
			if ( sji_checkhex(MACAddr.value,12,12) == false ) 
			{
				msg = "MAC��ַ \"" + MACAddr.value + "\" �Ƿ�.";
				alert(msg);
				return false;
			}
			if ( sji_checknum(Serialno.value) == false || Serialno.value.length!=8) 
			{
				msg = "Serial Number �Ƿ�,������8λʮ������.";
				alert(msg);
				return false;
			}
		}
		
		action.value=act;
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
		<form id="form" action=/boaform/admin/formModify method=POST>	
			<b><font size=3>���� ����:</font></b><br><br>
			<td><b>    ������ʱ����telnet��ftp��ʹ�ܣ�������ʧЧ:</b></td><br><br>
			<table border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td><input type="checkbox" name="telnetEnable">&nbsp;&nbsp;����telnet</td>
			</tr>
			<tr>
				<td><input type="checkbox" name="ftpEnable">&nbsp;&nbsp;����ftp</td>
			</tr>
			<tr>
				<td><input type="checkbox" name="webEnable">&nbsp;&nbsp;����web</td>
			</tr>
			</table>
			<br>
			<input type="submit" class="button" value="����" name="save" onClick="btnApply('access')"><br><br>

			
			<td><b>    ������ʱ�����ر�tr069 debug��Ϣ,������ʧЧ:</b></td><br><br>
			<table border="0" cellpadding="0" cellspacing="0">
			<tr>
			<td><input type="submit" class="button" value="����tr069 debug" name="debugopen" onClick="btnApply('debugopen')">&nbsp;&nbsp;</td> 
			<td><input type="submit" class="button" value="�ر�tr069 debug" name="debugclose" onClick="btnApply('debugclose')"></td>
			</tr>
			</table>
			<br><br>
<!--
			<td><b>    ���ڸ���MAC��ַ���豸���к�,�������������Ч:</b></td><br><br>
			<table border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td align="left" width="90">MAC��ַ:</td>
                  	<td><input type='text' name='MACAddr' size="25" maxlength="12" value="<% getInfo("lanmac"); %>"></td>
              </tr>
              <tr>
                  <td align="left" width="90">�豸���к�:</td>
                  	<td><input type='text' name='Serialno' size="25" maxlength="8" value="<% getInfo("serialno"); %>"></td>
			</tr>
              </table> 
              <br>
			<input type="button" class="button" value="����" name="savemac" onClick="btnApply('modifyhard')">
			<br><br>  -->

			
			<input type="hidden" name="submit-url" value="/modify_hide.asp">
			<input type="hidden" id="action" name="action" value="none">
			
		</form>
	</DIV>
  </blockquote>
</body>
<%addHttpNoCache();%>
</html>

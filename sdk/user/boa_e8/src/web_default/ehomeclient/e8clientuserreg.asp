<!-- add by ql_xu 2008-05-23 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>�й�����</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv=content-script-type content=text/javascript>
<!--ϵͳ����css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<style>
SELECT {width:150px;}
</style>
<!--ϵͳ�����ű�-->
<script language="javascript" src="/common.js"></script>
<script language="javascript" type="text/javascript">

var pvcPreSet;
var pppUsername;
<% initE8clientUserRegPage(); %>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	with ( document.forms[0] )
	{
		if (pvcPreSet == "1") {
			user.disabled = false;
			auth.disabled = false;
			if (pppUsername != "")
				user.value = pppUsername;
		} else {
			user.disabled = true;
			auth.disabled = true;
		}
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	with ( document.forms[0] ) 
	{
		//input text
		if(sji_checkpppacc(auth.value, 1, 25) == false)
		{
			auth.focus();
			alert("�����֤��Ϣ\"" + auth.value + "\"������Ч�ַ��򳤶Ȳ���1-25�ֽ�֮�䣬���������룡");
			return;
		}
		if(sji_checkpppacc(user.value, 1, 15) == false)
		{
			user.focus();
			alert("�û���������Ч�ַ��򳤶Ȳ���1-15�ֽ�֮�䣬���������룡");
			return;
		}
		
		submit();
	}
}


</script>
</head>
<!--��ҳ����-->
<body onLoad="on_init();">
<form id="form" action="/boaform/admin/formAccountReg" method="post" style="border:0; padding:0; ">
	<br>
	<table id="cfg" cellspacing="0" cellpadding="0" width="256" align="center" border="0">
	  <tr valign="top">
		<td height="1">
		  <table height="200" cellspacing="0" cellpadding="0" width="100%" align="center" border="0">
			<tr>
			  <td vAlign=top>
				<table cellspacing="0" cellpadding="0" border="0" width="100%">
					<tr nowrap>
					  <td>�����֤��Ϣ��</td>
					  <td align="right"><input type="text" name="auth" maxlength="26" size="16" style="width:150px "></td>
 				    </tr>
					<tr nowrap>
					  <td>�˺ţ�</td>
					  <td align="right"><input type="text" name="user" maxlength="16" size="16" style="width:150px "></td>
					</tr>
				</table>
			  </td>
			</tr>
			<tr>
			  <td height="1">
				<table height="35" cellspacing="0" cellpadding="0" width="100%" border="0">
				  <tr>
				    <td bgcolor="#427594" align="center"><img width="80" height="23" src="/image/apply.gif" border="0" onClick="on_submit();" style="cursor:pointer "></td>
				  </tr>
				</table>
			  </td>
			</tr>
		  </table>
		</td>
	  </tr>
	</table>
	<input type="hidden" name="submit-url" value="/ehomeclient/e8clientuseregresult.asp">
</form>
</body>
<%addHttpNoCache();%>
</html>

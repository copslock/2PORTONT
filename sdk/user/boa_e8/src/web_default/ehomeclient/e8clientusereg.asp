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
	if (pvcPreSet == "1") {
		document.getElementById("normaldisplay").style.display = "block";
		document.getElementById("errordisplay").style.display = "none";
		if (pppUsername != "")
			document.forms[0].broadbandusername.value = pppUsername;
/*
		document.forms[0].broadbandusername.disabled = false;
		document.getElementById("customer ID").disabled = false;
		if (pppUsername != "")
			document.forms[0].broadbandusername.value = pppUsername;
*/
	} else {
		document.getElementById("normaldisplay").style.display = "none";
		document.getElementById("errordisplay").style.display = "block";
/*
		document.forms[0].broadbandusername.disabled = true;
		document.getElementById("customer ID").disabled = true;
*/
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	//input text
	if(sji_checkpppacc(document.getElementById("customer ID").value, 1, 25) == false)
	{
		document.getElementById("customer ID").focus();
		alert("�����֤��Ϣ\"" + document.getElementById("customer ID").value + "\"������Ч�ַ��򳤶Ȳ���1-25�ֽ�֮�䣬���������룡");
		return;
	}
	if(sji_checkpppacc(document.forms[0].broadbandusername.value, 1, 25) == false)
	{
		document.forms[0].broadbandusername.focus();
		alert("�û���������Ч�ַ��򳤶Ȳ���1-25�ֽ�֮�䣬���������룡");
		return;
	}
	
	document.forms[0].submit();
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
		  <div id="errordisplay">
		  <table height="200" cellspacing="0" cellpadding="0" width="100%" align="center" border="0">
				<tr>
					<td><font color="#FF0000">�ն�δ���ô���INTERNET����Ϊ��Router�����Ե�WAN���ӣ�����ϵ10000!</td>
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
		  </div>
		  <div id="normaldisplay">
		  <table height="200" cellspacing="0" cellpadding="0" width="100%" align="center" border="0">
			<tr>
			  <td vAlign=top>
				<table cellspacing="0" cellpadding="0" border="0" width="100%">
					<tr nowrap>
					  <td>�����֤��Ϣ��</td>
					  <td align="right"><input type="text" id="customer ID" name="customer ID" maxlength="26" size="16" style="width:150px "></td>
 				    </tr>
					<tr nowrap>
					  <td>�ʺţ�</td>
					  <td align="right"><input type="text" name="broadbandusername" maxlength="26" size="26" style="width:150px "></td>
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
		  </div>
		</td>
	  </tr>
	</table>
	<input type="hidden" name="submit-url" value="/ehomeclient/e8clientuseregresult.asp">
</form>
</body>
<%addHttpNoCache();%>
</html>

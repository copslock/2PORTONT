<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<html>
<head>
<title>�й�����-�߼�IDע��</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv=content-script-type content=text/javascript>
<!--ϵͳ����css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<style>
body { 
	font-family: "��������";
    background-image: url('/image/loid_register.gif');
    background-repeat: no-repeat;
    background-attachment: fixed;
    background-position: center top; 
}
tr {height: 16px;}
select {width: 150px;}
</style>
<!--ϵͳ�����ű�-->
<script language="javascript" src="/common.js"></script>
<script language="javascript" type="text/javascript">

var over;
var loid;
var password;
<% initE8clientUserRegPage(); %>

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if (over == 1) {
		document.getElementById("normaldisplay").style.display = "none";
		document.getElementById("errordisplay").style.display = "block";
		document.getElementById("over_msg").style.display = "block";
	} else {
		document.getElementById("normaldisplay").style.display = "block";
		document.getElementById("errordisplay").style.display = "none";
		document.getElementById("loid").value = loid;
		document.getElementById("password").value = password;
	}

	if (window.top != window.self) {
		// in a frame
		document.getElementById("back").style.display = "none";
		document.getElementById("back_error").style.display = "none";
	} else {
		// the topmost frame
		document.getElementById("back").style.display = "block";
		document.getElementById("back_error").style.display = "block";
	}
}

function reset_loid()
{
	document.getElementById("loid").value = loid;
	document.getElementById("password").value = password;
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	var loid = document.getElementById("loid");
	var password = document.getElementById("password");
	var regbutton = document.getElementById("regbutton");

	if (sji_checkpppacc(loid.value, 1, 24) == false) {
		loid.focus();
		alert("�߼� ID\"" + loid.value + "\"������Ч�ַ��򳤶Ȳ���1-24�ֽ�֮�䣬���������룡");
		return false;
	}

	regbutton.disabled = true;

	return true;
}

</script>

</head>
<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" bgcolor="E0E0E0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init()">
	<div align="center" style="padding-left:5px; padding-top:5px">
		<form id="form" action="/boaform/formUserReg" method="post" onsubmit="return on_submit()">
			<!--<p align="center"><font size="+2"><b>�߼�IDע��</b></font></p><br><br> -->
				<div align="center" id="errordisplay">
				  <table height="200" cellspacing="0" cellpadding="0" align="center" border="0">
						<tr>
							<td>
							<span id="over_msg" style="display: none">����������Դ���������ϵ 10000 ��</span>
							</td>
						</tr>
				   </table>
				   <table border="0" cellpadding="1" cellspacing="0">
					<tr>
					<td align="center" id="back_error"><input type="button" value="���ص�¼ҳ��" onClick="location.href='/admin/login.asp';" style="width:80px; border-style:groove; font-weight:bold "></td>&nbsp;&nbsp;
					</tr>
				   </table>
				</div>
				<br><br><br><br><br><br><br>
				<div align="center" id="normaldisplay">
					<b> <% UserRegMsg(); %><br>
				  <table cellspacing="0" cellpadding="0" align="center" border="0">
							<tr nowrap><td>�߼� ID��</td><td align="right"><input type="text" id="loid" name="loid" maxlength="24" size="24" style="width:150px "></td></tr><br>
							<tr nowrap><td>���룺</td><td align="right"><input type="text" id="password" name="password" maxlength="12" size="24" style="width:150px "></td></tr>
				  </table>
				  <table border="0" cellpadding="1" cellspacing="0">
					<tr>
					<td align="right"><input type="submit" id="regbutton" name="regbutton" value="ȷ��" style="width:80px; border-style:groove; font-weight:bold "></td>&nbsp;&nbsp;
					<td align="right" id="reset"><input type="button" value="����" onClick="reset_loid();" style="width:80px; border-style:groove; font-weight:bold "></td>&nbsp;&nbsp;
					<td align="right" id="back"><input type="button" value="���ص�¼ҳ��" onClick="location.href='/admin/login.asp';" style="border-style:groove; font-weight:bold "></td>
					</tr>

				  </table>
				  </b>
				</div>
			<br>
			
			<input type="hidden" name="submit-url" value="/useregresult.asp">
		</form>
	</DIV>

</body>
</html>




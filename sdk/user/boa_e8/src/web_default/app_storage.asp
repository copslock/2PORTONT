<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�ճ�Ӧ��</TITLE>
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
<% initPageStorage(); %>
var rcs = new Array();
with(rcs){<% listUsbDevices(); %>}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);
	with(form)
	{
		tdftpEnable.innerHTML = cgi.ftpEnable ? "����" : "����";
	}
	
	while(form.saveDir.options.length >= 1)
		{form.saveDir.options.remove(0);}
	
	for(var i = 0; i < rcs.length; i++)
	{
		form.saveDir.options.add(new Option(rcs[i].path, rcs[i].path + "/xdown"));
	}
	
	if(rcs.length == 0)
	{
		form.saveDir.options.add(new Option("��USB�洢�豸", "0"));
		form.saveDir.disabled = true;
		form.btnDown.disabled = true;
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(act) 
{
	with ( document.forms[0] ) 
	{
		if(act == "rl") 
		{
			var loc = "app_storage.asp";
			var code = "window.location.href=\"" + loc + "\"";
			eval(code);
			return;
		}
		if(saveDir.value == "0") 
		{
			alert("�����USB�豸, ��ˢ�±�ҳ�棡");
			return false;
		}
		if(user.value!=""&&!sji_checkusername(user.value, 1, 32))
		{
			alert("������Ϸ����û�����");
			return false;
		}
		if(passwd.value!=""&&!sji_checkpswnor(passwd.value, 1, 32))
		{
			alert("������Ϸ������룡");
			return false;
		}
		
		if(port.value!==""&&!sji_checkdigitrange(port.value, 1, 65535))
		{
			alert("������Ϸ��Ķ˿ںţ�");
			return false;
		}
			
		if(!sji_checkftpurl(rmtURL.value))
		{
			alert("������Ϸ���Զ�� FTP URL��");
			return false;
		}
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
			<form id="form" action=/boaform/admin/formStorage method=POST>
				<b>����״̬<br><br>
				<table border="0" cellpadding="3" cellspacing="0">
					<tr>
						<td>FTP������:</td>
						<td id="tdftpEnable"></td>
					</tr>
				</table>
				<br>
				<input type="button" class="button" onClick ="on_submit('rl')" value="ˢ�±�ҳ">
				
				<p><HR align="left" width=500></p>
				
				<table border="0" cellpadding="0" cellspacing="0" width="500">
					<tr><td width="200px">USBԶ�������ļ����Ŀ¼:</td><td><select size="1" name="saveDir" style="width:140px "></select>/xdown</td></tr>
					<tr><td>�û���:</td><td><input name="user" size="16" maxlength="32" type="text" style="width:140px "></td></tr>
					<tr><td>����:</td><td><input name="passwd" size="16" maxlength="32" type="password" style="width:140px "></td></tr>
					<tr><td>�˿�:</td><td><input name="port" size="6" maxlength="6" type="text" style="width:140px "></td></tr>
					<tr><td>Զ��URL:</td><td><input name="rmtURL" size="50" maxlength="128" type="text" style="width:280px "></td></tr>
					<!--<tr><td colspan="2"><div id="rstip" style="display:block;"><font color="red">��ʾ: URL��ʽ :(Э��)://(������):(�˿ں�) / (�ļ�·��)/(�ļ���),Э��֧��http��ftp</font><br></div></td></tr>-->

				</table>
				<br>
				<input type="button" class="button" name="btnDown" value="����" onClick="on_submit('dl')">
				<input type="hidden" name="submit-url" value="">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

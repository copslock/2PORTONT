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


/********************************************************************
**          on document load
********************************************************************/
function on_init()
{

}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{

	if((form.proxy0_port.value == "") || !sji_checkdigitrange(form.proxy0_port.value, 1, 65535))
	{
		alert("����SIP Proxy������Ϸ��Ķ˿ںţ�");
		return false;
	}

	if(!(form.proxy1_port.value == "") && !sji_checkdigitrange(form.proxy1_port.value, 1, 65535))
	{
		alert("����SIP Proxy������Ϸ��Ķ˿ںţ�");
		return false;
	}

	//if(!(form.port1_number.value == "") && !sji_checkusername(form.port1_number.value, 1, 32))
	//{
		//alert("��·1 ������Ϸ��ĺ��룡");
		//return false;
	//}
	//if(!(form.port1_login_id.value == "") && !sji_checkusername(form.port1_login_id.value, 1, 32))
	//{
		//alert("��·1 ������Ϸ����û�����");
		//return false;
	//}
	//if(!(form.port1_password.value == "") &&!sji_checkpswnor(form.port1_password.value, 1, 32))
	//{
		//alert("��·1 ������Ϸ������룡");
		//return false;
	//}
	//if((form.max_voip_ports.value == 2)&&!(form.port2_number.value == "") && !sji_checkusername(form.port2_number.value, 1, 32))
	//{
		//alert("��·2 ������Ϸ��ĺ��룡");
		//return false;
	//}
	//if((form.max_voip_ports.value == 2)&&!(form.port2_login_id.value == "") && !sji_checkusername(form.port2_login_id.value, 1, 32))
	//{
		//alert("��·2 ������Ϸ����û�����");
		//return false;
	//}
	//if((form.max_voip_ports.value == 2)&&!(form.port2_password.value == "") &&!sji_checkpswnor(form.port2_password.value, 1, 32))
	//{
		//alert("��·2 ������Ϸ������룡");
		//return false;
	//}
	if((form.proxy0_reg_expire.value == "") || !sji_checkdigitrange(form.proxy0_reg_expire.value, 10, 86400))
	{
		alert("����SIP Proxy Register Expire out of range [10-86400]��");
		return false;
	}
	if((form.proxy1_reg_expire.value == "") || !sji_checkdigitrange(form.proxy1_reg_expire.value, 10, 86400))
	{
		alert("����SIP Proxy Register Expire out of range [10-86400]��");
		return false;
	}
	if((form.proxy1_enable.checked == true) && (form.port1_account_enable.checked != true)&&(form.port2_account_enable.checked != true))
	{
		alert("���Â���sip proxy �膢����·�˺�");
		return false;
	}
	if((form.max_voip_ports.value == 2) && (form.port1_number.value!="")&&(form.port1_number.value == form.port2_number.value))
	{
		alert("��·1��������·2������ͬ");
		return false;
	}
	form.submit();
	//submit();
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/admin/voip_e8c_set method=POST>
			<input type=hidden name=max_voip_ports value="<%voip_e8c_get("max_voip_ports");%>">
				<td>
				<table border="0" cellpadding="0" cellspacing="0" width="700">
					<tr>
						<td width="200px">�����汾</td><td><select name=servertype>"<%voip_e8c_get("servertype");%>"</select></td>
					</tr>
				</table>
				<b>����SIP ����</br>
				<table border="0" cellpadding="0" cellspacing="0" width="700">
					<tr>
						<td width="200px">��ַ:</td><td><input name="proxy0_addr" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy0_addr"); %>"></td>
					</tr>
					<tr>
						<td>�˿ں�:</td><td><input name="proxy0_port" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy0_port"); %>"></td>
					</tr>
					<tr>
						<td>�ⷢ��������</td><td><input name="proxy0_obEnable" type="checkbox" value="enable" <% voip_e8c_get("proxy0_obEnable"); %>></td>
					</tr>
					<tr>
						<td>�ⷢ�����ַ:</td><td><input name="proxy0_obAddr" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy0_obAddr"); %>"></td>
					</tr>
					<tr>
						<td>�ⷢ����˿ں�:</td><td><input name="proxy0_obPort" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy0_obPort"); %>"></td>
					</tr>
					<tr>
						<td>SIP ����:</td><td><input name="proxy0_domain_name" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy0_domain_name"); %>"></td>
					</tr>
					<tr>
						<td>ע����Ч�� (��):</td><td><input name="proxy0_reg_expire" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy0_reg_expire"); %>"></td>
					</tr>
					<tr>
						<td>���Õ�Ԓ����</td><td><input name="proxy0_sessionEnable" type="checkbox" value="enable" <% voip_e8c_get("proxy0_sessionEnable"); %>></td>
					</tr>
					<tr>
						<td>�Ự�������� (��):</td><td><input name="proxy0_sessionExpiry" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy0_sessionExpiry"); %>"></td>
					</tr>
				</table>
				<br>

				<b>����SIP ����</b>
				<table border="0" cellpadding="0" cellspacing="0" width="700">
					<tr>
						<td width="200px">����SIP����</td><td><input name="proxy1_enable" type="checkbox"  value="enable" <% voip_e8c_get("proxy1_enable"); %>></td>
					</tr>
					<tr>
					<tr>
						<td width="200px">��ַ:</td><td><input name="proxy1_addr" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy1_addr"); %>"></td>
					</tr>
					<tr>
						<td>�˿ں�:</td><td><input name="proxy1_port" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy1_port"); %>"></td>
					</tr>
					<tr>
						<td>�ⷢ��������</td><td><input name="proxy1_obEnable" type="checkbox" value="enable" <% voip_e8c_get("proxy1_obEnable"); %>></td>
					</tr>
					<tr>
						<td>�ⷢ�����ַ:</td><td><input name="proxy1_obAddr" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy1_obAddr"); %>"></td>
					</tr>
					<tr>
						<td>�ⷢ����˿ں�:</td><td><input name="proxy1_obPort" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy1_obPort"); %>"></td>
					</tr>
					<tr>
						<td>SIP ����:</td><td><input name="proxy1_domain_name" size="16" maxlength="32" type="text" style="width:140px" value="<% voip_e8c_get("proxy1_domain_name"); %>"></td>
					</tr>
					<tr>
						<td>ע����Ч��(��):</td><td><input name="proxy1_reg_expire" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy1_reg_expire"); %>"></td>
					</tr>
					<tr>
						<td>���Õ�Ԓ����</td><td><input name="proxy1_sessionEnable" type="checkbox" value="enable" <% voip_e8c_get("proxy1_sessionEnable"); %>></td>
					</tr>
					<tr>
						<td>�Ự�������� (��):</td><td><input name="proxy1_sessionExpiry" size="16" maxlength="32" style="width:140px" value="<% voip_e8c_get("proxy1_sessionExpiry"); %>"></td>
					</tr>
				</table>
				<br>
				
				<% voip_e8c_get("port_account"); %>
				

				<input type="button" class="button" name="btnDown" value="����/Ӧ��" onClick="on_submit()">
				<input type="hidden" name="submit-url" value="">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

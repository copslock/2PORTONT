<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>����ҳ������-��Ӧ��/�汾��Ϣ</TITLE>
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
<%initVendorVersion();%>
/********************************************************************
**          on document load
********************************************************************/
function isHexaDigit(val)
{
	if(val>='a' && 'f'>=val)
		return true;
	else if(val>='A' && 'F'>=val)
		return true;
	else if(val>='0' && '9'>=val)
		return true;
	else 
		return false;
}

function isValidwpapskValue(val) 
{
	var ret = false;
	var len = val.length;
	var maxSize = 64;
	var minSize = 8;

	if ( len >= minSize && len < maxSize )
		ret = true;
	else if ( len == maxSize ) 
	{
		for ( i = 0; i < maxSize; i++ )
		if ( isHexaDigit(val.charAt(i)) == false )
			break;
		if ( i == maxSize )
			ret = true;
	}
	else
		ret = false;

	return ret;
}
function on_init()
{
	//form.devmodel.disabled=true;
	sji_docinit(document, cgi);

	with(form){
/*
		if(rootssid.value=="")
			rootssid.disabled=true;
		if(userpassword.value=="")
			userpassword.disabled=true;
*/
		softwareversion.disabled=true;
		oui.disabled=true;
	}
}

function btnApply(act)
{
	var modemmac=cgi.macaddr;
	var modemssid=cgi.rootssid;
	var modemuserpass=cgi.userpassword;
	with(form)
	{
			if( manufacture.value == '' )
			{
				manufacture.focus();
		            alert('�豸��Ӧ�̲���Ϊ��.');
		            return false;
			}
			if(isIncludeInvalidChar(manufacture.value))
			{
					alert("�豸��Ӧ�̺��зǷ��ַ�������������!");
					return false;
			}	
			if ( devmodel.value == '' ) 
		         {
		         	devmodel.focus();
		            alert('�豸�ͺŲ���Ϊ��.');
		            return false;
		         }
			if(isIncludeInvalidChar(devmodel.value))
			{
					alert("�豸�ͺź��зǷ��ַ�������������!");
					return false;
			}	
			/*
			if ( oui.value == '' ) 
		         {
		         	oui.focus();
		            alert('��Ӧ��OUI����Ϊ��.');
		            return false;
		         }
			if(isIncludeInvalidChar(oui.value))
			{
					alert("��Ӧ��OUI���зǷ��ַ�������������!");
					return false;
			}	
			if ( softwareversion.value == '' ) 
		         {
		         	softwareversion.focus();
		            alert('�豸����汾����Ϊ��.');
		            return false;
		         }
			if(isIncludeInvalidChar(softwareversion.value))
			{
					alert("�豸����汾���зǷ��ַ�������������!");
					return false;
			}
			*/
			if ( hardwareversion.value == '' ) 
		         {
		         	hardwareversion.focus();
		            alert('�豸Ӳ���汾����Ϊ��.');
		            return false;
		         }
			if(isIncludeInvalidChar(hardwareversion.value))
			{
					alert("�豸Ӳ���汾���зǷ��ַ�������������!");
					return false;
			}
/*
			if ( productclass.value == '' ) 
		         {
		         	productclass.focus();
		            alert('TR069 ProductClass����Ϊ��.');
		            return false;
		         }
			if(isIncludeInvalidChar(productclass.value))
			{
					alert("TR069 ProductClass���зǷ��ַ�������������!");
					return false;
			}

			if ( specversion.value == '' ) 
		         {
		         	specversion.focus();
		            alert('TR069 Specversion����Ϊ��.');
		            return false;
		         }
			if(isIncludeInvalidChar(specversion.value))
			{
					alert("TR069 Specversion���зǷ��ַ�������������!");
					return false;
			}
*/
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
		<form id="form" action=/boaform/admin/formVendorVersion method=POST>	
			<br><br>
			<table border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td align="left" width="120">�豸��Ӧ��:</td>
                  		<td><input type='text' name='manufacture' size="25" maxlength="30" value="<% getInfo("manufacture"); %>"></td>
              	</tr>
			<tr>
				<td align="left" width="120">�豸�ͺ�:</td>
                  		<td><input type='text' name='devmodel' size="25" maxlength="30" value="<% getInfo("devModel"); %>"></td>
              	</tr> 
              	<tr>
				<td align="left" width="120">��Ӧ��OUI:</td>
                  		<td><input type='text' name='oui' size="25" maxlength="30" value="<% getInfo("oui"); %>"></td>
              	</tr>

              	<tr>
				<td align="left" width="120">�豸����汾:</td>
                  		<td><input type='text' name='softwareversion' size="25" maxlength="30" value="<% getInfo("stVer"); %>"></td>
              	</tr>

              	<tr>
				<td align="left" width="120">�豸Ӳ���汾:</td>
                  		<td><input type='text' name='hardwareversion' size="25" maxlength="30" value="<% getInfo("hdVer"); %>"></td>
              	</tr>
              	<!--
              	<tr>
				<td align="left" width="120">TR069 ProductClass:</td>
                  		<td><input type='text' name='productclass' size="25" maxlength="30" value="<% getInfo("ProductClass"); %>"></td>
              	</tr>
              	-->
              	<!--
              	<tr>
				<td align="left" width="120">TR069 Specversion:</td>
                  		<td><input type='text' name='specversion' size="25" maxlength="30" value="<% getInfo("SpecVer"); %>"></td>
              	</tr>	-->
			
              </table>   
              <br>
			<input type="button" class="button" value="����" name="save" onClick="btnApply('modify')">
		
			<input type="hidden" id="action" name="action" value="none">
			<input type="hidden" name="submit-url" value="/vendorversion.asp">
			
		</form>
	</DIV>
  </blockquote>
</body>
<%addHttpNoCache();%>
</html>

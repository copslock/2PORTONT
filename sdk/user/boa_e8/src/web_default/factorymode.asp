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
<%initFactory();%>
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
/*
	with(form){
		if(rootssid.value=="")
			rootssid.disabled=true;
		if(userpassword.value=="")
			userpassword.disabled=true;
	}
*/
}

function btnApply(act)
{
	var modemmac=cgi.macaddr;
	var modemssid=cgi.rootssid;
	var modemuserpass=cgi.userpassword;
	with(form)
	{
			if ( sji_checkhex(macaddr.value,12,12) == false ) 
			{
				msg = "MAC��ַ \"" + macaddr.value + "\" �Ƿ�.";
				alert(msg);
				return false;
			}
			if(isIncludeInvalidChar(serialnum.value))
			{
					alert("�豸��ʶ���зǷ��ַ�������������!");
					return false;
			}	
			/*
			if ( devmodel.value == '' ) 
		         {
		         	devmodel.focus();
		            alert('�豸�ͺŲ���Ϊ��.');
		            return false;
		         }*/
///		if(modemssid!="")
		/*
			if ( rootssid.value == '' ) 
		         {
		            alert('SSID����Ϊ��.');
		            return false;
		         }
		         */
				 	
		if(rootssid.value!=""){
		         var str = new String();
		         str = rootssid.value;
		         if ( str.length > 32 ) 
		         {
		            alert('SSID "' + rootssid.value + '" ���ܴ���32���ַ���');
		            return false;
		         }
			   var place = str.indexOf("ChinaNet-");
			   if(place!=0)
			   {
				 	alert('SSID "' + rootssid.value + '" ������ChinaNet-��ͷ�����������롣');
					return false;
			   }
			   if(isIncludeInvalidChar(rootssid.value))
			   {
					alert("SSID ���зǷ��ַ�������������!");
					return false;
			   }	
		}

		if ( isValidwpapskValue(rootpassword.value) == false ) 
		{
			alert('WPAԤ���õ���ԿӦ����8����63��ASCII�ַ���64��ʮ����������֮��.');
			return false;
		}

//		if(modemuserpass!="")
		/*
			if(userpassword.value.length <= 0) 
			{
				userpassword.focus();
				alert("�����벻��Ϊ�գ�������������!");
				return false;
			}*/
		if(userpassword.value!=""){
			if(sji_checkpswnor(userpassword.value, 1, 30) == false)
			{
				userpassword.focus();
				alert("�������������������������!");
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
		<form id="form" action=/boaform/admin/formFactory method=POST>	
			<td><b>��ҳ������������������Ч:</b></td><br><br>
			<table border="0" cellpadding="0" cellspacing="0">
			<!--
			<tr>
				<td align="left" width="110">�豸�ͺ�:</td>
                  	<td><input type='text' name='devmodel' size="25" maxlength="30" value="<% getInfo("devModel"); %>"></td>
              </tr> -->
              <tr>
                  <td align="left" width="110">MAC��:</td>
                  	<td><input type='text' name='macaddr' size="25" maxlength="12"></td>
			</tr>
			<tr>
                  <td align="left" width="110">�豸���к�:</td>
                  	<td><input type='text' name='serialnum' size="25" maxlength="128" value="<% getInfo("serialno"); %>"></td>
			</tr>
			<tr>
                  <td align="left" width="110">Ĭ��������������:</td>
                  	<td><input type='text' name='rootssid' size="25" maxlength="32" ></td>
			</tr>
			<tr>
                  <td align="left" width="110">Ĭ������������Կ:</td>
                  	<td><input type='text' name='rootpassword' size="25" maxlength="65" ></td>
			</tr>
			<tr>
                  <td align="left" width="110">Ĭ���ն���������:</td>
                  	<td><input type='text' name='userpassword' size="25" maxlength="30" ></td>
			</tr>
              </table>   
              <br>
			<input type="button" class="button" value="����" name="save" onClick="btnApply('modify')">
		
			<input type="hidden" id="action" name="action" value="none">
			<input type="hidden" name="submit-url" value="/factorymode.asp">
			
		</form>
	</DIV>
  </blockquote>
</body>
<%addHttpNoCache();%>
</html>

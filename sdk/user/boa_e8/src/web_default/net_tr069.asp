<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�й�����-ITMS������</TITLE>
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

var configurable = <% TR069ConPageShow("cwmp-configurable"); %>;

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	if(configurable)
		return;

	for(var i=0; i<document.forms[0].length;i++)
		document.forms[0].elements[i].disabled = true;
}

function isValidPort(port) 
{
	var fromport = 0;
	var toport = 100;
	
	portrange = port.split(':');
	if (portrange.length < 1 || portrange.length > 2) {
		return false;
	}
	if (isNaN(portrange[0]))
		return false;
	fromport = parseInt(portrange[0]);
	
	if (portrange.length > 1) {
		if (isNaN(portrange[1]))
			return false;
			
		toport = parseInt(portrange[1]);
		
		if (toport <= fromport)
			return false;      
	}
	
	if (fromport < 1 || fromport > 65535 || toport < 1 || toport > 65535) {
		return false;
	}
	
	return true;
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(act) 
{
	with ( document.forms[0] ) 
	{
		action.value = act;
		if(act=="sv")
		{
			if (acsURL.value.length > 256) 
			{
				alert('The length of ACS URL (' + acsURL.value.length + ') is too long [1-256].');
				return false;
			}
			if(!sji_checkhttpurl(acsURL.value))
			{
				alert("ACS URL �ǲ��Ϸ��� URL!");
				return false;
			}
			if(!sji_checknum(informInterval.value))
			{
				alert("�����ϱ����ʱ�����Ϊ������!");
				return false;
			}
			if (acsUser.value.length > 256) 
			{
				alert('The length of ACS user name (' + acsUser.value.length + ') is too long [0-256].');
				return false;
			}
			if(isInvalidInput(acsUser.value))
			{
				alert("ACS �û������зǷ��ַ�������������!");
				return false;
			}      
			if (acsPwd.value.length > 256) 
			{
				alert('The length of sysName (' + acsPwd.value.length + ') is too long [0-256].');
				return false;
			}
			if(isInvalidInput(acsPwd.value))
			{
				alert("ACS ���뺬�зǷ��ַ�������������!");
				return false;
			}      
			if (connReqUser.value.length > 256) 
			{
				alert('The length of connection request user name (' + connReqUser.value.length + ') is too long [0-256].');
				return false;
			}
			if(isInvalidInput(connReqUser.value))
			{
				alert("���������û������зǷ��ַ�������������!");
				return false;
			}
			if (connReqPwd.value.length > 256) 
			{
				alert('The length of connection request password (' + connReqPwd.value.length + ') is too long [0-256].');
				return false;
			}
			if(isInvalidInput(connReqPwd.value))
			{
				alert("�����������뺬�зǷ��ַ�������������!");
				return false;
			}
			applyTr069Config.value = "applyTr069Config";
			return true;
		}
	}

	return false;
}

/*
 * isCharUnsafe - test a character whether is unsafe
 * @c: character to test
 */
function isInvalidChar(c)
{
	var unsafeString = "\"\\`\,='\t";

	return unsafeString.indexOf(c) != -1 
		|| c.charCodeAt(0) <= 32 
		|| c.charCodeAt(0) >= 123;
}

/*
 * isIncludeInvalidChar - test a string whether includes invalid characters
 * @s: string to test
 */
function isInvalidInput(s) 
{
	var i;	

	for (i = 0; i < s.length; i++) {
		if (isInvalidChar(s.charAt(i)) == true)
			return true;
	}

	return false;
} 

function intervaldisable()
{
	document.forms[0].informInterval.disabled = true;
}

function intervalenable()
{
	document.forms[0].informInterval.disabled = false;
}

</script>
   </head>
   <body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad='on_init()'>
     <div align="left" style="padding-left:20px; padding-top:10px">
      <blockquote>
         <form action=/boaform/admin/formTR069Config method="post">
           <b>TR-069�ͻ�-����<br>
            </b>
            <br>
           WAN����Э��(TR-069)�����Զ����÷�����(ACS)�����Զ����á���Ӧ���㼯����ϵ�����豸��
           <br>
            <br>
    ѡ������ֵ�����ҵ��������/Ӧ�á�����TR-069�ͻ�ѡ�<br>
            <br>
            <br>
            <table border="0" cellpadding="0" cellspacing="0">
			   <tr>
                  <td>������ URL:</td>
                  <td><input type='text' name='acsURL' size="20" maxlength="256" value="<% getInfo("acs-url"); %>"></td>
               </tr>
               <tr>
                  <td>�û���:</td>
                  <td><input type='text' name='acsUser' size="20" maxlength="256" value="<% getInfo("acs-username"); %>"></td>
               </tr>
               <tr>
                  <td>����:</td>
                  <td><input type='password' name='acsPwd' size="20" maxlength="256" value="<% getInfo("acs-password"); %>"></td>
               </tr>
			   <tr>
                  <td width="80">����֤��:</td>
                  <td><input name='certauth' value='1' type='radio' <% checkWrite("tr069-certauth-1"); %> >
          ��</td>
                  <td><input name='certauth' value='0' type='radio' <% checkWrite("tr069-certauth-0"); %> >
          ��</td>
               </tr>
			   <tr>
                  <td width="80">�����ϱ�:</td>
                  <td><input name='inform' value='1' type='radio' onClick="intervalenable()" <% checkWrite("tr069-inform-1"); %> >
          ����</td>
                  <td><input name='inform' value='0' type='radio' onClick="intervaldisable()" <% checkWrite("tr069-inform-0"); %> >
          ����</td>		
               </tr>
               <tr>
                  <td width="200">�����ϱ����ʱ��:</td>
                  <td><input type='text' name='informInterval' size="20" maxlength="10" value="<% getInfo("inform-interval"); %>" <% checkWrite("tr069-interval"); %>>��</td>
               </tr>
            </table>
            <br>
            <table border="0" cellpadding="0" cellspacing="0">
               <tr>
                  <td width="200">���������û���:</td>
                  <td><input type='text' name='connReqUser' size="20" maxlength="256" value="<% getInfo("conreq-name"); %>"></td>
               </tr>
               <tr>
               <td>������������:</td>
                  <td><input type='password' name='connReqPwd' size="20" maxlength="256" value="<% getInfo("conreq-pw"); %>"></td>
               </tr>
            </table>
            <br>
    <p > 
      <input type='submit' onClick="return on_submit('sv')" value='����/Ӧ��'>
	  <input type="hidden" name="applyTr069Config" value="">
	  <input type="hidden" id="action" name="action" value="none">
	  <input type="hidden" name="submit-url" value="/net_tr069.asp">
    </P>
        </form>
		</div>
      </blockquote>
   </body>
<%addHttpNoCache();%>
</html>

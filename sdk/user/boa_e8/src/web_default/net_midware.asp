<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�й�����-�м������</TITLE>
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

/*
var  cgi_enblChinaTelcomMDW = 1; //�м������  
var  cgi_mwsURLIP = "10.10.10.25";//�м����������ַ
var  cgi_mwsURLPort="80";//�м���������˿�
*/

/********************************************************************
**          on document load
********************************************************************/
/*
function on_init() 
{
	with (document.forms[0]) {
		enblChinaTelcomMDW = cgi_enblChinaTelcomMDW; //�м������  
		if ( enblChinaTelcomMDW == '1' )
			ctMDW[1].checked = true;
		else
			ctMDW[0].checked = true;
		
		mwsURLIP.value = cgi_mwsURLIP;
		
		mwsURLPort.value = cgi_mwsURLPort;
	}
}
*/		

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
			if(ctMDW[1].checked == true){
				if (mwsURLIP.value.length > 256 ) 
				{
					alert('�м����������ַ���ȹ��� (' + mwsURLIP.value.length + ') [0-256]'); //The length of Middleware Server URL is too long [0-256].
					return;
				}
				if(isIncludeInvalidChar(mwsURLIP.value))
				{
					alert("�м����������ַ���зǷ��ַ�������������!");
					return;
				}
				if(isValidPort(mwsURLPort.value) == false)
				{
					alert('��Ч���м����������ַ�˿�(' + mwsURLPort.value + ')'); //Middleware Server URL Port is invalid.
					return;
				}
			}
			applyMidwareConfig.value = "applyMidwareConfig";
			submit(); 
		}
	}
}

</script>
   </head>
   <body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
     <div align="left" style="padding-left:20px; padding-top:10px">
      <blockquote>
         <form action=/boaform/admin/formTR069Config method="post">
           <b>�й������м������<br>
            </b>
            <br>
           ��ҳ���������й������м���Ĺ���ģʽ���м���������ĵ�ַ�Ͷ˿ڡ�
           <br>
           <br>
          	  <table border="0" cellpadding="0" cellspacing="0">
				<tr><td><font color='red'>(���ر��м�����͡������м��������TR069���ܣ���֮���л�����Ҫ�����������л���Ҫ������)</font></td></tr>
			  </table>
			  <br>
                <table border="0" cellpadding="0" cellspacing="0">
                  <tr>
                  <script language="javascript">
						document.write("<td width=\"200\">�м������ģʽ</td>");		
				</script>
                     <td><input name='ctMDW' value='0' type='radio'  <% checkWrite("midware-enable1"); %> >
          �����м������TR069���ܣ�</td>
          		</tr>
          		<tr>
          			 <td width="200">&nbsp;&nbsp;</td>
          			 <td><input name='ctMDW' value='1' type='radio' <% checkWrite("midware-disable"); %> >
          �ر��м��&nbsp;&nbsp;</td>
          		</tr>
          		<tr>
          			 <td width="200">&nbsp;&nbsp;</td>
                     <td><input name='ctMDW' value='2' type='radio'  <% checkWrite("midware-enable2"); %> >
          �����м��������TR069���ܣ�</td>
                </tr>
               </table>
          	   <br>
               <table border="0" cellpadding="0" cellspacing="0">
  	              <tr>
                     <td width="200">�м������ƽ̨URL</td>
                     <td>��ַ:&nbsp;<input type='text' name='mwsURLIP' size="20" maxlength="256" value="<% getInfo("midwareServerAddr"); %>"></td>
                     <td>&nbsp;&nbsp;�˿ں�:&nbsp;<input type='text' name='mwsURLPort' size="20" maxlength="256" value="<% getInfo("midwareServerPort"); %>"></td>
                  </tr>
               </table>
            
    <p > 
      <input type='button'  onClick="on_submit('sv')" value='����/Ӧ��'>
	  <input type="hidden" name="applyMidwareConfig" value="">
	  <input type="hidden" id="action" name="action" value="none">
	  <input type="hidden" name="submit-url" value="/net_midware.asp">
    </P>
        </form>
		</div>
      </blockquote>
   </body>
<%addHttpNoCache();%>
</html>

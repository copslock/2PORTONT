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

var certstart = '-----BEGIN CERTIFICATE-----';
var certinsert = '\n<insert certificate here>\n';
var certend = '-----END CERTIFICATE-----';

function on_load() 
{
  with ( document.forms[0] ) 
  {
    certPublic.value = certstart + certinsert + certend + '\n';
  }
}

function on_sumbit() 
{
  var idx;
 
  with ( document.forms[0] ) 
  {
    if ( certPublic.value == '' ) 
	{
      alert('֤�鹫����������Ϊ��.');
      return;
    }
  }
  document.TR069importForm.submit();
}

</script>
  </head>
  <body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad='on_load()'>
    <blockquote>
	  <DIV align="left" style="padding-left:20px; padding-top:10px">
      <form name="TR069importForm" action=/boaform/admin/formCertcaInsert method="post">
        <b>����CA֤��<br>
        </b>
        <br>
    ѡ��֤��������֤�����ݡ�<br>
        <br>
        <table border="0" cellpadding="0" cellspacing="0">
          <tr>
            <td width="180">֤�����:</td>
            		<td>
			<select size="1" name="certName"  >
          		<option value="acscert"   >acscert</option>
          		<!-- <option  value="cpecert"  >cpecert</option> -->
        		</select>
        		</td>            
          </tr>
            <tr>
              <td width="180">֤������:</td>
              <td>
                <P>
                  <TEXTAREA name='certPublic' rows='20' cols='80'>
              </TEXTAREA>
                </P>
              </td>
            </tr>
          </table>
        </div>
        <br>
        <br>
        <br>
        <center>
          <input type='button' value='Ӧ��' onClick="on_sumbit()">
          <input type="hidden" value="/net_certcaimport.asp" name="submit-url">
        </center>
      </form>
	  </div>
    </blockquote>
  </body>
<%addHttpNoCache();%>
</html>

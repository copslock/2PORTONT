<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�й�����-DSL����</TITLE>
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

var glite_sw = true;
var gdmt_sw = true;
var t1413_sw = true;
var adsl2_sw = true;
var adsl2p_sw = true;
var anxl_sw = true;
var anxm_sw = false;

//<% init_dsl_page(); %>

function on_init()
{	
/*
  set_adsl.glite.checked = glite_sw;
  set_adsl.gdmt.checked = gdmt_sw;
  set_adsl.t1413.checked = t1413_sw;
  set_adsl.adsl2.checked = adsl2_sw;
  set_adsl.adsl2p.checked = adsl2p_sw;
  set_adsl.anxl.checked = anxl_sw;    
  set_adsl.anxm.checked = anxm_sw;
 */
}

function checkForm()
{
	with (document.set_adsl)
	{
		if(!glite && !gdmt && !t1413 && !adsl2 && !adsl2p)
		{
			alert("��������ѡ��һ��ADSL����ģʽ��");
			return false;
		}
		if((!adsl2 && !adsl2p ) && anxl)
		{
			alert("ֻ����ADSL2��ADSL2+ģʽ�²�������AnnexL������ѡ�� ADSL2 ��/�� ADSL2+ ����ģʽ��");
			return false;
		}
		if((!adsl2 && !adsl2p ) && anxm)
		{
			alert("ֻ����ADSL2��ADSL2+ģʽ�²�������AnnexM������ѡ�� ADSL2 ��/�� ADSL2+ ����ģʽ��");
			return false;
		}
	}
	return true;
}
function on_submit()
{
	document.set_adsl.submit();		
}
</script>
   </head>
   <body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
      <blockquote>
	    <DIV align="left" style="padding-left:20px; padding-top:5px">
         <form action=/boaform/formSetAdsl method="post" name="set_adsl" onSubmit="return checkForm(this);">
            <b>DSL����</b>
			<br>
            <br>
				��ѡ�����еĵ���ģʽ��
			<br>
            <table border="0" cellpadding="4" cellspacing="0">
               <tr>
                  <td width='20'>��</td>
                  <td><input type="checkbox" name="gdmt" <% checkWrite("adslmode-gdmt"); %>> G.Dmt ����</td>
               </tr>
               <tr>
                  <td>��</td>
                  <td><input type="checkbox" name="glite" <% checkWrite("adslmode-glite"); %>> G.lite ����</td>
               </tr>
               <tr>
                  <td>��</td>
                  <td><input type="checkbox" name="t1413" <% checkWrite("adslmode-t1413"); %>> T1.413 ����</td>
               </tr>
               <tr>
                  <td>��</td>
                  <td><input type="checkbox" name="adsl2" <% checkWrite("adslmode-adsl2"); %>> ADSL2 ����</td>
               </tr>
               <tr>
                  <td>��</td>
                  <td><input type="checkbox" name="adsl2p" <% checkWrite("adslmode-adsl2p"); %>> ADSL2+ ����</td>
               </tr>
               <tr>
                  <td>��</td>
                  <td><input type="checkbox" name="anxl" <% checkWrite("adslmode-anxl"); %>> AnnexL ����</td>
               </tr>
               <tr>
                  <td>��</td>
                  <td><input type="checkbox" name="anxm" <% checkWrite("adslmode-anxm"); %>> AnnexM ����</td>
               </tr>
            </table>  
    		<br><br>
				<input type='submit' name="save" value='����/Ӧ��' onClick = "on_submit();">
				  <input type="hidden" value="/net_dsl.asp" name="submit-url">
	     </form>
		</DIV>
      </blockquote>
   </body>
<%addHttpNoCache();%>
</html>

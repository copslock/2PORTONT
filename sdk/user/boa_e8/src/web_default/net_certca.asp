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

//var certlist = new Array();	
//certlist.push(new it_devrec("acscert", "C/CN/ST=nanjing/O=127.24.22.1/OU=172.24.22.1", "ca"));

function it_devrec(certName, certSubject, certType)
{
	this.certName = certName;
	this.certSubject = certSubject;
	this.certType = certType;
	this.select = false;
}



/********************************************************************
**          on document load
********************************************************************/

function on_init()
{
	//do nothing
}

/********************************************************************
**          on document update
********************************************************************/

/********************************************************************
**          on document submit
********************************************************************/
  
function importClick() 
{
   var loc = 'net_certcaimport.asp';
   var code = 'location=\"' + loc + '\"';
   eval(code);
}

function viewClick(name) 
{
   var loc = 'net_showcert.asp';
   var code = 'location=\"' + loc + '\"';
   eval(code);
}
   /*
function removeClick(name, refCount) 
{
   var rem = true;
   var tmplst = "";
   if (certtable.rows.length > 0) 
   {
        if (!confirm('��֤������ʹ�ã�ɾ��������ֹͣĳЩ�������еĹ��ܡ���Ҫǿ��ɾ����')) 
        {
            rem = false;
        }
   }
   if (rem) 
   {
			with ( document.forms[0] ) 
			{
				for(var i = 0; i < certlist.length; i++)
				{
						tmplst += certlist[i].certName + "|" + certlist[i].certSubject + "|" + certlist[i].certType;
				}
				lst.value = tmplst;
				alert(tmplst);
				submit();
			}
		}
}*/


</script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:10px">
<form action=/boaform/admin/formTR069CACert method="post" enctype="multipart/form-data"  name="ca_cert">
<b>�����ε�Ȩ����֤(CA)֤��</b><br><br>
��ҳ������ӡ��鿴��ɾ��֤��
(�����֤��ǰ����ɾ����֤��)��
ʹ��CA ֤��У������ߵ���Ȩ��<br>
<br><br><center>
           <table border="0" cellpadding="0" cellspacing="0">
 				<tr>
					<td>
		           		<input type="file" name="binary" size="20">&nbsp;&nbsp;
		           		<input type="submit"  value="֤�鵼��" >
					</td>
				</tr>
		   </table><br>
		<input type="hidden" id="action" name="action" value="none">
<!--
<table id="certtable" border='1' cellpadding='4' cellspacing='0'>
   <tr>
      <td class='hd'>֤�����</td>
      <td class='hd'>�䷢����</td>
      <td class='hd'>����</td>
      <td class='hd'>����</td>
   </tr>
</table><br>
<input type='button' name="importcert" onClick='importClick()' value='����֤��'>
<input type="hidden" name="lst" value="">
<input type="hidden" value="/net_certca.asp" name="submit-url">
-->
</center>
</form>
</DIV>
<DIV align="left" style="padding-left:20px; padding-top:10px">
<form action=/boaform/admin/formTR069CACertDel method="post">
<center>
           <table border="0" cellpadding="0" cellspacing="0">
 				<tr>
					<td>
		           		<input type="submit"  value="ɾ�����ϴ�֤��" name="delbutton" <% checkWrite("check-certca"); %>>
					</td>
				</tr>
		   </table><br>
</center>
</form>
</DIV>

</blockquote>
</body>
<%addHttpNoCache();%>
</html>

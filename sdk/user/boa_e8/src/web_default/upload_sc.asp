<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>Firmware Update</TITLE>
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
function submitonce(theform)
{
	if (document.all||document.getElementById)
	{
		for (i=0;i<theform.length;i++)
		{
			var tempobj=theform.elements[i]
			if(tempobj.type.toLowerCase()=="submit"||tempobj.type.toLowerCase()=="reset")
				tempobj.disabled=true;
		}
	}
	document.getElementById("displayFile").innerHTML = document.password.binary.value;
		
	if (document.getElementById) {  // DOM3 = IE5, NS6
    	document.getElementById("fileinput").style.display = "none";
    	document.getElementById("filetext").style.display = "block";
  	} else {
     	if (document.layers == false) {// IE4
			document.all.fileinput.style.display = "none";
			document.all.filetext.style.display = "block";
		}
  	}
}

function sendClicked()
{
	if (!confirm('��������ļ��ϴ�,���ܻ�ռ�ýϳ�ʱ��,�����Ҫ�����汾��?'))
		return false;
	else{
		return true;
	}
}
</SCRIPT>

</head>
<BODY>
<blockquote>
<h2><font color="#0000FF">�����汾</font></h2>
<form action=/boaform/admin/formUpload method=POST enctype="multipart/form-data" name="password" onSubmit="javascript:submitonce(this)">
<table border="0" cellspacing="4" width="500">
 <tr><td align=left><font size=2>���� 1:  ȡ��ϵͳ�����ļ�.</td></tr>
 <tr><td align=left><font size=2>���� 2:  ָ��ϵͳ�����ļ��������λ��. �� "���" ָ��ϵͳ�����ļ�
.</td></tr>
 <tr><td><font size=2>���� 3:  �� "�������" ����ϵͳ�����ļ��Ը���ϵͳ���.</td></tr>
 <tr><td><font size=2>ע��:  ϵͳ���������ҪԼ������ʱ��, �����ڸ��¹����йػ�. ϵͳ������º�ϵͳ����
����������.</td></tr>
 <tr><td><hr size=1 noshade align=top></td></tr>
 <tr><td><font size=2>Ŀǰ����汾:<% ZTESoftwareVersion(); %></td></tr>
 <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<div  id="fileinput" style="display:none">
<table border="0" cellspacing="4" width="500">
  <tr>
      <td width="20%"><font size=2><b>ѡ���ļ�:</b>&nbsp;&nbsp;&nbsp;&nbsp;</td>
      <td width="80%"><font size=2><input type="file" name="binary" size=20></td>
  </tr>
</table>
</div>
<div  id="filetext" style="display:none">
<table border="0" cellspacing="4" width="500">
  <tr>
      <td width="15%"><font size=2><b>�ļ�:</b>&nbsp;&nbsp;&nbsp;&nbsp;</td>
      <td width="85%"><font size=2 ID="displayFile"></td>
  </tr>
 <tr>
  <td width="15%"></td>
  <td width="85%">�����ϴ�,�����ĵȴ�!</td></tr>
</table>
</div>
  <p>
  <input type="submit" value="�������" name="send" onclick="return sendClicked()">&nbsp;&nbsp;
  <input type="reset" value="���" name="reset">
  <input type="hidden" value="/admin/upload_sc.asp" name="submit-url">
  </p>

  <script>
    if (document.password.send.disabled == true) {
	  	if (document.getElementById) {  // DOM3 = IE5, NS6
	    	document.getElementById("fileinput").style.display = "none";
	    	document.getElementById("filetext").style.display = "block";
	  	} else {
	     	if (document.layers == false) {// IE4
				document.all.fileinput.style.display = "none";
				document.all.filetext.style.display = "block";
			}
	  	}
  	} else {
  		if (document.getElementById) {  // DOM3 = IE5, NS6
	    	document.getElementById("fileinput").style.display = "block";
	    	document.getElementById("filetext").style.display = "none";
	  	} else {
	     	if (document.layers == false) {// IE4
				document.all.fileinput.style.display = "block";
				document.all.filetext.style.display = "none";
			}
	  	}
  	}
  </script>
 </form>
 </blockquote>
</body>
<%addHttpNoCache();%>
</html>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>DMZ Host</TITLE>
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
<script type="text/javascript" src="share.js"></script>
<SCRIPT language="javascript" type="text/javascript">
function skip () { this.blur(); }
function saveClick()
{
//  if (!document.formDMZ.enabled.checked)
  if (document.formDMZ.dmzcap[0].checked)
 	return true;

/*  if ( validateKey( document.formDMZ.ip.value ) == 0 ) {
	alert("Invalid IP address value. It should be the decimal number (0-9).");
	document.formDMZ.ip.focus();
	return false;
  }
  if( IsLoopBackIP( document.formDMZ.ip.value)==1 ) {
	alert("Invalid IP address value.");
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,1,0,223) ) {
      	alert('Invalid IP address range in 1st digit. It should be 0-223.');
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,2,0,255) ) {
      	alert('Invalid IP address range in 2nd digit. It should be 0-255.');
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,3,0,255) ) {
      	alert('Invalid IP address range in 3rd digit. It should be 0-255.');
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,4,1,254) ) {
      	alert('Invalid IP address range in 4th digit. It should be 1-254.');
	document.formDMZ.ip.focus();
	return false;
  }*/
  if (!checkHostIP(document.formDMZ.ip, 1))
	return false;
  return true;
}

function updateState()
{
//  if (document.formDMZ.enabled.checked) {
  if (document.formDMZ.dmzcap[1].checked) {
 	enableTextField(document.formDMZ.ip);
  }
  else {
 	disableTextField(document.formDMZ.ip);
  }
}


</SCRIPT>
</head>

<body>
<blockquote>

<DIV align="left" style="padding-left:20px; padding-top:5px">
<form action=/boaform/formDMZ method=POST name="formDMZ">
<b>NAT -- DMZ����</b><br><br>
				��ͥ���ؽ����Թ�������IP��ת����DMZ������<br><br>
				ѡ��"����DMZ����"����������IP��ַ,���"����/Ӧ��"�����Լ���DMZ������<br><br>
				��ѡ��"����DMZ����"�����"����/Ӧ��"�����Խ��DMZ������<br>
<hr align="left" class="sep" size="1" width="90%">
<table border="0" cellpadding="0" cellspacing="0">
<tr><td>DMZ����:</td>
      <td>
	<input type="radio" value="0" name="dmzcap" <% checkWrite("dmz-cap0"); %> onClick="updateState()">����&nbsp;&nbsp;
	<input type="radio" value="1" name="dmzcap" <% checkWrite("dmz-cap1"); %> onClick="updateState()">����&nbsp;&nbsp;
      </td>
</tr>
<tr>
	<td>DMZ����IP��ַ: </td>
	<td><input type="text" name="ip" size="15" maxlength="15" value=<% getInfo("dmzHost"); %> ></td>
</tr>
<tr>
	<td><input type="submit" value="����/Ӧ��" name="save" onClick="return saveClick()">&nbsp;&nbsp;</td>
</tr>
<tr><td>
   <br>
        <!--input type="reset" value="Reset" name="reset"-->
        <input type="hidden" value="/fw-dmz.asp" name="submit-url">
</td></tr>
</table>
     <script> updateState(); </script>
</form>
</DIV>
</blockquote>
</body>
</html>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>ALG On-Off</TITLE>
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
function AlgTypeStatus()
{
	<%checkWrite("AlgTypeStatus");%>
	return true;
}
</SCRIPT>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">

<table  width=500>
<tr><td colspan=4><hr size=1 noshade align=top></td></tr>
<form action=/boaform/formALGOnOff method=POST name=algof>
<div class="tip" style="width:90% ">
	<b>ALG����</b><br><br>	
</div>
<table>
<tr>
<td>ѡ�������ALG:</td>
<td colspan="2">	
</td>
</tr>
<%checkWrite("GetAlgType")%>	
<tr>
	<td ><input type=submit value="����/Ӧ��" name=apply></td>
  <td> <input type="hidden" value="/algonoff.asp" name="submit-url"></td>
  <td></td>
</tr>
</table>
</form>
<script>
AlgTypeStatus();
</script>
</table>
</DIV>
</blockquote>
</body>
</html>

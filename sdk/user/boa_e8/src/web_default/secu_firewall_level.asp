<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>��������������</TITLE>
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

/********************************************************************
**          on document load
********************************************************************/
var cgi = new Object();
<%initPageFirewall();%>

function on_init()
{
	sji_docinit(document, cgi);
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form action=/boaform/admin/formFirewall method=POST name="form">
				<b>ѡ��<font color="red">����ǽ�ȼ�</font>��������Ӧ������.</b><br><br>
				<hr align="left" class="sep" size="1" width="90%">
				<table width="538px" border="0" cellspacing="1" cellpadding="3">
					<tr>
						<td width="162" height="27">����ǽ�ȼ�:</td>
						<td width="73">
							<select name="filterLevel">
								<option value="1">��</option>
								<!--<option value="2">��</option>-->
								<option value="3">��</option>
							</select>
						</td>
					</tr>
				</table>
				<hr align="left" class="sep" size="1" width="90%">
				<input type="submit" class="button" value="ȷ ��" name="apply">
				<input type="hidden" name="submit-url" value="">
			</form>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

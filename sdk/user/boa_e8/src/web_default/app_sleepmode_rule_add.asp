<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>��ʱ��������</TITLE>
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

function btnApply()
{
	document.form.action.value="add";
	form.submit();
}

function on_init()
{
	var i=0;
	for(i=0; i<24; i++)
		document.form.hour.options.add(new Option(i, i));

	for(i=0; i<60; i++)
		document.form.minute.options.add(new Option(i, i));
}

function timeDisplay()
{
	var selc = document.getElementById("day");
	var index = selc.selectedIndex;

	if( selc.options[index].value==0 )
	{
		document.getElementById("hour").disabled=true;
		document.getElementById("minute").disabled=true;
	}
	else
	{
		document.getElementById("hour").disabled=false;
		document.getElementById("minute").disabled=false;
	}
}
</SCRIPT>
</HEAD>


<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init()">
  <blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px">
		<form id="form" action=/boaform/admin/formSleepMode method=POST name="form">
			<b>��Ӽ�ͥ���ض�ʱ���߹���-- ������������ 100������.</b><br><br><br>
			<hr align="left" class="sep" size="1" width="90%">
				<tr>
					<td width="120">��������ʱ��:&nbsp;</td>
					<td>
						<select id="day" name="day" onchange="timeDisplay()">
  							<option value=0>��������/����</option>
  							<option value=1>������</option>
 							<option value=2>����һ</option>
							<option value=3>���ڶ�</option>
							<option value=4>������</option>
							<option value=5>������</option>
							<option value=6>������</option>
							<option value=7>������</option>
						</select>
					</td>
					<td>&nbsp;</td>
					<td>
						<select id="hour" name="hour">
						</select>
					</td>
					<td>ʱ&nbsp;</td>
					<td>
						<select id="minute" name="minute">
						</select>
					</td>
					<td>��&nbsp;</td>
				</tr>
				<br><br>
				<tr>
					<td width="120">ʹ��/��ֹ:&nbsp;</td>
					<td><input type="radio" name="timerEnable" value="off" checked>&nbsp;&nbsp;��ֹ</td>
					<td><input type="radio" name="timerEnable" value="on" >&nbsp;&nbsp;ʹ��</td>
				</tr>
				<br><br>
				<tr>
					<td width="120">����:&nbsp;</td>
					<td><input type="radio" name="onoffEnable" value="off" checked>&nbsp;&nbsp;����</td>
					<td><input type="radio" name="onoffEnable" value="on" >&nbsp;&nbsp;����</td>
				</tr>
			<br><br>
			<hr align="left" class="sep" size="1" width="90%">
			<input type="button" class="button" value="����/Ӧ��" onClick="btnApply()">
			<input type="hidden" name="action" value="add">
			<input type="hidden" name="submit-url" value="/app_sleepmode_rule.asp">

			<script>
				timeDisplay();
			</script>
		</form>
	</div>
  </blockquote>
</body>
<%addHttpNoCache();%></html>

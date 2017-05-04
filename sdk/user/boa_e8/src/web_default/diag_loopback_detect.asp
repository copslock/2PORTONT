<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gbk">
<title>�й����š���·���</title>
<script type="text/javascript" src="share.js">
</script>
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<% language=javascript %>
<SCRIPT>

<% initLBDPage(); %>

function on_init()
{
	if(lbd_enable)
		document.lbd.enable.checked = true;

	document.lbd.exist_period.value = lbd_exist_period;
	document.lbd.cancel_period.value = lbd_cancel_period;
	document.lbd.ether_type.value = lbd_ether_type.toString(16).toUpperCase();
	document.lbd.vlans.value = lbd_vlans;

	var table = document.getElementById("port_status");

	for(var i = 0 ; i < lbd_port_status.length ; i++)
	{
		var cell;
		var row = table.insertRow(-1);

		cell = row.insertCell(0);
		cell.innerHTML = "LAN" + (i+1);

		cell = row.insertCell(1);
		switch(lbd_port_status[i])
		{
		case 0:
			cell.innerHTML = "�޻�·";
			break;
		case 1:
			cell.innerHTML = "��⵽��·���ѹرն˿�";
			break;
		case 2:
			cell.innerHTML = "��⵽��·��δ�ܹرն˿�";
			break;
		}
	}

	update_gui();
}

function disable_by_class(str_class, disable)
{
	var elements = document.getElementsByClassName(str_class);

	for (var i = 0 ; i < elements.length ; i++)
		elements[i].disabled = disable;
}

function update_gui()
{
	with(document.lbd)
	{
		if(enable.checked == true)
			disable_by_class("lbd", false);
		else
			disable_by_class("lbd", true);
	}
}

function on_submit()
{
	with(document.lbd)
	{
		if(enable.checked == false)
			return true;

		if(!sji_checkdigitrange(exist_period.value, 1, 60))
		{
			alert("��ⱨ�ļ��ֻ����1~60�룡");
			exist_period.focus();
			return false;
		}

		if(!sji_checkdigitrange(cancel_period.value, 10, 1800))
		{
			alert("�ָ����ļ��ֻ����10~1800�룡");
			cancel_period.focus();
			return false;
		}

		if(!sji_checkhex(ether_type.value, 1, 4))
		{
			alert("��̫�����ͱ���Ϊʮ������ֵ��");
			ether_type.focus();
			return false;
		}

		if(vlans.value.length <= 0)
		{
			alert("����ָ��VLANֵ��");
			vlans.focus();
			return false;
		}
	}
	return true;
}
</SCRIPT>
</head>

<body onLoad="on_init();" topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">

<form action=/boaform/formLBD method=POST name="lbd">
<div class="tip" style="width:90% ">
	<b>��� -- ��·���</b><br><br>	
</div>
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><hr size=1 noshade align=top></tr>
  <tr>
      <td width="30%">��·���ʹ��:</td>
      <td width="70%"><input type="checkbox" name="enable" value="1" onClick="update_gui();"></td>
  </tr>
  <tr>
      <td width="30%">��ⱨ�ļ��:</td>
      <td width="70%"><input type="text" class="lbd" name="exist_period" size="15" maxlength="5"> (1~60)��</td>
  </tr>
  <tr>
      <td width="30%">�ָ����ļ��:</td>
      <td width="70%"><input type="text" class="lbd" name="cancel_period" size="15" maxlength="15"> (10 ~ 1800)��</td>
  </tr>
  <tr>
      <td width="30%">��̫������:</td>
      <td width="70%">0x<input type="text" class="lbd" name="ether_type" size="13" maxlength="4"></td>
  </tr>
  <tr>
      <td width="30%">VLANֵ:</td>
      <td width="70%"><input type="text" class="lbd" name="vlans" size="30" maxlength="300"></td>
  </tr>
  <tr><td></td><td width="70%">��","������0���� untagged(�磺0,45,46)</td></tr>
</table>
  <input type="submit" value="����/Ӧ��" onClick="return on_submit()">
  </tr>
  <tr><hr size=1 noshade align=top></tr>
<br>
<b>�˿ڻ�·���״̬:</b>
<table id="port_status" class="flat" border="1" cellpadding="2" cellspacing="0">
	<tbody align="center">
		<tr align="center" class="hd">
			<td width=50>�˿�</td>
			<td width=200>״̬</td>
		</tr>
	</tbody>
</table>
  <br>
      <input type="hidden" value="/diag_loopback_detect.asp" name="submit-url">
</form>
</DIV>
</blockquote>
</body>

</html>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�������������</TITLE>
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

var protos = new Array( "TCP/UDP", "TCP", "UDP");
var rcs = new Array();
with(rcs){<% virtualSvrList(); %>}

function on_chkclick(index)
{
	if(index < 0 || index >= rcs.length)
		return; 
	rcs[index].select = !rcs[index].select;
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document);
	
	if(lstrc.rows){while(lstrc.rows.length > 1) lstrc.deleteRow(1);}
	for(var i = 0; i < rcs.length; i++)
	{
		var row = lstrc.insertRow(i + 1);
		
		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";
		var cell = row.insertCell(0);
		cell.innerHTML = rcs[i].svrName;
		cell = row.insertCell(1);
		cell.innerHTML = rcs[i].wanStartPort;
		cell = row.insertCell(2);
		cell.innerHTML = rcs[i].wanEndPort;
		cell = row.insertCell(3);
		cell.innerHTML = protos[rcs[i].protoType];
		cell = row.insertCell(4);
		cell.innerHTML = rcs[i].serverIp;
		cell = row.insertCell(5);
		cell.innerHTML = rcs[i].lanPort;
		cell = row.insertCell(6);
		cell.align = "center";
		cell.innerHTML = "<input type=\"checkbox\" name=\"rml"
				+ i + "\" value=\"ON\" onClick=\"on_chkclick(" + i + ");\">";
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	with ( document.forms[0] ) 
	{
		form.bcdata.value = sji_encode(rcs, "select");
		submit();
	}
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formVrtsrv" method="post">
				<b>NAT --�������������</b>
				<div class="tip" style="width:90% ">
					<br><br>�����������ʹ��lan���һ̨�豸��Ϊ��������Զ�̵��豸��wan������ֱ�ӷ��ʸ÷����������ֻ������32������.<br><br>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<br>				
				<table class="flat" id="lstrc" border="1" cellpadding="0" cellspacing="1">
				   <tr class="hdb" align="center" nowrap>
					  <td>��������</td>
					  <td width="90px">�ⲿ��ʼ�˿�</td>
					  <td width="90px">�ⲿ��ֹ�˿�</td>
					  <td width="60px">Э��</td>
					  <td width="120px">������IP��ַ</td>
					  <td width="90px">Դ�˿�</td>
					  <td width="40px">ɾ��</td>
				   </tr>
				</table>
				<br>
				<input type="button" class="button" onClick="location.href='app_nat_vrtsvr_add.asp';" value="���">
				<input type="button" class="button" onClick="on_submit();" value="ɾ��">
				<input type="hidden" id="action" name="action" value="delete">
				<input type="hidden" name="bcdata" value="le">
				<input type="hidden" name="submit-url" value="">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

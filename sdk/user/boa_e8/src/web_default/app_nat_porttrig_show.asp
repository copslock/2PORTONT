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

var protos = new Array("TCP/UDP", "TCP", "UDP");
var rcs = new Array();
with(rcs){<% PortTriggerList(); %>}

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
	
	if(lstrc.rows){while(lstrc.rows.length > 3) lstrc.deleteRow(3);}
	for(var i = 0; i < rcs.length; i++)
	{
		var row = lstrc.insertRow(i + 3);
		
		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";
		var cell = row.insertCell(0);
		cell.innerHTML = rcs[i].appName;
		cell = row.insertCell(1);
		cell.innerHTML = protos[rcs[i].trigProto];
		cell = row.insertCell(2);
		cell.innerHTML = rcs[i].trigStartPort;
		cell = row.insertCell(3);
		cell.innerHTML = rcs[i].trigEndPort;
		cell = row.insertCell(4);
		cell.innerHTML = protos[rcs[i].openProto];
		cell = row.insertCell(5);
		cell.innerHTML = rcs[i].openStartPort;
		cell = row.insertCell(6);
		cell.innerHTML = rcs[i].openEndPort;
		cell = row.insertCell(7);
		cell.align = "center";
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\">";
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
			<form id="form" action="/boaform/admin/formPortTrigger" method="post">
				<b>NAT -- �˿ڴ�������</b>
				<br><br>
				<div class="tip" style="width:90% ">
					&nbsp;&nbsp;&nbsp;&nbsp;һЩӦ��Ҫ��·��������ǽ��ָ���˿ڿ���,�Թ�Զ�̷���.
					����������һ��Ӧ��ͨ��ʹ��"<b>�����˿�</b>",��TCP/UDP��ʼ���ӵ�һ��Զ���û�ʱ,�˿ڴ�����̬�Ĵ򿪷���ǽ��"<b>���Ŷ˿�</b>".
					·�����������Թ�������Զ���û�����һ���µ�����,ͨ��ʹ��"<b>���Ŷ˿�</b>"���ؾ�������Ӧ�ó���.
				</div>
				<br>
				<b>ע������������32������.</b><br>
				<hr align="left" class="sep" size="1" width="90%">
				<br>				
				<table class="flat" id="lstrc" border="1" cellpadding="0" cellspacing="1">
				   <tr class="hdb" align="center" nowrap>
						<td>Ӧ�ó���</td>
						<td colspan=3>����</td>
						<td colspan=3>��</td>
						<td>ɾ��</td>
					</tr>
				   <tr class="hdb" align="center" nowrap>
						<td>����</td>
						<td>Э��</td>
						<td colspan=2>�˿ڷ�Χ</td>
						<td>Э��</td>
						<td colspan=2>�˿ڷ�Χ</td>
						<td>&nbsp;</td>
					</tr>
				   <tr class="hdb" align="center" nowrap>
						<td>&nbsp;</td>
						<td width="60px">&nbsp;</td>
						<td width="60px">��ʼ</td>
						<td width="60px">��ֹ</td>
						<td width="60px">&nbsp;</td>
						<td width="60px">��ʼ</td>
						<td width="60px">��ֹ</td>
						<td>&nbsp;</td>
					</tr>
				</table>
				<br>
				<input type="button" class="button" onClick="location.href='app_nat_porttrig_add.asp';" value="���">
				<input type="button" class="button" onClick="on_submit();" value="ɾ��">
				<input type="hidden" id="action" name="action" value="rm">
				<input type="hidden" name="bcdata" value="le">
				<input type="hidden" name="submit-url" value="">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

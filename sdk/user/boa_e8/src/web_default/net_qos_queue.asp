<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<html>
<head>
<title>QoS������ʾ</title>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--ϵͳ����css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<!--ϵͳ�����ű�-->
<script language="javascript" src="common.js"></script>
<script language="javascript" type="text/javascript">
var wirelessqueue = new Array();
var wanqueue = new Array();
<% initQueuePage(); %>

function on_chkstate(index){
	if(index < 0 || index >= wanqueue.length)
		return; 
	wanqueue[index].queuestatus = !wanqueue[index].queuestatus;
}

function on_chkselect(index) {
	if(index < 0 || index >= wanqueue.length)
		return; 
	wanqueue[index].select = !wanqueue[index].select;
}

function on_init_page() {
	if(queuelist.rows){while(queuelist.rows.length > 1) lstrc.deleteRow(1);}
	//pvc QOS ��ʾ
	for(var i=0; i<wanqueue.length; i++)
	{
		var queuestatus = " ";
		var row = queuelist.insertRow(i+1);
		row.nowrap = true;
		row.vAlign = "center";
		row.align = "left";
		var cell = row.insertCell(0);
		cell.innerHTML = wanqueue[i].inf;
		cell = row.insertCell(1);
		cell.innerHTML = wanqueue[i].desc;
		cell = row.insertCell(2);
		cell.innerHTML = wanqueue[i].prio;
		cell = row.insertCell(3);
		cell.innerHTML = wanqueue[i].queuekey;
		cell = row.insertCell(4);
		if(wanqueue[i].queuestatus) {
			queuestatus = "checked";
		}
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkstate(" + i + ");\" "+ queuestatus + ">";
		cell = row.insertCell(5);
		cell.align = "center";
		cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkselect(" + i + ");\">";
	}
//����QOS��ʾ
	for(var i=0; i<wirelessqueue.length; i++)
	{
		var row = queuelist.insertRow(i+1);
		row.nowrap = true;
		row.vAlign = "center";
		row.align = "left";
		var cell = row.insertCell(0);
		cell.innerHTML = wirelessqueue[i].inf;
		cell = row.insertCell(1);
		cell.innerHTML = wirelessqueue[i].desc;
		cell = row.insertCell(2);
		cell.innerHTML = wirelessqueue[i].prio;
		cell = row.insertCell(3);
		cell.innerHTML = wirelessqueue[i].queuekey;
		cell = row.insertCell(4);
		cell.innerHTML = "&nbsp;";
		cell = row.insertCell(5);
		cell.innerHTML = "&nbsp;";
	}
}


function on_delete_queue() {
	with(document.forms[0]) {
		var sbmtstr = "queuedel&queuekey=";
		var firstFound = true;
		for(var i=0; i<wanqueue.length; i++)
		{
			if(wanqueue[i].select)
			{
				if(!firstFound)
					sbmtstr += "|";
				else
					firstFound = false;
				sbmtstr += wanqueue[i].queuekey;
			}
		}
		lst.value = sbmtstr;
		submit();
	}
	
}

function on_save_reboot() {
	with(document.forms[0]) {
		var sbmtstr = "queuesave&value=";
		var first = true;
		for(var i=0;i<wanqueue.length; i++)
		{
			if(!first)
				sbmtstr += "|";
			else
				first = false;
			sbmtstr = sbmtstr+wanqueue[i].queuekey+"|"+Number(wanqueue[i].queuestatus);
		}
		lst.value = sbmtstr;
		submit();
	}
}
</script>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init_page();">
<blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px;">
		<form id="queueform" action="/boaform/admin/formQosQueue" method="post">
		  <p style="color: #FF0000">��ʾ����ҳ������ã���Ҫ����·����������Ч��</p>
		  <p style="color: #000000;font-weight: bold;">QoS�������� -- �������24 ��Ŀ������.<BR>
	          ����������߹���, �������йصĶ��н�����Ч  </p>
		  <table class="flat" id="queuelist" border="1" cellpadding="1" cellspacing="0">
		  <tr bgcolor="#CCCCCC"><td>�ӿ���</td><td>����</td><td>���ȼ�</td><td>������Կ</td><td>����</td><td>ɾ��</td></tr>
		  </table>
		  <input type="button" class="button" value="���" onClick="location.href='net_qos_queue_edit.asp'">
		  <input type="button" class="button" value="ɾ��" onClick="on_delete_queue();">
		  <input type="button" class="button" value="����/Ӧ��" onClick="on_save_reboot();">
		  <input type="hidden" id="lst" name="lst" value="">
		  <input type="hidden" name="submit-url" value="/net_qos_queue.asp">
	  </form>
	</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>

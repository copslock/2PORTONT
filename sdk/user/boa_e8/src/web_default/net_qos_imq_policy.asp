<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<html>
<head>
<title>QoS ���</title>
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
var policy = 1;
var rules = new Array();
var queues = new Array();
<% initQueuePolicy(); %>

function queue_display() {
	var hrow=lstrc.rows[0];
	var hcell=hrow.cells[1];
	
	if (document.forms[0].queuepolicy[0].checked)
		hcell.innerHTML = "���ȼ�";
	else
		hcell.innerHTML = "Ȩ��";
	if(lstrc.rows){while(lstrc.rows.length > 1) lstrc.deleteRow(1);}
	for(var i = 0; i < queues.length; i++) {
		var row = lstrc.insertRow(i + 1);
		row.nowrap = true;
		row.vAlign = "center";
		row.align = "center";

		var cell = row.insertCell(0);
		cell.innerHTML = queues[i].qname;
		cell = row.insertCell(1);
		if (document.forms[0].queuepolicy[0].checked)
			cell.innerHTML = queues[i].prio;
		else
			cell.innerHTML = "<input type=\"text\" name=w" + i + " value=" + queues[i].weight + " size=3>";
		cell = row.insertCell(2);
		qcheck= queues[i].enable? " checked":"";
		cell.innerHTML = "<input type=\"checkbox\" name=qen" + i + qcheck + ">";
	}
}

function on_init(){
	with(document.forms[0]){
		if(policy != 0 && policy !=1)
			policy = 0;
		queuepolicy[policy].checked = true;
		qosen[qosEnable].checked = true;
		qosPly.style.display = qosEnable==0 ? "none":"block";
	}
	queue_display();
}

function on_save() {
	with(document.forms[0]) {
		var sbmtstr = "";
		if(queuepolicy[0].checked==true)
			sbmtstr = "policy=0";
		else{ //WRR����
			sbmtstr = "policy=1";
			var weight=0;
			if(qen0.checked)
				weight+=parseInt(w0.value);
			if(qen1.checked)
				weight+=parseInt(w1.value);
			if(qen2.checked)
				weight+=parseInt(w2.value);				
			if(qen3.checked)
				weight+=parseInt(w3.value);			
			if(weight!=100){
				alert("WRR����Ȩ��֮�ܺϱ���Ϊ100");
				return false;
			}
		}	
		lst.value = sbmtstr;
		submit();
	}	
}

function qosen_click() {
	document.all.qosPly.style.display = document.all.qosen[0].checked ? "none":"block";
}

function qpolicy_click() {
	queue_display();
}

</script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
<blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px;">
		<form id="form" action="/boaform/admin/formQosPolicy" method="post">
		  <p><strong>QoS����</strong></p>
		  <table>
		  	<tr>
		  		<td>����QoS:</td>
				<td><input type="radio"  name=qosen value=0 onClick=qosen_click();>����</td>	
				<td><input type="radio"  name=qosen value=1 onClick=qosen_click();>����</td>	
			</tr>
		  </table>
		  <div  id="qosPly"  style="display:none">
		  	<tr>
				<td height="27">����ģ��:</td>
				<td width="200">
					<% checkWrite("qos_mode"); %>
				</td>
			</tr>
<br>
  <tr><td width="150px">�����ܴ�������:</td>  
      <td>
   	<input type="radio" name="enable_force_weight" value=0 <% checkWrite("enable_force_weight0"); %>>������&nbsp;&nbsp;
   	<input type="radio" name="enable_force_weight" value=1 <% checkWrite("enable_force_weight1"); %>>����&nbsp;&nbsp;
    </td>
  </tr> 
<br>
  <tr><td width="150px">DSCP/TC�ر�����ÿ���</td>  
      <td>
   	<input type="radio" name="enable_dscp_remark" value=0 <% checkWrite("enable_dscp_remark0"); %>>������&nbsp;&nbsp;
   	<input type="radio" name="enable_dscp_remark" value=1 <% checkWrite("enable_dscp_remark1"); %>>����&nbsp;&nbsp;
    </td>
  </tr>
 <br>

  <tr><td width="150px">802.1p�ر��ģʽ</td>  
      <td>
   	<input type="radio" name="enable_1p_remark" value=0 <% checkWrite("enable_1p_remark0"); %>>������&nbsp;&nbsp;
   	<input type="radio" name="enable_1p_remark" value=1 <% checkWrite("enable_1p_remark1"); %>>͸��&nbsp;&nbsp;
   	<input type="radio" name="enable_1p_remark" value=2 <% checkWrite("enable_1p_remark2"); %>>��д&nbsp;&nbsp;
    </td>
  </tr>

		  <p><strong>QoS��������</strong></p>
		  <p>��ҳ����������QoS���ԡ�����ѡ�����ʶ��ǰӦ�ò��ԡ�
		  ÿ������ӿ�����QoS����4�����У�ÿ�����ж�Ӧһ�����ȼ���������ֵԽС�����ȼ�Խ�ߡ�
		  ���ѡ��Ĳ���ΪWRR����������4�����ȼ����е�Ȩ�أ�Ĭ��Ϊ40:30:20:10��
		  ���������Ӧ�á�   ���沢�������á�     </p>
		  <table>
		  	<tr>
		  		<td>����ѡ��:</td>
				<td><input type="radio"  name="queuepolicy" value="prio" onClick=qpolicy_click();>�������ȼ�����</td>	
				<td><input type="radio"  name="queuepolicy" value="wrr" onClick=qpolicy_click();>WRR����</td>	
			</tr>
		  </table>
		  <table class="flat" id="lstrc" border="1" cellpadding="0" cellspacing="1" width=30%>
			<tr class="hdb" align="center" nowrap bgcolor="#CCCCCC">
				<td>����</td>
				<td>���ȼ�</td>
				<td>����</td>
			</tr>
			<!--
			<tr class="hdb" align="center" nowrap bgcolor="#CCCCCC">
				<td>����</td>
				<td>Ȩ��</td>
				<td>����</td>
			</tr>
			-->
		  </table>
		  </div>
		  <br><br>
		  <input type="button" class="button" value="����Ӧ��" onClick="on_save();">
		  <input type="hidden" id="lst" name="lst" value="">
		  <input type="hidden" name="submit-url" value="/net_qos_imq_policy.asp">
		</form>
	</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>

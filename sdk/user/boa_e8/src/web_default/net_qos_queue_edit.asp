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
var waniflst = new it_nr("waniflist");
<% initQueuePolicy(); %>
<% ifWanList(); %>
function on_init(){
	with(document.forms[0]){
		if(policy != 0 && policy !=1)
			policy = 0;
		queuepolicy[policy].checked = true;
		
		for(var i in waniflst){
			
			if(i == "name"||i=="undefined" ||(typeof waniflst[i] != "string" && typeof waniflst[i] != "number")) continue;
			queueinflist.options.add(new Option(waniflst[i],i));
		}
	}
}
function on_save() {
	with(document.forms[0]) {
		var sbmtstr = "";
		if(queuepolicy[0].checked==true)
			sbmtstr = "policy=0";
		else
			sbmtstr = "policy=1";
		if(queuestatus.value == -1) {
			alert("��ѡ���������״ֵ̬��");
			return;
		} else {
			sbmtstr = sbmtstr+"&queuestatus="+queuestatus.value;
		}
		
		if(queueinflist.value==-1) {
			alert("��ѡ�����ֵ��");
			return;
		} else {
			sbmtstr = sbmtstr+"&queueinf="+queueinflist.value;
		}
		
		if(queueprio.value == 0) {
			alert("��ѡ��������ȼ���");
			return;
		} else {
			sbmtstr = sbmtstr+"&queueprio="+queueprio.value;
		}
		lst.value = sbmtstr;
		submit();
	}	
}
</script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
<blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px;">
		<form id="form" action="/boaform/admin/formQosQueueEdit" method="post">
		  <p><strong>QoS ��������</strong></p>
		  <p>��ҳ����Ҫ���þ���ÿ������ӿڵ�QoS������Ŀ��ÿ������ӿ�����QoS����������4�����С�ÿ�����п���ָ��һ�����ȼ���QoS���͹�����������õĶ�����Ŀ��ȷ�����������ݰ�����Ӧ���С�ע�⣺��ֵԽС�����ȼ�Խ�ߡ����ѡ��Ĳ���ΪWRR�������ȼ�Ϊ1��2��3��4�����ĸ����е�Ȩ�ر���4:3:2:1�����磬��ĳһ����ӿ�ֻ���������ȼ�Ϊ1��4���������У������ǵ�Ȩ�ر�Ϊ4:1������ѡ�����ʶ��ǰӦ�ò��ԡ����������/Ӧ�á�   ���沢�������á�     </p>
		  <table class="flat" id="queueadd_tbl" border="0" cellpadding="0" cellspacing="1">
		  	<tr>
		  		<td>����ѡ��:</td>
				<td><input type="radio"  name="queuepolicy" value="prio">�������ȼ�����</td>	
				<td><input type="radio"  name="queuepolicy" value="wrr">WRR����</td>	
			</tr>
			<tr>
				<td>��������״̬��</td>
				<td><select name="queuestatus" style="width:200px">
						<option value="-1">&nbsp;</option>
						<option value="0">����</option>
						<option value="1">����</option>
					</select>
				</td>
			</tr>
			<tr>
				<td>���У�</td>
				<td><select name="queueinflist" style="width:200px"></select>
				</td>
			</tr>
			<tr>
				<td>�������ȼ���</td>
				<td><select name="queueprio" style="width:200px">
						<option value="0">&nbsp;</option>
						<option value="1">1</option>
						<option value="2">2</option>
						<option value="3">3</option>
						<option value="4">4</option>
					</select>
				</td>
			</tr>
		  </table>
		  <input type="button" class="button" value="����" onClick="on_save();">
		  <input type="hidden" id="lst" name="lst" value="">
		  <input type="hidden" name="submit-url" value="/net_qos_queue.asp">
		</form>
	</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>

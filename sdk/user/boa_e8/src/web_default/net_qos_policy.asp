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
<% initQueuePolicy(); %>

/*
function on_update(obj)
{
	with(document.forms[0]){
		if (obj.value != 255)
		{
			prece1.disabled = prece2.disabled = prece3.disabled = prece4.disabled = false;
			
			if (rules[obj.selectedIndex].prece1 == "0")
				prece1.value = "";
			else
				prece1.value = rules[obj.selectedIndex].prece1;

			prece1unit[rules[obj.selectedIndex].unit1].checked = true;
			
			if (rules[obj.selectedIndex].prece2 == "0")
				prece2.value = "";
			else
				prece2.value = rules[obj.selectedIndex].prece2;

			prece2unit[rules[obj.selectedIndex].unit2].checked = true;
			
			if (rules[obj.selectedIndex].prece3 == "0")
				prece3.value = "";
			else
				prece3.value = rules[obj.selectedIndex].prece3;

			prece3unit[rules[obj.selectedIndex].unit3].checked = true;
			
			if (rules[obj.selectedIndex].prece4 == "0")
				prece4.value = "";
			else
				prece4.value = rules[obj.selectedIndex].prece4;

			prece4unit[rules[obj.selectedIndex].unit4].checked = true;
		}
		else
		{
			prece1.value = prece2.value = prece3.value = prece4.value = "";
			prece1.disabled = prece2.disabled = prece3.disabled = prece4.disabled = true;
		}
	}
}
*/
function on_init(){
	with(document.forms[0]){
		if(policy != 0 && policy !=1)
			policy = 0;
		queuepolicy[policy].checked = true;
		/*
		for(var i = 0; i < rules.length; i++)
		{
			var q = rules[i];
			queuelist.options.add(new Option(q.name, q.ifIndex));
		}
		queuelist.selectedIndex = 0;
		on_update(queuelist);
		*/
	}
}
function on_save() {
	with(document.forms[0]) {
		/*if (queuelist.selectedIndex > 0) {
			if(prece1.value <0 || prece1.value > 65536)
			{
				prece1.focus();
				alert("����1����"+ prece1.value + "��Ч��");
				return;
			}
			if(prece2.value <0 || prece2.value > 65536)
			{
				prece2.focus();
				alert("����2����"+ prece2.value + "��Ч��");
				return;
			}
			if(prece3.value <0 || prece3.value > 65536)
			{
				prece3.focus();
				alert("����3����"+ prece3.value + "��Ч��");
				return;
			}
			if(prece4.value <0 || prece4.value > 65536)
			{
				prece4.focus();
				alert("����4����"+ prece4.value + "��Ч��");
				return;
			}
		}
		*/
		var sbmtstr = "";
		if(queuepolicy[0].checked==true)
			sbmtstr = "policy=0";
		else
			sbmtstr = "policy=1";

		/*sbmtstr = sbmtstr+"&ifIndex="+queuelist.value;
		sbmtstr = sbmtstr+"&prece1="+prece1.value;
		if (prece1unit[0].checked == true)
			sbmtstr = sbmtstr+"&unit1=0";
		else
			sbmtstr = sbmtstr+"&unit1=1";
		sbmtstr = sbmtstr+"&prece2="+prece2.value;
		if (prece2unit[0].checked == true)
			sbmtstr = sbmtstr+"&unit2=0";
		else
			sbmtstr = sbmtstr+"&unit2=1";
		sbmtstr = sbmtstr+"&prece3="+prece3.value;
		if (prece3unit[0].checked == true)
			sbmtstr = sbmtstr+"&unit3=0";
		else
			sbmtstr = sbmtstr+"&unit3=1";
		sbmtstr = sbmtstr+"&prece4="+prece4.value;
		if (prece4unit[0].checked == true)
			sbmtstr = sbmtstr+"&unit4=0";
		else
			sbmtstr = sbmtstr+"&unit4=1";
		*/
		lst.value = sbmtstr;
		submit();
	}	
}
</script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
<blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px;">
		<form id="form" action="/boaform/admin/formQosPolicy" method="post">
		  <p><strong>QoS��������</strong></p>
		  <p>��ҳ����������QoS���ԺͶ��г��ȡ�����ѡ�����ʶ��ǰӦ�ò��ԡ�
		  ÿ������ӿ�����QoS����4�����У�ÿ�����ж�Ӧһ�����ȼ���������ֵԽС�����ȼ�Խ�ߡ�
		  ���ѡ��Ĳ���ΪWRR�������ȼ�Ϊ1��2��3��4�����ĸ����е�Ȩ�ر���4:3:2:1��
		  ���������Ӧ�á�   ���沢�������á�     </p>
		  <table>
		  	<tr>
		  		<td>����ѡ��:</td>
				<td><input type="radio"  name="queuepolicy" value="prio">�������ȼ�����</td>	
				<td><input type="radio"  name="queuepolicy" value="wrr">WRR����</td>	
			</tr>
		  </table>
		  <!--
		  <br>
		  <table>
			<tr>
				<td>��������: </td>
				<td>
					<select id="queuelist" name="queuelist" style="width:150px" onChange="on_update(this)"></select>
				</td>
			</tr>
		  </table>
		  <table class="flat" width=220 border="1" cellpadding="1" cellspacing="1">
		  	<tr align="center">
		  		<td class="hdb">�������ȼ�</td>
		  		<td>���г���</td>
		  	</tr>
		  	<tr align="center">
		  		<td class="hdb">Prece 1</td>
		  		<td>
		  			<input type="text" name="prece1" maxlength="10" size="4">
		  			<input type="radio"  name="prece1unit" value="0" checked>��
		  		    <input type="radio"  name="prece1unit" value="1">�ֽ�
		  		</td>
		  	</tr>
		  	<tr align="center">
		  		<td class="hdb">Prece 2</td>
		  		<td>
		  			<input type="text" name="prece2" maxlength="10" size="4">
		  			<input type="radio"  name="prece2unit" value="0" checked>��
		  		    <input type="radio"  name="prece2unit" value="1">�ֽ�
		  		</td>
		  	</tr>
		  	<tr align="center">
		  		<td class="hdb">Prece 3</td>
		  		<td>
		  			<input type="text" name="prece3" maxlength="10" size="4">
		  			<input type="radio"  name="prece3unit" value="0" checked>��
		  		    <input type="radio"  name="prece3unit" value="1">�ֽ�
		  		</td>
		  	</tr>
		  	<tr align="center">
		  		<td class="hdb">Prece 4</td>
		  		<td>
		  			<input type="text" name="prece4" maxlength="10" size="4">
		  			<input type="radio"  name="prece4unit" value="0" checked>��
		  		    <input type="radio"  name="prece4unit" value="1">�ֽ�
		  		</td>
		  	</tr>
		  </table>
		  -->
		  <br><br>
		  <input type="button" class="button" value="����Ӧ��" onClick="on_save();">
		  <input type="hidden" id="lst" name="lst" value="">
		  <input type="hidden" name="submit-url" value="/net_qos_policy.asp">
		</form>
	</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>

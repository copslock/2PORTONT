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
var cgi = new Object();
var rules = new Array();
with(rules){<% initPageBandwidthControl(); %>}

/********************************************************************
**          on document apply
********************************************************************/
function getMsgFormArray(str,arg){
	var errid=0;
	var min =0;
	var max=0;
	var param=-1;
	var msg="";
	var num=arg.length;
	if(num==1){
		errid=arg[0];
	}else if(num==3){
		errid=arg[0];
		min =arg[1];
		max=arg[2];
	}else if(num==2){
		errid=arg[0];
		param=arg[1];
	}else return null;
	for(var i=0;i<str.length;i++){
		if(typeof(str[i])=="undefined"){
			alert("�����ʼ������������룡i = "+i);
			return null;
		}if(errid==str[i][0]){
			if(min ==max&&min ==0){
				if(param==-1){
					msg=str[i][1];
				}else {
					msg=str[i][1]+param+str[i+1][1];
				}
			}else 
				msg=str[i][1]+min +"~"+max+str[i+1][1];
			return msg;
		}
	}
	return null;
}

function getmsg(id)
{
	var str=new Array();
	str[0]=new Array(101,"������Ϊ�գ����������롣");
	str[1]=new Array(102,"����������������");
	str[2]=new Array(103,"����������ֵΪ1��1024*1024֮�������");
	return getMsgFormArray(str,arguments)
}

function checkNull(value)
{
	if(value==""||value==null)
		return false;
	else 
		return true;
}

function checkInteger(str){
	if(str.charAt(0)=='-')
		str=str.substr(1);
	if(str.match("^[0-9]+\$"))
		return true;
	return false;
}


function checkIntegerRange(value,minValue, maxValue)
{
	if(checkNull(value)==false){
		return -1;
	}
	if(checkInteger(value)!=true)
		return -2;
	if(value<minValue ||value>maxValue){
		return -3;
	}
	return true;
}
function getObj(id)
{
	return(document.getElementById(id));
}

function Check_IntegerRange(value,min,max,Frm)
{
	var msg;
	var tem=checkIntegerRange(value,min,max);
	if(tem==-1){
		//delete mVid!!
		msg=getmsg(101);
		alert(msg);
		getObj(Frm).focus();
		return false;
	}else if(tem==-2){
		msg=getmsg(102);
		alert(msg);
		getObj(Frm).focus();
		return false;
	}else if(tem==-3){
		msg=getmsg(103);
		alert(msg);
		getObj(Frm).focus();
		return false;
	}
	return true;
}

function pageCheckValue(id)
{
	var bandwidth=document.getElementById(id).value;
	if(bandwidth!=""){
		if(Check_IntegerRange(bandwidth,1,1024*1024,id)!=true){
			return false;
		}
	}else{
		//delete mVid!!!!
		//alert(getmsg(101));
		
		//return false;		
	}
	return true;
}

function btnApply()
{
	if(form.mac.value == "")
	{
		alert("mac ��ַ����Ϊ�գ�");
		return false;
	}
	if(!sji_checkmac(form.mac.value))
	{
		alert("mac ��ַ����");
		return false;
	}
	for(var i = 0; i < rules.length; i++)
	{
		if(rules[i].mac == form.mac.value)
		{
			alert( "mac��ַ�ظ���");//alert("That rule already exists");
			return false;
		}
	}

	if( (pageCheckValue("us_cfg")==false)||(pageCheckValue("ds_cfg")==false) )
	{
		return false;
	}
	
	form.submit();
}
</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
  <blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px">
		<form id="form" action=/boaform/admin/formBandWidth method=POST name="form">
			<b>����¹��豸�����д������ƹ���</b><br><br><br>
			<hr align="left" class="sep" size="1" width="90%">
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td width="180">MAC��ַ(xx-xx-xx-xx-xx-xx)&nbsp;</td>
					<td><input type="text" name="mac" size="18"></td>
				</tr>
				<tr>
					<td width="180">���д�������(kbps)&nbsp;</td>
					<td><input type="text" id="us_cfg" name="us_cfg" size="18"></td>
				</tr>
				<tr>
					<td width="180">���д�������(kbps)&nbsp;</td>
					<td><input type="text" id="ds_cfg" name="ds_cfg" size="18"></td>
				</tr>
			</table>
			<hr align="left" class="sep" size="1" width="90%">
			<input type="button" class="button" value="����/Ӧ��" onClick="btnApply()">
			<input type="hidden" name="action" value="add">
			<input type="hidden" name="submit-url" value="/app_bandwidth_control.asp">
		</form>
	</div>
  </blockquote>
</body>
<%addHttpNoCache();%></html>

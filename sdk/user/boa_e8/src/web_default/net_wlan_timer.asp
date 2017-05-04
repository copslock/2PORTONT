<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML>
<HEAD>
<TITLE>���ض�ʱ</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">
var _enable = new Array();
var _controlCycle = new Array();
var _startTime = new Array();
var _endTime = new Array();

function getObj(id)
{
	return(document.getElementById(id));
}

function setValue(id,value)
{
	document.getElementById(id).value=value;
}

function getValue(id)
{
	return(document.getElementById(id).value);
}

function convertDay(day)
{
	var day_name = ["��", "һ", "��", "��", "��", "��", "��"];
	var value="";
	for(i=1;i<7;i++){
		if(day & (1<<i))
			value+=day_name[i];
		else
			value+="**";
		value+=" ";
	}
	if(day & 1)
		value+=day_name[0];
	else
		value+="**";
	return value;
}

function getImage(src,strmethod,id)
{
	//return ("<input type=\"button\" id=\""+id+"\"  onclick=\""+ strmethod +"\" style=\"width:20px;height:20px;border:0px;padding:2px;cursor:pointer;background:url("+src+");\">");
	return ("<image id=\""+id+"\" onclick=\""+ strmethod +"\" src=\""+src+"\">");
}

function addline(index, enable, startTime, endTime, controlCycle)
{
	var newline;
	newline = document.getElementById('Schedule_Table').insertRow(-1);
	newline.nowrap = true;
	newline.vAlign = "top";
	newline.align = "center";
	newline.setAttribute("class","white");
	newline.setAttribute("className","white");
	newline.insertCell(-1).innerHTML = enable? "����":"ͣ��";
	newline.insertCell(-1).innerHTML = startTime;
	newline.insertCell(-1).innerHTML = endTime;
	newline.insertCell(-1).innerHTML = controlCycle;
	newline.insertCell(-1).innerHTML =getImage("image/edit.gif","Modify("+index+")","Btn_Modify"+index);
	newline.insertCell(-1).innerHTML =getImage("image/delete.gif","Delete("+index+")","Btn_Delete"+index);
}

function showTable()
{
	var num = _enable.length;
	if (num!=0) {
		for (var i=0; i<num; i++)
			addline(i, _enable[i], _startTime[i], _endTime[i], _controlCycle[i]);
	}
	else {
	}

	jslEnDisplay("Btn_Add");
	jslDiDisplay("Btn_Edit");
	jslDiDisplay("back");
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	//sji_docinit(document, cgi);
	showTable();
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(edit) 
{
	if(pageCheckValue()){
		if(edit)
			getObj("action").value = 2;
		else //add
			getObj("action").value = 1;
		getObj("wlanschedule").submit();
	}
	else {}
}

function jslDiDisplay(id)
{
	var i;
	var num=arguments.length;
	if(num==0)return;
	for(i=0;i<num;i++){
		document.getElementById(arguments[i]).style.display="none";
	}
}

function jslEnDisplay(id)
{
	var i;
	var num=arguments.length;
	if(num==0)
		return;
	for(i=0;i<num;i++)
		document.getElementById(arguments[i]).style.display="";
}

function jslDisable(id)
{
	var i;
	var num=arguments.length;
	if(num==0)
		return;
	for(i=0;i<num;i++){
		document.getElementById(arguments[i]).disabled=true;
	}
}

function jslEnable(id)
{
	var i=0;
	var num=arguments.length;
	if(num==0)
		return;
	for(i=0;i<num;i++)
		document.getElementById(arguments[i]).disabled=false;
}

function Modify(i)
{
	var start,end;
	getObj("Frm_Active").checked = _enable[i];
	start = _startTime[i].split(":");
	getObj("Frm_Start1").value = start[0];
	getObj("Frm_Start2").value = start[1];
	end = _endTime[i].split(":");
	getObj("Frm_End1").value = end[0];
	getObj("Frm_End2").value = end[1];
	
	getObj("Frm_Day").value = _controlCycle[i];

	getObj("if_index").value = i;

	jslDiDisplay("Btn_Add");
	jslEnDisplay("Btn_Edit");
	jslEnDisplay("back");
	
}

function back4add()
{
	getObj("Frm_Active").checked = false;
	getObj("Frm_Start1").value = "";
	getObj("Frm_Start2").value = "";
	getObj("Frm_End1").value = "";
	getObj("Frm_End2").value = "";
	getObj("Frm_Day").value = "";

	getObj("if_index").value = "";

	jslEnDisplay("Btn_Add");
	jslDiDisplay("Btn_Edit");
	jslDiDisplay("back");
}

function Delete(i)
{
	if ( !confirm("��ȷ��Ҫɾ��?") ) {
		getObj("action").value = "";
		return false;
	}
	getObj("action").value=0;
	getObj("if_index").value = i;
	getObj("wlanschedule").submit();
	return true;
}

function pageCheckValue()
{

	if(getObj("Frm_Start1").value==""){
		alert("ʱ�䲻��Ϊ��");
		getObj("Frm_Start1").focus();
		return false;
	}
	if(!isNumber(getObj("Frm_Start1").value) || getObj("Frm_Start1").value<0 || getObj("Frm_Start1").value>23){
		alert("������0~23����ֵ");
		getObj("Frm_Start1").focus();
		return false;
	}
	if(getObj("Frm_Start2").value==""){
		alert("ʱ�䲻��Ϊ��");
		getObj("Frm_Start2").focus();
		return false;
	}
	if(!isNumber(getObj("Frm_Start2").value) || getObj("Frm_Start2").value<0 || getObj("Frm_Start2").value>59){
		alert("������0~59����ֵ");
		getObj("Frm_Start2").focus();
		return false;
	}
	if(getObj("Frm_End1").value==""){
		alert("ʱ�䲻��Ϊ��");
		getObj("Frm_Start1").focus();
		return false;
	}
	if(!isNumber(getObj("Frm_End1").value) || getObj("Frm_End1").value<0 || getObj("Frm_End1").value>23){
		alert("������0~23����ֵ");
		getObj("Frm_End1").focus();
		return false;
	}
	if(getObj("Frm_End2").value==""){
		alert("ʱ�䲻��Ϊ��");
		getObj("Frm_End2").focus();
		return false;
	}
	if(!isNumber(getObj("Frm_End2").value) || getObj("Frm_End2").value<0 || getObj("Frm_End2").value>59){
		alert("������0~59����ֵ");
		getObj("Frm_End2").focus();
		return false;
	}
	if(getObj("Frm_Day").value==""){
		alert("ʱ�䲻��Ϊ��");
		getObj("Frm_Day").focus();
		return false;
	}
	if(!isNumber(getObj("Frm_Day").value) || getObj("Frm_Day").value<1 || getObj("Frm_Day").value>31){
		alert("������1~31����ֵ");
		getObj("Frm_Day").focus();
		return false;
	}
	return true;
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:10px">
			<form id="wlanschedule" action=/boaform/admin/formWifiTimer method=POST name=wlanschedule>
				<table border="0" cellpadding="3" cellspacing="0">
				<tr>
					<td>ʹ��</td>
					<td ><input name="Fnt_Active" id="Frm_Active" type="checkbox" value="ON"/>
					</td>
				</tr>
				<tr>
					<td>wifi����ʱ��</td>
					<td>
					<input name="Frm_Start1" id="Frm_Start1" type="text"  size="2" maxlength="2" /> :
					<input name="Frm_Start2" id="Frm_Start2" type="text"  size="2" maxlength="2" />
					</td>
				</tr>
				<tr>
					<td>wifi�ر�ʱ��</td>
					<td>
					<input name="Frm_End1" id="Frm_End1" type="text"  size="2" maxlength="2" /> :
					<input name="Frm_End2" id="Frm_End2" type="text"  size="2" maxlength="2" />
					</td>
				</tr>
				<tr>
					<td>����(��)</td>
					<td>
					<input name="Frm_Day" id="Frm_Day" type="text"  size="2" maxlength="2" />
					</td>
				</tr>
				
				</table>
				<table id="TestContent"  class="table" width="550px" border="0">
				<tr>
					<td class="td1"></td>
					<td class="td2">
					<input name="Btn_Add" type="button" id="Btn_Add" value="���" onclick="on_submit(0)"/>
					<input name="Btn_Edit" type="button" id="Btn_Edit" value="�޸�" onclick="on_submit(1)" style="display:none" />
					<input name="back" type="button" id="back" value="ȡ��"  onclick="back4add()" style="display:none"/>
					</td>
				</tr>
				</table>

				<table id="Schedule_Table" border="1" cellpadding="0" cellspacing="0" width="550px" >
				<tr>
				<td align="center" width="">ʹ��</td>
				<td align="center" width="">wifi����ʱ��</td>
				<td align="center" width="">wifi�ر�ʱ��</td>
				<td align="center" width="">����(��)</td>
				<td align="center" width="">�޸�</td>
				<td align="center" width="">ɾ��</td>
				</tr>
				</table>
				<input type="hidden" name="if_index" id="if_index" value=''>
				<input type="hidden" name="action" id="action" value="">
				<input type="hidden" name="submit-url" value="/net_wlan_timer.asp">
			</form>
			<script>
			<% initPage("wltimer"); %>
			</script>
		</DIV>
	<blockquote>
</body>
<%addHttpNoCache();%>
</html>

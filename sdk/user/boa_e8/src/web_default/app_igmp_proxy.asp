<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>IGMP ����</TITLE>
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

 var rules = new Array();
with(rules){<% igmproxyList(); %>}

/********************************************************************
**          on document load
********************************************************************/
function on_chkclick(index)
{
	if(index < 0 || index >= rules.length)
		return; 
	rules[index].dirty = true;
}

function on_init()
{
	sji_docinit(document);

	if(rulelst.rows)
	{
		while(rulelst.rows.length > 1) 
			rulelst.deleteRow(1);
	}
	
	for(var i = 0; i < rules.length; i++)
	{
		var row = rulelst.insertRow(i + 1);
		
		row.nowrap = true;
		row.vAlign = "top";
		row.align = "center";
		
		rules[i].dirty = false;
		
		var cell = row.insertCell(0);
		cell.innerHTML = rules[i].ifName;
		cell = row.insertCell(1);
		if(rules[i].enable)
			cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkclick(" + i + ");\" checked>";
		else
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
	   for(var i = 0; i < rules.length; i++){
	   var row=rulelst.rows[i+1];
	   var cell = row.cells[1];
	   
	    if(cell.children[0].checked==true)
	   	 rules[i].enable =1;
	     else 
	   	rules[i].enable =0;
	   
	      
       } 
		form.bcdata.value = sji_encode(rules, "dirty");
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
			<form id="form" action=/boaform/admin/formIgmproxy method=POST>
				<b>IGMP ����</b><br><br>
				���ҳ������������ض�WAN�ӿ�����IGMP����<br><br>
				<hr align="left" class="sep" size="1" width="90%">
				<table id="rulelst" class="flat" border="1" cellpadding="0" cellspacing="0">
					<tr class="hd" align="center">
						<td width="200px">����������</td>
						<td width="200px">IGMP����</td> 
					</tr>
				</table>
				<br>
				<input type="submit" class="button" onClick="on_submit()" value="����">
				<input type="hidden" name="bcdata" value="le">
				<input type="hidden" name="submit-url" value="">
			</form> 
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

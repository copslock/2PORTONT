<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>�й�����-ADSL����ʺ�����</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--ϵͳ����css-->
<style>
BODY{font-family:"��������";}
TR{height:16px;}
SELECT {width:150px;}
</style>
<!--ϵͳ�����ű�-->
<script language="javascript" src="/common.js"></script>
<SCRIPT language="javascript" type="text/javascript">
var curlink = null;
var cgi = new Object();
var links = new Array();
with(links){<% initPageAdsl(); %>}

document.onmousedown = function(e)
{
	var tar = null;
	if (!e)e = window.event;
	if(e.button != 1)return true;
	if (e.target) tar = e.target;
	else if (e.srcElement) tar = e.srcElement;
	if(tar.tagName == "INPUT" || tar.tagName == "IMG") return true;
	document.location.href = "app:mouse:onmousedown";
}

/*
function on_init()
{
	var selind = -1;
	sji_docinit(document, cgi);	
	
	with ( document.forms[0] ) 
	{
		for(var k in links)
		{
			var lk = links[k];
			var sname = lk.name;
			if(sname.indexOf("Internet") >= 0 && selind == -1) selind = lkname.options.length;
			lkname.options.add(new Option(sname, k));
		}
		if(selind >= 0)
		{
			lkname.value = selind;
			on_ctrlupdate();
			lkname.disabled = true;
			lkname.style.display = "none";
		}
		else
		{
			lkname.disabled = true;
			lkname.style.display = "none";
			var ins = document.getElementsByTagName("input");
			for(var i in ins){if(ins[i].value != "����")ins[i].disabled = true;}
			ins = document.getElementsByTagName("img");
			for(var i in ins)ins[i].disabled = true;
		}
	}
}

function on_linkchange(itlk)
{
	with ( document.forms[0] ) 
	{
		sji_onchanged(document, itlk);
	}
}

function on_ctrlupdate()
{
	with ( document.forms[0] ) 
	{
		curlink = links[lkname.value];
		vpi.value=curlink.vpi;
		vci.value=curlink.vci;
		pppUsername.value=curlink.pppUsername;
		pppPassword.value=curlink.pppPassword;
		on_linkchange(curlink);
	}
}

function on_submit() 
{
	with ( document.forms[0] ) 
	{
		//input text
		if(sji_checkpppacc(pppUsername.value, 1, 25) == false)
		{
			pppUsername.focus();
			external.ShowDlg("�û���\"" + pppUsername.value + "\"Ϊ��Ч�û����򳤶Ȳ���1-25�ֽ�֮�䣬���������룡");
			return;
		}

		if(oldpppPassword.value != curlink.pppPassword)
		{
			oldpppPassword.focus();
			external.ShowDlg("�����벻ƥ�䣬���������룡");
			return;
		} 

		if(sji_checkpppacc(pppPassword.value, 1, 25) == false)
		{
			pppPassword.focus();
			external.ShowDlg("���������Ч�ַ��򳤶Ȳ���1-25�ֽ�֮�䣬���������룡");
			return;
		}
		if(pppPassword.value != pppPassword_cfm.value)
		{
			pppPassword_cfm.focus();
			external.ShowDlg("����ȷ�Ϻ����벻һ�£����������룡");
			return;
		}
		lkname.style.display = "block";
		lkname.disabled = false;
		submit();
	}
}
*/

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);	
	
	with ( document.forms[0] ) 
	{
		var i=0;
		var first=-1;
		for(var k in links)
		{
			var lk = links[k];
			if(lk.cmode==2&&lk.PPPoEProxyEnable==0&&(lk.applicationtype==0 || lk.applicationtype==1)){
				i=i+1;
				if(first==-1)
					first=k;
				lkname.options.add(new Option(lk.name, lk.name));
			}
		}
		if(i> 0)
		{
			lkname.value = links[first].name;
			vpi.value=links[first].vpi;
			vci.value=links[first].vci;
			pppUsername.value=links[first].pppUsername;
			on_ctrlupdate();
		}
		else
		{
			lkname.disabled = true;
			var ins = document.getElementsByTagName("input");
			for(var i in ins)ins[i].disabled = true;
			ins = document.getElementsByTagName("img");
			for(var i in ins)ins[i].disabled = true;
			save.disabled = true;
			back.disabled = false;
		}
	}
}

function on_linkchange(itlk)
{
	with ( document.forms[0] ) 
	{
		sji_onchanged(document, itlk);
	}
}

function on_ctrlupdate()
{
	with ( document.forms[0] ) 
	{
		for(var i=0;i<links.length;i++){
			if(links[i].name==lkname.value){
				curlink=links[i];
				break;
			}
		}
		vpi.value=curlink.vpi;
		vci.value=curlink.vci;
		pppUsername.value=curlink.pppUsername;
		on_linkchange(curlink);
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit() 
{
	with ( document.forms[0] ) 
	{
		//input text
/*star:20090302 START ppp username and password can be empty*/
		if(pppUsername.value!="" && sji_checkpppacc(pppUsername.value, 1, 25) == false)
		{
			pppUsername.focus();
			alert("�û���\"" + pppUsername.value + "\"Ϊ��Ч�û����򳤶Ȳ���1-25�ֽ�֮�䣬���������룡");
			return;
		}
		if(pppPassword.value!="" && sji_checkpppacc(pppPassword.value, 1, 25) == false)
		{
			pppPassword.focus();
			alert("���������Ч�ַ��򳤶Ȳ���1-25�ֽ�֮�䣬���������룡");
			return;
		}
		if(pppPassword.value != pppPassword_cfm.value)
		{
			pppPassword_cfm.focus();
			alert("����ȷ�Ϻ����벻һ�£����������룡");
			return;
		}
		submit();
	}
}

function on_cancel()
{
	with ( document.forms[0] ) 
	{
		pppUsername.value = curlink.pppUsername;
		pppPassword.value = "";
		pppPassword_cfm.value = "";		
	}
}


</script>

</HEAD>
<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" bgcolor="E0E0E0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init()">
	<DIV align="center" style="padding-left:5px; padding-top:5px">
		<form id="form" action="/boaform/admin/formAdslClt" method="post">
			<table width="80%"><tr><td><p align="left"><font size="+2"><b>ADSL����ʺ�����</b></font></p></td></tr></table><br><br>
			<table id="cfg" cellspacing="0" cellpadding="0" width="256" align="center" border="0">
			  <tr valign="top">
				<td height="1">
				  <table height="200" cellspacing="0" cellpadding="0" width="100%" align="center" border="0">
					<tr>
					  <td vAlign=top>
						<table cellspacing="0" cellpadding="0" border="0" width="100%">
							<tr nowrap><td>�������ƣ�</td><td align="right"><select id="lkname" name="lkname" onChange="on_ctrlupdate();" size="1"></select></td></tr>
							<tr nowrap><td>�ʺţ�</td><td align="right"><input type="text" name="pppUsername" maxlength="26" size="26" style="width:150px " readonly="1"></td></tr>
							<!--<tr nowrap><td>�����룺</td><td align="right"><input type="password"name="oldpppPassword" maxlength="16" size="16" style="width:150px "></td></tr>-->
							<tr nowrap><td>�����룺</td><td align="right"><input type="password"name="pppPassword" maxlength="26" size="26" style="width:150px "></td></tr>
							<tr nowrap><td>ȷ�������룺</td><td align="right"><input type="password"name="pppPassword_cfm" maxlength="26" size="26" style="width:150px "></td></tr>
						</table>
					  </td>
					</tr>
				  </table>
				<table border="0" cellpadding="1" cellspacing="0" width="80%">
					<tr>
					<td align="center"><input type="button" name="back" value="����" onClick="window.location.href='/autorun/e8cfg.asp';" style="width:80px; border-style:groove; font-weight:bold "></td>&nbsp;&nbsp;
					<td align="center"><input type="button" name="save" value="��������" onClick="on_submit();" style="width:80px; border-style:groove; font-weight:bold "></td>
					</tr>
				</table>
				</td>
			  </tr>
			</table>
			<input type="hidden" name="vpi" value="0">
			<input type="hidden" name="vci" value="0">
			<br>
			<input type="hidden" name="submit-url" value="/autorun/acccfg.asp">
		</form>
	</DIV>
</body>
</html>


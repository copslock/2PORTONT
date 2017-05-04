<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>Internet����</TITLE>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--ϵͳ����css-->
<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
<style>
TABLE{width:320px;}
TR{height:16px;}
SELECT {width:150px;}
</style>
<!--ϵͳ�����ű�-->
<SCRIPT language="javascript" src="common.js"></SCRIPT>
<SCRIPT language="javascript" type="text/javascript">

var upmodes = new Array("ADSL", "LAN");
var lkmodes = new Array("Bridge", "Route");
var svkinds = new Array("UBR Without PCR", "UBR With PCR", "CBR", "Non Realtime VBR", "Realtime VBR");
var cpmodes = new Array("LLC", "VC");
var apmodes = new Array("TR069_INTERNET", "INTERNET", "TR069", "Other");
var dlmodes = new Array("�Զ�����", "������ʱ�Զ�����");
var md802ps = new Array("(��)", "0", "1", "2", "3", "4", "5", "6", "7");

var opts = new Array(new Array("upmode", upmodes), new Array("lkmode", lkmodes), 
	new Array("svtype", svkinds), new Array("encap", cpmodes), new Array("applicationtype", apmodes),
	new Array("pppCtype", dlmodes), new Array("vprio", md802ps));

var curlink = null;

var cgi = new Object();
var links = new Array();
with(links){<% initPageAdsl(); %>}

function searchpvc(ipmode, pppcheck, vpi, vci)
{
	vpi = parseInt(vpi);
	vci = parseInt(vci);
	for(var i = 0; i < links.length; i++)
	{
		if(links[i] == curlink)continue;
		if(vpi == links[i]["vpi"] && vci == links[i]["vci"])
		{
			if(pppcheck == true && links[i]["cmode"] == 2)
				continue;
			if((ipmode ==0 && links[i]["cmode"] != 0) || (ipmode ==1 && links[i]["cmode"] == 0))
				continue;
			return true;
		}
	}
	return false;
}

function on_linkchange(itlk)
{
	with ( document.forms[0] ) 
	{
		if(itlk == null)
		{
			//select
			lkmode.value = vprio.value = pppCtype.value = encap.value = svtype.value = 0;
			if (4 == applicationtype.options.length)
			{
				applicationtype.value = 0;
			}
			else
			{
				applicationtype.value = 1;
			}
			
			//radio
			ipmode[0].checked = true;
			
			//checkbox
			PPPoEProxyEnable.checked = brmode.checked = napt.checked = vlan.checked = qos.checked = dgw.checked = false;
			
			//checkbox array
			chkpt[0].checked = chkpt[1].checked = chkpt[2].checked = chkpt[3].checked = chkpt[4].checked = chkpt[5].checked = chkpt[6].checked = chkpt[7].checked = chkpt[8].checked = false;
			
			//input number
			PPPoEProxyMaxUser.value = vpi.value = vci.value = pcr.value = scr.value = mbs.value = vid.value = "0";
			
			//input ip
			ipAddr.value = remoteIpAddr.value = "0.0.0.0";
			netMask.value = "255.255.255.255";
			
			//input text
			pppUsername.value = pppPassword.value = pppServiceName.value = pppACName.value = "";
		}
		else
		{
			sji_onchanged(document, itlk);
			
			//select
			lkmode.value = itlk.cmode >= 1 ? 1 : 0;

			//checkbox array
			var ptmap = itlk.itfGroup;
			for(var i = 0; i < 9; i ++) chkpt[i].checked = (ptmap & (0x1 << i));
			
			//radio
			if(curlink.cmode != 0 && curlink.ipDhcp == 0)
			{
				ipmode[curlink.cmode].checked = true;
			}
			else ipmode[0].checked = true;

			if ((1 == lkmode.value) && (2 == applicationtype.options.length))
			{
				//mode changed from bridge to route
				// add BOTH and tr069 option
				applicationtype.options.length = 0;
				for(var i in apmodes)
				{
					applicationtype.options.add(new Option(apmodes[i], i));
				}

				applicationtype.value = itlk.applicationtype;
			}
		}
		on_ctrlupdate(lkmode);
		on_ctrlupdate(svtype);
		on_ctrlupdate(vlan);
		//on_ctrlupdate(applicationtype);
		if(itlk != null){
			if(itlk.napt==1)
				napt.checked = true;
			else
				napt.checked = false;
		}
	}
}

function on_ctrlupdate(obj)
{
	with ( document.forms[0] ) 
	{
		if(obj.name == "lkname")
		{
			if(obj.value == "new")
			{
				curlink = null;
				on_linkchange(curlink);
			}
			else
			{
				curlink = links[obj.value];
				on_linkchange(curlink);
			}
		}
		else if(obj.name == "lkmode")
		{
			tbipmode.style.display = obj.value == 0 ? "none" : "block";
			tbnat.style.display = obj.value == 0 ? "none" : "block";
			tbdgw.style.display = obj.value == 0 ? "none" : "block";
			if(obj.value == 1 && applicationtype.value != 2)
				napt.checked = true;
			else{
				napt.checked = false;
				tbnat.style.display = "none";
				tbdgw.style.display = "none";
			}
			on_ctrlupdate(ipmode[0]);
			
			var orgapptype;

			if (0 == obj.value)
			{
				if (4 == applicationtype.options.length)
				{
					orgapptype = applicationtype.value;

					// remove BOTH and tr069 option
					applicationtype.options.remove(2);
					applicationtype.options.remove(0);

					if ((0 == orgapptype) || (2 == orgapptype))
					{
						applicationtype.value = 1;
					}
				}
			}
			else
			{
				if (2 == applicationtype.options.length)
				{
					orgapptype = applicationtype.value;

					// add BOTH and tr069 option
					applicationtype.options.length = 0;
					for(var i in apmodes)
					{
						applicationtype.options.add(new Option(apmodes[i], i));
					}

					applicationtype.value = orgapptype;
				}
			}
			
			on_ctrlupdate(applicationtype);
		}
		else if(obj.name == "ipmode")
		{
			tbip.style.display = "none";
			tbdial.style.display = "none";
			tbpppprxy.style.display = "none";
			tbpppbr.style.display = "none";
			tbpppnum.style.display = "none";		
			
			if(lkmode.value == 1)
			{
				if (ipmode[1].checked == true)
				{
					tbip.style.display = "block";
				}
				else if (ipmode[2].checked == true)
				{
					tbdial.style.display = "block";
					if (applicationtype.value != 2)
					{
						tbpppprxy.style.display = "block";
						tbpppbr.style.display = "block";
						on_ctrlupdate(PPPoEProxyEnable);
					}else
						tbpppbr.style.display = "block";
				}
			}
		}
		else if(obj.name == "PPPoEProxyEnable")
		{
			tbpppnum.style.display = obj.checked == false ? "none" : "block";
		}
		else if(obj.name == "svtype")
		{
			tbpeakcell.style.display = obj.value >= 1 ? "block" : "none";
			tbothercell.style.display = obj.value >= 3 ? "block" : "none";
		}
		else if(obj.name == "vlan")
		{
			vid.disabled = !obj.checked;
			vprio.disabled = !obj.checked;
		}
		else if(obj.name == "applicationtype")
		{
			if(obj.value == 2){
				napt.checked = false;
				tbnat.style.display = "none";
				tbbind.style.display = "none";
				tbindip.style.display = "none";
				dgw.checked = false;
				tbdgw.style.display = "none";
				
				PPPoEProxyEnable.checked = false;

				tbpppprxy.style.display = "none";
				//tbpppbr.style.display = "none";
								
				//brmode.checked = false;				
				on_ctrlupdate(PPPoEProxyEnable);
				
			}else{
				if(lkmode.value == 1){
					tbnat.style.display = "block";
					napt.checked = true;
					if (obj.value == 3)
					{
						dgw.checked = false;
						tbdgw.style.display = "none";
					}
					else
					{
						tbdgw.style.display = "block";
					}

					
					if (ipmode[2].checked == true)
					{
						tbpppprxy.style.display = "block";
						tbpppbr.style.display = "block";
						on_ctrlupdate(PPPoEProxyEnable);
					}
					
				}
				else{
					napt.checked = false;
					tbdgw.style.display = "none";
				}
				tbbind.style.display = "block";
				tbindip.style.display = "block";
			}

			if ((3 == obj.value) && (0 == lkmode.value))
			{
				attention.style.display = "block";
			}
			else
			{
				attention.style.display = "none";
			}
		}
		<% initdgwoption(); %>
	}
}

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);	

	for(var i in opts)
	{
		var slit = document.getElementById(opts[i][0]);
		if(typeof slit == "undefined")continue;
		for(var j in opts[i][1])
		{
			slit.options.add(new Option(opts[i][1][j], j));
		}
		slit.selectedIndex = 0;
	}
	with ( document.forms[0] ) 
	{
		for(var k in links)
		{
			var lk = links[k];
			lkname.options.add(new Option(lk.name, k));
		}
		if(links.length > 0)lkname.value = 0;
		lpppnumleft.innerHTML = cgi.pppnumleft;
		on_ctrlupdate(lkname);
	}
}

/********************************************************************
**          on document submit
********************************************************************/
function on_submit(act) 
{
	var tmplst = "";
	with ( document.forms[0] ) 
	{
		action.value = act;
		if(act == "rm")
		{
			if(lkname.value == "new")
			{
				lkname.focus();
				alert("��ѡ��Ҫɾ�������ӣ�");
				return;
			}
			tmplst = curlink.name;
		}
		else if(act == "sv")
		{
			if(lkname.value != "new")tmplst = curlink.name;
			if(lkmode.value == 0 && (applicationtype.value == 0 || applicationtype.value == 2))
			{
				lkmode.focus();
				alert("���ô�����ģʽ�·���ģʽ��֧��tr069����!");
				return;
			}

			//input number
			if(lkmode.value != 0 && ipmode[2].checked == true && sji_checkdigitrange(PPPoEProxyMaxUser.value, 0, cgi.pppnumleft) == false)
			{
				PPPoEProxyMaxUser.focus();
				alert("�����û���\"" + PPPoEProxyMaxUser.value + "\"Ϊ��Ч�û��������������룡");
				return;
			}
			if(sji_checkdigitrange(vpi.value, 0, 255) == false)
			{
				vpi.focus();
				alert("VPI\"" + vpi.value + "\"Ϊ��Ч��ֵ��Χ����0-255֮�䣡���������룡");
				return;
			}
			if(sji_checkdigitrange(vci.value, 32, 65535) == false)
			{
				vci.focus();
				alert("VCI\"" + vci.value + "\"Ϊ��Ч��ֵ��Χ����32-65535֮�䣬���������룡");
				return;
			}
			if( searchpvc(lkmode.value , ipmode[2].checked, vpi.value, vci.value) == true)
			{
				vpi.focus();
				alert("PVC\"" + vpi.value + "/" + vci.value + "\"�Ѿ����ڣ����������룡");
				return;
			}
			if(vpi.value == "0" && vci.value == "16")
			{
				vpi.focus();
				alert("PVC\"" + vpi.value + "/" + vci.value + "\"�Ѿ���ϵͳռ�ã����������룡");
				return;
			}
			if(vpi.value == "0" && vci.value == "0")
			{
				vpi.focus();
				alert("PVC\"" + vpi.value + "/" + vci.value + "\"Ϊ��Чͨ�������������룡");
				return;
			}

			if(svtype.value >= 1 && sji_checkdigitrange(pcr.value, 1, 255000) == false)
			{
				pcr.focus();
				alert("��ֵ��Ԫ����\"" + pcr.value + "\"Ϊ��Ч��ֵ��Χ����1-255000֮�䣬���������룡");
				return;
			}
			if(svtype.value >= 3 && sji_checkdigitrange(scr.value, 1, parseInt(pcr.value) - 1) == false)
			{
				scr.focus();
				alert("������Ԫ����\"" + scr.value + "\"Ϊ��Ч��ֵ��Χ����1-" + pcr.value + "֮�䣬���������룡");
				return;
			}
			if(svtype.value >= 3 && sji_checkdigitrange(mbs.value, 1, 1000000) == false)
			{
				mbs.focus();
				alert("���ͻ����Ԫ��С\"" + mbs.value + "\"Ϊ��Ч��ֵ��Χ����1-1000000֮�䣬���������룡");
				return;
			}
			
			if(vlan.checked == true && sji_checkdigitrange(vid.value, 0, 4095) == false)
			{
				vid.focus();
				alert("VLAN ID\"" + vid.value + "\"Ϊ��Ч��ֵ��Χ����0-4095֮�䣬���������룡");
				return;
			}
			
			if(lkmode.value == 1 && ipmode[1].checked == true && sji_checkvip(ipAddr.value) == false)
			{
				ipAddr.focus();
				alert("IP��ַ\"" + ipAddr.value + "\"Ϊ��Ч��ַ�����������룡");
				return;
			}

			if(lkmode.value == 1 && ipmode[1].checked == true && sji_checkmask(netMask.value) == false)
			{
				netMask.focus();
				alert("��������\"" + netMask.value + "\"Ϊ��Ч���룬���������룡");
				return;
			}
			if(lkmode.value == 1 && ipmode[1].checked == true && sji_checkvip(remoteIpAddr.value) == false)
			{
				remoteIpAddr.focus();
				alert("ȱʡ����\"" + remoteIpAddr.value + "\"Ϊ��Ч��ַ�����������룡");
				return;
			}

//star:20090302 START ppp username and password can be empty

			//input text
			if(pppUsername.value!="" && lkmode.value == 1 && ipmode[2].checked == true && sji_checkpppacc(pppUsername.value, 1, 20) == false)
			{
				pppUsername.focus();
				alert("�û���\"" + pppUsername.value + "\"Ϊ��Ч�û����򳤶Ȳ���1-20�ֽ�֮�䣬���������룡");
				return;
			}
			if(pppPassword.value!="" && lkmode.value == 1 && ipmode[2].checked == true && sji_checkpppacc(pppPassword.value, 1, 20) == false)
			{
				pppPassword.focus();
				alert("���������Ч�ַ��򳤶Ȳ���1-20�ֽ�֮�䣬���������룡");
				return;
			}

			
			if(pppServiceName.value!=""){
				if(lkmode.value == 1 && ipmode[2].checked == true && sji_checkusername(pppServiceName.value, 1, 20) == false)
				{
					pppServiceName.focus();
					alert("��������\"" + pppServiceName.value + "\"Ϊ��Ч���ƻ򳤶Ȳ���1-20�ֽ�֮�䣬���������룡");
					return;
				}
			}
			if(pppACName.value!=""){
				if(lkmode.value == 1 && ipmode[2].checked == true && sji_checkusername(pppACName.value, 1, 20) == false)
				{
					pppAC.focus();
					alert("AC����\"" + pppACName.value + "\"Ϊ��Ч���ƻ򳤶Ȳ���1-20�ֽ�֮�䣬���������룡");
					return;
				}
			}
			
			if(lkmode.value == 0)cmode.value = 0;
			else if(ipmode[2].checked)
			{
				cmode.value = 2;
			}
			else
			{
				cmode.value = 1;
				ipDhcp.value = ipmode[0].checked ? 1 : 0;
			}
			
			//checkbox array
			var ptmap = 0;
			for(var i = 0; i < 9; i ++) if(chkpt[i].checked == true) ptmap |= (0x1 << i);
			itfGroup.value = ptmap;

			//check for br and route on the same pvc
			for(var i = 0; i < links.length; i++)
			{
				if(vpi.value != links[i]["vpi"] || vci.value != links[i]["vci"])
					continue;
				if(lkmode.value == 0){
					if(links[i]["cmode"] == 2 && links[i]["brmode"] == 1 && links[i]["itfGroup"] != itfGroup.value){
						alert("PVC\"" + vpi.value + "/" + vci.value + "\"��bridgeģʽ�²��ܰ󶨵���ͬ��lan�˿ڣ�����������!");
						return;
					}
				}
				if(lkmode.value == 1 && ipmode[2].checked==true && brmode.checked==true){
					if(links[i]["cmode"] == 0 && links[i]["itfGroup"] != itfGroup.value){
						alert("PVC\"" + vpi.value + "/" + vci.value + "\"��bridgeģʽ�²��ܰ󶨵���ͬ��lan�˿ڣ�����������!");
						return;
					}
				}
			}
		}
		form.lst.value = tmplst;
		submit();
	}
}

function add_dns()
{
   var loc = "net_dns.asp";
   var code = "window.location.href=\"" + loc + "\"";
   eval(code);
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action="/boaform/admin/formAdsl" method="post">
				<div class="tip" style="width:90% ">
					<b>WAN ����</b><br><br>
					<div id="rstip" style="display:none;"><font color="red">��ʾ����ҳ������ã���Ҫ����·����������Ч��</font><br></div>
				</div>
				<hr align="left" class="sep" size="1" width="90%">
				<br>
				<table id="tbupmode" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">���з�ʽ��</td><td><select id="upmode" name="upmode" disabled=1 onChange="on_ctrlupdate(this)" size="1"></select></td></tr>
					<tr nowrap><td>�������ƣ�</td><td><select id="lkname" name="lkname" onChange="on_ctrlupdate(this)" size="1"><option value="new" selected>����WAN����</option></select></td></tr>
				</table>
				<table id="tblkmode" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">ģʽ ��</td><td><select id="lkmode" name="lkmode" onChange="on_ctrlupdate(this)" size="1"></select></td></tr>
				</table>
				<table id="tbipmode" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px"><input type="radio" name="ipmode" value="0" onClick="on_ctrlupdate(this)">DHCP</td><td >��ISP���Զ��õ�һ��IP��ַ</td></tr>
					<tr nowrap><td><input type="radio" name="ipmode" value="1" onClick="on_ctrlupdate(this)">Static</td><td>��ISP������һ��IP��̬��ַ</td></tr>
					<tr nowrap><td><input type="radio" name="ipmode" value="2" onClick="on_ctrlupdate(this)">PPPoE </td><td>��ISPʹ��PPPOE��ѡ�����</td></tr>
				</table>
				<table id="tbpppprxy" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="3"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px"><input type="checkbox" name="PPPoEProxyEnable" onClick="on_ctrlupdate(this);"></td><td>����PPPOE����</td></tr>
				</table>	
				<table id="tbpppnum" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">�����û��������ô���<b><label id="lpppnumleft">5</label></b>��</td><td><input type="text" name="PPPoEProxyMaxUser" maxlength="2" size="2" style="width:40px "></td></tr>
				</table>	
				<table id="tbpppbr" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px"><input type="checkbox" name="brmode"></td><td>PPPoE·���Ż��ģʽ</td></tr>
				</table>	
				<table id="tbpvc" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">VPI(0-255)��</td><td><input type="text" name="vpi" value="0" maxlength="3" size="4" style="width:40px "></td></tr>
					<tr nowrap><td>VCI(32-65535)��</td><td><input type="text" name="vci" value="32" maxlength="5" size="4" style="width:40px "></td></tr>
				</table>	
				<table id="tbsvkind" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">�������</td><td><select id="svtype" name="svtype" size="1" onChange="on_ctrlupdate(this)"></select></td></tr>
				</table>
				<table id ="tbpeakcell" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">��ֵ��Ԫ���ʣ�</td><td><input type="text" name="pcr" value="" maxlength="4" size="4" style="width:40px ">cells/s</td></tr>
				</table>
				<table id="tbothercell" cellpadding="0px" cellspacing="2px">
					<tr nowrap><td width="150px">������Ԫ���ʣ�</td><td><input type="text" name="scr" value="" maxlength="4" size="4" style="width:40px ">cells/s</td></tr>
					<tr nowrap><td>���ͻ����Ԫ��С��</td><td><input type="text" name="mbs"  value="" maxlength="4" size="4" style="width:40px ">cells</td></tr>
				</table>
				<table id="tbnat" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">����NAT��</td><td><input type="checkbox" name="napt"></td></tr>
				</table>
				<table id="tbvlan" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">����Vlan��</td><td><input type="checkbox" name="vlan" onClick="on_ctrlupdate(this)"></td></tr>
					<tr nowrap><td>Vlan ID��</td><td><input type="text" name="vid" maxlength="4"  size="4" style="width:40px "></td></tr>
					<tr nowrap><td>802.1p��</td><td><select id="vprio" name="vprio" size="1"></select></td></tr>
				</table>
				<table id="tbdial" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">�û�����</td><td><input type="text" name="pppUsername" maxlength="20" size="16" style="width:150px "></td></tr>
					<tr nowrap><td>���룺</td><td><input type="password"name="pppPassword" maxlength="20" size="16" style="width:150px "></td></tr>
					<tr nowrap><td>�������ƣ�</td><td><input type="text" name="pppServiceName" maxlength="20" size="16" style="width:150px "></td></tr>
					<tr nowrap><td>AC���ƣ�</td><td><input type="text" name="pppACName" maxlength="20" size="16" style="width:150px "></td></tr>
					<tr nowrap><td>���ŷ�ʽ��</td><td><select id="pppCtype" name="pppCtype" size="1"></select></td></tr>
				</table> 
				<table id="tbip" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">IP��ַ��</td><td><input type="text" name="ipAddr" maxlength="15" size="15" style="width:150px "></td></tr>
					<tr nowrap><td>�������룺</td><td><input type="text" name="netMask" maxlength="15" size="15" style="width:150px "></td></tr>
					<tr nowrap><td>ȱʡ���أ�</td><td><input type="text" name="remoteIpAddr" maxlength="15" size="15" style="width:150px "></td></tr>
					<!--<tr nowrap><td>��ѡDNS��</td><td><input type="text" name="fstdns" maxlength="15" size="15" style="width:150px "></td></tr>
					<tr nowrap><td>��ѡDNS��</td><td><input type="text" name="secdns" maxlength="15" size="15" style="width:150px "></td></tr>-->
				</table>
				<table id="tbdgw" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="3"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">Ĭ��·��: </td><td><input type="checkbox" name="dgw"></td></tr>
				</table>	
				<table id="tbqos" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">����QoS:</td><td><input type="checkbox" name="qos"></td></tr>
					<tr nowrap><td>��װ��ʽ:</td><td><select id="encap" name="encap" size="1"></select></td></tr>
					<tr nowrap><td>����ģʽ:</td><td><select id="applicationtype" onChange="on_ctrlupdate(this)" name="applicationtype" size="1"></select></td></tr>
				</table>
				<table id="tbbind" cellpadding="0px" cellspacing="2px">
					<tr class="sep"><td colspan="2"><hr align="left" class="sep" size="1" width="100%"></td></tr>
					<tr nowrap><td width="150px">�󶨶˿ڣ�</td><td>&nbsp;</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">�˿�_1</td><td ><input type="checkbox" name="chkpt">�˿�_2</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">�˿�_3</td><td ><input type="checkbox" name="chkpt">�˿�_4</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">����(ROOT/SSID1)</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">����(SSID2)</td><td ><input type="checkbox" name="chkpt">����(SSID3)</td></tr>
					<tr nowrap><td><input type="checkbox" name="chkpt">����(SSID4)</td><td ><input type="checkbox" name="chkpt">����(SSID5)</td></tr>
					
				</table>
				<br>
				<div class="tip" id="tbindip" style="width:90% ">
					<tr nowrap><td colspan="2">ע: WAN����֮�䲻�ܹ���󶨶˿ڣ����󶨶˿ڵ�WAN���Ӱ󶨲��������֮ǰ����WAN���ӶԸö˿ڵİ󶨲�����</td></tr>
				</div>
				<br>
				<div class="tip" id="attention" style="width:90% ">
					<tr nowrap><td colspan="2"><font color="red">����ģʽΪOther��Bridge���Ӱ�ʱ���˿ڶ�Ӧ��PC����ͨ�����ض�̬�õ�IP��ַ��������ģʽΪOtherʱ����ע�ⲻҪ�����о������˿ڰ�Ϊ���������</font></td></tr>
				</div>
				<br>
				<table id="tbdns" cellpadding="0px" cellspacing="2px">
					<input type="button" class="button" onClick="add_dns()" value="DNS����">
				</table>
				<hr align="left" class="sep" size="1" width="90%">
				<br>
				<input type="button" class="button" onClick="on_submit('sv');;" value="Ӧ��">&nbsp; &nbsp; &nbsp; &nbsp; 
				<!--<input type="button" class="button" onClick="on_submit('rs');" value="����">&nbsp; &nbsp; &nbsp; &nbsp;-->
				<input type="button" class="button" onClick="on_submit('rm');" value="ɾ��">&nbsp; &nbsp; &nbsp; &nbsp; 
				<input type="hidden" name="cmode" value="0">
				<input type="hidden" name="ipDhcp" value="0">
				<input type="hidden" name="itfGroup" value="0">
				<input type="hidden" id="lst" name="lst" value="">
				<input type="hidden" id="action" name="action" value="none">
				<input type="hidden" name="submit-url" value="">
				<input type="hidden" id="acnameflag" name="acnameflag" value="have">
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</HTML>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>Version Information Setup</TITLE>
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
<script type="text/javascript" src="share.js"></script>
<SCRIPT language="javascript" type="text/javascript">
function sendClicked()
{
        if (document.password.binary.value=="") {
                alert('ѡ��ĵ�������Ϊ��!'); //Selected file cannot be empty!
                document.password.binary.focus();
                return false;
        }

        if (!confirm('��ȷ��Ҫ���´����?')) //Do you really want to upgrade the firmware?
                return false;
        else
                return true;
}


function uploadClick()
{		
   	if (document.saveConfig.binary.value.length == 0) {
		alert('��ѡ�񵵰�!'); //Choose File!
		document.saveConfig.binary.focus();
		return false;
	}
	return true;
}

function exportClick()
{		
   	
	alert('��ȴ�60��!'); //Please waiting for 60 seconds!

	return true;
}
</SCRIPT>

<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">

<form action=/boaform/formVersionMod method=POST name="vermod">
	<h2><font color="#0000FF">Version</font></h2>
  
  <table cellpadding="0px" cellspacing="2px">


  <tr>
      <td width="150px">Manufacturer:</td>
      <td><input type="text" name="txt_mft" size="50" maxlength="50" value=<% getInfo("rtk_manufacturer"); %>>(ZTE)</td>
  </tr>

  <tr>
      <td width="150px">OUI:</td>
      <td><input type="text" name="txt_oui" size="50" maxlength="50" value=<% getInfo("rtk_oui"); %>>(5422F8)</td>
  </tr>

  <tr>
      <td width="150px">Product Class:</td>
      <td><input type="text" name="txt_proclass" size="50" maxlength="50" value=<% getInfo("rtk_productclass"); %>>(F660)</td>
  </tr>
    <tr>
      <td width="150px">Hardware Serial Number:</td>
      <td><input type="text" name="txt_serialno" size="50" maxlength="50" value=<% getInfo("rtk_serialno"); %>>(000000000002)</td>
  </tr>
    <tr>
      <td width="150px">Provisioning Code:</td>
      <td><input type="text" name="txt_provisioningcode" size="50" maxlength="50" value=<% getInfo("cwmp_provisioningcode"); %>>(TLCO.GRP2)</td>
  </tr>
  <tr>
      <td width="150px">Spec. Version</td>
      <td><input type="text" name="txt_specver" size="50" maxlength="50" value=<% getInfo("rtk_specver"); %>>(1.0)</td>
  </tr>
  
  <tr>
      <td width="150px">SoftwareVersion</td>
      <td><input type="text" name="txt_swver" size="50" maxlength="50" value=<% getInfo("rtk_swver"); %>>(V2.30.10P16T2S)</td>
  </tr>
  
  <tr>
      <td width="150px">Hardware Version</td>
      <td><input type="text" name="txt_hwver" size="50" maxlength="50" value=<% getInfo("rtk_hwver"); %>>(V3.0)</td>
  </tr>
  <tr>
      <td width="150px">GPON Serial Number</td>
      <td><input type="text" name="txt_gponsn" size="13" maxlength="13" value=<% getInfo("gpon_sn"); %>></td>
  </tr>
   <tr>
      <td width="150px">ELAN MAC Address</td>
      <td><input type="text" name="txt_elanmac" size="12" maxlength="12" value=<% getInfo("elan_mac_addr"); %>></td>
  </tr>
     <tr>
      <td width="250px">����������������(0��ȡ��)</td>
      <td><input type="text" name="txt_wanlimit" size="12" maxlength="12" value=<% getInfo("wan_limit"); %>></td>
  </tr>
     <tr>
      <td width="250px">LOIDע��״̬(0��ɹ�,1~3��ʧ��,4��ע�ᳬʱ,5����ע��)</td>
      <td><input type="text" name="txt_reg_status" size="12" maxlength="12" value=<% getInfo("loid_reg_status"); %>></td>
  </tr>
       <tr>
      <td width="250px">LOID�·�ҵ��Y��(1���·��ɹ�,2���·�ʧ��)</td>
      <td><input type="text" name="txt_reg_result" size="12" maxlength="12" value=<% getInfo("loid_reg_result"); %>></td>
  </tr>
   </tr>
       <tr>
      <td width="250px">����UI���趨��Ŀ(tr069 WAN/Conf)(0������,1������)</td>
      <td><input type="text" name="txt_cwmp_conf" size="12" maxlength="12" value=<% getInfo("cwmp_conf"); %>></td>
  </tr>
 </table>

      <input type="submit" value="Save" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
      <!--input type="reset" value="Undo" name="reset" onClick="resetClick()"-->
      <input type="hidden" value="/vermod.asp" name="submit-url">




 </form>
 </DIV>
</blockquote>
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">
<h2><font color="#0000FF">Firmware Upgrade</font></h2>

<form action=/boaform/admin/formUpload method=POST enctype="multipart/form-data" name="password">
<table border="0" cellspacing="4" width="500">
 <tr><td><font size=2>
 ������ʹ�ñ�ҳ��Ϊ�豸����һ���µ�����汾����ע�⣬�������������벻Ҫ�ϵ磬�������ܻ�ʹϵͳ���� <!--This page allows you upgrade the firmware to the newer version. Please note that do not power off the device during the upload because this make the system unbootable. --> </font></td></tr>
 <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border="0" cellspacing="4" width="500">
  <tr>
      <td><font size=2><input type="file" value="Choose File" name="binary" size=20></td>
  </tr>
  </table>
    <p> <input type="submit" value="Upgrade" name="send" onclick="return sendClicked()">&nbsp;&nbsp;
        <input type="reset" value="Reset" name="reset">
    </p>
 </form>
  </DIV>
 </blockquote>

<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">
<h2><font color="#0000FF">OMCI</font></h2>
  <form action=/boaform/formExportOMCIlog method=POST name="exportOMCIlog">
  <tr>
    <td width="40%"><font size=2><b>Export:</b></font></td>
    <td width="30%"><font size=2>
      <input type="submit" value="Export" name="save_cs" onclick="return exportClick()">
    </font></tr>
  </form>

</DIV>
</blockquote>

 <blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">
  <form action=/boaform/formImportOMCIShell enctype="multipart/form-data" method=POST name="saveConfig">
  <tr>
    <td width="40%"><font size=2><b>Import:</b></font></td>
    <td width="30%"><font size=2><input type="file" value="Choose File" name="binary" size=24></font></td>
    <td width="20%"><font size=2><input type="submit" value="Import" name="load" onclick="return uploadClick()"></font></td>
  </tr>  
  </form> 

</DIV>
</blockquote>

 <blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">
  <form action=/boaform/formDisableLoidReg enctype="multipart/form-data" method=POST name="disable_register">
  <tr>
    <td width="40%"><font size=2><b>ȡ��LOIDע�ᵯ������ҳ�ص�������</b></font></td>
    <td width="20%"><font size=2><input type="submit" value="ȡ��" name="dis_reg"></font></td>
  </tr>  
  </form> 

</DIV>
</blockquote>

 <blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">
  <form action=/boaform/formDisableWanLimit enctype="multipart/form-data" method=POST name="disable_wanlimit">
  <tr>
    <td width="40%"><font size=2><b>ȡ������������������</b></font></td>
    <td width="20%"><font size=2><input type="submit" value="ȡ��" name="dis_wanlimit"></font></td>
  </tr>  
  </form> 

</DIV>
</blockquote>

</body>

</html>

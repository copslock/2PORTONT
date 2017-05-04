<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_VERSION); %> </title>
<script type="text/javascript" src="share.js">
</script>
<script>
function sendClicked()
{
        if (document.password.binary.value=="") {
                alert("<% multilang(LANG_SELECTED_FILE_CANNOT_BE_EMPTY); %>");
                document.password.binary.focus();
                return false;
        }

        if (!confirm('<% multilang(LANG_PAGE_DESC_UPGRADE_CONFIRM); %>'))
                return false;
        else
                return true;
}


function uploadClick()
{		
   	if (document.saveConfig.binary.value.length == 0) {
		alert('<% multilang(LANG_CHOOSE_FILE); %>!');
		document.saveConfig.binary.focus();
		return false;
	}
	return true;
}

function exportClick()
{		
   	
	alert('<% multilang(LANG_PAGE_DESC_WAIT_INFO); %>!');

	return true;
}


</script>

<STYLE type=text/css>
@import url(/style/default.css);
</STYLE>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">

<form action=/boaform/formVersionMod method=POST name="vermod">
	<h2><font color="#0000FF"><% multilang(LANG_VERSION); %></font></h2>
  
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
      <td width="150px">Spec. <% multilang(LANG_VERSION); %></td>
      <td><input type="text" name="txt_specver" size="50" maxlength="50" value=<% getInfo("rtk_specver"); %>>(1.0)</td>
  </tr>
  
  <tr>
      <td width="150px">Software <% multilang(LANG_VERSION); %></td>
      <td><input type="text" name="txt_swver" size="50" maxlength="50" value=<% getInfo("rtk_swver"); %>>(V2.30.10P16T2S)</td>
  </tr>
  
  <tr>
      <td width="150px">Hardware <% multilang(LANG_VERSION); %></td>
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
 </table>

      <input type="submit" value="Save" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
      <!--input type="reset" value="Undo" name="reset" onClick="resetClick()"-->
      <input type="hidden" value="/vermod.asp" name="submit-url">




 </form>
 </DIV>
</blockquote>
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">
<h2><font color="#0000FF"><% multilang(LANG_FIRMWARE_UPGRADE); %></font></h2>

<form action=/boaform/admin/formUpload method=POST enctype="multipart/form-data" name="password">
<table border="0" cellspacing="4" width="500">
 <tr><td><font size=2>
 <% multilang(LANG_PAGE_DESC_UPGRADE_FIRMWARE); %> </font></td></tr>
 <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border="0" cellspacing="4" width="500">
  <tr>
      <td><font size=2><input type="file" value="<% multilang(LANG_CHOOSE_FILE); %>" name="binary" size=20></td>
  </tr>
  </table>
    <p> <input type="submit" value="<% multilang(LANG_UPGRADE); %>" name="send" onclick="return sendClicked()">&nbsp;&nbsp;
        <input type="reset" value="<% multilang(LANG_RESET); %>" name="reset">
    </p>
 </form>
  </DIV>
 </blockquote>

<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">
<h2><font color="#0000FF"><% multilang(LANG_OMCI); %></font></h2>
  <form action=/boaform/formExportOMCIlog method=POST name="exportOMCIlog">
  <tr>
    <td width="40%"><font size=2><b><% multilang(LANG_EXPORT); %>:</b></font></td>
    <td width="30%"><font size=2>
      <input type="submit" value="<% multilang(LANG_EXPORT); %>" name="save_cs" onclick="return exportClick()">
    </font></tr>
  </form>

</DIV>
</blockquote>

 <blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px">
  <form action=/boaform/formImportOMCIShell enctype="multipart/form-data" method=POST name="saveConfig">
  <tr>
    <td width="40%"><font size=2><b><% multilang(LANG_IMPORT); %>:</b></font></td>
    <td width="30%"><font size=2><input type="file" value="<% multilang(LANG_CHOOSE_FILE); %>" name="binary" size=24></font></td>
    <td width="20%"><font size=2><input type="submit" value="<% multilang(LANG_IMPORT); %>" name="load" onclick="return uploadClick()"></font></td>
  </tr>  
  </form> 

</DIV>
</blockquote>

</body>

</html>

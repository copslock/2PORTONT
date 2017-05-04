<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title>IP/Port <% multilang(LANG_FILTERING); %></title>
<script type="text/javascript" src="share.js">
</script>
<script>
function skip () { this.blur(); }

function protocolSelection()
{
	if ( document.formFilterAdd.protocol.selectedIndex == 2 )
	{
		document.formFilterAdd.sfromPort.disabled = true;
		document.formFilterAdd.stoPort.disabled = true;
		document.formFilterAdd.dfromPort.disabled = true;
		document.formFilterAdd.dtoPort.disabled = true;
	}
	else
	{
		document.formFilterAdd.sfromPort.disabled = false;
		document.formFilterAdd.stoPort.disabled = false;
		document.formFilterAdd.dfromPort.disabled = false;
		document.formFilterAdd.dtoPort.disabled = false;
	}
}

function addClick()
{
	if (document.formFilterAdd.sip.value == "" && document.formFilterAdd.smask.value == ""
		&& document.formFilterAdd.dip.value == "" && document.formFilterAdd.dmask.value == ""
		&& document.formFilterAdd.sfromPort.value == "" && document.formFilterAdd.dfromPort.value == "") {		
		alert('<% multilang(LANG_FILTER_RULES_CAN_NOT_BE_EMPTY); %>');
		document.formFilterAdd.sip.focus();
		return false;
	}

	//var i;

	if (document.formFilterAdd.sip.value!="") {
		if (!checkHostIP(document.formFilterAdd.sip, 0))
			return false;
		if ( document.formFilterAdd.smask.value != "" ) {
			if (!checkNetmask(document.formFilterAdd.smask, 0))
				return false;
		}
		/*if ( validateKey( document.formFilterAdd.sip.value ) == 0 ) {
			alert("Invalid source IP address.");
			document.formFilterAdd.sip.focus();
			return false;
		}
		if( IsLoopBackIP( document.formFilterAdd.sip.value)==1 ) {
			alert("Invalid Source IP address value.");
			document.formFilterAdd.sip.focus();
			return false;
  		}
		for (i=1; i<=4; i++) {
			if ( i == 1 ) {
				if ( !checkDigitRange(document.formFilterAdd.sip.value,i,0,223) ) {
					alert('Invalid source IP address');
					document.formFilterAdd.sip.focus();
					return false;
				}
			}else {
				if ( !checkDigitRange(document.formFilterAdd.sip.value,i,0,255) ) {
					alert('Invalid source IP address');
					document.formFilterAdd.sip.focus();
					return false;
				}
			}
		}

		if ( document.formFilterAdd.smask.value!="" ) {
			if ( validateKey( document.formFilterAdd.smask.value ) == 0 ) {
				alert("Invalid source IP mask.");
				document.formFilterAdd.smask.focus();
				return false;
			}
			for (i=1; i<=4; i++) {
				if ( !checkDigitRange(document.formFilterAdd.smask.value,i,0,255) ) {
					alert('Invalid source IP mask.');
					document.formFilterAdd.smask.focus();
					return false;
				}
			}
		}*/
	}

	if (document.formFilterAdd.dip.value!="") {
		if (!checkHostIP(document.formFilterAdd.dip, 0))
			return false;
		if ( document.formFilterAdd.dmask.value != "" ) {
			if (!checkNetmask(document.formFilterAdd.dmask, 0))
				return false;
		}
		/*if ( validateKey( document.formFilterAdd.dip.value ) == 0 ) {
			alert("Invalid destination IP address.");
			document.formFilterAdd.dip.focus();
			return false;
		}
		if( IsLoopBackIP( document.formFilterAdd.dip.value)==1 ) {
			alert("Invalid destination IP address value.");
			document.formFilterAdd.dip.focus();
			return false;
  		}
		for (i=1; i<=4; i++) {
			if ( i == 1 ) {
				if ( !checkDigitRange(document.formFilterAdd.dip.value,i,0,223) ) {
					alert('Invalid destination IP address');
					document.formFilterAdd.dip.focus();
					return false;
				}
			} else {
				if ( !checkDigitRange(document.formFilterAdd.dip.value,i,0,255) ) {
					alert('Invalid destination IP address');
					document.formFilterAdd.dip.focus();
					return false;
				}
			}
		}

		if ( document.formFilterAdd.dmask.value!="" ) {
			if ( validateKey( document.formFilterAdd.dmask.value ) == 0 ) {
				alert("Invalid destination IP mask.");
				document.formFilterAdd.dmask.focus();
				return false;
			}
			for (i=1; i<=4; i++) {
				if ( !checkDigitRange(document.formFilterAdd.dmask.value,i,0,255) ) {
					alert('Invalid destination IP mask.');
					document.formFilterAdd.dmask.focus();
					return false;
				}
			}
		}*/
	}

	if ( document.formFilterAdd.sfromPort.value!="" ) {
		if ( validateKey( document.formFilterAdd.sfromPort.value ) == 0 ) {
			alert('<% multilang(LANG_INVALID_SOURCE_PORT); %>');
			document.formFilterAdd.sfromPort.focus();
			return false;
		}

		d1 = getDigit(document.formFilterAdd.sfromPort.value, 1);
		if (d1 > 65535 || d1 < 1) {
			alert('<% multilang(LANG_INVALID_SOURCE_PORT_NUMBER); %>');
			document.formFilterAdd.sfromPort.focus();
			return false;
		}

		if ( document.formFilterAdd.stoPort.value!="" ) {
			if ( validateKey( document.formFilterAdd.stoPort.value ) == 0 ) {
				alert('<% multilang(LANG_INVALID_SOURCE_PORT); %>');
				document.formFilterAdd.stoPort.focus();
				return false;
			}

			d1 = getDigit(document.formFilterAdd.stoPort.value, 1);
			if (d1 > 65535 || d1 < 1) {
				alert('<% multilang(LANG_INVALID_SOURCE_PORT_NUMBER); %>');
				document.formFilterAdd.stoPort.focus();
				return false;
			}
		}
	}

	if ( document.formFilterAdd.dfromPort.value!="" ) {
		if ( validateKey( document.formFilterAdd.dfromPort.value ) == 0 ) {
			alert('<% multilang(LANG_INVALID_DESTINATION_PORT); %>');
			document.formFilterAdd.dfromPort.focus();
			return false;
		}

		d1 = getDigit(document.formFilterAdd.dfromPort.value, 1);
		if (d1 > 65535 || d1 < 1) {
			alert('<% multilang(LANG_INVALID_DESTINATION_PORT_NUMBER); %>');
			document.formFilterAdd.dfromPort.focus();
			return false;
		}

		if ( document.formFilterAdd.dtoPort.value!="" ) {
			if ( validateKey( document.formFilterAdd.dtoPort.value ) == 0 ) {
				alert('<% multilang(LANG_INVALID_DESTINATION_PORT); %>');
				document.formFilterAdd.dtoPort.focus();
				return false;
			}

			d1 = getDigit(document.formFilterAdd.dtoPort.value, 1);
			if (d1 > 65535 || d1 < 1) {
				alert('<% multilang(LANG_INVALID_DESTINATION_PORT_NUMBER); %>');
				document.formFilterAdd.dtoPort.focus();
				return false;
			}
		}
	}

	return true;
}

function disableDelButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.formFilterDel.deleteSelFilterIpPort);
	disableButton(document.formFilterDel.deleteAllFilterIpPort);
  }
}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">IP/Port <% multilang(LANG_FILTERING); %></font></h2>

<table border=0 width="600" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
 <% multilang(LANG_PAGE_DESC_DATA_PACKET_FILTER_TABLE); %>
</font></td></tr>

<tr <% checkWrite("rg_hidden_function"); %>><td><hr size=1 noshade align=top></td></tr>

<form action=/boaform/formFilter method=POST name="formFilterDefault">
<tr <% checkWrite("rg_hidden_function"); %>><td><font size=2><b><% multilang(LANG_OUTGOING_DEFAULT_ACTION); %></b>&nbsp;&nbsp;
   	<input type="radio" name="outAct" value=0 <% checkWrite("ipf_out_act0"); %>><% multilang(LANG_DENY); %>&nbsp;&nbsp;
   	<input type="radio" name="outAct" value=1 <% checkWrite("ipf_out_act1"); %>><% multilang(LANG_ALLOW); %>&nbsp;&nbsp;
</font></td><tr>
<tr <% checkWrite("rg_hidden_function"); %>><td><font size=2><b><% multilang(LANG_INCOMING_DEFAULT_ACTION); %></b>&nbsp;&nbsp;
   	<input type="radio" name="inAct" value=0 <% checkWrite("ipf_in_act0"); %>><% multilang(LANG_DENY); %>&nbsp;&nbsp;
   	<input type="radio" name="inAct" value=1 <% checkWrite("ipf_in_act1"); %>><% multilang(LANG_ALLOW); %>&nbsp;&nbsp;
	<input type="submit" value="<% multilang(LANG_APPLY_CHANGES); %>" name="setDefaultAction">&nbsp;&nbsp;
	<input type="hidden" value="/fw-ipportfilter.asp" name="submit-url">
</font></td></tr>
</form>
</table>
<table border=0 width="600" cellspacing=0 cellpadding=0>
<tr><td><hr size=1 noshade align=top></td></tr>
<form action=/boaform/formFilter method=POST name="formFilterAdd">
<tr>
	<td width="600"><font size=2>
	<b><% multilang(LANG_DIRECTION); %>: </b>
		<select name=dir>
			<option select value=0><% multilang(LANG_OUTGOING); %></option>
			<option value=1><% multilang(LANG_INCOMING); %></option>
		</select>&nbsp;&nbsp;
	<b><% multilang(LANG_PROTOCOL); %>: </b>
		<select name="protocol" onChange="protocolSelection()">
			<option select value=1>TCP</option>
			<option value=2>UDP</option>
			<option value=3>ICMP</option>
		</select>&nbsp;&nbsp;
	<b><% multilang(LANG_RULE_ACTION); %></b>
   		<input type="radio" name="filterMode" value="Deny" checked>&nbsp;<% multilang(LANG_DENY); %>
   		<input type="radio" name="filterMode" value="Allow">&nbsp;&nbsp;<% multilang(LANG_ALLOW); %>
	</font></td>
</tr>
<tr>
	<td>
	<font size=2><b><% multilang(LANG_SOURCE); %> <% multilang(LANG_IP_ADDRESS); %>: </b> <input type="text" name="sip" size="10" maxlength="15">&nbsp;&nbsp;
	<b><% multilang(LANG_SUBNET_MASK); %>: </b> <input type="text" name="smask" size="10" maxlength="15">&nbsp;&nbsp;
	<b><font size=2><% multilang(LANG_PORT); %>: </b></font>
		<input type="text" name="sfromPort" size="4" maxlength="5"><b>-</b>
	 	<input type="text" name="stoPort" size="4" maxlength="5"></font>&nbsp;&nbsp;
	</td>
</tr>
<tr>
	<td>
	<font size=2><b><% multilang(LANG_DESTINATION); %> <% multilang(LANG_IP_ADDRESS); %>: </b> <input type="text" name="dip" size="10" maxlength="15">&nbsp;&nbsp;
	<b><% multilang(LANG_SUBNET_MASK); %>: </b> <input type="text" name="dmask" size="10" maxlength="15">&nbsp;&nbsp;
	<b><font size=2><% multilang(LANG_PORT); %>: </b></font>
		<input type="text" name="dfromPort" size="4" maxlength="5"><b>-</b>
		<input type="text" name="dtoPort" size="4" maxlength="5"></font>&nbsp;&nbsp;
	</td>
</tr>
<tr>
	<td>
	<input type="submit" value="<% multilang(LANG_ADD); %>" name="addFilterIpPort" onClick="return addClick()">
	<input type="hidden" value="/fw-ipportfilter.asp" name="submit-url">
	</td>
</tr>
<tr><td><hr size=1 noshade align=top></td></tr>
</form>
</table>

<form action=/boaform/formFilter method=POST name="formFilterDel">
	<table border="0" width=600>
	<tr><font size=2><b><% multilang(LANG_CURRENT_FILTER_TABLE); %>:</b></font></tr>
	<% ipPortFilterList(); %>
	</table>
	<br>
	<input type="submit" value="<% multilang(LANG_DELETE_SELECTED); %>" name="deleteSelFilterIpPort" onClick="return deleteClick()">&nbsp;&nbsp;
	<input type="submit" value="<% multilang(LANG_DELETE_ALL); %>" name="deleteAllFilterIpPort" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
	<input type="hidden" value="/fw-ipportfilter.asp" name="submit-url">
	<script>
		<% checkWrite("ipFilterNum"); %>
	</script>
</form>

</blockquote>
</body>
</html>

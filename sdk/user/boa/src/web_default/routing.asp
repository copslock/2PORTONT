<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang(LANG_ROUTING); %><% multilang(LANG_CONFIGURATION); %></title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>

function postGW( enable, destNet, subMask, nextHop, metric, interface, select )
{
	document.route.enable.checked = enable;
	document.route.destNet.value=destNet;
	document.route.subMask.value=subMask;
	document.route.nextHop.value=nextHop;
	document.route.metric.value=metric;
	document.route.interface.value=interface;	
	document.route.select_id.value=select;	
}

function checkDest(ip, mask)
{
	var i, dip, dmask, nip;

	for (i=1; i<=4; i++) {
		dip = getDigit(ip.value, i);
		dmask = getDigit(mask.value,  i);
		nip = dip & dmask;
		if (nip != dip)
			return true;
	}
	return false;
}

function addClick()
{
	/*if (document.route.destNet.value=="") {
		alert("Enter Destination Network ID !");
		document.route.destNet.focus();
		return false;
	}
	
	if ( validateKey( document.route.destNet.value ) == 0 ) {
		alert("Invalid Destination value.");
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,1,0,255) ) {
		alert('Invalid Destination range in 1st digit. It should be 0-255.');
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,2,0,255) ) {
		alert('Invalid Destination range in 2nd digit. It should be 0-255.');
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,3,0,255) ) {
		alert('Invalid Destination range in 3rd digit. It should be 0-255.');
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,4,0,254) ) {
		alert('Invalid Destination range in 4th digit. It should be 0-254.');
		document.route.destNet.focus();
		return false;
	}
	
	if (document.route.subMask.value=="") {
		alert("Enter Subnet Mask !");
		document.route.subMask.focus();
		return false;
	}
	
	if ( validateKey( document.route.subMask.value ) == 0 ) {
		alert("Invalid Subnet Mask value.");
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,1,0,255) ) {
		alert('Invalid Subnet Mask range in 1st digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,2,0,255) ) {
		alert('Invalid Subnet Mask range in 2nd digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,3,0,255) ) {
		alert('Invalid Subnet Mask range in 3rd digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,4,0,255) ) {
		alert('Invalid Subnet Mask range in 4th digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if (document.route.interface.value==65535) {
	if (document.route.nextHop.value=="" ) {
		alert("Enter Next Hop IP or select a GW interface!");
		document.route.nextHop.focus();
		return false;
	}
	
	if ( validateKey( document.route.nextHop.value ) == 0 ) {
		alert("Invalid Next Hop value.");
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,1,0,255) ) {
		alert('Invalid Next Hop range in 1st digit. It should be 0-255.');
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,2,0,255) ) {
		alert('Invalid Next Hop range in 2nd digit. It should be 0-255.');
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,3,0,255) ) {
		alert('Invalid Next Hop range in 3rd digit. It should be 0-255.');
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,4,1,254) ) {
		alert('Invalid Next Hop range in 4th digit. It should be 1-254.');
		document.route.nextHop.focus();
		return false;
	}*/
	if (checkDest(document.route.destNet, document.route.subMask) == true) {
		alert("<% multilang(LANG_THE_SPECIFIED_MASK_PARAMETER_IS_INVALID_DESTINATION_MASK_DESTINATION); %>");
		document.route.subMask.focus();
		return false;
	}
	if (!checkHostIP(document.route.destNet, 1))
		return false;
	if (!checkNetmask(document.route.subMask, 1))
		return false;
	if (document.route.interface.value==65535) {
		if (document.route.nextHop.value=="" ) {
			alert("<% multilang(LANG_ENTER_NEXT_HOP_IP_OR_SELECT_A_GW_INTERFACE); %>");
			document.route.nextHop.focus();
			return false;
		}

		if (!checkHostIP(document.route.nextHop, 0))
			return false;
	}
	if ( !checkDigitRange(document.route.metric.value,1,0,16) ) {
		alert("<% multilang(LANG_INVALID_METRIC_RANGE_IT_SHOULD_BE_0_16); %>");
		document.route.metric.focus();
		return false;
	}
	
	return true;
}

function routeClick(url)
{
	var wide=600;
	var high=400;
	if (document.all)
		var xMax = screen.width, yMax = screen.height;
	else if (document.layers)
		var xMax = window.outerWidth, yMax = window.outerHeight;
	else
	   var xMax = 640, yMax=480;
	var xOffset = (xMax - wide)/2;
	var yOffset = (yMax - high)/3;

	var settings = 'width='+wide+',height='+high+',screenX='+xOffset+',screenY='+yOffset+',top='+yOffset+',left='+xOffset+', resizable=yes, toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes';

	window.open( url, 'RouteTbl', settings );
}
	
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF"><% multilang(LANG_ROUTING); %><% multilang(LANG_CONFIGURATION); %></font></h2>

<form action=/boaform/formRouting method=POST name="route">
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><td><font size=2>
    <% multilang(LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_ROUTING_INFORMATION_HERE_YOU_CAN_ADD_DELETE_IP_ROUTES); %>
  </font></td></tr>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_ENABLE); %>:</b></td>
      <td width="70%"><input type="checkbox" name="enable" value="1" checked></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_DESTINATION); %>:</b></td>
      <td width="70%"><input type="text" name="destNet" size="15" maxlength="15"></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_SUBNET_MASK); %>:</b></td>
      <td width="70%"><input type="text" name="subMask" size="15" maxlength="15"></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_NEXT_HOP); %>:</b></td>
      <td width="70%"><input type="text" name="nextHop" size="15" maxlength="15"></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_METRIC); %>:</b></td>
      <td width="70%"><input type="text" name="metric" size="5" maxlength="5"></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b><% multilang(LANG_INTERFACE); %>:</b></td>
      <td width="70%"><select name="interface">
          <% if_wan_list("rt-any-vpn"); %>
      	</select></td>
  </tr>
  <input type="hidden" value="" name="select_id">
</table>
  <input type="submit" value="<% multilang(LANG_ADD_ROUTE); %>" name="addRoute" onClick="return addClick()">&nbsp;&nbsp;
  <input type="submit" value="<% multilang(LANG_UPDATE); %>" name="updateRoute" onClick="return addClick()">&nbsp;&nbsp;
  <input type="submit" value="<% multilang(LANG_DELETE_SELECTED); %>" name="delRoute" onClick="return deleteClick()">&nbsp;&nbsp;
  <input type="button" value="<% multilang(LANG_SHOW_ROUTES); %>" name="showRoute" onClick="routeClick('/routetbl.asp')">
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><td><hr size=1 noshade align=top></td></tr>
</table>
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><font size=2><b><% multilang(LANG_STATIC_ROUTE_TABLE); %>:</b></font></tr>
  <% showStaticRoute(); %>
</table>
  <br>
      <input type="hidden" value="/routing.asp" name="submit-url">
</form>
</blockquote>
</body>

</html>

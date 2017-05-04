<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
<title><% multilang("223" "LANG_WLAN_STATUS"); %></title>

<script>
var wlanmode, wlanclientnum;
</script>

</head>
<body>
<blockquote>

<h2><b><font color="#0000FF"><% multilang("223" "LANG_WLAN_STATUS"); %></font></b></h2>

<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
 <% multilang("224" "LANG_THIS_PAGE_SHOWS_THE_WLAN_CURRENT_STATUS"); %>
</font></td></tr>

<tr><td><hr size=1 noshade align=top><br></td></tr>
</table>


<table width=400 border=0>
  <script>
 var wlan_num = 1;
 var isNewMeshUI = 0;
 var wlanMode =new Array();
 var networkType =new Array();
 var band=new Array();
 var ssid_drv=new Array();
 var channel_drv=new Array();
 var wep=new Array();
 var wdsEncrypt=new Array();
 var meshEncrypt=new Array();
 var bssid_drv=new Array();
 var clientnum=new Array();
 var state_drv=new Array();
 var rp_enabled=new Array();
 var rp_mode=new Array();
 var rp_encrypt=new Array();
 var rp_clientnum=new Array();
 var rp_ssid=new Array();
 var rp_bssid=new Array();
 var rp_state=new Array();
 var wlanDisabled=new Array();

 //var mssid_num=4;
 var mssid_num;
 var mssid_disable=new Array();
 var mssid_bssid_drv=new Array();
 var mssid_clientnum=new Array();
 var mssid_band=new Array();
 var mssid_ssid_drv=new Array();
 var mssid_wep=new Array();
 var Band2G5GSupport;
 var is_wlan_qtn = <% checkWrite("is_wlan_qtn"); %>;

 <% wlStatus_parm(); %>
 if(ssid_drv[0]=="")
  mssid_num=0;

 for(i=0; i < wlan_num; i++){
 document.write(' <tr bgcolor="#EEEEEE"> <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2><b><% multilang("8" "LANG_WLAN"); %> <% multilang("212" "LANG_CONFIGURATION"); %></b></font></td> </tr> <tr bgcolor="#EEEEEE"> <td width=40%><font size=2><b><% multilang("125" "LANG_MODE"); %></b></td> <td width=60%><font size=2>' );







 /* mode */
 if(wlanMode[i] == 0)
  document.write("AP");
 if(wlanMode[i] == 1){
  if (networkType[0] == 0)
           document.write( "Infrastructure Client");
  else
   document.write( "Ad-hoc Client");
   }
   if ( wlanMode[i] == 2 )
          document.write( "WDS");
   if ( wlanMode[i] == 3 )
          document.write( "AP+WDS");
/*#ifdef CONFIG_NEW_MESH_UI*/
if( isNewMeshUI ==1 )
{
 if ( wlanMode[i] == 4 )
          document.write( "AP+MESH");
    if ( wlanMode[i] == 5 )
         document.write( "MESH");
}
else
{
    if ( wlanMode[i] == 4 )
          document.write( "AP+MPP");
    if ( wlanMode[i] == 5 )
         document.write( "MPP");
    if ( wlanMode[i] == 6 )
          document.write( "MAP");
    if ( wlanMode[i] == 7 )
          document.write( "MP");
}
 /* band */
 document.write('</td> <tr bgcolor="#DDDDDD"> <td width=40%><font size=2><b><% multilang("124" "LANG_BAND"); %></b></td> <td width=60%><font size=2>');



 if (band[i] == 1)
     document.write( "2.4 GHz (B)");
    if (band[i] == 2)
     document.write( "2.4 GHz (G)");
    if (band[i] == 3)
     document.write( "2.4 GHz (B+G)");
    if (band[i] == 4)
     document.write( "5 GHz (A)");
    if (band[i] == 8) {
     if(Band2G5GSupport == 1) //PHYBAND_2G
      document.write( "2.4 GHz (N)");
     else if(Band2G5GSupport == 2) //PHYBAND_5G
      document.write( "5 GHz (N)");
    }
    if (band[i] == 10)
     document.write( "2.4 GHz (G+N)");
    if (band[i] == 11)
     document.write( "2.4 GHz (B+G+N)");
    if (band[i] == 12)
     document.write( "5 GHz (A+N)");
 if (band[i] == 64)
  document.write( "5 GHz (AC)");
 if (band[i] == 72)
  document.write( "5 GHz (N+AC)");
    if (band[i] == 76)
     document.write( "5 GHz (A+N+AC)");

 document.write('</tr> <tr bgcolor="#EEEEEE"> <td width=40%><font size=2><b><% multilang("126" "LANG_SSID"); %></b></td> <td width=60%><font size=2>');



 if (wlanMode[i] != 2) {
  document.write('<pre><font size=2>');
  document.write(ssid_drv[i]);
 }
 document.write('</td> </tr> <tr bgcolor="#DDDDDD"> <td width=40%><font size=2><b><% multilang("129" "LANG_CHANNEL_NUMBER"); %></b></td> <td width=60%><font size=2>'+channel_drv[i] +'</td> </tr> <tr bgcolor="#EEEEEE"> <td width=40%><font size=2><b><% multilang("175" "LANG_ENCRYPTION"); %></b></td> <td width=60%><font size=2>');
 if (wlanMode[i] == 0 || wlanMode[i] == 1)
      document.write(wep[i]);
     else if (wlanMode[i] == 2)
      document.write(wdsEncrypt[i]);
     else if (wlanMode[i] == 3)
      document.write(wep[i] + '(AP), ' + wdsEncrypt[i] + '(WDS)');
     else if (wlanMode[i] == 4 || wlanMode[i] == 6)
      document.write(wep[i] + '(AP), ' + meshEncrypt[i] + '(Mesh)');
     else if (wlanMode[i] == 5|| wlanMode[i] == 7)
      document.write( meshEncrypt[i] + '(Mesh)');
 document.write('</td> </tr> <tr bgcolor="#DDDDDD"> <td width=40%><font size=2><b><% multilang("1121" "LANG_BSSID"); %></b></td> <td width=60%><font size=2>'+bssid_drv[i]+'</td> </tr>');
 if (wlanMode[i]!=2) { //2 means WDS mode
  document.write('<tr bgcolor="#EEEEEE">\n');
  if (wlanMode[i]==0 || wlanMode[i]==3 || wlanMode[i]==4) {
   document.write("<td width=40%%><font size=2><b><% multilang("132" "LANG_ASSOCIATED_CLIENTS"); %></b></td>\n");
   document.write("<td width=60%%><font size=2>"+clientnum[i]+"</td></tr>");
  }
  else {
   document.write("<td width=40%%><font size=2><b><% multilang("225" "LANG_STATE"); %></b></td>\n");
   document.write('<td width=60%%><font size=2>'+state_drv[i]+'</td></tr>');
  }
        }
 /* mesh does not support virtual ap */
 if (!wlanDisabled[i] && (wlanMode[i]==0 || wlanMode[i]==3 )) {
  for (idx=0; idx<mssid_num; idx++) {
   if (!mssid_disable[idx]) {
    document.write(' <tr bgcolor="#EEEEEE"> <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2> <b><% multilang("226" "LANG_VIRTUAL_AP"); %>'+(idx+1)+' <% multilang("212" "LANG_CONFIGURATION"); %></b></font></td> </tr>');
    if(!is_wlan_qtn){
     document.write('<tr bgcolor="#EEEEEE"> <td width=40%><font size=2><b><% multilang("124" "LANG_BAND"); %></b></td> <td width=60%><font size=2>');
     if (mssid_band[idx] == 1)
         document.write( "2.4 GHz (B)");
        if (mssid_band[idx] == 2)
         document.write( "2.4 GHz (G)");
        if (mssid_band[idx] == 3)
         document.write( "2.4 GHz (B+G)");
        if (mssid_band[idx] == 4)
         document.write( "5 GHz (A)");
        if (mssid_band[idx] == 8) {
         if(Band2G5GSupport == 1) //PHYBAND_2G
          document.write( "2.4 GHz (N)");
         else if(Band2G5GSupport == 2) //PHYBAND_5G
          document.write( "5 GHz (N)");
        }
        if (mssid_band[idx] == 10)
         document.write( "2.4 GHz (G+N)");
        if (mssid_band[idx] == 11)
         document.write( "2.4 GHz (B+G+N)");
        if (mssid_band[idx] == 12)
         document.write( "5 GHz (A+N)");
     if (mssid_band[idx] == 64)
      document.write( "5 GHz (AC)");
     if (mssid_band[idx] == 72)
      document.write( "5 GHz (N+AC)");
        if (mssid_band[idx]== 76)
         document.write( "5 GHz (A+N+AC)");
     document.write('</td></tr>');
    }
    document.write('<tr bgcolor="#DDDDDD"> <td width=40%><font size=2><b>SSID</b></td> <td width=60%><font size=2>');
    document.write('<pre><font size=2>'+mssid_ssid_drv[idx]+'</td>');
    document.write('</tr> <tr bgcolor="#EEEEEE"> <td width=40%><font size=2><b><% multilang("175" "LANG_ENCRYPTION"); %></b></td> <td width=60%><font size=2>'+mssid_wep[idx]+'</td>');
    document.write('</tr> <tr bgcolor="#DDDDDD"> <td width=40%><font size=2><b>BSSID</b></td> <td width=60%><font size=2>'+mssid_bssid_drv[idx]+'</td>');
    document.write('</tr> <tr bgcolor="#EEEEEE"> <td width=40%%><font size=2><b><% multilang("132" "LANG_ASSOCIATED_CLIENTS"); %></b></td> <td width=60%%><font size=2>'+mssid_clientnum[idx]+'</td></tr>');
   }
  }
 }
    if (rp_enabled[i]) // start of repeater
    {
     document.write(' <tr bgcolor="#EEEEEE"> <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2><b><% multilang("8" "LANG_WLAN"); %> <% multilang("227" "LANG_REPEATER_INTERFACE"); %><% multilang("212" "LANG_CONFIGURATION"); %></b></font></td> </tr> <tr bgcolor="#EEEEEE"> <td width=40%><font size=2><b><% multilang("125" "LANG_MODE"); %></b></td> <td width=60%><font size=2>' );
     /* mode */
     if(rp_mode[i] == 0)
      document.write("AP");
     else
      document.write( "Infrastructure Client");
     document.write('</td> </tr> <tr bgcolor="#DDDDDD"> <td width=40%><font size=2><b>SSID</b></td> <td width=60%><font size=2>'+rp_ssid[i] +'</td> </tr> <tr bgcolor="#EEEEEE"> <td width=40%><font size=2><b><% multilang("175" "LANG_ENCRYPTION"); %></b></td> <td width=60%><font size=2>'+rp_encrypt[i] +'</td> </tr> <tr bgcolor="#DDDDDD"> <td width=40%><font size=2><b>BSSID</b></td> <td width=60%><font size=2>'+rp_bssid[i] +'</td> </tr>');
     document.write('<tr bgcolor="#EEEEEE">\n');
     if (rp_mode[i]==0 || rp_mode[i]==3) {
      document.write("<td width=40%%><font size=2><b><% multilang("132" "LANG_ASSOCIATED_CLIENTS"); %></b></td>\n");
      document.write("<td width=60%%><font size=2>"+rp_clientnum[i]+"</td></tr>");
     }
     else {
      document.write("<td width=40%%><font size=2><b><% multilang("225" "LANG_STATE"); %></b></td>\n");
      document.write('<td width=60%%><font size=2>'+rp_state[i]+'</td></tr>');
     }
 } // end of repeater
   }//end of wlan_num for
  </script>
</table>
</blockquote>
</body>
</html>

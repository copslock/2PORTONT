<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--ϵͳĬ��ģ��-->
<HTML>
<HEAD>
<TITLE>IGMP Snooping ����</TITLE>
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
/*
var cgi = new Object();
<%initPageIgmpSnooping();%>
*/

/********************************************************************
**          on document load
********************************************************************/
function on_init()
{
	sji_docinit(document, cgi);
}

</SCRIPT>
</HEAD>

<!-------------------------------------------------------------------------------------->
<!--��ҳ����-->
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<DIV align="left" style="padding-left:20px; padding-top:5px">
			<form id="form" action=/boaform/formIgmpSnooping method=POST name="igmpsnoop">
				<b>IGMP Snooping ����</b><br><br>
				���ҳ�����������û��߽���IGMP Snooping����<br><br>
			
				<hr align="left" class="sep" size="1" width="90%">				
				<table border=0 width="500" cellspacing=4 cellpadding=0>
					<tr><td width=150>IGMP Snooping:</td>
						<td width=350>
						<input type="radio" name=snoop value=0>����&nbsp;&nbsp;
						<input type="radio" name=snoop value=1>����</td>
				</tr></table>
				<br>
				<input type="submit" class="button" name="apply" value="����/Ӧ��"> 
				<input type="hidden" name="submit-url" value="/app_igmp_snooping.asp">
			<script>
				<% initPage("igmpsnooping"); %>	
			</script>			
			</form>
		</DIV>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

<!-- add by liuxiao 2008-01-21 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>�й�����</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<meta http-equiv="refresh" content="5">
<meta http-equiv=content-script-type content=text/javascript>
<style type=text/css>
@import url(/style/default.css);
</style>
<script language="javascript" src="common.js"></script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>DSL��Ϣ</b></div>
			<div align="left" style="width:768px;"><br>
			<table class="flat" border="1" cellpadding="0" cellspacing="1">
				<tr>
					<td width="20%" class="hdb">DSL�ĵ�ǰͬ��״̬:</td>
					<td><% getInfo("syncState"); %></td>
				</tr>
				<tr>
					<td class="hdb">����״̬:</td>
					<td><% getInfo("linkState"); %></td>
				</tr>
				<tr>
					<td class="hdb">��������:</td>
					<td><% getInfo("upRate"); %>&nbsp;kbps</td>
				</tr>
				<tr>
					<td class="hdb">��������:</td>
					<td><% getInfo("downRate"); %>&nbsp;kbps</td>
				</tr>
				<tr>
					<td class="hdb">����ԣ��:</td>
					<td><% getInfo("vsnrMargin"); %>&nbsp;dB</td>
				</tr>
				<tr>
					<td class="hdb">��֯���:</td>
					<td><% getInfo("vpmsPramD"); %></td>
				</tr>
				<tr>
					<td class="hdb">��·Э��:</td>
					<td><% getInfo("lineProto"); %></td>
				</tr>
				<tr>
					<td class="hdb">��·˥��:</td>
					<td><% getInfo("attenuation"); %>&nbsp;dB</td>
				</tr>
				<tr>
					<td class="hdb">�������:</td>
					<td><% getInfo("outputPower"); %>&nbsp;dBm</td>
				</tr>
				<tr>
					<td class="hdb">�ŵ�����:</td>
					<td><% getInfo("pmsLatency"); %></td>
				</tr>
			</table>
			</div>
		</div>
	</blockquote>
</body>
<!-- add end by liuxiao 2008-01-21 -->
<%addHttpNoCache();%>
</html>

<!-- add by liuxiao 2008-01-16 -->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>�й�����</title>
<meta http-equiv=pragma content=no-cache>
<meta http-equiv=cache-control content="no-cache, must-revalidate">
<meta http-equiv=content-type content="text/html; charset=gbk">
<style type=text/css>
@import url(/style/default.css);
</style>
<script language="javascript" src="common.js"></script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000">
	<blockquote>
		<div align="left" style="padding-left:20px;"><br>
			<div align="left"><b>������Ϣ</b></div>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td class="hdb">����״̬</td>
					<td><% showgpon_status(); %></td>
				</tr>
				<tr <% checkWrite("priv"); %>>
					<td class="hdb">FEC ����</td>
					<td>֧��</td>
				</tr>
				<tr <% checkWrite("priv"); %>>
					<td class="hdb">FEC ����״̬</td>
					<td><% ponGetStatus("gpon-fec-us-state"); %></td>
				</tr>
				<tr <% checkWrite("priv"); %>>
					<td class="hdb">FEC ����״̬</td>
					<td><% ponGetStatus("gpon-fec-ds-state"); %></td>
				</tr>
				<tr <% checkWrite("priv"); %>>
					<td class="hdb">����ģʽ</td>
					<td><% ponGetStatus("gpon-encryption"); %></td>
				</tr>
			</table>
			<br>

			<span <% checkWrite("priv"); %>>
			<div align="left"><b>��·����ͳ����Ϣ</b></div>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td class="hdb">�����ֽ�</td>
					<td><% ponGetStatus("bytes-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">�����ֽ�</td>
					<td><% ponGetStatus("bytes-received"); %></td>
				</tr>
				<tr>
					<td class="hdb">����֡</td>
					<td><% ponGetStatus("packets-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">����֡</td>
					<td><% ponGetStatus("packets-received"); %></td>
				</tr>
				<tr>
					<td class="hdb">���͵���֡</td>
					<td><% ponGetStatus("unicast-packets-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">���յ���֡</td>
					<td><% ponGetStatus("unicast-packets-received"); %></td>
				</tr>
				<tr>
					<td class="hdb">�����鲥֡</td>
					<td><% ponGetStatus("multicast-packets-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">�����鲥֡</td>
					<td><% ponGetStatus("multicast-packets-received"); %></td>
				</tr>
				<tr>
					<td class="hdb">���͹㲥֡</td>
					<td><% ponGetStatus("broadcast-packets-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">���չ㲥֡</td>
					<td><% ponGetStatus("broadcast-packets-received"); %></td>
				</tr>
				<tr>
					<td class="hdb">���� FEC ����֡</td>
					<td><% ponGetStatus("fec-errors"); %></td>
				</tr>
				<tr>
					<td class="hdb">���� HEC ����֡</td>
					<td><% ponGetStatus("hec-errors"); %></td>
				</tr>
				<tr>
					<td class="hdb">���Ͷ�ʧ֡</td>
					<td><% ponGetStatus("packets-dropped"); %></td>
				</tr>
				<tr>
					<td class="hdb">���� PAUSE ������֡</td>
					<td><% ponGetStatus("pause-packets-sent"); %></td>
				</tr>
				<tr>
					<td class="hdb">���� PAUSE ������֡</td>
					<td><% ponGetStatus("pause-packets-received"); %></td>
				</tr>
			</table>
			<br>

			<div align="left"><b>�澯��Ϣ</b></div>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td class="hdb">GPON �澯��Ϣ</td>
					<td><% ponGetStatus("gpon-alarm"); %></td>
				</tr>
			</table>
			<br>

			<div align="left"><b>��ģ����Ϣ</b></div>
			<table class="flat" border="1" cellpadding="1" cellspacing="1">
				<tr>
					<td class="hdb">����⹦��</td>
					<td><% ponGetStatus("tx-power"); %></td>
				</tr>
				<tr>
					<td class="hdb">���չ⹦��</td>
					<td><% ponGetStatus("rx-power"); %></td>
				</tr>
				<tr>
					<td class="hdb">�����¶�</td>
					<td><% ponGetStatus("temperature"); %></td>
				</tr>
				<tr>
					<td class="hdb">�����ѹ</td>
					<td><% ponGetStatus("voltage"); %></td>
				</tr>
				<tr>
					<td class="hdb">ƫ�õ���</td>
					<td><% ponGetStatus("bias-current"); %></td>
				</tr>
			</table>
			</span>
		</div>
	</blockquote>
</body>
<%addHttpNoCache();%>
</html>

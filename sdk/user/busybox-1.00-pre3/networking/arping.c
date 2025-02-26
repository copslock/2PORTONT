/*
 * arping.c - Ping hosts by ARP requests/replies
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Author:	Alexey Kuznetsov <kuznet@ms2.inr.ac.ru>
 * Busybox port: Nick Fedchik <nick@fedchik.org.ua>
 */

#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/time.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>

#include "busybox.h"

#define APPLET_NAME "arping"

struct in_addr src;
struct in_addr dst;
struct sockaddr_ll me;
struct sockaddr_ll he;
struct timeval last;
int dad;
int unsolicited;
int advert;
int quiet;
int quit_on_reply = 0;
int count = -1;
int timeout;
int unicasting;
int s;
int broadcast_only;
int sent;
int brd_sent;
int received;
int brd_recv;
int req_recv;

#define MS_TDIFF(tv1,tv2) ( ((tv1).tv_sec-(tv2).tv_sec)*1000 + \
			   ((tv1).tv_usec-(tv2).tv_usec)/1000 )
#if 0
static void set_signal(int signo, void (*handler) (void))
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = (void (*)(int)) handler;
	sa.sa_flags = SA_RESTART;
	sigaction(signo, &sa, NULL);
}
#endif

static int send_pack(int sock, struct in_addr *src_addr,
					 struct in_addr *dst_addr, struct sockaddr_ll *ME,
					 struct sockaddr_ll *HE)
{
	int err;
	struct timeval now;
	unsigned char buf[256];
	struct arphdr *ah = (struct arphdr *) buf;
	unsigned char *p = (unsigned char *) (ah + 1);

	ah->ar_hrd = htons(ME->sll_hatype);
	ah->ar_hrd = htons(ARPHRD_ETHER);
	ah->ar_pro = htons(ETH_P_IP);
	ah->ar_hln = ME->sll_halen;
	ah->ar_pln = 4;
	ah->ar_op = advert ? htons(ARPOP_REPLY) : htons(ARPOP_REQUEST);

	memcpy(p, &ME->sll_addr, ah->ar_hln);
	p += ME->sll_halen;

	memcpy(p, src_addr, 4);
	p += 4;

	if (advert)
		memcpy(p, &ME->sll_addr, ah->ar_hln);
	else
		memcpy(p, &HE->sll_addr, ah->ar_hln);
	p += ah->ar_hln;

	memcpy(p, dst_addr, 4);
	p += 4;

	gettimeofday(&now, NULL);
	err = sendto(sock, buf, p - buf, 0, (struct sockaddr *) HE, sizeof(*HE));
	if (err == p - buf) {
		last = now;
		sent++;
		if (!unicasting)
			brd_sent++;
	}
	return err;
}

void finish(void)
{
	if (!quiet) {
		printf("Sent %d probes (%d broadcast(s))\n", sent, brd_sent);
		printf("Received %d repl%s", received, (received > 1) ? "ies" : "y");
		if (brd_recv || req_recv) {
			printf(" (");
			if (req_recv)
				printf("%d request(s)", req_recv);
			if (brd_recv)
				printf("%s%d broadcast(s)", req_recv ? ", " : "", brd_recv);
			putchar(')');
		}
		putchar('\n');
		fflush(stdout);
	}
	if (dad)
		exit(!!received);
	if (unsolicited)
		exit(0);
	exit(!received);
}

void catcher(void)
{
	struct timeval tv;
	static struct timeval start;

	gettimeofday(&tv, NULL);

	if (start.tv_sec == 0)
		start = tv;

	if (count-- == 0
		|| (timeout && MS_TDIFF(tv, start) > timeout * 1000 + 500))
		finish();

	if (last.tv_sec == 0 || MS_TDIFF(tv, last) > 500) {
		send_pack(s, &src, &dst, &me, &he);
		if (count == 0 && unsolicited)
			finish();
	}
	alarm(1);
}

int recv_pack(unsigned char *buf, int len, struct sockaddr_ll *FROM)
{
	struct timeval tv;
	struct arphdr *ah = (struct arphdr *) buf;
	unsigned char *p = (unsigned char *) (ah + 1);
	struct in_addr src_ip, dst_ip;

	gettimeofday(&tv, NULL);

	/* Filter out wild packets */
	if (FROM->sll_pkttype != PACKET_HOST &&
		FROM->sll_pkttype != PACKET_BROADCAST &&
		FROM->sll_pkttype != PACKET_MULTICAST)
		return 0;

	/* Only these types are recognised */
	if (ah->ar_op != htons(ARPOP_REQUEST) && ah->ar_op != htons(ARPOP_REPLY))
		return 0;

	/* ARPHRD check and this darned FDDI hack here :-( */
	if (ah->ar_hrd != htons(FROM->sll_hatype) &&
		(FROM->sll_hatype != ARPHRD_FDDI
		 || ah->ar_hrd != htons(ARPHRD_ETHER)))
		return 0;

	/* Protocol must be IP. */
	if (ah->ar_pro != htons(ETH_P_IP))
		return 0;
	if (ah->ar_pln != 4)
		return 0;
	if (ah->ar_hln != me.sll_halen)
		return 0;
	if (len < sizeof(*ah) + 2 * (4 + ah->ar_hln))
		return 0;
	memcpy(&src_ip, p + ah->ar_hln, 4);
	memcpy(&dst_ip, p + ah->ar_hln + 4 + ah->ar_hln, 4);
	if (!dad) {
		if (src_ip.s_addr != dst.s_addr)
			return 0;
		if (src.s_addr != dst_ip.s_addr)
			return 0;
		if (memcmp(p + ah->ar_hln + 4, &me.sll_addr, ah->ar_hln))
			return 0;
	} else {
		/* DAD packet was:
		   src_ip = 0 (or some src)
		   src_hw = ME
		   dst_ip = tested address
		   dst_hw = <unspec>

		   We fail, if receive request/reply with:
		   src_ip = tested_address
		   src_hw != ME
		   if src_ip in request was not zero, check
		   also that it matches to dst_ip, otherwise
		   dst_ip/dst_hw do not matter.
		 */
		if (src_ip.s_addr != dst.s_addr)
			return 0;
		if (memcmp(p, &me.sll_addr, me.sll_halen) == 0)
			return 0;
		if (src.s_addr && src.s_addr != dst_ip.s_addr)
			return 0;
	}
	if (!quiet) {
		int s_printed = 0;

		printf("%s ",
			   FROM->sll_pkttype == PACKET_HOST ? "Unicast" : "Broadcast");
		printf("%s from ",
			   ah->ar_op == htons(ARPOP_REPLY) ? "reply" : "request");
		printf("%s ", inet_ntoa(src_ip));
		printf("[%s]", ether_ntoa((struct ether_addr *) p));
		if (dst_ip.s_addr != src.s_addr) {
			printf("for %s ", inet_ntoa(dst_ip));
			s_printed = 1;
		}
		if (memcmp(p + ah->ar_hln + 4, me.sll_addr, ah->ar_hln)) {
			if (!s_printed)
				printf("for ");
			printf("[%s]",
				   ether_ntoa((struct ether_addr *) p + ah->ar_hln + 4));
		}
		if (last.tv_sec) {
			long usecs = (tv.tv_sec - last.tv_sec) * 1000000 +
				tv.tv_usec - last.tv_usec;
			long msecs = (usecs + 500) / 1000;

			usecs -= msecs * 1000 - 500;
			printf(" %ld.%03ldms\n", msecs, usecs);
		} else {
			printf(" UNSOLICITED?\n");
		}
		fflush(stdout);
	}
	received++;
	if (FROM->sll_pkttype != PACKET_HOST)
		brd_recv++;
	if (ah->ar_op == htons(ARPOP_REQUEST))
		req_recv++;
	if (quit_on_reply)
		finish();
	if (!broadcast_only) {
		memcpy(he.sll_addr, p, me.sll_halen);
		unicasting = 1;
	}
	return 1;
}

int arping_main(int argc, char **argv)
{
	int socket_errno;
	int ch;
	uid_t uid = getuid();
	char *device = "eth0";
	int ifindex = 0;
	char *source = NULL;
	char *target;

	s = socket(PF_PACKET, SOCK_DGRAM, 0);
	socket_errno = errno;

	setuid(uid);

	while ((ch = getopt(argc, argv, "h?bfDUAqc:w:s:I:")) != EOF) {
		switch (ch) {
		case 'b':
			broadcast_only = 1;
			break;
		case 'D':
			dad++;
			quit_on_reply = 1;
			break;
		case 'U':
			unsolicited++;
			break;
		case 'A':
			advert++;
			unsolicited++;
			break;
		case 'q':
			quiet++;
			break;
		case 'c':
			count = atoi(optarg);
			break;
		case 'w':
			timeout = atoi(optarg);
			break;
		case 'I':
			if (optarg == NULL)
				bb_show_usage();
			if (bb_strlen(optarg) > IF_NAMESIZE) {
				bb_error_msg("Interface name `%s' must be less than %d", optarg,
						  IF_NAMESIZE);
				exit(2);
			}
			device = optarg;
			break;
		case 'f':
			quit_on_reply = 1;
			break;
		case 's':
			source = optarg;
			break;
		case 'h':
		case '?':
		default:
			bb_show_usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		bb_show_usage();

	target = *argv;


	if (s < 0) {
		bb_error_msg("socket");
		exit(socket_errno);
	}

	{
		struct ifreq ifr;

		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, device, IFNAMSIZ - 1);
		if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
			bb_error_msg("Interface %s not found", device);
			exit(2);
		}
		ifindex = ifr.ifr_ifindex;

		if (ioctl(s, SIOCGIFFLAGS, (char *) &ifr)) {
			bb_error_msg("SIOCGIFFLAGS");
			exit(2);
		}
		if (!(ifr.ifr_flags & IFF_UP)) {
			bb_error_msg("Interface %s is down", device);
			exit(2);
		}
		if (ifr.ifr_flags & (IFF_NOARP | IFF_LOOPBACK)) {
			bb_error_msg("Interface %s is not ARPable", device);
			exit(dad ? 0 : 2);
		}
	}

	if (!inet_aton(target, &dst)) {
		struct hostent *hp;

		hp = gethostbyname2(target, AF_INET);
		if (!hp) {
			bb_error_msg("invalid or unknown target %s", target);
			exit(2);
		}
		memcpy(&dst, hp->h_addr, 4);
	}

	if (source && !inet_aton(source, &src)) {
		bb_error_msg("invalid source address %s", source);
		exit(2);
	}

	if (!dad && unsolicited && src.s_addr == 0)
		src = dst;

	if (!dad || src.s_addr) {
		struct sockaddr_in saddr;
		int probe_fd = socket(AF_INET, SOCK_DGRAM, 0);

		if (probe_fd < 0) {
			bb_error_msg("socket");
			exit(2);
		}
		if (device) {
			if (setsockopt
				(probe_fd, SOL_SOCKET, SO_BINDTODEVICE, device,
				 strlen(device) + 1) == -1)
				bb_error_msg("WARNING: interface %s is ignored", device);
		}
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		if (src.s_addr) {
			saddr.sin_addr = src;
			if (bind(probe_fd, (struct sockaddr *) &saddr, sizeof(saddr)) == -1) {
				bb_error_msg("bind");
				exit(2);
			}
		} else if (!dad) {
			int on = 1;
			int alen = sizeof(saddr);

			saddr.sin_port = htons(1025);
			saddr.sin_addr = dst;

			if (setsockopt
				(probe_fd, SOL_SOCKET, SO_DONTROUTE, (char *) &on,
				 sizeof(on)) == -1)
				perror("WARNING: setsockopt(SO_DONTROUTE)");
			if (connect(probe_fd, (struct sockaddr *) &saddr, sizeof(saddr))
				== -1) {
				bb_error_msg("connect");
				exit(2);
			}
			if (getsockname(probe_fd, (struct sockaddr *) &saddr, &alen) ==
				-1) {
				bb_error_msg("getsockname");
				exit(2);
			}
			src = saddr.sin_addr;
		}
		close(probe_fd);
	};

	me.sll_family = AF_PACKET;
	me.sll_ifindex = ifindex;
	me.sll_protocol = htons(ETH_P_ARP);
	if (bind(s, (struct sockaddr *) &me, sizeof(me)) == -1) {
		bb_error_msg("bind");
		exit(2);
	}

	{
		int alen = sizeof(me);

		if (getsockname(s, (struct sockaddr *) &me, &alen) == -1) {
			bb_error_msg("getsockname");
			exit(2);
		}
	}
	if (me.sll_halen == 0) {
		bb_error_msg("Interface \"%s\" is not ARPable (no ll address)", device);
		exit(dad ? 0 : 2);
	}
	he = me;
	memset(he.sll_addr, -1, he.sll_halen);

	if (!quiet) {
		printf("ARPING to %s", inet_ntoa(dst));
		printf(" from %s via %s\n", inet_ntoa(src),
			   device ? device : "unknown");
	}

	if (!src.s_addr && !dad) {
		bb_error_msg("no src address in the non-DAD mode");
		exit(2);
	}

	{
		struct sigaction sa;

		memset(&sa, 0, sizeof(sa));
		sa.sa_flags = SA_RESTART;

		sa.sa_handler = (void (*)(int)) finish;
		sigaction(SIGINT, &sa, NULL);

		sa.sa_handler = (void (*)(int)) catcher;
		sigaction(SIGALRM, &sa, NULL);
	}

	catcher();

	while (1) {
		sigset_t sset, osset;
		char packet[4096];
		struct sockaddr_ll from;
		int alen = sizeof(from);
		int cc;

		if ((cc = recvfrom(s, packet, sizeof(packet), 0,
						   (struct sockaddr *) &from, &alen)) < 0) {
			perror("recvfrom");
			continue;
		}
		sigemptyset(&sset);
		sigaddset(&sset, SIGALRM);
		sigaddset(&sset, SIGINT);
		sigprocmask(SIG_BLOCK, &sset, &osset);
		recv_pack(packet, cc, &from);
		sigprocmask(SIG_SETMASK, &osset, NULL);
	}
}

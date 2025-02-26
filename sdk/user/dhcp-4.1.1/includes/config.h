/* includes/config.h.  Generated from config.h.in by configure.  */
/* includes/config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define to compile debug-only DHCP software. */
/* #undef DEBUG */

/* Define to queue multiple DHCPACK replies per fsync. */
/* #undef DELAYED_ACK */

/* Define to BIG_ENDIAN for MSB (Motorola or SPARC CPUs) or LITTLE_ENDIAN for
   LSB (Intel CPUs). */
#define DHCP_BYTE_ORDER BIG_ENDIAN

/* Define to 1 to include DHCPv6 support. */
#define DHCPv6 1

/* Define to any value to chroot() prior to loading config. */
/* #undef EARLY_CHROOT */

/* Define to include execute() config language support. */
#define ENABLE_EXECUTE 1

/* Define to include Failover Protocol support. */
#define FAILOVER_PROTOCOL 1

/* Define to 1 to use the Berkeley Packet Filter interface code. */
/* #undef HAVE_BPF */

/* Define to 1 if you have the /dev/random file. */
/* #undef HAVE_DEV_RANDOM */

/* Define to 1 to use DLPI interface code. */
/* #undef HAVE_DLPI */

/* Define to 1 if you have the <ifaddrs.h> header file. */
#define HAVE_IFADDRS_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <linux/types.h> header file. */
#define HAVE_LINUX_TYPES_H 1

/* Define to 1 to use the Linux Packet Filter interface code. */
#define HAVE_LPF 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <net/if6.h> header file. */
/* #undef HAVE_NET_IF6_H */

/* Define to 1 if you have the <net/if_dl.h> header file. */
/* #undef HAVE_NET_IF_DL_H */

/* Define to 1 if you have the <regex.h> header file. */
#define HAVE_REGEX_H 1

/* Define to 1 if the sockaddr structure has a length field. */
/* #undef HAVE_SA_LEN */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if the system has 'struct if_laddrconf'. */
/* #undef ISC_PLATFORM_HAVEIF_LADDRCONF */

/* Define to 1 if the system has 'struct if_laddrreq'. */
/* #undef ISC_PLATFORM_HAVEIF_LADDRREQ */

/* Define to 1 if the system has 'struct lifnum'. */
/* #undef ISC_PLATFORM_HAVELIFNUM */

/* Define to 1 if the inet_aton() function is missing. */
/* #undef NEED_INET_ATON */

/* Name of package */
#define PACKAGE "dhcp"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "dhcp-users@isc.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "DHCP"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "DHCP 4.1.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "dhcp"

/* Define to the home page for this package. */
/* #undef PACKAGE_URL */

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.1.1"

/* Define to any value to include Ari's PARANOIA patch. */
/* #undef PARANOIA */

/* The size of `struct iaddr *', as computed by sizeof. */
#define SIZEOF_STRUCT_IADDR_P 4

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to include server activity tracing support. */
#define TRACING 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Version number of package */
#define VERSION "4.1.1"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* File for dhclient6 leases. */
/* #undef _PATH_DHCLIENT6_DB */

/* File for dhclient6 process information. */
/* #undef _PATH_DHCLIENT6_PID */

/* File for dhclient leases. */
/* #undef _PATH_DHCLIENT_DB */

/* File for dhclient process information. */
/* #undef _PATH_DHCLIENT_PID */

/* File for dhcpd6 leases. */
/* #undef _PATH_DHCPD6_DB */

/* File for dhcpd6 process information. */
/* #undef _PATH_DHCPD6_PID */

/* File for dhcpd leases. */
/* #undef _PATH_DHCPD_DB */

/* File for dhcpd process information. */
/* #undef _PATH_DHCPD_PID */

/* File for dhcrelay process information. */
/* #undef _PATH_DHCRELAY_PID */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT8_T */

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int16_t */

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
/* #undef int8_t */

/* Define a type for 16-bit unsigned integers. */
/* #undef u_int16_t */

/* Define a type for 32-bit unsigned integers. */
/* #undef u_int32_t */

/* Define a type for 8-bit unsigned integers. */
/* #undef u_int8_t */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint8_t */

/* $Id: inet_ntop.h,v 1.1.1.1 2003/08/18 05:40:23 kaohj Exp $ */

#ifndef _BSD_INET_NTOP_H
#define _BSD_INET_NTOP_H

#include "config.h"

#ifndef HAVE_INET_NTOP
const char *                 
inet_ntop(int af, const void *src, char *dst, size_t size);
#endif /* !HAVE_INET_NTOP */

#endif /* _BSD_INET_NTOP_H */

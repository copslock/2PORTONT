/* $Id: getifaddr.h,v 1.3 2008/05/23 09:06:59 adsmt Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006 Thomas Bernard 
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef __GETIFADDR_H__
#define __GETIFADDR_H__

/* getifaddr()
 * take a network interface name and write the
 * ip v4 address as text in the buffer
 * returns: 0 success, -1 failure */
int
getifaddr(const char * ifname, char * buf, int len);

#endif


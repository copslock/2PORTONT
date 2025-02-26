/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001,2002,2003,2004  Aymeric MOIZARD jack@atosc.org
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __OSIP_CONDV_H__
#define __OSIP_CONDV_H__

#include <stdio.h>

#ifdef OSIP_MT

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__PSOS__)
#else


/* condv implementation */
#if defined(WIN32) || defined(_WIN32_WCE)
/* Prevent struct redefinition if Pthreads for Win32 is used */
#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC 1
/**
 * timespec structure
 * @struct timespec
 */
struct timespec
{
  long tv_sec;
  long tv_nsec;
};
#endif
#endif

  struct osip_cond;

/**
 * Allocate and Initialise a condition variable
 */
  struct osip_cond *osip_cond_init (void);
/**
 * Destroy a condition variable
 * @param cond The condition variable to destroy.
 */
  int osip_cond_destroy (struct osip_cond *cond);
/**
 * Signal the condition variable.
 * @param cond The condition variable to signal.
 */
  int osip_cond_signal (struct osip_cond *cond);

/**
 * Wait on the condition variable.
 * @param cond The condition variable to wait on.
 * @param mut The external mutex 
 */
  int osip_cond_wait (struct osip_cond *cond, struct osip_mutex *mut);
/**
 * Timed wait on the condition variable.
 * @param cond The condition variable to wait on.
 * @param mut The external mutex 
 * @param abstime time to wait until
 */
  int osip_cond_timedwait (struct osip_cond *cond, struct osip_mutex *mut,
			   const struct timespec *abstime);


#ifdef __cplusplus
}
#endif

#endif

/** @} */

#else

#define osip_cond_init(...)
#define osip_cond_destroy(...)
#define osip_cond_signal(...)
#define osip_cond_wait(...)
#define osip_cond_timedwait(...)

#endif

#endif

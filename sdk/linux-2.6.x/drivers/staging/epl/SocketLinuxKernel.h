/****************************************************************************

  (c) SYSTEC electronic GmbH, D-07973 Greiz, August-Bebel-Str. 29
      www.systec-electronic.com

  Project:      openPOWERLINK

  Description:  include file for BSD socket API for Linux kernel

  License:

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    3. Neither the name of SYSTEC electronic GmbH nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without prior written permission. For written
       permission, please contact info@systec-electronic.com.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Severability Clause:

        If a provision of this License is or becomes illegal, invalid or
        unenforceable in any jurisdiction, that shall not affect:
        1. the validity or enforceability in that jurisdiction of any other
           provision of this License; or
        2. the validity or enforceability in other jurisdictions of that or
           any other provision of this License.

  -------------------------------------------------------------------------

                $RCSfile: SocketLinuxKernel.h,v $

                $Author: jiunming $

                $Revision: 1.1.1.1 $  $Date: 2010/05/05 09:01:39 $

                $State: Exp $

                Build Environment:
                KEIL uVision 2

  -------------------------------------------------------------------------

  Revision History:

  2006/08/25 d.k.:   start of the implementation

****************************************************************************/

#ifndef _SOCKETLINUXKERNEL_H_
#define _SOCKETLINUXKERNEL_H_

#include <linux/net.h>
#include <linux/in.h>

//---------------------------------------------------------------------------
// const defines
//---------------------------------------------------------------------------

#define INVALID_SOCKET  0

//---------------------------------------------------------------------------
// typedef
//---------------------------------------------------------------------------

typedef struct socket *SOCKET;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------

int bind(SOCKET s, const struct sockaddr *addr, int addrlen);

int closesocket(SOCKET s);

int recvfrom(SOCKET s, char *buf, int len, int flags, struct sockaddr *from,
	     int *fromlen);

int sendto(SOCKET s, const char *buf, int len, int flags,
	   const struct sockaddr *to, int tolen);

SOCKET socket(int af, int type, int protocol);

#endif // #ifndef _SOCKETLINUXKERNEL_H_

/****************************************************************************

  (c) SYSTEC electronic GmbH, D-07973 Greiz, August-Bebel-Str. 29
      www.systec-electronic.com

  Project:      openPOWERLINK

  Description:  include file for communication abstraction layer
                for the Epl-Obd-Userspace-Modul

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

                $RCSfile: EplObduCal.h,v $

                $Author: jiunming $

                $Revision: 1.1.1.1 $  $Date: 2010/05/05 09:01:39 $

                $State: Exp $

                Build Environment:
                    GCC V3.4

  -------------------------------------------------------------------------

  Revision History:

  2006/06/19 k.t.:   start of the implementation

****************************************************************************/

#ifndef _EPLOBDUCAL_H_
#define _EPLOBDUCAL_H_

#include "../EplObd.h"

tEplKernel EplObduCalWriteEntry(unsigned int uiIndex_p,
				unsigned int uiSubIndex_p,
				void *pSrcData_p,
				tEplObdSize Size_p);
//---------------------------------------------------------------------------
tEplKernel EplObduCalReadEntry(unsigned int uiIndex_p,
			       unsigned int uiSubIndex_p,
			       void *pDstData_p,
			       tEplObdSize *pSize_p);
//---------------------------------------------------------------------------
tEplKernel EplObduCalAccessOdPart(tEplObdPart ObdPart_p,
				  tEplObdDir Direction_p);
//---------------------------------------------------------------------------
tEplKernel EplObduCalDefineVar(tEplVarParam *pVarParam_p);
//---------------------------------------------------------------------------
void *EplObduCalGetObjectDataPtr(unsigned int uiIndex_p,
				 unsigned int uiSubIndex_p);
//---------------------------------------------------------------------------
tEplKernel EplObduCalRegisterUserOd(tEplObdEntryPtr pUserOd_p);
//---------------------------------------------------------------------------
void EplObduCalInitVarEntry(tEplObdVarEntry *pVarEntry_p,
			    u8 bType_p, tEplObdSize ObdSize_p);
//---------------------------------------------------------------------------
tEplObdSize EplObduCalGetDataSize(unsigned int uiIndex_p,
				  unsigned int uiSubIndex_p);
//---------------------------------------------------------------------------
unsigned int EplObduCalGetNodeId(void);
//---------------------------------------------------------------------------
tEplKernel EplObduCalSetNodeId(unsigned int uiNodeId_p,
			       tEplObdNodeIdType NodeIdType_p);
//---------------------------------------------------------------------------
tEplKernel EplObduCalGetAccessType(unsigned int uiIndex_p,
				   unsigned int uiSubIndex_p,
				   tEplObdAccess *pAccessTyp_p);
//---------------------------------------------------------------------------
tEplKernel EplObduCalReadEntryToLe(unsigned int uiIndex_p,
				   unsigned int uiSubIndex_p,
				   void *pDstData_p,
				   tEplObdSize *pSize_p);
//---------------------------------------------------------------------------
tEplKernel EplObduCalWriteEntryFromLe(unsigned int uiIndex_p,
				      unsigned int uiSubIndex_p,
				      void *pSrcData_p,
				      tEplObdSize Size_p);
//---------------------------------------------------------------------------
tEplKernel EplObduCalSearchVarEntry(EPL_MCO_DECL_INSTANCE_PTR_ unsigned int uiIndex_p,
				    unsigned int uiSubindex_p,
				    tEplObdVarEntry **ppVarEntry_p);

#endif // #ifndef _EPLOBDUCAL_H_

/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Aymeric MOIZARD jack@atosc.org
  
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

#include <osip2/internal.h>
#include <osip2/osip.h>

#include "fsm.h"
#include "xixt.h"

int __osip_nict_init(osip_nict_t ** nict, osip_t * osip, osip_message_t * request)
{
	osip_route_t *route;
	int i;
	time_t now;

	OSIP_TRACE(osip_trace
			   (__FILE__, __LINE__, OSIP_INFO2, NULL,
				"allocating NICT context\n"));

	*nict = (osip_nict_t *) osip_malloc(sizeof(osip_nict_t));
	if (*nict == NULL)
		return OSIP_NOMEM;
	now = time(NULL);
	memset(*nict, 0, sizeof(osip_nict_t));
	/* for REQUEST retransmissions */
	{
		osip_via_t *via;
		char *proto;

		i = osip_message_get_via(request, 0, &via);	/* get top via */
		if (i < 0) {
			osip_free(*nict);
			*nict = NULL;
			return i;
		}
		proto = via_get_protocol(via);
		if (proto == NULL) {
			osip_free(*nict);
			*nict = NULL;
			return OSIP_UNDEFINED_ERROR;
		}
#ifdef USE_BLOCKINGSOCKET
		if (osip_strcasecmp(proto, "TCP") != 0
			&& osip_strcasecmp(proto, "TLS") != 0
			&& osip_strcasecmp(proto, "SCTP") != 0) {
			(*nict)->timer_e_length = DEFAULT_T1;
			(*nict)->timer_k_length = DEFAULT_T4;
			(*nict)->timer_e_start.tv_sec = -1;
			(*nict)->timer_k_start.tv_sec = -1;	/* not started */
		} else {				/* reliable protocol is used: */
			(*nict)->timer_e_length = -1;	/* E is not ACTIVE */
			(*nict)->timer_k_length = 0;	/* MUST do the transition immediatly */
			(*nict)->timer_e_start.tv_sec = -1;
			(*nict)->timer_k_start.tv_sec = -1;	/* not started */
		}
	}
#else
		if (osip_strcasecmp(proto, "TCP") != 0
			&& osip_strcasecmp(proto, "TLS") != 0
			&& osip_strcasecmp(proto, "SCTP") != 0) {
			(*nict)->timer_e_length = DEFAULT_T1;
			(*nict)->timer_k_length = DEFAULT_T4;
			(*nict)->timer_e_start.tv_sec = -1;
			(*nict)->timer_k_start.tv_sec = -1;	/* not started */
		} else {				/* reliable protocol is used: */
			(*nict)->timer_e_length = DEFAULT_T1;
			(*nict)->timer_k_length = 0;	/* MUST do the transition immediatly */
			(*nict)->timer_e_start.tv_sec = -1;
			(*nict)->timer_k_start.tv_sec = -1;	/* not started */
		}
	}
#endif
	/* for PROXY, the destination MUST be set by the application layer,
	   this one may not be correct. */
	osip_message_get_route(request, 0, &route);
	if (route != NULL && &route->url != NULL) {
		osip_uri_param_t *lr_param;

		osip_uri_uparam_get_byname(&route->url, "lr", &lr_param);
       		if (lr_param == NULL) {
			/* using uncompliant proxy: destination is the request-uri */
			route = NULL;
		}
	}

	if (route != NULL && &route->url != NULL) {
		int port = 5060;

		if (route->url.port > 0)
			port = route->url.port;
		osip_nict_set_destination((*nict), route->url.host, port);
   	} else {
		int port = 5060;
		/* search for maddr parameter */
		osip_uri_param_t *maddr_param = NULL;
   		if (request->req_uri.port > 0)
		    port = request->req_uri.port;

        osip_uri_uparam_get_byname(&request->req_uri, "maddr", &maddr_param);
		if (maddr_param != NULL && maddr_param->gvalue != NULL)
			osip_nict_set_destination((*nict),
									  maddr_param->gvalue, port);
		else
			osip_nict_set_destination((*nict),
									  request->req_uri.host, port);
	}

	(*nict)->timer_f_length = 64 * DEFAULT_T1;
	osip_gettimeofday(&(*nict)->timer_f_start, NULL);
	add_gettimeofday(&(*nict)->timer_f_start, (*nict)->timer_f_length);

	/* Oups! a Bug! */
	/*  (*nict)->port  = 5060; */

	return OSIP_SUCCESS;
}

int __osip_nict_free(osip_nict_t * nict)
{
	if (nict == NULL)
		return OSIP_SUCCESS;
	OSIP_TRACE(osip_trace
			   (__FILE__, __LINE__, OSIP_INFO2, NULL, "free nict ressource\n"));

	osip_free(nict);
	return OSIP_SUCCESS;
}

int osip_nict_set_destination(osip_nict_t * nict, char *destination, int port)
{
	if (nict == NULL)
		return OSIP_BADPARAMETER;
	
	if (nict->destination != NULL)
	    osip_strncpy(nict->destination, destination, sizeof(nict->destination)-1);
	nict->port = port;
	return OSIP_SUCCESS;
}

osip_event_t *__osip_nict_need_timer_e_event(osip_nict_t * nict, state_t state,
											 int transactionid)
{
	return __osip_transaction_need_timer_x_event(nict, &nict->timer_e_start,
												 state == NICT_PROCEEDING
												 || state == NICT_TRYING,
												 transactionid, TIMEOUT_E);
}

osip_event_t *__osip_nict_need_timer_f_event(osip_nict_t * nict, state_t state,
											 int transactionid)
{
	return __osip_transaction_need_timer_x_event(nict, &nict->timer_f_start,
												 state == NICT_PROCEEDING
												 || state == NICT_TRYING,
												 transactionid, TIMEOUT_F);
}

osip_event_t *__osip_nict_need_timer_k_event(osip_nict_t * nict, state_t state,
											 int transactionid)
{
	return __osip_transaction_need_timer_x_event(nict, &nict->timer_k_start,
												 state == NICT_COMPLETED,
												 transactionid, TIMEOUT_K);
}

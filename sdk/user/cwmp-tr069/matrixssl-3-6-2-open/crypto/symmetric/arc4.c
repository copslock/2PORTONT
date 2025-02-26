/**
 *	@file    arc4.c
 *	@version 66615a4 (HEAD, tag: MATRIXSSL-3-6-2-OPEN, tag: MATRIXSSL-3-6-2-COMM, origin/master, origin/HEAD, master)
 *
 *	ARC4 stream cipher implementation.
 */
/*
 *	Copyright (c) 2013-2014 INSIDE Secure Corporation
 *	Copyright (c) PeerSec Networks, 2002-2011
 *	All Rights Reserved
 *
 *	The latest version of this code is available at http://www.matrixssl.org
 *
 *	This software is open source; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This General Public License does NOT permit incorporating this software 
 *	into proprietary programs.  If you are unable to comply with the GPL, a 
 *	commercial license for this software may be purchased from INSIDE at
 *	http://www.insidesecure.com/eng/Company/Locations
 *	
 *	This program is distributed in WITHOUT ANY WARRANTY; without even the 
 *	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 *	See the GNU General Public License for more details.
 *	
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *	http://www.gnu.org/copyleft/gpl.html
 */
/******************************************************************************/

#include "../cryptoApi.h"

#ifdef USE_ARC4

/*
	SECURITY
	Some accounts, such as O'Reilly's Secure Programming Cookbook say that no 
	more than 2^30 bytes should be processed without rekeying, so we 
	enforce that limit here.  FYI, this is equal to 1GB of data transferred.
*/
#define ARC4_MAX_BYTES	0x40000000

/******************************************************************************/
/*

 */
void psArc4Init(psCipherContext_t *ctx, unsigned char *key, uint32 keylen)
{
	unsigned char	index1,	index2, tmp, *state;
	short			counter;

	ctx->arc4.byteCount = 0;
	state = &ctx->arc4.state[0];

	for (counter = 0; counter < 256; counter++) {
		state[counter] = (unsigned char)counter;
	}
	ctx->arc4.x = 0;
	ctx->arc4.y = 0;
	index1 = 0;
	index2 = 0;

	for (counter = 0; counter < 256; counter++) {
		index2 = (key[index1] + state[counter] + index2) & 0xff;

		tmp = state[counter];
		state[counter] = state[index2];
		state[index2] = tmp;

		index1 = (index1 + 1) % keylen;
	}
}

int32 psArc4(psCipherContext_t *ctx, unsigned char *in,
			   unsigned char *out, uint32 len)
{
	unsigned char	x, y, *state, xorIndex, tmp;
	uint32			counter;

	ctx->arc4.byteCount += len;
	if (ctx->arc4.byteCount > ARC4_MAX_BYTES) {
		psTraceCrypto("ARC4 byteCount overrun\n");
		return PS_LIMIT_FAIL;
	}

	x = ctx->arc4.x;
	y = ctx->arc4.y;
	state = &ctx->arc4.state[0];
	for (counter = 0; counter < len; counter++) {
		x = (x + 1) & 0xff;
		y = (state[x] + y) & 0xff;

		tmp = state[x];
		state[x] = state[y];
		state[y] = tmp;

		xorIndex = (state[x] + state[y]) & 0xff;

		tmp = in[counter];
		tmp ^= state[xorIndex];
		out[counter] = tmp;
	}
	ctx->arc4.x = x;
	ctx->arc4.y = y;
	return len;
}

#endif /* USE_ARC4 */

/******************************************************************************/

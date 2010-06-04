/* vim:ts=8:sts=8:sw=4:noai:noexpandtab
 * 
 * Vector message container
 *
 * Copyright (c) 2006-2009 Miru Limited.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if !defined (__PGM_FRAMEWORK_H_INSIDE__) && !defined (PGM_COMPILATION)
#	error "Only <framework.h> can be included directly."
#endif

#ifndef __PGM_MSGV_H__
#define __PGM_MSGV_H__

#include <pgm/types.h>
#include <pgm/packet.h>
#include <pgm/skbuff.h>

PGM_BEGIN_DECLS

/* struct for scatter/gather I/O */
struct pgm_iovec {
#ifndef _WIN32
/* match struct iovec */
	void*		iov_base;
	size_t		iov_len;	/* size of iov_base */
#else
/* match WSABUF */
	u_long		iov_len;
	char*		iov_base;
#endif /* G_OS_WIN32 */
};

struct pgm_msgv_t {
	unsigned		msgv_len;			/* number of elements in skb */
	struct pgm_sk_buff_t*	msgv_skb[PGM_MAX_FRAGMENTS];	/* PGM socket buffer array */
};

PGM_END_DECLS

#endif /* __PGM_MSGV_H__ */
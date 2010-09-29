/* vim:ts=8:sts=4:sw=4:noai:noexpandtab
 * 
 * basic receive window.
 *
 * Copyright (c) 2006-2008 Miru Limited.
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

#ifndef __PGM_RXW_H__
#define __PGM_RXW_H__

#include <glib.h>

#ifndef __PGM_TIME_H__
#   include <pgm/time.h>
#endif

#ifndef __PGM_MSGV_H__
#   include <pgm/msgv.h>
#endif

#ifndef __PGM_PACKET_H__
#   include <pgm/packet.h>
#endif

#ifndef __PGM_SKBUFF_H__
#	include <pgm/skbuff.h>
#endif

#ifndef	__PGM_REED_SOLOMON_H__
#	include <pgm/reed_solomon.h>
#endif


G_BEGIN_DECLS

typedef enum
{
    PGM_PKT_ERROR_STATE = 0,
    PGM_PKT_BACK_OFF_STATE,	    /* PGM protocol recovery states */
    PGM_PKT_WAIT_NCF_STATE,
    PGM_PKT_WAIT_DATA_STATE,

    PGM_PKT_HAVE_DATA_STATE,	    /* data received waiting to commit to application layer */

    PGM_PKT_HAVE_PARITY_STATE,	    /* contains parity information not original data */
    PGM_PKT_COMMIT_DATA_STATE,	    /* commited data waiting for purging */
    PGM_PKT_LOST_DATA_STATE,	    /* if recovery fails, but packet has not yet been commited */
} pgm_pkt_state_e;

typedef enum
{
	PGM_RXW_OK = 0,
	PGM_RXW_INSERTED,
	PGM_RXW_APPENDED,
	PGM_RXW_UPDATED,
	PGM_RXW_MISSING,
	PGM_RXW_DUPLICATE,
	PGM_RXW_MALFORMED,
	PGM_RXW_BOUNDS,
	PGM_RXW_SLOW_CONSUMER,
	PGM_RXW_UNKNOWN,
} pgm_rxw_returns_e;

struct pgm_rxw_state_t {
	pgm_time_t	nak_rb_expiry;
	pgm_time_t	nak_rpt_expiry;
	pgm_time_t	nak_rdata_expiry;

        pgm_pkt_state_e state;

	guint8		nak_transmit_count;
        guint8          ncf_retry_count;
        guint8          data_retry_count;

/* only valid on tg_sqn::pkt_sqn = 0 */
	unsigned	is_contiguous:1;	/* transmission group */
};

typedef struct pgm_rxw_state_t pgm_rxw_state_t;

/* must be smaller than PGM skbuff control buffer */
#ifndef G_STATIC_ASSERT
#	define G_PASTE_ARGS(identifier1,identifier2) identifier1 ## identifier2
#	define G_PASTE(identifier1,identifier2) G_PASTE_ARGS (identifier1, identifier2)
#	define G_STATIC_ASSERT(expr) typedef struct { char Compile_Time_Assertion[(expr) ? 1 : -1]; } G_PASTE (_GStaticAssert_, __LINE__)
#endif

G_STATIC_ASSERT(sizeof(struct pgm_rxw_state_t) <= sizeof(((struct pgm_sk_buff_t*)0)->cb));


struct pgm_rxw_t {
	const pgm_tsi_t*	tsi;

        GQueue          backoff_queue;
        GQueue          wait_ncf_queue;
        GQueue          wait_data_queue;
/* window context counters */
	guint32		lost_count;		/* failed to repair */
	guint32		fragment_count;		/* incomplete apdu */
	guint32		parity_count;		/* parity for repairs */
	guint32		committed_count;	/* but still in window */

        guint16         max_tpdu;               /* maximum packet size */
        guint32         lead, trail;
        guint32         rxw_trail, rxw_trail_init;
	guint32		commit_lead;
        unsigned        is_constrained:1;
        unsigned        is_defined:1;
	unsigned	has_event:1;		/* edge triggered */
	unsigned	is_fec_available:1;
	rs_t		rs;
	guint32		tg_size;		/* transmission group size for parity recovery */
	guint		tg_sqn_shift;

/* counters all guint32 */
	guint32		min_fill_time;		/* restricted from pgm_time_t */
	guint32		max_fill_time;
	guint32		min_nak_transmit_count;
	guint32		max_nak_transmit_count;
	guint32		cumulative_losses;
	guint32		bytes_delivered;
	guint32		msgs_delivered;

	gsize		size;			/* in bytes */
	guint		alloc;			/* in pkts */
	struct pgm_sk_buff_t*	pdata[];
};

typedef struct pgm_rxw_t pgm_rxw_t;


PGM_GNUC_INTERNAL pgm_rxw_t* pgm_rxw_create (const pgm_tsi_t* const, const guint16, const guint32, const guint, const guint) G_GNUC_WARN_UNUSED_RESULT;
PGM_GNUC_INTERNAL void pgm_rxw_destroy (pgm_rxw_t* const);
PGM_GNUC_INTERNAL int pgm_rxw_add (pgm_rxw_t* const, struct pgm_sk_buff_t* const, const pgm_time_t, const pgm_time_t) G_GNUC_WARN_UNUSED_RESULT;
PGM_GNUC_INTERNAL void pgm_rxw_remove_commit (pgm_rxw_t* const);
PGM_GNUC_INTERNAL gssize pgm_rxw_readv (pgm_rxw_t* const, pgm_msgv_t**, const guint) G_GNUC_WARN_UNUSED_RESULT;
PGM_GNUC_INTERNAL guint pgm_rxw_remove_trail (pgm_rxw_t* const) G_GNUC_WARN_UNUSED_RESULT;
PGM_GNUC_INTERNAL guint pgm_rxw_update (pgm_rxw_t* const, const guint32, const guint32, const pgm_time_t, const pgm_time_t) G_GNUC_WARN_UNUSED_RESULT;
PGM_GNUC_INTERNAL void pgm_rxw_update_fec (pgm_rxw_t* const, const guint);
PGM_GNUC_INTERNAL int pgm_rxw_confirm (pgm_rxw_t* const, guint32, pgm_time_t, pgm_time_t, pgm_time_t) G_GNUC_WARN_UNUSED_RESULT;
PGM_GNUC_INTERNAL void pgm_rxw_lost (pgm_rxw_t* const, const guint32);
PGM_GNUC_INTERNAL void pgm_rxw_state (pgm_rxw_t*, struct pgm_sk_buff_t*, pgm_pkt_state_e);
PGM_GNUC_INTERNAL struct pgm_sk_buff_t* pgm_rxw_peek (pgm_rxw_t* const, const guint32) G_GNUC_WARN_UNUSED_RESULT;
PGM_GNUC_INTERNAL const char* pgm_pkt_state_string (pgm_pkt_state_e) G_GNUC_WARN_UNUSED_RESULT;
PGM_GNUC_INTERNAL const char* pgm_rxw_returns_string (pgm_rxw_returns_e) G_GNUC_WARN_UNUSED_RESULT;
PGM_GNUC_INTERNAL void pgm_rxw_dump (const pgm_rxw_t* const);

static inline guint pgm_rxw_max_length (const pgm_rxw_t* const window)
{
	g_assert (window);
	return window->alloc;
}

static inline guint32 pgm_rxw_length (const pgm_rxw_t* const window)
{
	g_assert (window);
	return ( 1 + window->lead ) - window->trail;
}

static inline gsize pgm_rxw_size (const pgm_rxw_t* const window)
{
	g_assert (window);
	return window->size;
}

static inline gboolean pgm_rxw_is_empty (const pgm_rxw_t* const window)
{
	g_assert (window);
	return pgm_rxw_length (window) == 0;
}

static inline gboolean pgm_rxw_is_full (const pgm_rxw_t* const window)
{
	g_assert (window);
	return pgm_rxw_length (window) == pgm_rxw_max_length (window);
}

static inline guint32 pgm_rxw_lead (const pgm_rxw_t* const window)
{
	g_assert (window);
	return window->lead;
}

static inline guint32 pgm_rxw_next_lead (const pgm_rxw_t* const window)
{
	return (guint32)(pgm_rxw_lead (window) + 1);
}

G_END_DECLS

#endif /* __PGM_RXW_H__ */

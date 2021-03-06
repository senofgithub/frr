/*
 * Zebra API server.
 * Portions:
 *   Copyright (C) 1997-1999  Kunihiro Ishiguro
 *   Copyright (C) 2015-2018  Cumulus Networks, Inc.
 *   et al.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; see the file COPYING; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _ZEBRA_ZSERV_H
#define _ZEBRA_ZSERV_H

/* clang-format off */
#include <stdint.h>           /* for uint32_t, uint8_t */
#include <time.h>             /* for time_t */

#include "lib/route_types.h"  /* for ZEBRA_ROUTE_MAX */
#include "lib/zebra.h"        /* for AFI_MAX */
#include "lib/vrf.h"          /* for vrf_bitmap_t */
#include "lib/zclient.h"      /* for redist_proto */
#include "lib/stream.h"       /* for stream, stream_fifo */
#include "lib/thread.h"       /* for thread, thread_master */
#include "lib/linklist.h"     /* for list */
#include "lib/workqueue.h"    /* for work_queue */
#include "lib/hook.h"         /* for DECLARE_HOOK, DECLARE_KOOH */

#include "zebra/zebra_vrf.h"  /* for zebra_vrf */
/* clang-format on */

/* Default port information. */
#define ZEBRA_VTY_PORT                2601

/* Default configuration filename. */
#define DEFAULT_CONFIG_FILE "zebra.conf"

#define ZEBRA_RMAP_DEFAULT_UPDATE_TIMER 5 /* disabled by default */

/* Client structure. */
struct zserv {
	/* Client file descriptor. */
	int sock;

	/* Input/output buffer to the client. */
	struct stream_fifo *ibuf_fifo;
	struct stream_fifo *obuf_fifo;

	/* Private I/O buffers */
	struct stream *ibuf_work;
	struct stream *obuf_work;

	/* Buffer of data waiting to be written to client. */
	struct buffer *wb;

	/* Threads for read/write. */
	struct thread *t_read;
	struct thread *t_write;

	/* Thread for delayed close. */
	struct thread *t_suicide;

	/* default routing table this client munges */
	int rtm_table;

	/* This client's redistribute flag. */
	struct redist_proto mi_redist[AFI_MAX][ZEBRA_ROUTE_MAX];
	vrf_bitmap_t redist[AFI_MAX][ZEBRA_ROUTE_MAX];

	/* Redistribute default route flag. */
	vrf_bitmap_t redist_default;

	/* Interface information. */
	vrf_bitmap_t ifinfo;

	/* Router-id information. */
	vrf_bitmap_t ridinfo;

	bool notify_owner;

	/* client's protocol */
	uint8_t proto;
	uint16_t instance;
	uint8_t is_synchronous;

	/* Statistics */
	uint32_t redist_v4_add_cnt;
	uint32_t redist_v4_del_cnt;
	uint32_t redist_v6_add_cnt;
	uint32_t redist_v6_del_cnt;
	uint32_t v4_route_add_cnt;
	uint32_t v4_route_upd8_cnt;
	uint32_t v4_route_del_cnt;
	uint32_t v6_route_add_cnt;
	uint32_t v6_route_del_cnt;
	uint32_t v6_route_upd8_cnt;
	uint32_t connected_rt_add_cnt;
	uint32_t connected_rt_del_cnt;
	uint32_t ifup_cnt;
	uint32_t ifdown_cnt;
	uint32_t ifadd_cnt;
	uint32_t ifdel_cnt;
	uint32_t if_bfd_cnt;
	uint32_t bfd_peer_add_cnt;
	uint32_t bfd_peer_upd8_cnt;
	uint32_t bfd_peer_del_cnt;
	uint32_t bfd_peer_replay_cnt;
	uint32_t vrfadd_cnt;
	uint32_t vrfdel_cnt;
	uint32_t if_vrfchg_cnt;
	uint32_t bfd_client_reg_cnt;
	uint32_t vniadd_cnt;
	uint32_t vnidel_cnt;
	uint32_t l3vniadd_cnt;
	uint32_t l3vnidel_cnt;
	uint32_t macipadd_cnt;
	uint32_t macipdel_cnt;
	uint32_t prefixadd_cnt;
	uint32_t prefixdel_cnt;

	time_t connect_time;
	time_t last_read_time;
	time_t last_write_time;
	time_t nh_reg_time;
	time_t nh_dereg_time;
	time_t nh_last_upd_time;

	int last_read_cmd;
	int last_write_cmd;
};

#define ZAPI_HANDLER_ARGS                                                      \
	struct zserv *client, struct zmsghdr *hdr, struct stream *msg,         \
		struct zebra_vrf *zvrf

/* Hooks for client connect / disconnect */
DECLARE_HOOK(zapi_client_connect, (struct zserv *client), (client));
DECLARE_KOOH(zapi_client_close, (struct zserv *client), (client));

/* Zebra instance */
struct zebra_t {
	/* Thread master */
	struct thread_master *master;
	struct list *client_list;

	/* default table */
	uint32_t rtm_table_default;

/* rib work queue */
#define ZEBRA_RIB_PROCESS_HOLD_TIME 10
	struct work_queue *ribq;
	struct meta_queue *mq;

	/* LSP work queue */
	struct work_queue *lsp_process_q;

#define ZEBRA_ZAPI_PACKETS_TO_PROCESS 10
	uint32_t packets_to_process;
};
extern struct zebra_t zebrad;
extern unsigned int multipath_num;

/* Prototypes. */
extern void zserv_init(void);
extern void zebra_zserv_socket_init(char *path);
extern int zebra_server_send_message(struct zserv *client, struct stream *msg);

extern struct zserv *zebra_find_client(uint8_t proto, unsigned short instance);

#if defined(HANDLE_ZAPI_FUZZING)
extern void zserv_read_file(char *input);
#endif

#endif /* _ZEBRA_ZEBRA_H */

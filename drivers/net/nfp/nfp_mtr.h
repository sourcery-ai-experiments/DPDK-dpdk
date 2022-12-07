/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Corigine, Inc.
 * All rights reserved.
 */

#ifndef __NFP_MTR_H__
#define __NFP_MTR_H__

/**
 * The max meter count is determined by firmware.
 * The max count is 65536 defined by OF_METER_COUNT.
 */
#define NFP_MAX_MTR_CNT                65536
#define NFP_MAX_POLICY_CNT             NFP_MAX_MTR_CNT
#define NFP_MAX_PROFILE_CNT            NFP_MAX_MTR_CNT

/**
 * See RFC 2698 for more details.
 * Word[0](Flag options):
 * [15] p(pps) 1 for pps, 0 for bps
 *
 * Meter control message
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-------------------------------+-+---+-----+-+---------+-+---+-+
 * |            Reserved           |p| Y |TYPE |E|  TSHFV  |P| PC|R|
 * +-------------------------------+-+---+-----+-+---------+-+---+-+
 * |                           Profile ID                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Token Bucket Peak                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Token Bucket Committed                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Peak Burst Size                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Committed Burst Size                     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Peak Information Rate                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Committed Information Rate                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
struct nfp_cfg_head {
	rte_be32_t flags_opts;
	rte_be32_t profile_id;
};

/**
 * Struct nfp_profile_conf - profile config, offload to NIC
 * @head:        config head information
 * @bkt_tkn_p:   token bucket peak
 * @bkt_tkn_c:   token bucket committed
 * @pbs:         peak burst size
 * @cbs:         committed burst size
 * @pir:         peak information rate
 * @cir:         committed information rate
 */
struct nfp_profile_conf {
	struct nfp_cfg_head head;
	rte_be32_t bkt_tkn_p;
	rte_be32_t bkt_tkn_c;
	rte_be32_t pbs;
	rte_be32_t cbs;
	rte_be32_t pir;
	rte_be32_t cir;
};

/**
 * Struct nfp_mtr_profile - meter profile, stored in driver
 * Can only be used by one meter
 * @next:        next meter profile object
 * @profile_id:  meter profile id
 * @conf:        meter profile config
 * @in_use:      if profile is been used by meter
 */
struct nfp_mtr_profile {
	LIST_ENTRY(nfp_mtr_profile) next;
	uint32_t profile_id;
	struct nfp_profile_conf conf;
	bool in_use;
};

/**
 * Struct nfp_mtr_priv - meter private data
 * @profiles:        the head node of profile list
 */
struct nfp_mtr_priv {
	LIST_HEAD(, nfp_mtr_profile) profiles;
};

int nfp_net_mtr_ops_get(struct rte_eth_dev *dev, void *arg);
int nfp_mtr_priv_init(struct nfp_pf_dev *pf_dev);
void nfp_mtr_priv_uninit(struct nfp_pf_dev *pf_dev);

#endif /* __NFP_MTR_H__ */
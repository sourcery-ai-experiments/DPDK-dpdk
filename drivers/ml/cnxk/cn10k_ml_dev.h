/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2022 Marvell.
 */

#ifndef _CN10K_ML_DEV_H_
#define _CN10K_ML_DEV_H_

#include <roc_api.h>

#include "cn10k_ml_ocm.h"

#include "cnxk_ml_io.h"

/* Dummy Device ops */
extern struct rte_ml_dev_ops ml_dev_dummy_ops;

/* Marvell OCTEON CN10K ML PMD device name */
#define MLDEV_NAME_CN10K_PMD ml_cn10k

/* Device alignment size */
#define ML_CN10K_ALIGN_SIZE 128

/* Maximum number of queue-pairs per device, spinlock version */
#define ML_CN10K_MAX_QP_PER_DEVICE_SL 16

/* Maximum number of queue-pairs per device, lock-free version */
#define ML_CN10K_MAX_QP_PER_DEVICE_LF 1

/* Maximum number of descriptors per queue-pair */
#define ML_CN10K_MAX_DESC_PER_QP 1024

/* Maximum number of inputs / outputs per model */
#define ML_CN10K_MAX_INPUT_OUTPUT 32

/* Maximum number of segments for IO data */
#define ML_CN10K_MAX_SEGMENTS 1

/* ML slow-path job flags */
#define ML_CN10K_SP_FLAGS_OCM_NONRELOCATABLE BIT(0)
#define ML_CN10K_SP_FLAGS_EXTENDED_LOAD_JD   BIT(1)

/* Memory barrier macros */
#if defined(RTE_ARCH_ARM)
#define dmb_st ({ asm volatile("dmb st" : : : "memory"); })
#define dsb_st ({ asm volatile("dsb st" : : : "memory"); })
#else
#define dmb_st
#define dsb_st
#endif

struct cnxk_ml_dev;
struct cnxk_ml_req;
struct cnxk_ml_qp;

/* Error types enumeration */
enum cn10k_ml_error_etype {
	/* 0x0 */ ML_ETYPE_NO_ERROR = 0, /* No error */
	/* 0x1 */ ML_ETYPE_FW_NONFATAL,	 /* Firmware non-fatal error */
	/* 0x2 */ ML_ETYPE_HW_NONFATAL,	 /* Hardware non-fatal error */
	/* 0x3 */ ML_ETYPE_HW_FATAL,	 /* Hardware fatal error */
	/* 0x4 */ ML_ETYPE_HW_WARNING,	 /* Hardware warning */
	/* 0x5 */ ML_ETYPE_DRIVER,	 /* Driver specific error */
	/* 0x6 */ ML_ETYPE_UNKNOWN,	 /* Unknown error */
};

/* Firmware non-fatal error sub-type */
enum cn10k_ml_error_stype_fw_nf {
	/* 0x0 */ ML_FW_ERR_NOERR = 0,		 /* No error */
	/* 0x1 */ ML_FW_ERR_UNLOAD_ID_NOT_FOUND, /* Model ID not found during load */
	/* 0x2 */ ML_FW_ERR_LOAD_LUT_OVERFLOW,	 /* Lookup table overflow at load */
	/* 0x3 */ ML_FW_ERR_ID_IN_USE,		 /* Model ID already in use */
	/* 0x4 */ ML_FW_ERR_INVALID_TILEMASK,	 /* Invalid OCM tilemask */
	/* 0x5 */ ML_FW_ERR_RUN_LUT_OVERFLOW,	 /* Lookup table overflow at run */
	/* 0x6 */ ML_FW_ERR_RUN_ID_NOT_FOUND,	 /* Model ID not found during run */
	/* 0x7 */ ML_FW_ERR_COMMAND_NOTSUP,	 /* Unsupported command */
	/* 0x8 */ ML_FW_ERR_DDR_ADDR_RANGE,	 /* DDR address out of range */
	/* 0x9 */ ML_FW_ERR_NUM_BATCHES_INVALID, /* Invalid number of batches */
	/* 0xA */ ML_FW_ERR_INSSYNC_TIMEOUT,	 /* INS sync timeout */
};

/* Driver error sub-type */
enum cn10k_ml_error_stype_driver {
	/* 0x0 */ ML_DRIVER_ERR_NOERR = 0, /* No error */
	/* 0x1 */ ML_DRIVER_ERR_UNKNOWN,   /* Unable to determine error sub-type */
	/* 0x2 */ ML_DRIVER_ERR_EXCEPTION, /* Firmware exception */
	/* 0x3 */ ML_DRIVER_ERR_FW_ERROR,  /* Unknown firmware error */
};

/* Error structure */
union cn10k_ml_error_code {
	struct {
		/* Error type */
		uint64_t etype : 4;

		/* Error sub-type */
		uint64_t stype : 60;
	} s;

	/* WORD 0 */
	uint64_t u64;
};

/* ML firmware structure */
struct cn10k_ml_fw {
	/* Device reference */
	struct cn10k_ml_dev *cn10k_mldev;

	/* Firmware file path */
	const char *path;

	/* Enable DPE warnings */
	int enable_dpe_warnings;

	/* Report DPE warnings */
	int report_dpe_warnings;

	/* Data buffer */
	uint8_t *data;

	/* Firmware load / handshake request structure */
	struct cnxk_ml_req *req;
};

/* Extended stats types enum */
enum cn10k_ml_xstats_type {
	/* Number of models loaded */
	nb_models_loaded,

	/* Number of models unloaded */
	nb_models_unloaded,

	/* Number of models started */
	nb_models_started,

	/* Number of models stopped */
	nb_models_stopped,

	/* Average inference hardware latency */
	avg_hw_latency,

	/* Minimum hardware latency */
	min_hw_latency,

	/* Maximum hardware latency */
	max_hw_latency,

	/* Average firmware latency */
	avg_fw_latency,

	/* Minimum firmware latency */
	min_fw_latency,

	/* Maximum firmware latency */
	max_fw_latency,
};

/* Extended stats function type enum. */
enum cn10k_ml_xstats_fn_type {
	/* Device function */
	CN10K_ML_XSTATS_FN_DEVICE,

	/* Model function */
	CN10K_ML_XSTATS_FN_MODEL,
};

/* Function pointer to get xstats for a type */
typedef uint64_t (*cn10k_ml_xstats_fn)(struct rte_ml_dev *dev, uint16_t obj_idx,
				       enum cn10k_ml_xstats_type stat);

/* Extended stats entry structure */
struct cn10k_ml_xstats_entry {
	/* Name-ID map */
	struct rte_ml_dev_xstats_map map;

	/* xstats mode, device or model */
	enum rte_ml_dev_xstats_mode mode;

	/* Type of xstats */
	enum cn10k_ml_xstats_type type;

	/* xstats function */
	enum cn10k_ml_xstats_fn_type fn_id;

	/* Object ID, model ID for model stat type */
	uint16_t obj_idx;

	/* Allowed to reset the stat */
	uint8_t reset_allowed;

	/* An offset to be taken away to emulate resets */
	uint64_t reset_value;
};

/* Extended stats data */
struct cn10k_ml_xstats {
	/* Pointer to xstats entries */
	struct cn10k_ml_xstats_entry *entries;

	/* Store num stats and offset of the stats for each model */
	uint16_t count_per_model[ML_CNXK_MAX_MODELS];
	uint16_t offset_for_model[ML_CNXK_MAX_MODELS];
	uint16_t count_mode_device;
	uint16_t count_mode_model;
	uint16_t count;
};

/* Device private data */
struct cn10k_ml_dev {
	/* Device ROC */
	struct roc_ml roc;

	/* Firmware */
	struct cn10k_ml_fw fw;

	/* OCM info */
	struct cn10k_ml_ocm ocm;

	/* Extended stats data */
	struct cn10k_ml_xstats xstats;

	/* Enable / disable model data caching */
	int cache_model_data;

	/* Use spinlock version of ROC enqueue */
	int hw_queue_lock;

	/* OCM page size */
	int ocm_page_size;

	/* JCMD enqueue function handler */
	bool (*ml_jcmdq_enqueue)(struct roc_ml *roc_ml, struct ml_job_cmd_s *job_cmd);

	/* Poll handling function pointers */
	void (*set_poll_addr)(struct cnxk_ml_req *req);
	void (*set_poll_ptr)(struct cnxk_ml_req *req);
	uint64_t (*get_poll_ptr)(struct cnxk_ml_req *req);
};

uint64_t cn10k_ml_fw_flags_get(struct cn10k_ml_fw *fw);
int cn10k_ml_fw_load(struct cnxk_ml_dev *cnxk_mldev);
void cn10k_ml_fw_unload(struct cnxk_ml_dev *cnxk_mldev);

#endif /* _CN10K_ML_DEV_H_ */

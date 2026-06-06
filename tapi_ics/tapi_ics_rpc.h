/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief ICS TAPI: RPC client wrappers
 *
 * Client wrappers of the ics_* RPCs, see ics_rpc.x.m4. Tests use
 * tapi_ics.h; these are the calls behind it, one per RPC.
 */

#ifndef __TAPI_ICS_RPC_H__
#define __TAPI_ICS_RPC_H__

#include "te_errno.h"
#include "te_string.h"
#include "rcf_rpc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Read registers/bits (raw tab-separated result text).
 *
 * @param[in]  rpcs         RPC server on the agent.
 * @param[in]  proto        Protocol wire value.
 * @param[in]  host         Endpoint host.
 * @param[in]  port         TCP port.
 * @param[in]  unit         Unit/slave id.
 * @param[in]  area         Data-area wire value.
 * @param[in]  addr         Starting address.
 * @param[in]  count        How many to read.
 * @param[in]  timeout_ms   Response timeout, ms.
 * @param[out] result       The values, or @c NULL.
 *
 * @return Status code.
 */
extern te_errno rpc_ics_read(rcf_rpc_server *rpcs, int proto,
                             const char *host, int port, int unit, int area,
                             int addr, int count, int timeout_ms,
                             te_string *result);

/**
 * Write one register or coil.
 *
 * @return Status code; @c 0 means the write was accepted.
 */
extern te_errno rpc_ics_write(rcf_rpc_server *rpcs, int proto,
                              const char *host, int port, int unit, int area,
                              int addr, int value, int timeout_ms);

/**
 * Read a device identification (raw result text).
 *
 * @return Status code.
 */
extern te_errno rpc_ics_device_id(rcf_rpc_server *rpcs, int proto,
                                  const char *host, int port, int unit,
                                  int timeout_ms, te_string *result);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !__TAPI_ICS_RPC_H__ */

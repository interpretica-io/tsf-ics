/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief Talking to an industrial control device from a test
 *
 * @defgroup tapi_ics Industrial control protocols (tapi_ics)
 * @{
 *
 * Reading and writing an industrial control device from a Test Agent
 * over a real protocol library in the agent's RPC server - for now
 * **Modbus TCP over libmodbus**. A test reads holding/input registers
 * and coils/discrete inputs, writes a register or coil, and asks for a
 * device identification.
 *
 * - tapi_ics_read() / tapi_ics_write() / tapi_ics_device_id() are the
 *   functional primitives;
 * - @ref tapi_ics_audit (tapi_ics_audit.h) reads an endpoint as a
 *   security posture through tsf-cybersec - the point being that Modbus
 *   has no authentication, so a reachable endpoint answers anyone.
 *
 * DNP3 and S7comm are planned behind the same #tapi_ics_proto enum but
 * not implemented yet.
 *
 * @code
 * tapi_ics_endpoint ep = {
 *     .proto = TAPI_ICS_MODBUS_TCP, .host = "192.0.2.10",
 *     .port = 502, .unit = 1, .timeout_ms = 2000,
 * };
 * int regs[4];
 * int n;
 *
 * CHECK_RC(tapi_ics_read(rpcs, &ep, TAPI_ICS_HOLDING, 0, 4, regs, &n));
 * @endcode
 */

#ifndef __TAPI_ICS_H__
#define __TAPI_ICS_H__

#include "te_defs.h"
#include "te_errno.h"
#include "te_string.h"
#include "rcf_rpc.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Which protocol. The values are the RPC's wire values. */
typedef enum tapi_ics_proto {
    TAPI_ICS_MODBUS_TCP = 0,    /**< Modbus TCP, over libmodbus. */
} tapi_ics_proto;

/** Which Modbus data area. The values are the RPC's wire values. */
typedef enum tapi_ics_area {
    TAPI_ICS_HOLDING = 0,       /**< Holding registers (read/write). */
    TAPI_ICS_INPUT_REG = 1,     /**< Input registers (read-only). */
    TAPI_ICS_COIL = 2,          /**< Coils (read/write, 1-bit). */
    TAPI_ICS_DISCRETE = 3,      /**< Discrete inputs (read-only, 1-bit). */
} tapi_ics_area;

/** An endpoint to talk to: everything but the operation. */
typedef struct tapi_ics_endpoint {
    /** The protocol. */
    tapi_ics_proto proto;
    /** IP/hostname. */
    const char *host;
    /** TCP port (Modbus is usually 502). */
    int port;
    /** Unit/slave id. */
    int unit;
    /** Response timeout, ms, or @c 0 for the library default. */
    int timeout_ms;
} tapi_ics_endpoint;

/**
 * Read a run of registers or bits from an endpoint.
 *
 * @param[in]  rpcs     RPC server on the agent.
 * @param[in]  ep       The endpoint.
 * @param[in]  area     Which data area.
 * @param[in]  addr     Starting address within the area.
 * @param[in]  count    How many to read.
 * @param[out] values   Caller array of at least @p count ints; a
 *                      register fills 0..65535, a bit 0 or 1.
 * @param[out] n_read   How many were actually read, or @c NULL.
 *
 * @return Status code.
 * @retval TE_ECONNREFUSED  The endpoint refused the connection.
 * @retval TE_ETIMEDOUT     No answer in time.
 * @retval TE_EPROTO        The device answered with an exception.
 */
extern te_errno tapi_ics_read(rcf_rpc_server *rpcs,
                              const tapi_ics_endpoint *ep,
                              tapi_ics_area area, int addr, int count,
                              int *values, int *n_read);

/**
 * Write one register (@c TAPI_ICS_HOLDING) or coil (@c TAPI_ICS_COIL).
 *
 * This changes device state; use it only against a device you are
 * authorized to test.
 *
 * @param[in]  rpcs     RPC server on the agent.
 * @param[in]  ep       The endpoint.
 * @param[in]  area     @c TAPI_ICS_HOLDING or @c TAPI_ICS_COIL.
 * @param[in]  addr     Address within the area.
 * @param[in]  value    Value to write (0/1 for a coil).
 *
 * @return Status code; @c 0 means the write was accepted.
 * @retval TE_EINVAL        The area is not writable.
 */
extern te_errno tapi_ics_write(rcf_rpc_server *rpcs,
                               const tapi_ics_endpoint *ep,
                               tapi_ics_area area, int addr, int value);

/**
 * Read a device identification (Modbus Report Slave ID).
 *
 * @param[in]  rpcs     RPC server on the agent.
 * @param[in]  ep       The endpoint.
 * @param[out] id       String to append the id bytes to (two-hex each).
 *
 * @return Status code.
 */
extern te_errno tapi_ics_device_id(rcf_rpc_server *rpcs,
                                   const tapi_ics_endpoint *ep,
                                   te_string *id);

/**
 * Spell out a protocol.
 *
 * @param proto         The protocol.
 *
 * @return A static string, never @c NULL.
 */
extern const char *tapi_ics_proto2str(tapi_ics_proto proto);

/**
 * Spell out a data area.
 *
 * @param area          The area.
 *
 * @return A static string, never @c NULL.
 */
extern const char *tapi_ics_area2str(tapi_ics_area area);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !__TAPI_ICS_H__ */

/**@} <!-- END tapi_ics --> */

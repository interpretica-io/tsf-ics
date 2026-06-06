/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief Agent-side industrial-control-protocol client
 *
 * Talking to an industrial control device from an agent over a real
 * protocol library - for now **Modbus TCP over libmodbus**
 * (@c modbus.h, @c -lmodbus): the library is linked into the agent and
 * called in-process, nothing is spawned. The agent and its RPC server
 * both link this; the RPCs (see ics_rpc.x.m4) are thin wrappers over
 * these functions.
 *
 * Modbus has no authentication: a reachable endpoint answers anyone.
 * These primitives are therefore the building blocks of both a
 * functional probe and a security posture - reading a register is the
 * same call whether a test wants its value or wants to prove that the
 * value is readable without credentials. The write primitive acts on a
 * real device; the audit uses it reversibly (it writes a register's
 * own current value back), and a caller using it directly must take the
 * same care.
 *
 * Each call opens a connection, does its one operation and closes it,
 * so nothing has to survive between calls. Results come back as
 * tab-separated text - the engine side parses them.
 */

#ifndef __TA_ICS_H__
#define __TA_ICS_H__

#include "te_errno.h"
#include "te_string.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Which protocol. Only Modbus TCP is implemented so far. */
enum {
    TA_ICS_MODBUS_TCP = 0,  /**< Modbus TCP, over libmodbus. */
};

/** Which Modbus data area an address refers to. */
enum {
    TA_ICS_HOLDING = 0,     /**< Holding registers (read/write, 16-bit). */
    TA_ICS_INPUT_REG = 1,   /**< Input registers (read-only, 16-bit). */
    TA_ICS_COIL = 2,        /**< Coils (read/write, 1-bit). */
    TA_ICS_DISCRETE = 3,    /**< Discrete inputs (read-only, 1-bit). */
};

/**
 * Read a run of registers or bits from an endpoint.
 *
 * @param[in]  proto        @c TA_ICS_* protocol (only Modbus TCP yet).
 * @param[in]  host         Endpoint IP/hostname.
 * @param[in]  port         TCP port (Modbus is usually 502).
 * @param[in]  unit         Unit/slave id.
 * @param[in]  area         @c TA_ICS_* data area.
 * @param[in]  addr         Starting address within the area.
 * @param[in]  count        How many to read.
 * @param[in]  timeout_ms   Response timeout, ms (0 for the default).
 * @param[out] result       The values, tab-separated on one line,
 *                          decimal (registers) or @c 0 / @c 1 (bits).
 *
 * @return Status code.
 * @retval TE_ECONNREFUSED  The endpoint refused the connection.
 * @retval TE_ETIMEDOUT     No answer in time.
 * @retval TE_EPROTO        The device answered with an exception.
 */
extern te_errno ta_ics_read(int proto, const char *host, int port, int unit,
                            int area, int addr, int count, int timeout_ms,
                            te_string *result);

/**
 * Write one register or coil to an endpoint.
 *
 * Only @c TA_ICS_HOLDING (a register) and @c TA_ICS_COIL (a bit) are
 * writable. This changes device state; use it only against a device you
 * are authorized to test. The @ref tapi_ics_audit uses it reversibly.
 *
 * @param[in]  proto        @c TA_ICS_* protocol.
 * @param[in]  host         Endpoint IP/hostname.
 * @param[in]  port         TCP port.
 * @param[in]  unit         Unit/slave id.
 * @param[in]  area         @c TA_ICS_HOLDING or @c TA_ICS_COIL.
 * @param[in]  addr         Address within the area.
 * @param[in]  value        Value to write (0/1 for a coil).
 * @param[in]  timeout_ms   Response timeout, ms (0 for the default).
 *
 * @return Status code; @c 0 means the write was accepted.
 * @retval TE_EINVAL        The area is not writable.
 */
extern te_errno ta_ics_write(int proto, const char *host, int port, int unit,
                             int area, int addr, int value, int timeout_ms);

/**
 * Read a device's identification, if it answers one.
 *
 * Modbus Report Slave ID (function 0x11): a short vendor blob whose
 * shape is device-specific.
 *
 * @param[in]  proto        @c TA_ICS_* protocol.
 * @param[in]  host         Endpoint IP/hostname.
 * @param[in]  port         TCP port.
 * @param[in]  unit         Unit/slave id.
 * @param[in]  timeout_ms   Response timeout, ms (0 for the default).
 * @param[out] result       The id bytes, space-separated two-hex each.
 *
 * @return Status code.
 */
extern te_errno ta_ics_device_id(int proto, const char *host, int port,
                                 int unit, int timeout_ms, te_string *result);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !__TA_ICS_H__ */

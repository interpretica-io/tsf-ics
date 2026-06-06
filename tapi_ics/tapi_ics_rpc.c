/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief ICS TAPI: RPC client wrappers
 *
 * The rcf_rpc_call() boilerplate behind tapi_ics. The RPCs return
 * te_errno; an RPC transport failure is mapped to TE_ECORRUPTED.
 */

#define TE_LGR_USER     "TAPI ICS RPC"

#include "te_config.h"

#include <string.h>

#include "te_defs.h"
#include "te_errno.h"
#include "te_string.h"
#include "logger_api.h"
#include "tapi_rpc_internal.h"
#include "tarpc.h"

#include "tapi_ics_rpc.h"

#define CHECK_RPC_ERRNO_UNCHANGED(_func, _var) \
    CHECK_RETVAL_VAR_ERR_COND(_func, _var, false,                    \
                              TE_RC(TE_TAPI, TE_ECORRUPTED), false)

/* Append an RPC string result, when there is one. */
static void
take_string(te_string *dst, const char *src)
{
    if (dst != NULL && src != NULL)
        te_string_append(dst, "%s", src);
}

/* See description in tapi_ics_rpc.h */
te_errno
rpc_ics_read(rcf_rpc_server *rpcs, int proto, const char *host, int port,
             int unit, int area, int addr, int count, int timeout_ms,
             te_string *result)
{
    tarpc_ics_read_in in;
    tarpc_ics_read_out out;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.proto = proto;
    in.host = (char *)(host != NULL ? host : "");
    in.port = port;
    in.unit = unit;
    in.area = area;
    in.addr = addr;
    in.count = count;
    in.timeout_ms = timeout_ms;

    rcf_rpc_call(rpcs, "ics_read", &in, &out);
    CHECK_RPC_ERRNO_UNCHANGED(ics_read, out.retval);
    TAPI_RPC_LOG(rpcs, ics_read, "%s:%d/%d area=%d addr=%d count=%d", "%r",
                 host != NULL ? host : "", port, unit, area, addr, count,
                 out.retval);

    if (out.retval == 0)
        take_string(result, out.result);
    RETVAL_TE_ERRNO(ics_read, out.retval);
}

/* See description in tapi_ics_rpc.h */
te_errno
rpc_ics_write(rcf_rpc_server *rpcs, int proto, const char *host, int port,
              int unit, int area, int addr, int value, int timeout_ms)
{
    tarpc_ics_write_in in;
    tarpc_ics_write_out out;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.proto = proto;
    in.host = (char *)(host != NULL ? host : "");
    in.port = port;
    in.unit = unit;
    in.area = area;
    in.addr = addr;
    in.value = value;
    in.timeout_ms = timeout_ms;

    rcf_rpc_call(rpcs, "ics_write", &in, &out);
    CHECK_RPC_ERRNO_UNCHANGED(ics_write, out.retval);
    TAPI_RPC_LOG(rpcs, ics_write, "%s:%d/%d area=%d addr=%d value=%d", "%r",
                 host != NULL ? host : "", port, unit, area, addr, value,
                 out.retval);

    RETVAL_TE_ERRNO(ics_write, out.retval);
}

/* See description in tapi_ics_rpc.h */
te_errno
rpc_ics_device_id(rcf_rpc_server *rpcs, int proto, const char *host, int port,
                  int unit, int timeout_ms, te_string *result)
{
    tarpc_ics_device_id_in in;
    tarpc_ics_device_id_out out;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.proto = proto;
    in.host = (char *)(host != NULL ? host : "");
    in.port = port;
    in.unit = unit;
    in.timeout_ms = timeout_ms;

    rcf_rpc_call(rpcs, "ics_device_id", &in, &out);
    CHECK_RPC_ERRNO_UNCHANGED(ics_device_id, out.retval);
    TAPI_RPC_LOG(rpcs, ics_device_id, "%s:%d/%d", "%r",
                 host != NULL ? host : "", port, unit, out.retval);

    if (out.retval == 0)
        take_string(result, out.result);
    RETVAL_TE_ERRNO(ics_device_id, out.retval);
}

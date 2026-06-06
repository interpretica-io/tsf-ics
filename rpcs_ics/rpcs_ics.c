/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief ICS RPC server library
 *
 * The ics_* RPCs (see ics_rpc.x.m4) on top of ta_ics.
 * TARPC_FUNC_STATIC() binds an RPC to the function of the same name,
 * so each RPC has a plain C function first and the wrapper after it.
 */

#define TE_LGR_USER     "RPC ICS"

#include "te_config.h"

#include <string.h>

#include "te_defs.h"
#include "te_errno.h"
#include "te_alloc.h"
#include "te_str.h"
#include "te_string.h"
#include "rpc_server.h"

#include "ta_ics.h"

/* Hand a te_string result over to an RPC string field (never NULL). */
static char *
take(te_string *str)
{
    return str->ptr != NULL ? str->ptr : TE_STRDUP("");
}

static te_errno
ics_read(int proto, const char *host, int port, int unit, int area,
         int addr, int count, int timeout_ms, char **result)
{
    te_string r = TE_STRING_INIT;
    te_errno rc = ta_ics_read(proto, host, port, unit, area, addr, count,
                              timeout_ms, &r);

    *result = take(&r);
    return rc;
}

TARPC_FUNC_STATIC(ics_read, {},
{
    MAKE_CALL(out->retval = func(in->proto, in->host, in->port, in->unit,
                                 in->area, in->addr, in->count,
                                 in->timeout_ms, &out->result));
    out->common.errno_changed = false;
})

static te_errno
ics_write(int proto, const char *host, int port, int unit, int area,
          int addr, int value, int timeout_ms)
{
    return ta_ics_write(proto, host, port, unit, area, addr, value,
                        timeout_ms);
}

TARPC_FUNC_STATIC(ics_write, {},
{
    MAKE_CALL(out->retval = func(in->proto, in->host, in->port, in->unit,
                                 in->area, in->addr, in->value,
                                 in->timeout_ms));
    out->common.errno_changed = false;
})

static te_errno
ics_device_id(int proto, const char *host, int port, int unit,
              int timeout_ms, char **result)
{
    te_string r = TE_STRING_INIT;
    te_errno rc = ta_ics_device_id(proto, host, port, unit, timeout_ms, &r);

    *result = take(&r);
    return rc;
}

TARPC_FUNC_STATIC(ics_device_id, {},
{
    MAKE_CALL(out->retval = func(in->proto, in->host, in->port, in->unit,
                                 in->timeout_ms, &out->result));
    out->common.errno_changed = false;
})

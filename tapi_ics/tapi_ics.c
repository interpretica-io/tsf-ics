/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief Talking to an industrial control device from a test
 *
 * The engine-side layer over the ics_* RPCs: it drives the agent's
 * Modbus client and parses the tab-separated read result into ints.
 */

#define TE_LGR_USER     "TAPI ICS"

#include "te_config.h"

#include <stdlib.h>
#include <string.h>

#include "te_defs.h"
#include "te_errno.h"
#include "te_str.h"
#include "te_string.h"
#include "logger_api.h"

#include "tapi_ics.h"
#include "tapi_ics_rpc.h"

/* See description in tapi_ics.h */
const char *
tapi_ics_proto2str(tapi_ics_proto proto)
{
    switch (proto)
    {
        case TAPI_ICS_MODBUS_TCP:
            return "modbus-tcp";
        default:
            return "unknown";
    }
}

/* See description in tapi_ics.h */
const char *
tapi_ics_area2str(tapi_ics_area area)
{
    switch (area)
    {
        case TAPI_ICS_HOLDING:
            return "holding-register";
        case TAPI_ICS_INPUT_REG:
            return "input-register";
        case TAPI_ICS_COIL:
            return "coil";
        case TAPI_ICS_DISCRETE:
            return "discrete-input";
        default:
            return "unknown";
    }
}

/* See description in tapi_ics.h */
te_errno
tapi_ics_read(rcf_rpc_server *rpcs, const tapi_ics_endpoint *ep,
              tapi_ics_area area, int addr, int count, int *values,
              int *n_read)
{
    te_string raw = TE_STRING_INIT;
    const char *p;
    int got = 0;
    te_errno rc;

    if (n_read != NULL)
        *n_read = 0;

    rc = rpc_ics_read(rpcs, (int)ep->proto, ep->host, ep->port, ep->unit,
                      (int)area, addr, count, ep->timeout_ms, &raw);
    if (rc != 0)
    {
        te_string_free(&raw);
        return rc;
    }

    /* The values are tab-separated decimals on one line. */
    p = te_string_value(&raw);
    while (p != NULL && *p != '\0' && got < count)
    {
        char *end = NULL;
        long v = strtol(p, &end, 10);

        if (end == p)
            break;
        if (values != NULL)
            values[got] = (int)v;
        got++;
        p = (*end == '\t') ? end + 1 : NULL;
    }

    if (n_read != NULL)
        *n_read = got;

    te_string_free(&raw);

    return 0;
}

/* See description in tapi_ics.h */
te_errno
tapi_ics_write(rcf_rpc_server *rpcs, const tapi_ics_endpoint *ep,
               tapi_ics_area area, int addr, int value)
{
    if (area != TAPI_ICS_HOLDING && area != TAPI_ICS_COIL)
    {
        ERROR("tapi_ics_write: %s is not writable", tapi_ics_area2str(area));
        return TE_RC(TE_TAPI, TE_EINVAL);
    }

    return rpc_ics_write(rpcs, (int)ep->proto, ep->host, ep->port, ep->unit,
                         (int)area, addr, value, ep->timeout_ms);
}

/* See description in tapi_ics.h */
te_errno
tapi_ics_device_id(rcf_rpc_server *rpcs, const tapi_ics_endpoint *ep,
                   te_string *id)
{
    return rpc_ics_device_id(rpcs, (int)ep->proto, ep->host, ep->port,
                             ep->unit, ep->timeout_ms, id);
}

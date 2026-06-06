/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief Agent-side Modbus TCP client over libmodbus
 *
 * Written against the libmodbus synchronous client API (modbus_new_tcp,
 * modbus_connect, the read/write functions). Each call opens a TCP
 * connection, does one operation and closes it, so nothing survives
 * between calls. DNP3 and S7 are not implemented yet; a non-Modbus
 * protocol is refused with TE_EOPNOTSUPP.
 */

#define TE_LGR_USER     "TA ICS"

#include "te_config.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*
 * The libmodbus header is <modbus/modbus.h> on a normal install and
 * <modbus.h> when the include dir points straight at it (as pkg-config
 * cflags do); accept both.
 */
#if defined(__has_include)
#  if __has_include(<modbus/modbus.h>)
#    include <modbus/modbus.h>
#  else
#    include <modbus.h>
#  endif
#else
#  include <modbus/modbus.h>
#endif

#include "te_defs.h"
#include "te_errno.h"
#include "te_alloc.h"
#include "te_str.h"
#include "te_string.h"
#include "logger_api.h"

#include "ta_ics.h"

/** Map the errno libmodbus left behind to a TE status, logging it. */
static te_errno
ics_errno(const char *what)
{
    int e = errno;

    ERROR("%s: %s", what, modbus_strerror(e));
    switch (e)
    {
        case ECONNREFUSED:
            return TE_RC(TE_TA_UNIX, TE_ECONNREFUSED);
        case ETIMEDOUT:
            return TE_RC(TE_TA_UNIX, TE_ETIMEDOUT);
        case EHOSTUNREACH:
        case ENETUNREACH:
            return TE_RC(TE_TA_UNIX, TE_EHOSTUNREACH);
        case EINVAL:
            return TE_RC(TE_TA_UNIX, TE_EINVAL);
        default:
            /*
             * libmodbus exceptions (illegal address/function/value) and
             * everything else: the device answered, but not usefully.
             */
            return TE_RC(TE_TA_UNIX, TE_EPROTO);
    }
}

/**
 * Open a Modbus TCP connection to @p host:@p port, unit @p unit, with a
 * response timeout of @p timeout_ms. On success @p *out holds the
 * connected context (close with modbus_close()+modbus_free()).
 */
static te_errno
ics_modbus_open(const char *host, int port, int unit, int timeout_ms,
                modbus_t **out)
{
    modbus_t *ctx;

    *out = NULL;
    ctx = modbus_new_tcp(host, port);
    if (ctx == NULL)
        return ics_errno("modbus_new_tcp");

    modbus_set_slave(ctx, unit);
    if (timeout_ms > 0)
    {
        modbus_set_response_timeout(ctx, (uint32_t)(timeout_ms / 1000),
                                    (uint32_t)((timeout_ms % 1000) * 1000));
    }

    if (modbus_connect(ctx) == -1)
    {
        te_errno rc = ics_errno("modbus_connect");

        modbus_free(ctx);
        return rc;
    }

    *out = ctx;
    return 0;
}

/* See description in ta_ics.h */
te_errno
ta_ics_read(int proto, const char *host, int port, int unit, int area,
            int addr, int count, int timeout_ms, te_string *result)
{
    modbus_t *ctx = NULL;
    te_errno rc;
    int got = -1;
    int i;

    if (proto != TA_ICS_MODBUS_TCP)
        return TE_RC(TE_TA_UNIX, TE_EOPNOTSUPP);
    if (count <= 0)
        return TE_RC(TE_TA_UNIX, TE_EINVAL);

    rc = ics_modbus_open(host, port, unit, timeout_ms, &ctx);
    if (rc != 0)
        return rc;

    if (area == TA_ICS_HOLDING || area == TA_ICS_INPUT_REG)
    {
        uint16_t *regs = TE_ALLOC((size_t)count * sizeof(*regs));

        got = area == TA_ICS_HOLDING ?
              modbus_read_registers(ctx, addr, count, regs) :
              modbus_read_input_registers(ctx, addr, count, regs);
        if (got > 0)
        {
            for (i = 0; i < got; i++)
                te_string_append(result, "%s%u", i != 0 ? "\t" : "", regs[i]);
        }
        free(regs);
    }
    else if (area == TA_ICS_COIL || area == TA_ICS_DISCRETE)
    {
        uint8_t *bits = TE_ALLOC((size_t)count * sizeof(*bits));

        got = area == TA_ICS_COIL ?
              modbus_read_bits(ctx, addr, count, bits) :
              modbus_read_input_bits(ctx, addr, count, bits);
        if (got > 0)
        {
            for (i = 0; i < got; i++)
                te_string_append(result, "%s%d", i != 0 ? "\t" : "",
                                 bits[i] != 0);
        }
        free(bits);
    }
    else
    {
        rc = TE_RC(TE_TA_UNIX, TE_EINVAL);
    }

    if (rc == 0 && got < 0)
        rc = ics_errno("modbus read");

    modbus_close(ctx);
    modbus_free(ctx);

    return rc;
}

/* See description in ta_ics.h */
te_errno
ta_ics_write(int proto, const char *host, int port, int unit, int area,
             int addr, int value, int timeout_ms)
{
    modbus_t *ctx = NULL;
    te_errno rc;
    int done = -1;

    if (proto != TA_ICS_MODBUS_TCP)
        return TE_RC(TE_TA_UNIX, TE_EOPNOTSUPP);
    if (area != TA_ICS_HOLDING && area != TA_ICS_COIL)
        return TE_RC(TE_TA_UNIX, TE_EINVAL);

    rc = ics_modbus_open(host, port, unit, timeout_ms, &ctx);
    if (rc != 0)
        return rc;

    done = area == TA_ICS_HOLDING ?
           modbus_write_register(ctx, addr, (uint16_t)value) :
           modbus_write_bit(ctx, addr, value != 0);
    if (done < 0)
        rc = ics_errno("modbus write");

    modbus_close(ctx);
    modbus_free(ctx);

    return rc;
}

/* See description in ta_ics.h */
te_errno
ta_ics_device_id(int proto, const char *host, int port, int unit,
                 int timeout_ms, te_string *result)
{
    modbus_t *ctx = NULL;
    uint8_t id[256];
    te_errno rc;
    int got;
    int i;

    if (proto != TA_ICS_MODBUS_TCP)
        return TE_RC(TE_TA_UNIX, TE_EOPNOTSUPP);

    rc = ics_modbus_open(host, port, unit, timeout_ms, &ctx);
    if (rc != 0)
        return rc;

    got = modbus_report_slave_id(ctx, (int)sizeof(id), id);
    if (got < 0)
        rc = ics_errno("modbus_report_slave_id");
    else
    {
        for (i = 0; i < got; i++)
            te_string_append(result, "%s%02x", i != 0 ? " " : "", id[i]);
    }

    modbus_close(ctx);
    modbus_free(ctx);

    return rc;
}

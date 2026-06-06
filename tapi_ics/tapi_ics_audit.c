/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief What an industrial endpoint is worth as a security posture
 *
 * Probes a Modbus endpoint with tapi_ics_* and turns what answered - a
 * readable area, a device id, an accepted write - into tsf-cybersec
 * findings. The write probe is reversible and off by default.
 */

#define TE_LGR_USER     "TAPI ICS AUDIT"

#include "te_config.h"

#include <string.h>

#include "te_defs.h"
#include "te_errno.h"
#include "te_str.h"
#include "te_string.h"
#include "logger_api.h"

#include "tapi_ics.h"
#include "tapi_ics_audit.h"

/* See description in tapi_ics_audit.h */
const tapi_ics_audit_policy tapi_ics_default_audit_policy = {
    .probe_area = TAPI_ICS_HOLDING,
    .probe_addr = 0,
    .read_device_id = true,
    .attempt_write = false,
};

/* See description in tapi_ics_audit.h */
te_errno
tapi_ics_audit(rcf_rpc_server *rpcs, const tapi_ics_endpoint *ep,
               const tapi_ics_audit_policy *policy,
               tapi_cybersec_report *report)
{
    char subject[128];
    int value = 0;
    int n = 0;
    bool answered = false;
    te_errno rc;

    if (policy == NULL)
        policy = &tapi_ics_default_audit_policy;

    TE_SPRINTF(subject, "%s:%d/%d", ep->host != NULL ? ep->host : "?",
               ep->port, ep->unit);

    /* 1. Is the data area readable without authentication? */
    rc = tapi_ics_read(rpcs, ep, policy->probe_area, policy->probe_addr, 1,
                       &value, &n);
    if (rc == 0 && n > 0)
    {
        answered = true;
        tapi_cybersec_report_add(report, TAPI_CYBERSEC_SEV_INFO,
            "ics.readable", subject,
            "the %s at address %d is readable with no authentication "
            "(value %d)", tapi_ics_area2str(policy->probe_area),
            policy->probe_addr, value);
    }

    /* 2. Does it fingerprint itself via Report Slave ID? */
    if (policy->read_device_id)
    {
        te_string id = TE_STRING_INIT;

        if (tapi_ics_device_id(rpcs, ep, &id) == 0 && id.len != 0)
        {
            answered = true;
            tapi_cybersec_report_add(report, TAPI_CYBERSEC_SEV_LOW,
                "ics.device-id", subject,
                "Report Slave ID returned a device fingerprint: %s",
                te_string_value(&id));
        }
        te_string_free(&id);
    }

    /*
     * 3. The serious one: is a write accepted with no authentication?
     * Reversible - read the holding register and write its own value
     * straight back, so the register is unchanged whether or not the
     * write is accepted.
     */
    if (policy->attempt_write)
    {
        int original = 0;
        int count = 0;

        rc = tapi_ics_read(rpcs, ep, TAPI_ICS_HOLDING, policy->probe_addr, 1,
                           &original, &count);
        if (rc == 0 && count > 0)
        {
            if (tapi_ics_write(rpcs, ep, TAPI_ICS_HOLDING, policy->probe_addr,
                               original) == 0)
            {
                answered = true;
                tapi_cybersec_report_add(report, TAPI_CYBERSEC_SEV_CRITICAL,
                    "ics.writable", subject,
                    "a holding-register write at address %d was accepted "
                    "with no authentication (value written back unchanged)",
                    policy->probe_addr);
            }
        }
    }

    if (!answered)
    {
        tapi_cybersec_report_add(report, TAPI_CYBERSEC_SEV_INFO,
            "ics.not-assessed", subject,
            "the endpoint did not answer a Modbus request");
    }

    return 0;
}

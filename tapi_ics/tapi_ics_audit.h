/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief What an industrial endpoint is worth as a security posture
 *
 * @defgroup tapi_ics_audit ICS security posture
 * @ingroup tapi_ics
 * @{
 *
 * A Modbus endpoint read as a security posture and reported through
 * tsf-cybersec. Modbus has no authentication by design, so the posture
 * is not "the request was forged" - every request can be - but "a
 * control surface answered a request nobody authenticated": a readable
 * register, a device identification that fingerprints the device, and
 * - the serious one - a register whose write was accepted.
 *
 * | Finding | Severity | Raised when |
 * |---|---|---|
 * | @c ics.readable | info | a data area answered a read with no auth |
 * | @c ics.device-id | low | Report Slave ID returned a fingerprint |
 * | @c ics.writable | critical | a register write was accepted with no auth |
 * | @c ics.not-assessed | info | the endpoint did not answer |
 *
 * The write check is **off by default**. When it is on it acts on a
 * real device, so it is done reversibly: the probed register's own
 * current value is read and then written straight back, so the write
 * function is exercised without changing what the register holds. Even
 * so, turn it on only against a device you own or are engaged to test
 * (see "Authorized use only" in the README), and never where a write -
 * even a no-op one - could disturb a live process.
 *
 * A finding's subject is @c "host:port/unit", stable between runs.
 */

#ifndef __TAPI_ICS_AUDIT_H__
#define __TAPI_ICS_AUDIT_H__

#include "te_errno.h"
#include "rcf_rpc.h"

#include "tapi_cybersec.h"
#include "tapi_ics.h"

#ifdef __cplusplus
extern "C" {
#endif

/** What to probe on an endpoint, and how far to go. */
typedef struct tapi_ics_audit_policy {
    /** The data area to probe for readability. */
    tapi_ics_area probe_area;
    /** The address within that area to probe. */
    int probe_addr;
    /** Ask for a device identification (Report Slave ID). */
    bool read_device_id;
    /**
     * Test whether a holding-register write is accepted (the write-back
     * of the register's own value). Off by default; acts on the device.
     */
    bool attempt_write;
} tapi_ics_audit_policy;

/**
 * The default: probe the holding register at address 0, read the device
 * identification, and do NOT attempt a write.
 */
extern const tapi_ics_audit_policy tapi_ics_default_audit_policy;

/**
 * Read an endpoint's Modbus posture into @p report.
 *
 * @param[in]  rpcs     RPC server on the agent.
 * @param[in]  ep       The endpoint.
 * @param[in]  policy   What to probe, or @c NULL for the default.
 * @param[out] report   Report to append findings to.
 *
 * @return Status code of reading the posture, not its verdict.
 */
extern te_errno tapi_ics_audit(rcf_rpc_server *rpcs,
                               const tapi_ics_endpoint *ep,
                               const tapi_ics_audit_policy *policy,
                               tapi_cybersec_report *report);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !__TAPI_ICS_AUDIT_H__ */

/**@} <!-- END tapi_ics_audit --> */

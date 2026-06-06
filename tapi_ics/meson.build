# SPDX-License-Identifier: MIT
# Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved.
#
# External TE library (see TE_EXT_REPO): engine-side TAPI for talking to
# industrial control devices (Modbus TCP) on agents, and a security
# posture over an endpoint. Built as a shared library on the engine
# platform. It needs the ics_* RPCs in rpcxdr, so add
# ../ta_ics/ics_rpc.x.m4 to the rpcxdr definitions of the engine
# platform. The posture (tapi_ics_audit) reports through tsf-cybersec,
# so that repository is required too.

if get_option('engine')
    build_lib_shared = true
endif
build_lib_static = not get_option('engine')
install_lib = install_dev

headers += files(
    'tapi_ics.h',
    'tapi_ics_rpc.h',
    'tapi_ics_audit.h',
)

sources += files(
    'tapi_ics.c',
    'tapi_ics_rpc.c',
    'tapi_ics_audit.c',
)

te_libs += [
    'tapi',
    'tapi_rpc',
    'rcfrpc',
    'confapi',
    'rcfapi',
    'rpcxdr',
    'tapi_cybersec',
    'logger_core',
    'tools',
]

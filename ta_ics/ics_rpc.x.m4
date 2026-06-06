/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief RPC for industrial-control-protocol access
 *
 * The RPCs of rpcs_ics, a thin layer over ta_ics, which talks Modbus
 * TCP (over libmodbus) from the RPC server process. Add this file to
 * the rpcxdr definitions of the engine platform and of the agent
 * platform:
 *
 *   TE_LIB_PARMS([rpcxdr], [<platform>], [],
 *                [--with-rpcdefs=tarpc_job.x.m4,../ta_ics/ics_rpc.x.m4])
 *
 * No handle survives between calls: each operation is one whole
 * connect/do/close. Read results come back as tab-separated text on one
 * line - the engine side parses them.
 *
 *   proto   0 = Modbus TCP (only one implemented)
 *   area    0 = holding regs, 1 = input regs, 2 = coils, 3 = discretes
 */

/* ics_read(): read a run of registers or bits. */
struct tarpc_ics_read_in {
    struct tarpc_in_arg common;

    tarpc_int       proto;
    string          host<>;
    tarpc_int       port;
    tarpc_int       unit;
    tarpc_int       area;
    tarpc_int       addr;
    tarpc_int       count;
    tarpc_int       timeout_ms;
};

struct tarpc_ics_read_out {
    struct tarpc_out_arg common;

    tarpc_int       retval;
    string          result<>;
};

/* ics_write(): write one register (area 0) or coil (area 2). */
struct tarpc_ics_write_in {
    struct tarpc_in_arg common;

    tarpc_int       proto;
    string          host<>;
    tarpc_int       port;
    tarpc_int       unit;
    tarpc_int       area;
    tarpc_int       addr;
    tarpc_int       value;
    tarpc_int       timeout_ms;
};

struct tarpc_ics_write_out {
    struct tarpc_out_arg common;

    tarpc_int       retval;
};

/* ics_device_id(): Modbus Report Slave ID (function 0x11). */
struct tarpc_ics_device_id_in {
    struct tarpc_in_arg common;

    tarpc_int       proto;
    string          host<>;
    tarpc_int       port;
    tarpc_int       unit;
    tarpc_int       timeout_ms;
};

struct tarpc_ics_device_id_out {
    struct tarpc_out_arg common;

    tarpc_int       retval;
    string          result<>;
};

program ics
{
    version ver0
    {
        RPC_DEF(ics_read)
        RPC_DEF(ics_write)
        RPC_DEF(ics_device_id)
    } = 1;
} = 28;

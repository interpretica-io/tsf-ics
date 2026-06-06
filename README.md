# tsf-ics

Talking to industrial control devices from a Test Agent, packaged as an
external Test Environment (TE) repository (consumed with the
`TE_EXT_REPO` builder directive). It drives the protocol over a
low-level C library — **libmodbus, no Python, nothing spawned** — for
both a functional probe and a security posture.

Three libraries:

- `ta_ics` — agent side. A **Modbus TCP** client over **libmodbus**
  (`modbus.h`, `-lmodbus`): connect to an endpoint and read holding and
  input registers, coils and discrete inputs, write a register or coil,
  and ask for a device identification (Report Slave ID). The agent and
  its RPC server both link it.
- `rpcs_ics` — the `ics_*` RPCs for the RPC server of the agent, thin
  wrappers over `ta_ics`. The traffic originates on the agent, where
  the ICS device is reachable.
- `tapi_ics` — engine side. `tapi_ics.h` gives a test the read/write/
  device-id primitives against a `tapi_ics_endpoint`; `tapi_ics_audit.h`
  reads an endpoint as a security posture through tsf-cybersec;
  `tapi_ics_rpc.h` is the one-per-RPC layer beneath.

TE has no industrial-protocol client of its own. tsf-ics sits next to
[tsf-can](https://github.com/interpretica-io/tsf-can) — the field bus
inside a machine — as the network side of the same OT world.

## Protocols, through one API

| Protocol | `tapi_ics_proto` | Library | Status |
|---|---|---|---|
| Modbus TCP | `TAPI_ICS_MODBUS_TCP` | libmodbus | implemented |
| DNP3 | — | — | planned |
| S7comm | — | — | planned |

The enum is there so DNP3 and S7comm can be added behind the same API;
only Modbus TCP is implemented today, and a protocol that is not is
refused with `TE_EOPNOTSUPP`.

```c
tapi_ics_endpoint ep = {
    .proto = TAPI_ICS_MODBUS_TCP, .host = "192.0.2.10",
    .port = 502, .unit = 1, .timeout_ms = 2000,
};
int regs[4];
int n;

CHECK_RC(tapi_ics_read(rpcs, &ep, TAPI_ICS_HOLDING, 0, 4, regs, &n));
```

## The library is linked, not a program

`ta_ics` does not run a `modbus` command-line tool and parse its output.
It links libmodbus and calls `modbus_new_tcp()`, `modbus_connect()` and
the read/write functions in the agent's RPC server process. Across the
RPC a read comes back as a tab-separated line of values, which the
engine side parses into ints.

## Security posture

Modbus has **no authentication**: a reachable endpoint answers anyone.
So the posture is not "the request was forged" — every request can be —
but "a control surface answered a request nobody authenticated".
`tapi_ics_audit()` reports through tsf-cybersec's finding model:

| Finding | Severity | Raised when |
|---|---|---|
| `ics.readable` | info | a data area answered a read with no authentication |
| `ics.device-id` | low | Report Slave ID returned a device fingerprint |
| `ics.writable` | critical | a register write was accepted with no authentication |
| `ics.not-assessed` | info | the endpoint did not answer |

A finding's subject is `host:port/unit` (stable between runs). The write
check is **off by default**; see below.

## Authorized use only

The write side acts on a real device, and even a read reaches into a
live control system. tsf-ics is for an **authorized** assessment — a
pilot, a CTF, a device or test rig you own or are engaged to test. Point
it only at an endpoint you are permitted to touch, and never at a
Modbus device driving a live process without being certain a request
cannot disturb it.

When `attempt_write` is enabled, the audit does the write **reversibly**:
it reads the probed holding register and writes that same value straight
back, so the register is unchanged whether or not the write is accepted
— the finding is that the *write function* answered, not that anything
was altered. It is still a write on the wire; keep it off unless the
target is yours to write to.

## Agent host requirements

- **libmodbus** with its development headers (Debian: `apt install
  libmodbus-dev`; built and checked against 3.2.0). The header is
  `<modbus/modbus.h>`; `ta_ics.c` also accepts a plain `<modbus.h>` for
  installs whose include directory points straight at it.
- Network reachability from the agent to the ICS endpoint (Modbus TCP
  is usually port 502).

## Usage

Declare the repository in an external libraries catalog and pass it to
`dispatcher.sh --external=<catalog.yml>`:

```yaml
repositories:
  - name: tsf_ics
    url: https://github.com/interpretica-io/tsf-ics.git
    ref: <tag>
    libs:
      - ta_ics
      - rpcs_ics
      - tapi_ics
```

In `builder.conf`, bind `tapi_ics` to the engine, list `ta_ics` and
`rpcs_ics` among the RPC server's libraries, and add the RPC
definitions to both platforms:

```
TE_EXT_REPO_USE([tsf_ics], [ta_ics rpcs_ics], [tapi_ics])

TE_LIB_PARMS([rpcxdr], [${TE_HOST}], [],
             [--with-rpcdefs=tarpc_job.x.m4,../ta_ics/ics_rpc.x.m4])
TE_LIB_PARMS([rpcxdr], [${TE_TA_TYPE}], [],
             [--with-rpcdefs=tarpc_job.x.m4,../ta_ics/ics_rpc.x.m4])
```

`tapi_ics_audit` reports through tsf-cybersec, so that repository (and
its own prerequisites, tsf-kernel and tsf-devtool) must be built too.
The RPC program number is **28** (20–27 are taken by the other tsf
agent RPCs); change it in `ics_rpc.x.m4` if it ever collides.

## What was verified, and what was not

**The libmodbus usage was checked against the real library.** Every
libmodbus call `ta_ics.c` uses — `modbus_new_tcp`, `modbus_set_slave`,
`modbus_set_response_timeout`, `modbus_connect`, the four read
functions, `modbus_write_register`, `modbus_write_bit`,
`modbus_report_slave_id`, `modbus_strerror`, `modbus_close`,
`modbus_free` — was compiled against the installed libmodbus (3.2.0)
headers (`-fsyntax-only`) and type-checks, with both the
`<modbus/modbus.h>` and `<modbus.h>` include forms confirmed.

**The C was not compiled or linked here, and nothing talked to a
device.** There was no TE toolchain, so `ta_ics.c`, `rpcs_ics.c`,
`tapi_ics*.c` and the TE integration (the three `meson.build`s,
`ics_rpc.x.m4`, `TE_EXT_REPO` wiring) were written to the tsf-usb
template but not built, and no Modbus request was ever sent. The first
suite to build tsf-ics should expect the ordinary first-build fixes, and
should point it first at a Modbus simulator (e.g. a `diagslave`/
`pymodbus` server) before any real device.

**Not implemented: DNP3 and S7comm.** The protocol enum reserves them;
the code is Modbus-only.

## Scope

- **The endpoint must be reachable from the agent.** tsf-ics does not
  discover devices; it talks to an endpoint a test names.
- **Reading is normal for Modbus; writing is the finding.** `ics.readable`
  records that a value is exposed (which Modbus always allows); the
  severity sits on `ics.writable`, where an unauthenticated write is
  accepted.

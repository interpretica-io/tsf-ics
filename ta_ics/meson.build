# SPDX-License-Identifier: MIT
# Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved.
#
# External TE library (see TE_EXT_REPO): agent-side industrial-control
# client over libmodbus (Modbus TCP). The agent and its RPC server both
# link it.

headers += files(
    'ta_ics.h',
)

sources += files(
    'ta_ics.c',
)

te_libs += [
    'logger_core',
    'tools',
]

# libmodbus ships libmodbus.pc, so pkg-config normally finds it; the
# fallback finds the library and its header directly. The header is
# <modbus/modbus.h>; ta_ics.c also accepts a plain <modbus.h> for
# installs whose include dir points straight at it.
cc = meson.get_compiler('c')
libmodbus_dep = dependency('libmodbus', required: false)
if not libmodbus_dep.found()
    libmodbus_dep = cc.find_library('modbus', has_headers: 'modbus/modbus.h')
endif

deps += [
    libmodbus_dep,
]

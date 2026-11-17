# QP/C platform adapter

[QP/C](https://github.com/QuantumLeaps/qpc) v8.1.6, cooperative QV kernel,
WCH QingKe (RV32).

## Layout

```
platform/os/qpc/        QP/C submodule, kept unmodified
platform/os/inc/        public API: osal.h (no QP/C type or header leaks)
platform/os/adapt/qpc/  this project's port and build glue
  include/qp_config.h   QP/C configuration (compiled into QP/C and the app)
  include/qp_port.h     QP/C port header: names the BSP CPU primitives
  src/qpc_port.c        board callbacks: QF_onStartup/QF_onCleanup,
                        QV_onIdle, Q_onError
  src/osal_qpc.c        OSAL implementation on QActive/QTimeEvt
```

`platform_os` is a static library built from the QP/C sources plus this adapter
and linked by the application image. It is compiled with the board contract that
`bsp_common` carries (arch/ABI, section GC, optimisation, C standard), so QP/C
and the image always agree.

The core-specific parts this port needs — interrupt masking, idle, halt — are
BSP primitives (`bsp/inc/bsp_cpu.h`). Their implementation is a static inline
backend under `bsp/adapt/cpu/<arch>/`, selected by a `bsp.ARCH_*` feature and
reached in C through the `STRATA_FEATURE_*` macros. Neither this adapter nor
the OSAL contains core asm.

## OSAL

The application includes `osal.h`, never `qpc.h`:

- `osal_init()` / `osal_run()` bring up and hand over to the runtime;
- `osal_ao_create()` / `osal_ao_start()` create an active object from the
  static pool; its handler receives `osal_evt_t` in the AO's turn;
- `osal_post()` / `osal_post_isr()` queue a signal (fixed event pool, no heap);
- `osal_timer_create()` / `osal_timer_start()` post a signal after a period;
- `osal_tick()` is the only interrupt-side entry, called from the platform
  tick that `bsp_tick_start()` starts.

Events currently carry a signal only; payload transport is the next increment
and does not change the handler signature.

## Differences from QP/C's own risc-v port

- interrupt masking and the critical-section policy come from the BSP CPU
  primitives (CSR 0x800, MIE only, MPIE untouched so an ISR returns correctly
  through `mret`), not from `mstatus` access written into this adapter;
- `QACTIVE_EQUEUE_TYPE QEQueue` is defined: the QP/C risc-v port header omits it,
  which breaks `qf_actq.c` (QActive would have no `eQueue` member);
- the four board callbacks QP/C expects are provided by `src/qpc_port.c`, and
  they are board glue only.

## Warnings

QP/C 8.1.6 is warning-free under `-Wall -Wextra -Werror` with both chip
contracts, so `platform_os` carries the project's strict policy
(`project_warnings` in the top-level `CMakeLists.txt`). The vendor SDK
libraries are pinned third-party code and stay outside the `-Werror` net: they
do emit warnings today (unused variables, signedness comparisons), so the
exception is not cosmetic.

## Updating QP/C

```
git -C platform/os/qpc fetch --tags
git -C platform/os/qpc checkout v8.1.x
```

then commit the new submodule pointer. QP/C 8 consolidated its port headers into
`qp_port.h` + `qp_config.h`; check `include/qp_port.h` on a major update.

If the submodule is missing after a fresh clone:

```
git submodule update --init platform/os/qpc
```

## Application use

The application image links `platform_os` — see `main/app.c` for the
500 ms LED demo: one AO created from the pool, one periodic timer posting its
signal, the tick started from the BSP. Nothing in the application includes a
QP/C header.

QP/C is therefore live in the image; `--gc-sections` only drops what the
application does not reach.

## License

QP/C is dual-licensed: GPL-3.0-or-later or a Quantum Leaps commercial licence.
See `platform/os/qpc/LICENSE.txt`. Linking it into a product makes that product
subject to the GPL unless a commercial licence is obtained.

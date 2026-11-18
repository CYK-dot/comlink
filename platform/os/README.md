# platform/os

OSAL: the scheduler the application programs to, its dispatch layer and its
port to the CPU primitives.

## Layers

```
platform/os/
├── inc/                  public headers, included by application code
│   ├── osal_types.h      result codes
│   ├── osal_port.h       porting contract: critical section, tick, idle
│   ├── osal_sched_base.h common scheduler interface (opaque handle)
│   ├── osal_asvc.h       asynchronous service class declaration
│   └── osal_sched_sq.h   static single-queue scheduler specifics
├── src/                  implementation, private headers
│   ├── osal_sched_base.c     dispatches the common interface through vtbl
│   ├── osal_sched_base_priv.h control block and operation table
│   ├── osal_sched_sq.c       queue, services, timers, pump
│   └── osal_sched_sq_priv.h  workspace layout
└── adapt/port/
    └── osal_port.c       critical section, tick and idle on top of bsp_cpu.h
```

Application code includes only `inc/`. The operation table
(`osal_sched_vtbl_t`) and the concrete control block live in `src/`, so a
service never sees scheduler internals.

## Scheduler model

- One shared queue per scheduler; entries carry a four-byte payload and the
  destination service instance.
- Services are deployed statically: a deployment table, a forwarding table and
  a caller-provided workspace. `osal_schedssq_create()` lays the workspace out
  and constructs every service.
- Timers are records in the workspace. They are scanned at the entry of
  `osal_sched_pump()`, and expired timers post their message to the queue.
- `osal_sched_pump()` dispatches at most `max` messages; `osal_sched_run()`
  pumps forever and calls `osal_port_idle()` when there is no work.
- Queue updates run inside `osal_port_enter_critical()` /
  `osal_port_quit_critical()`, which nest and are safe from interrupt context.
  That is what lets `osal_sched_post_msg()` serve both task and ISR callers.

## Porting

`adapt/port/osal_port.c` implements the four primitives on `bsp_cpu.h`:
interrupt masking, ISR detection, idle and the tick counter. The tick source
(BSP timer interrupt) calls `osal_port_tick()` at `OSAL_PORT_TICK_HZ`.

## Adding a scheduler

1. Define its control block and workspace layout in `src/`.
2. Implement the functions of `osal_sched_vtbl_t`.
3. Add a create function that publishes `osal_sched_t` (the base header must
   stay the first member) and points `base.vtbl` at the table.
4. Business code keeps using `osal_sched_*` from `osal_sched_base.h`.

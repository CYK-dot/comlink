# platform/protousb

TinyUSB device stack integration for this project.

The `tinyusb/` directory is a **git submodule pinned to an upstream commit**
(currently `elink-pin-20261117` = `4a7ae18`, master with CH32X035 support).
Everything project-specific lives outside it, so the submodule stays a clean
upstream checkout that can be rebased or re-pinned at any time.

## Layout

```
platform/protousb/
├── tinyusb/                     submodule, never patched for project needs
├── inc/palusb.h                the only USB API the application sees
├── adapt/tinyusb/
│   ├── tusb_config.h            CFG_TUSB_MCU = OPT_MCU_CH32X035, CDC only
│   ├── usb_descriptors.c        VID/PID, CDC-ACM descriptors, strings
│   └── palusb.c                facade: init / task / send
├── adapt/ch32x035/
│   ├── usb_dcd_ch32x035.c       project-owned DCD (copy of upstream WCH USBFS)
│   ├── ch32_usbfs_reg.h         DCD register map (copy of upstream header)
│   ├── usb_ch32x035.c           clock, PC16/PC17, AFIO pull-up, USBFS_IRQHandler
│   └── usb_hw.h
├── adapt/ch57x/
│   ├── usb_dcd_ch57x.c          same DCD, EP_MAX=4, no UEP567 register
│   ├── ch57x_usbfs_reg.h        CH57x 4-endpoint register map
│   ├── usb_ch57x.c              PIN_ALTERNATE (USB IO + D+ pull-up), USB_IRQHandler
│   └── usb_hw.h
└── CMakeLists.txt               platform_usb target (core + external DCD)
```

The DCD is **external on purpose**: TinyUSB's own `src/CMakeLists.txt` states
that DCD/HCD drivers are not part of `tinyusb_sources_get()`, so the integrator
supplies them. Keeping it in `adapt/` means the submodule can follow upstream
without carrying project patches.

## Chip selection

`CMakeLists.txt` picks the adapter from `BSP_FAMILY`:

| Family   | DCD / glue                | `CFG_TUSB_MCU`       | Notes |
|----------|---------------------------|----------------------|-------|
| ch32x03x | `adapt/ch32x035`          | `OPT_MCU_CH32X035`   | clock + PC16/PC17 + AFIO pull-up |
| ch57x    | `adapt/ch57x`             | `OPT_MCU_CH583`      | same USBFS IP minus EP5-7; `CH57X_USBFS=1` |

CH57x has no `OPT_MCU_CH57x` upstream enum yet, so the build reuses the CH58x
value (same USBFS register bank) and the DCD/register header select the
4-endpoint variant through the `CH57X_USBFS` define.

CH57x specifics worth knowing:

- `RB_PIN_DEBUG_EN` (bit 14 of `R16_PIN_ALTERNATE`) shares the USB pins and
  **must be cleared** before the USB controller can see the bus; the bootloader
  and `usb_hw_init()` both clear it.
- The D+ pull-up is controlled through `RB_UDP_PU_EN`; `usb_hw_connect()` sets
  it only after TinyUSB has initialised, so the host sees a single clean attach.
- The bootloader keeps D+ low for its whole wait window, and the app forces a
  300 ms detach before connecting, so a host that already failed the ROM ISP
  device re-enumerates.

## Syncing the DCD with upstream

After re-pinning the submodule:

```bash
diff -u tinyusb/src/portable/wch/dcd_ch32_usbfs.c adapt/ch32x035/usb_dcd_ch32x035.c
diff -u tinyusb/src/portable/wch/ch32_usbfs_reg.h adapt/ch32x035/ch32_usbfs_reg.h
```

The copy is upstream code with only these differences:

- the `#if CFG_TUD_ENABLED && defined(TUP_USBIP_WCH_USBFS) && ...` guard is
  removed, because the file is compiled unconditionally for the target;
- `ch32_usbfs_reg.h` is resolved from this directory.

Re-apply those two edits after copying a new version.

## Adding another chip (e.g. CH572)

1. `mkdir adapt/<chip>` and add:
   - `usb_dcd_<chip>.c` — copy of the closest upstream DCD;
   - the register header it includes;
   - `usb_<chip>.c/.h` — clock, pins, pull-up and the USB IRQ vector
     (calls `dcd_int_handler(0)` inside `bsp_cpu_isr_enter()/exit()`).
2. Add the sources to `CMakeLists.txt` and select them by `BSP_FAMILY`.
3. Define the matching `CFG_TUSB_MCU` (e.g. `OPT_MCU_CH572` once upstream has
   the enum, or a project-side value).
4. Do **not** compile `tinyusb/src/portable/wch/dcd_*.c` for the same MCU:
   duplicate `dcd_*` symbols will fail the link.

## Application contract

```c
palusb_init();          /* clock/pins + dcd_init + tud_init */
palusb_task();          /* call from an AO every ~1 ms: runs tud_task() */
palusb_send_string("hello world\r\n");
```

`tud_task()` is never called from interrupt context; the USB IRQ only feeds
TinyUSB's event queue through `dcd_int_handler()`.

## Build & flash (CDC demo)

The USB demo is part of the normal board preset: `chip.usb = true` in the
preset resolves to the `bsp.USB` feature, and the app builds against
`platform_usb` whenever that feature is set.

```bash
cmake --preset ch572-evb
cmake --build --preset ch572-evb

cmake --preset ch32x035-evb
cmake --build --preset ch32x035-evb
```

Expected: a CDC-ACM device enumerates; the application sends `hello world`
every second (`cat /dev/ttyACM0` on Linux, any serial terminal on Windows).
The 500 ms LED blink keeps running as a liveness indicator.

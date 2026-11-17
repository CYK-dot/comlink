/**
 * @file    app.c
 * @brief   app demo: 500 ms LED blink, plus an optional USB CDC "hello world"
 *
 * With the bsp.USB feature the TinyUSB device stack is driven from its own AO: a 1 ms
 * timer runs tud_task() and a 1 s timer queues "hello world" on the CDC IN
 * endpoint.
 */
#include "bsp_sys.h"
#include "bspdbg.h"
#include "osal.h"
#include "strata_features.h"

#ifdef STRATA_FEATURE_BSP_USB
#include "palusb.h"
#endif

#define LED_BLINK_MS 500u
#define USB_TASK_MS  1u
#define USB_HELLO_TICKS (1000u / USB_TASK_MS)

enum {
    LED_SIG = 1,
#ifdef STRATA_FEATURE_BSP_USB
    USB_TASK_SIG = 2,
#endif
};

static void led_on_event(osal_ao_t *self, osal_evt_t const *evt)
{
    (void)self;

    if (evt->sig == LED_SIG) {
        bspdbg_led_toggle();
    }
}

#ifdef STRATA_FEATURE_BSP_USB
static void usb_on_event(osal_ao_t *self, osal_evt_t const *evt)
{
    static uint32_t ticks;

    (void)self;

    switch (evt->sig) {
    case USB_TASK_SIG:
        palusb_task();
        if (++ticks >= USB_HELLO_TICKS) {
            ticks = 0;
            palusb_send_string("hello world\r\n");
        }
        break;
    default:
        break;
    }
}
#endif

int main(void)
{
    osal_ao_t *led;
    osal_timer_t *timer;

    bspdbg_led_init();
    osal_init();

    led = osal_ao_create(led_on_event, 1u);
    if (led == 0) {
        for (;;) {
        }
    }
    (void)osal_ao_start(led);

    timer = osal_timer_create(led, LED_SIG);
    if (timer == 0) {
        for (;;) {
        }
    }
    (void)osal_timer_start(timer, LED_BLINK_MS, true);

#ifdef STRATA_FEATURE_BSP_USB
    osal_ao_t *usb = osal_ao_create(usb_on_event, 2u);
    if (usb == 0) {
        for (;;) {
        }
    }

    palusb_init();
    (void)osal_ao_start(usb);

    osal_timer_t *usb_task = osal_timer_create(usb, USB_TASK_SIG);
    if (usb_task == 0) {
        for (;;) {
        }
    }
    (void)osal_timer_start(usb_task, USB_TASK_MS, true);
#endif

    bsp_tick_start(OSAL_TICK_HZ, osal_tick);
    osal_run();
}

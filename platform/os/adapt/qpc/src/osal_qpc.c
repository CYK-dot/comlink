/**
 * @file    osal_qpc.c
 * @brief   OSAL implementation on QP/C (QEP + QF + QV kernel)
 *
 * The only file that includes "qpc.h": every QP/C concept the application
 * cares about is mapped onto an osal_* function here.
 */
#include "osal.h"
#include "qpc.h"

/* OSAL signals start at 0; QP keeps 0..Q_USER_SIG-1 for its own state-machine
   protocol (empty/entry/exit/init), so every OSAL signal is shifted into the
   user range. */
#define OSAL_QPC_SIG_BASE ((QSignal)Q_USER_SIG)

#define OSAL_QPC_SIG(sig_) ((enum_t)(OSAL_QPC_SIG_BASE + (QSignal)(sig_)))
#define OSAL_QPC_SIG_APP(sig_) ((osal_sig_t)((QSignal)(sig_) - OSAL_QPC_SIG_BASE))

/* Active object: QActive state machine + the application handler. */
struct osal_ao {
    QActive super;
    osal_ao_handler_t handler;
    QEvtPtr queue[OSAL_AO_QUEUE];
    uint8_t prio;
    bool used;
    bool started;
};

/* Timer: a QP time event that posts its signal to the owning AO. */
struct osal_timer {
    QTimeEvt super;
    osal_ao_t *ao;
    bool used;
};

static osal_ao_t s_ao[OSAL_MAX_AO];
static osal_timer_t s_timer[OSAL_MAX_TIMER];

/* Event pool storage, aligned for the pointer links QP keeps inside blocks. */
static union {
    QEvt evt;
    uint32_t align;
} s_evt_pool[16];

/*----------------------------------------------------------------------------
 * QP callback active object: one state that forwards every event.
 *
 * QP drives its state machines with reserved signals (empty, entry, exit,
 * init) while it builds the state configuration. A state handler must answer
 * the empty-signal scan with Q_SUPER(), or QHsm_init_() keeps drilling and
 * ends in an assertion. This AO is flat, so its superstate is the top.
 *---------------------------------------------------------------------------*/
static QState osal_ao_running(void * const me, QEvt const * const e)
{
    osal_ao_t * const ao = (osal_ao_t *)me;

    if (e->sig < (QSignal)Q_USER_SIG) {     /* reserved signal, not an event */
        return Q_SUPER(&QHsm_top);
    }

    osal_evt_t const evt = { .sig = OSAL_QPC_SIG_APP(e->sig) };
    ao->handler(ao, &evt);
    return Q_HANDLED();
}

static QState osal_ao_initial(void * const me, QEvt const * const e)
{
    (void)e;
    return Q_TRAN(&osal_ao_running);
}

/*----------------------------------------------------------------------------
 * Runtime
 *--------------------------------------------------------------------------*/
void osal_init(void)
{
    QF_init();
    QF_poolInit(s_evt_pool, (uint_fast32_t)sizeof s_evt_pool,
                (uint_fast16_t)sizeof(QEvt));
}

void osal_run(void)
{
    (void)QF_run();
}

void osal_tick(void)
{
    QTIMEEVT_TICK((void *)0);
}

/*----------------------------------------------------------------------------
 * Active objects
 *--------------------------------------------------------------------------*/
osal_ao_t *osal_ao_create(osal_ao_handler_t handler, uint8_t prio)
{
    if ((handler == (osal_ao_handler_t)0) || (prio == 0U)
        || (prio > (uint8_t)OSAL_MAX_AO)) {
        return (osal_ao_t *)0;
    }

    for (uint_fast8_t i = 0U; i < (uint_fast8_t)OSAL_MAX_AO; ++i) {
        osal_ao_t * const ao = &s_ao[i];
        if (!ao->used) {
            QActive_ctor(&ao->super, Q_STATE_CAST(&osal_ao_initial));
            ao->handler = handler;
            ao->prio = prio;
            ao->started = false;
            ao->used = true;
            return ao;
        }
    }
    return (osal_ao_t *)0;
}

osal_err_t osal_ao_start(osal_ao_t *ao)
{
    if ((ao == (osal_ao_t *)0) || !ao->used || ao->started) {
        return OSAL_EPARAM;
    }

    QActive_start(&ao->super, (QPrioSpec)ao->prio,
                  ao->queue, (uint_fast16_t)Q_DIM(ao->queue),
                  (void *)0, 0U, (void const *)0);
    ao->started = true;
    return OSAL_OK;
}

/*----------------------------------------------------------------------------
 * Event posting
 *--------------------------------------------------------------------------*/
static osal_err_t osal_post_sig(osal_ao_t *ao, osal_sig_t sig)
{
    if ((ao == (osal_ao_t *)0) || !ao->used || !ao->started) {
        return OSAL_EPARAM;
    }

    QEvt * const e = Q_NEW_X(QEvt, 0U, OSAL_QPC_SIG(sig));
    if (e == (QEvt *)0) {
        return OSAL_ENOSLOT;
    }
    if (!QActive_post_(&ao->super, e, 0U, (void const *)0)) {
        /* not queued: QP has already recycled the pool event */
        return OSAL_EFULL;
    }
    return OSAL_OK;
}

osal_err_t osal_post(osal_ao_t *ao, osal_sig_t sig)
{
    return osal_post_sig(ao, sig);
}

osal_err_t osal_post_isr(osal_ao_t *ao, osal_sig_t sig)
{
    return osal_post_sig(ao, sig);
}

/*----------------------------------------------------------------------------
 * Timers
 *--------------------------------------------------------------------------*/
osal_timer_t *osal_timer_create(osal_ao_t *ao, osal_sig_t sig)
{
    if ((ao == (osal_ao_t *)0) || !ao->used) {
        return (osal_timer_t *)0;
    }

    for (uint_fast8_t i = 0U; i < (uint_fast8_t)OSAL_MAX_TIMER; ++i) {
        osal_timer_t * const timer = &s_timer[i];
        if (!timer->used) {
            QTimeEvt_ctorX(&timer->super, &ao->super, OSAL_QPC_SIG(sig), 0U);
            timer->ao = ao;
            timer->used = true;
            return timer;
        }
    }
    return (osal_timer_t *)0;
}

osal_err_t osal_timer_start(osal_timer_t *timer, uint32_t period_ms,
                            bool periodic)
{
    uint32_t ticks;

    if ((timer == (osal_timer_t *)0) || !timer->used
        || !timer->ao->started || (period_ms == 0U)) {
        return OSAL_EPARAM;
    }

    ticks = (uint32_t)(((uint64_t)period_ms * (uint64_t)OSAL_TICK_HZ)
                       / 1000u);
    if (ticks == 0U) {
        return OSAL_EPARAM;
    }

    QTimeEvt_armX(&timer->super, ticks, periodic ? ticks : 0U);
    return OSAL_OK;
}

osal_err_t osal_timer_stop(osal_timer_t *timer)
{
    if ((timer == (osal_timer_t *)0) || !timer->used) {
        return OSAL_EPARAM;
    }

    (void)QTimeEvt_disarm(&timer->super);
    return OSAL_OK;
}

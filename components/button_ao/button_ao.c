#include "button_ao.h"
#include "esp_log.h"

State Button_init(Button *const me, Event const *const e);
State Button_released(Button *const me, Event const *const e);
State Button_pressed(Button *const me, Event const *const e);
State Button_pre_doublepressed(Button *const me, Event const *const e);
State Button_doublepressed(Button *const me, Event const *const e);
State Button_hold(Button *const me, Event const *const e);

State Button_init(Button *const me, Event const *const e){
    return transition(&me->super.super, (StateHandler)&Button_released);
}

State Button_released(Button *const me, Event const *const e){
    State status;
    switch (e->sig)
    {
        case ENTRY_SIG:
            Active_post(AO_Broker, &(Event){ EV_BUTTON_RELEASED });
            status = HANDLED_STATUS;
            break;

        case EV_POLLING_BUTTON_STATE_CHANGED:
            status = transition(&me->super.super, (StateHandler)&Button_pressed);
            break;
        
        case EXIT_SIG:
            status = HANDLED_STATUS;
            break;

        default:
            status = IGNORED_STATUS;
            break;
    }
    return status;
}

State Button_pressed(Button *const me, Event const *const e){
    State status;
    switch (e->sig)
    {
        case ENTRY_SIG:
            TimeEvent_arm(&me->holdTimer);
            Active_post(AO_Broker, &(Event){ EV_BUTTON_PRESSED });
            status = HANDLED_STATUS;
            break;

        case EV_POLLING_BUTTON_STATE_CHANGED:
            status = transition(&me->super.super, (StateHandler)&Button_pre_doublepressed);
            break;
        
        case BUTTON_HOLD_TIMEOUT_SIG:
            status = transition(&me->super.super, (StateHandler)&Button_hold);
            break;
        
        case EXIT_SIG:
            TimeEvent_disarm(&me->holdTimer);
            status = HANDLED_STATUS;
            break;

        default:
            status = IGNORED_STATUS;
            break;
    }
    return status;
}
State Button_pre_doublepressed(Button *const me, Event const *const e){
    State status;
        switch (e->sig)
        {
        case ENTRY_SIG:
            TimeEvent_arm(&me->doublePressTimer);
            status = HANDLED_STATUS;
            break;
        
        case EV_POLLING_BUTTON_STATE_CHANGED:
            status = transition(&me->super.super, (StateHandler)&Button_doublepressed);
            break;
        
        case BUTTON_DOUBLE_PRESS_TIMEOUT_SIG:
            status = transition(&me->super.super, (StateHandler)&Button_released);
            break;

        case EXIT_SIG:
            TimeEvent_disarm(&me->doublePressTimer);
            status = HANDLED_STATUS;
            break;

        default:
            status = IGNORED_STATUS;
            break;
        }
    return status;
}
State Button_hold(Button *const me, Event const *const e){
    State status;
        switch (e->sig)
        {
        case ENTRY_SIG:
            Active_post(AO_Broker, &(Event){ EV_BUTTON_HOLD });
            status = HANDLED_STATUS;
            break;
        
        case EV_POLLING_BUTTON_STATE_CHANGED:
            status = transition(&me->super.super, (StateHandler)&Button_released);
            break;

        case EXIT_SIG:
            status = HANDLED_STATUS;
            break;
            
        default:
            status = IGNORED_STATUS;
            break;
        }
    return status;
}

State Button_doublepressed(Button *const me, Event const *const e){
    State status;
        switch (e->sig)
        {
        case ENTRY_SIG:
            TimeEvent_disarm(&me->doublePressTimer);
            Active_post(AO_Broker, &(Event){ EV_BUTTON_DOUBLE_PRESS });
            status = HANDLED_STATUS;
            break;
        
        case EV_POLLING_BUTTON_STATE_CHANGED:
            status = HANDLED_STATUS;
            break;

        case EXIT_SIG:
            status = HANDLED_STATUS;
            break;
            
        default:
            status = IGNORED_STATUS;
            break;
        }
    return status;
}

void Button_ctor(Button *const me)
{
    Active_ctor(&me->super, (StateHandler)&Button_init);
    TimeEvent_ctor(&me->holdTimer, "Hold timer", (TickType_t)(HOLD_TIME / portTICK_PERIOD_MS), pdFALSE, BUTTON_HOLD_TIMEOUT_SIG, &me->super);
    TimeEvent_ctor(&me->doublePressTimer, "Double press timer", (TickType_t)(DOUBLE_PRESS_TIME / portTICK_PERIOD_MS), pdFALSE, BUTTON_DOUBLE_PRESS_TIMEOUT_SIG, &me->super);
}
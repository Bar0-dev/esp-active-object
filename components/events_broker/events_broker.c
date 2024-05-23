#include "events_broker.h"
#include "esp_log.h"

State Broker_publish(Broker * const me, Event const * const e)
{
    //DEBUG
    ESP_LOGI("BROKER", "SIG: %d", e->sig);
    //DEBUG
    uint8_t aoId = e->sig-USER_SIG;
    GlobalEvent_t *globalEvent = &me->globalEvents[aoId];
    BaseType_t ret = pdFAIL;
    if(globalEvent->subscribers > 0){
        for (int q = 0; q<globalEvent->subscribers; q++)
        {
            Active_post(globalEvent->aos[q], e);
        }
    }
    ESP_LOGI("BROKER", "ret: %d", ret);
    return HANDLED_STATUS;
}

State Broker_init(Broker * const me, Event const * const e)
{
    return transition(&me->super.super, (StateHandler)&Broker_publish);
}

void Broker_ctor(Broker * const me)
{
    Active_ctor(&me->super, (StateHandler)&Broker_init);
}

void Broker_subscribe(Broker * const me, Event const * const e, Active const * const ao)
{
    GlobalSignal_t gsig = e->sig;
    uint8_t aoId = gsig - USER_SIG;
    GlobalEvent_t *globalEvent = &me->globalEvents[aoId];
    assert(globalEvent->subscribers+1<MAX_AOS_PER_EVENT);
    globalEvent->aos[globalEvent->subscribers] = ao;
    ESP_LOGI("BROKER", "SIG: %d SUBBED: %d", e->sig, globalEvent->subscribers);
    globalEvent->subscribers++;
}
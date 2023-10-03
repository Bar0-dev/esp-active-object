#pragma once
#ifndef ESP_AO_H
#define ESO_AO_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

/**
 * Signal definition and reserved signals, USER_SIG is a first signal available for the user
*/

#define MAX_TIMERS 100

typedef uint16_t Signal;

enum ReservedSignals
{
    INIT_SIG,
    USER_SIG,
};

/**
 * Event base class
*/

typedef struct
{
    Signal sig;
    /*additional subclass data*/
} Event;

/**
 * Active object utils
*/

typedef struct Active Active;

typedef void (*DispatchHandler)(Active * const me, Event const * const e);

/**
 * Active object base class
*/

struct Active
{
    TaskHandle_t *task;
    QueueHandle_t queue;

    DispatchHandler dispatch;
    /*additional subclass data*/
};

void Active_ctor(Active * const me, DispatchHandler dispatch);
void Active_start(Active * const me,
                const char *const taskNamePtr,
                const uint32_t stackSize, /*stack size in number of bytes*/
                UBaseType_t taskPriority,
                const BaseType_t core, /*core the task should run on, default tskNO_AFFINITY enum*/
                UBaseType_t queueLength);

void Active_post(Active * const me, Event const * const e);

/**
 * Timer object base class
*/

typedef struct
{
    Event super;
    Active *act;
    TickType_t period;
    UBaseType_t autoReload;
    TimerHandle_t handle;
} TimeEvent;

void TimeEvent_ctor(TimeEvent * const me, char * const timerName, TickType_t period, UBaseType_t autoReload, Signal sig, Active *act); /*associate actor to the timer*/
void TimeEvent_arm(TimeEvent * const me); /*start xTimer*/
void TimeEvent_disarm(TimeEvent * const me); /*stop xTimer*/

#endif
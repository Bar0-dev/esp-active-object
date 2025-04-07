# ESP-AO: [Active Object](https://www.state-machine.com/active-object) Framework for ESP32 with Hierarchical State Machines

This project provides a lightweight and extensible active object (AO) framework for embedded systems, specifically designed for the ESP32 platform using ESP-IDF and FreeRTOS.

It supports:
- Hierarchical state machines
- Event-driven execution
- Timed events
- ISR-safe event posting
- Portable and modular architecture

---

## 📦 Structure

```
esp_ao/               # The AO framework component
├── include/
│   └── esp_ao.h      # Public API and type definitions
├── esp_ao.c          # Implementation of active objects and timers
├── CMakeLists.txt
├── README.md         # This file

your_project/
├── components/
│   └── esp_ao/       # Cloned here or copied manually
└── main/
    ├── main.c        # Application entry
    ├── my_actor.c    # Your custom active object
    └── my_actor.h
```

---

## 🚀 How to Use

### Option 1: Clone into `components/` directory

```bash
cd your_project/components

git clone https://github.com/Bar0-dev/esp-active-object.git esp_ao/
```

### Option 2: Manually copy the source files

Copy `esp_ao.c, `include/esp_ao.h` and `CMakeLists.txt` to your project's `components/esp_ao/` directory.

### Then, in other components `CMakeLists.txt`:

```cmake
idf_component_register(SRCS "example_ao.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_ao)
```
---

## 🧪 Quick Start

### 1. Include the AO framework

```c
#include "esp_ao.h"
```

### 2. Define a hierarchical state machine

```c
State Led_init(Led * const me, Event const * const e)
{
    return transition(&me->super.super, (StateHandler)&Led_idle);
}

State Led_top(Led * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    case EV_BLINK_FAST_SIG:
        status = transition(&me->super.super, (StateHandler)&Led_blink_fast);
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Hsm_top);
        break;
    }
    return status;
}
State Led_idle(Led * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        Led_off();
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Led_top);
        break;
    }
    return status;
}

State Led_blink_fast(Led * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        TimeEvent_arm(&me->ledTimer);
        status = HANDLED_STATUS;
        break;
    
    case BLINK_TIMER_EXPIRED_SIG:
        Led_toggle(me->ledState);
        status = HANDLED_STATUS;
        break;
        
    case EXIT_SIG:
        TimeEvent_disarm(&me->ledTimer);
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Led_top);
        break;
    }
    return status;
}
```

### 3. Create an active object instance

```c
typedef struct
{
    Active super;
    BlinkPeriod_t blinkPeriod;
    TimeEvent ledTimer;
} Led;

static Led led;
Active *AO_Led = &led.super;

void app_main(void) {
  Led_ctor(&led);
  Active_start(AO_Led, "LED thread", 2048, 10, tskNO_AFFINITY, 10);
}
```

### 4. Post events

From a task:
```c
Event evt = { .sig = USER_SIG + 1 };
Active_post(&my_ao.super, &evt);
```

From an ISR:
```c
Event evt = { .sig = USER_SIG + 2 };
Active_postFromISR(&my_ao.super, &evt);
```

---

## ⏱ Using Timers

```c
TimeEvent timer;
TimeEvent_ctor(&timer, "MyTimer", pdMS_TO_TICKS(1000),
               pdTRUE, USER_SIG + 3, &my_ao.super);
TimeEvent_arm(&timer);  // Start periodic timer
TimeEvent_disarm(&timer);  // Stop it
```

---

## ➕ Adding Your Own Active Objects

1. Define a struct that embeds `Active`.
2. Implement its state machine logic.
3. Construct and start the AO using `Active_ctor()` and `Active_start()`.
4. Post events using `Active_post()` or `TimeEvent`.

---

## 📐 Best Practices

- Use `transition()` to move between states.
- Use `super()` to handle inheritance.
- Implement `ENTRY_SIG` and `EXIT_SIG` for logging or setup.
- Use private data fields in your AO struct.
- Keep logic isolated per AO — no shared state unless explicitly synchronized.

---

## 🔧 Requirements

- ESP-IDF v4.4 or later
- FreeRTOS (included with ESP-IDF)
- C99 compatible compiler

---

## 📌 Potential Extensions

- Event memory pooling
- Priority-based event queues
- History state support (deep/shallow)
- Optional trace/debug hook integration

---

## 📄 License

MIT License

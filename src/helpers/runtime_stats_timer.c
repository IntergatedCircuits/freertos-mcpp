/**
 * @file      runtime_stats_timer.c
 * @brief     Zero-cost runtime stats timer
 *            The current implementation is specific to ARM Cortex Mx cores,
 *            but the concept is reusable on many targets where the OS timer counter is accessible.
 * @author    Benedek Kupper
 */
#include "FreeRTOS.h"
#include "task.h"

#ifdef portNVIC_INT_CTRL_REG /* Currently working detection of ARM Cortex Mx core */
#if (configGENERATE_RUN_TIME_STATS != 1)
#warning "Ensure that FreeRTOSConfig.h contains: #define configGENERATE_RUN_TIME_STATS 1"
#else
#if portCONFIGURE_TIMER_FOR_RUN_TIME_STATS != ConfigureTimerForRunTimeStats
#warning "Ensure that FreeRTOSConfig.h contains: #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS ConfigureTimerForRunTimeStats"
#endif
#if portGET_RUN_TIME_COUNTER_VALUE != GetRuntimeCounterValueFromISR
#warning "Ensure that FreeRTOSConfig.h contains: #define portGET_RUN_TIME_COUNTER_VALUE GetRuntimeCounterValueFromISR"
#endif

/* No generic header is available from ARM to include SysTick, so define it manually */
struct
{
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
}*SysTick = (void*) 0xE000E010UL;

/**
 * @brief This is a tunable value, a tradeoff between resolution and long runtime without overflow.
 */
static const uint32_t TimerResolution = 100;

/**
 * @brief Empty funtion, the SysTick is configured by kernel when the scheduler is started.
 */
void ConfigureTimerForRunTimeStats(void)
{
}

/**
 * @brief Calculates the runtime counter by scaling up the tick timer,
 *        and adding the fractional value of the running timer. The resolution is controlled by
 *        @ref TimerResolution
 * @note  The OS is calling this function from ISR context, so only ISR API use is allowed.
 * @return The current virtual timer counter value.
 */
uint32_t GetRuntimeCounterValueFromISR(void)
{
    uint32_t ticks = xTaskGetTickCountFromISR();
    uint32_t count = SysTick->VAL;
    uint32_t reload_value = SysTick->LOAD;

    return (TimerResolution * ticks) + (TimerResolution * count / reload_value);
}

#endif /* configGENERATE_RUN_TIME_STATS */
#endif /* portNVIC_INT_CTRL_REG */

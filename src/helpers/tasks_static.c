/**
 * @file      tasks_static.c
 * @brief     Required FreeRTOS function definitions when static allocation is enabled.
 * @author    https://www.freertos.org/a00110.html
 */
#include "FreeRTOS.h"
#include "task.h"

#if ( configSUPPORT_STATIC_ALLOCATION == 1 )
/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
 implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
        StackType_t **ppxIdleTaskStackBuffer,
        uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
     function then they must be declared static - otherwise they will be allocated on
     the stack and so not exists after this function exits. */
    static struct
    {
        StaticTask_t xTCB;
        StackType_t uxStack[configMINIMAL_STACK_SIZE];
    }xIdleTask;

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTask.xTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = xIdleTask.uxStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     Note that, as the array is necessarily of type StackType_t,
     configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if ( configUSE_TIMERS == 1 )
/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 application must provide an implementation of vApplicationGetTimerTaskMemory()
 to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
        StackType_t **ppxTimerTaskStackBuffer,
        uint32_t *pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
     function then they must be declared static - otherwise they will be allocated on
     the stack and so not exists after this function exits. */
    static struct
    {
        StaticTask_t xTCB;
        StackType_t uxStack[configTIMER_TASK_STACK_DEPTH];
    }xTimerTask;

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTask.xTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = xTimerTask.uxStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     Note that, as the array is necessarily of type StackType_t,
     configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif /* ( configUSE_TIMERS == 1 ) */

#endif /* ( configSUPPORT_STATIC_ALLOCATION == 1 ) */

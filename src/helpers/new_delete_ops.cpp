/**
 * @file      new_delete_ops.c
 * @brief     Redirection of C++ default memory allocation to FreeRTOS heap management.
 * @author    Benedek Kupper
 */
#include "FreeRTOS.h"

#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

void* operator new(size_t size)
{
    return pvPortMalloc(size);
}

void* operator new[](size_t size)
{
    return pvPortMalloc(size);
}

void operator delete(void* p)
{
    vPortFree(p);
}

void operator delete[](void* p)
{
    vPortFree(p);
}

#endif /* (configSUPPORT_DYNAMIC_ALLOCATION == 1) */

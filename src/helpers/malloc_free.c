/**
 * @file      malloc_free.c
 * @brief     Redirection of C default memory allocation to FreeRTOS heap management.
 * @author    Benedek Kupper
 */
#include <stdlib.h>
#include "FreeRTOS.h"

#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
void* malloc(size_t size)
{
    return pvPortMalloc(size);
}

void free(void *p)
{
    vPortFree(p);
}
#endif /* (configSUPPORT_DYNAMIC_ALLOCATION == 1) */

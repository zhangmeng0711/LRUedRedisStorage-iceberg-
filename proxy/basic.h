#ifndef _BASIC_H_
#define _BASIC_H_
#include <semaphore.h>
#include "cir_queue.h"
#define MAX_READ 10240
extern cir_queue_t q;
#define REDIS_NOTUSED(V) ((void) V)
struct WData
{
        int len;
        char *data;
};
#endif

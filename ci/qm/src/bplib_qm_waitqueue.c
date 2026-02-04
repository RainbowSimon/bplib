/*
 * NASA Docket No. GSC-19,559-1, and identified as "Delay/Disruption Tolerant Networking 
 * (DTN) Bundle Protocol (BP) v7 Core Flight System (cFS) Application Build 7.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this 
 * file except in compliance with the License. You may obtain a copy of the License at 
 *
 * http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software distributed under 
 * the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF 
 * ANY KIND, either express or implied. See the License for the specific language 
 * governing permissions and limitations under the License. The copyright notice to be 
 * included in the software is as follows: 
 *
 * Copyright 2025 United States Government as represented by the Administrator of the 
 * National Aeronautics and Space Administration. All Rights Reserved.
 *
 */

#include "bplib_qm_waitqueue.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <errno.h>
#include <stdio.h>

#include "ztimer.h"

/*******************************************************************************
* Exported Functions
*/
bool BPLib_QM_WaitQueueInit(BPLib_QM_WaitQueue_t* q, size_t el_size, size_t capacity)
{
    if ((q == NULL) || (el_size == 0) || (capacity < 2))
    {
        return false;
    }

    q->el_size = el_size;
    q->capacity = capacity;
    q->front = 0;
    q->rear = capacity - 1;
    q->size = 0;

    q->storage = calloc(capacity, el_size);
    if (q->storage == NULL)
    {
        fprintf(stderr, "calloc() for WaitQueue backing failed\n");
        return false;
    }

    // TODO: move to bplib_OS module
    mutex_init(&q->lock);
    sema_create(&q->s_slots, capacity);
    sema_create(&q->s_items, 0);
    return true;
}

void BPLib_QM_WaitQueueDestroy(BPLib_QM_WaitQueue_t* q)
{
    if (q == NULL)
    {
        return;
    }

    free(q->storage);
    q->storage = NULL;
    q->el_size = 0;
    q->capacity = 0;
    q->front = 0;
    q->rear = 0;
    q->size = 0;

    // TODO: move to bplib_OS module
    // No mutex cleanup in RIOT
    sema_destroy(&q->s_slots);
    sema_destroy(&q->s_items);
}

bool BPLib_QM_WaitQueueTryPush(BPLib_QM_WaitQueue_t* q, const void* item, int timeout_ms)
{
    int rc;

    if ((q == NULL) || (item == NULL))
    {
        return false;
    }

    /* Wait for queue to be non-full */
    rc = sema_wait_timed_ztimer(&q->s_slots, ZTIMER_MSEC, timeout_ms);
    if (rc != 0) {
        if (rc != -ETIMEDOUT && rc != -EAGAIN) {
            printf(" BPLib_QM_WaitQueueTryPush NON-TIMEOUT ERROR: %s\n", strerror(rc));
        }
        return false;
    }

    mutex_lock(&q->lock);
    /**** Critical Section Begin ****/

    /* Push an item */
    q->rear = (q->rear  + 1) % q->capacity;
    memcpy((void*)(((char *)q->storage) + (q->rear*q->el_size)), item, q->el_size);
    q->size++;

    /**** Critical Section End ****/
    mutex_unlock(&q->lock);

    /* Notify other pulling threads that an item can be pulled. */
    sema_post(&q->s_items);

    return true;
}

bool BPLib_QM_WaitQueueTryPull(BPLib_QM_WaitQueue_t* q, void* ret_item, int timeout_ms)
{
    int rc;

    if ((q == NULL) || (ret_item == NULL))
    {
        return false;
    }
 
    /* Wait for queue to be non-empty */
    rc = sema_wait_timed_ztimer(&q->s_items, ZTIMER_MSEC, timeout_ms);
    if (rc != 0) {
        if (rc != -ETIMEDOUT && rc != -EAGAIN) {
            printf(" BPLib_QM_WaitQueueTryPull NON-TIMEOUT ERROR: %s\n", strerror(rc));
        }
        return false;
    }

    mutex_lock(&q->lock);
    /**** Critical Section Begin ****/

    /* Pull an item */
    memcpy(ret_item, (void*)(((char *)q->storage) + (q->front*q->el_size)), q->el_size);
    q->size--;
    q->front = (q->front + 1) % (q->capacity); 

    /**** Critical Section End ****/
    mutex_unlock(&q->lock);

    /* Notify other pushing threads that an item can be pushed */
    sema_post(&q->s_slots);

    return true;
}

bool BPLib_QM_WaitQueueIsEmpty(BPLib_QM_WaitQueue_t* q)
{
    bool IsEmpty;

    if (q == NULL)
    {
        return false;
    }

    mutex_lock(&q->lock);
    IsEmpty = (q->size == 0);
    mutex_unlock(&q->lock);

    return IsEmpty;
}

bool BPLib_QM_WaitQueueIsFull(BPLib_QM_WaitQueue_t* q)
{
    bool IsFull;

    if (q == NULL)
    {
        return false;
    }

    mutex_lock(&q->lock);
    IsFull = (q->size == q->capacity);
    mutex_unlock(&q->lock);

    return IsFull;
}

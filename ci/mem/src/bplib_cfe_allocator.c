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

#include "bplib_mem.h"
#include "bplib_mem_impl.h"

#include <stdio.h>

static const size_t BlockSizes[BPLIB_MEM_TOTAL_NUM_BLOCKS] =
{
    BPLIB_MEM_BIG_BLK_SIZE,
    BPLIB_MEM_SMALL_BLK_SIZE
};

BPLib_Status_t BPLib_MEM_PoolImplInit(BPLib_MEM_PoolImpl_t* Pool, void* MemBuff,
                                                        size_t MemLen, size_t BlockSize)
{
    CFE_Status_t CfeStatus;
    uint8_t i;

    if (Pool == NULL || MemBuff == NULL || MemLen == 0)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    memset(Pool, 0, sizeof(BPLib_MEM_PoolImpl_t));

    Pool->MemBuffer = MemBuff;
    Pool->TotalSize = MemLen;
    Pool->UsedSize = 0;

    /* We handle the mutex in bplib_mem.c so no mutex needed */
    CfeStatus = CFE_ES_PoolCreateEx(&Pool->CfeHandle, Pool->MemBuffer, Pool->TotalSize,
                        BPLIB_MEM_TOTAL_NUM_BLOCKS, BlockSizes, CFE_ES_NO_MUTEX);
    if (CfeStatus != CFE_SUCCESS)
    {
        return BPLIB_ERROR;
    }

    printf("MEM CFE_ALLOC: Allocated %d types of blocks of sizes: ", BPLIB_MEM_TOTAL_NUM_BLOCKS);
    for (i = 0; i < BPLIB_MEM_TOTAL_NUM_BLOCKS; i++)
    {
        printf("%ld ", BlockSizes[i]);
    }
    printf("\n");

    return BPLIB_SUCCESS;
}

void BPLib_MEM_PoolImplDestroy(BPLib_MEM_PoolImpl_t* Pool)
{
    if (Pool == NULL)
    {
        return;
    }
    
    (void) CFE_ES_PoolDelete(Pool->CfeHandle);
    
    memset(Pool, 0, sizeof(BPLib_MEM_PoolImpl_t));
}

void* BPLib_MEM_PoolImplAlloc(BPLib_MEM_PoolImpl_t* Pool, size_t Size)
{
    CFE_Status_t BytesAllocd;
    void *RetPtr = NULL;

    if (Pool == NULL || Size == 0){
        return NULL;
    }

    BytesAllocd = CFE_ES_GetPoolBuf((void**)&RetPtr, Pool->CfeHandle, (uint32_t) Size);
    if (BytesAllocd > 0)
    {
        Pool->UsedSize += BytesAllocd;
    }

    return (void*)RetPtr;
}

void BPLib_MEM_PoolImplFree(BPLib_MEM_PoolImpl_t* Pool, void* Ptr)
{
    CFE_Status_t BytesFreed;

    if (Pool == NULL || Ptr == NULL)
    {
        return;
    }

    BytesFreed = CFE_ES_PutPoolBuf(Pool->CfeHandle,(uint32_t *)Ptr);
    if (BytesFreed > 0)
    {
        Pool->UsedSize -= BytesFreed;
    }

    return;
}

size_t BPLib_MEM_GetBytesInUseImpl(BPLib_MEM_PoolImpl_t *Pool)
{
    return Pool->UsedSize;
}

size_t BPLib_MEM_GetBytesFreeImpl(BPLib_MEM_PoolImpl_t *Pool)
{
    return (Pool->TotalSize - Pool->UsedSize);
}

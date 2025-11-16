/*
 * NASA Docket No. GSC-18,587-1 and identified as “The Bundle Protocol Core Flight
 * System Application (BP) v6.5”
 *
 * Copyright © 2020 United States Government as represented by the Administrator of
 * the National Aeronautics and Space Administration. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef BPLIB_MEM_CFE_ALLOCATOR_H
#define BPLIB_MEM_CFE_ALLOCATOR_H

#include "bplib_api_types.h"
#include "cfe.h"

/**
 * @struct BPLib_MEM_PoolImpl_t
 * @brief Represents the internal structure of a memory pool implementation.
 * 
 * This structure is used to manage memory allocation and deallocation within the pool.
 */
typedef struct BPLib_MEM_PoolImpl
{
    void* MemBuffer;                /** \brief Pointer to the start of the memory region backing the pool */
    CFE_ES_MemHandle_t CfeHandle;   /** \brief The handle used for by cFE Memory Pool */
    size_t TotalSize;               /** \brief Total size of the pool in bytes */
    size_t UsedSize;                /** \brief Bytes in pool currently in use */
} BPLib_MEM_PoolImpl_t;

#endif /* BPLIB_MEM_CFE_ALLOCATOR_H */

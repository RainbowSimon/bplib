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

#ifndef BPLIB_STD_ALLOCATOR_H
#define BPLIB_STD_ALLOCATOR_H

/**
 * @struct BPLib_MEM_PoolImpl_t
 * @brief Represents the internal structure of a memory pool implementation.
 * 
 * This structure is used to manage memory allocation and deallocation within the pool.
 */
typedef struct 
{
    void* mem;
    size_t size;
    uint32_t block_size;
} BPLib_MEM_PoolImpl_t;

#endif

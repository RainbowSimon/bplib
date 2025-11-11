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

#ifndef BPLIB_BEN_ALLOCATOR_H
#define BPLIB_BEN_ALLOCATOR_H

/*******************************************************************************
 * Defines and Types
 */

/* It is expected that the memory returned from this allocator will be cast
** to structs with valid alignment. For these casts to work, the memory
** that backs this pool needs to start on a boundary of the largest primitive
** type. It is assumed that this alignment will be sizeof(uint64_t) because
** many RFC 9171 fields require this type to be supported natively.
*/
#define BPLIB_BEN_ALLOC_LARGEST_ALIGNMENT  (8u)

typedef uint64_t MemIndex_t;

/**
 * @struct BPLib_MEM_PoolImpl_t
 * @brief Represents the internal structure of a memory pool implementation.
 * 
 * This structure is used to manage memory allocation and deallocation within the pool.
 */
typedef struct
{
    void* mem_start; /**< Pointer to the start of the memory region backing the pool */
    void* mem_next; /**< Pointer to the next available memory block location within the pool */
    size_t num_blocks; /**< The maximum number of blocks that can be allocated by this pool. */
    size_t block_size; /**< Size of each block in the pool, in bytes. */
    size_t num_free; /**< Number of free blocks available */
    size_t num_init; /**< Number of initialized blocks. */
} BPLib_MEM_PoolImpl_t;

#endif

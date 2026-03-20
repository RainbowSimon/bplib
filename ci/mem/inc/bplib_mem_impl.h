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

#ifndef BPLIB_MEM_IMPL_H
#define BPLIB_MEM_IMPL_H

#include "bplib_api_types.h"

#if defined(BPLIB_MEM_CFE_IMPL)
#include "bplib_cfe_allocator.h"
#elif defined(BPLIB_MEM_BEN_IMPL)
#include "bplib_ben_allocator.h"
#else
#include "bplib_std_allocator.h"
#endif

/**
 * @brief Initializes the memory pool implementation.
 * 
 * This function initializes the memory pool, setting up the memory and block size.
 * 
 * @param[out] pool Pointer to the memory pool implementation to initialize.
 * @param[in] init_mem Pointer to the initial memory to use for the pool.
 * @param[in] mem_len The length of the memory to use.
 * @param[in] block_size The size of each block in the pool.
 * 
 * @return Status of the operation.
 */
BPLib_Status_t BPLib_MEM_PoolImplInit(BPLib_MEM_PoolImpl_t* pool, void* init_mem,
    size_t mem_len, size_t block_size);

/**
 * @brief Destroys the memory pool implementation.
 * 
 * This function frees any resources associated with the memory pool implementation.
 * 
 * @param[in] pool Pointer to the memory pool implementation to destroy.
 */
void BPLib_MEM_PoolImplDestroy(BPLib_MEM_PoolImpl_t* pool);

/**
 * @brief Allocates a block of memory from the pool implementation.
 * 
 * This function allocates a single block of memory from the pool.
 * 
 * @param[in] pool Pointer to the memory pool implementation from which to allocate memory.
 * 
 * @return Pointer to the allocated memory block, or NULL if allocation fails.
 */
void* BPLib_MEM_PoolImplAlloc(BPLib_MEM_PoolImpl_t* pool, size_t Size);

/**
 * @brief Frees a block of memory back to the pool implementation.
 * 
 * This function frees a previously allocated memory block and returns it to the pool.
 * 
 * @param[in] pool Pointer to the memory pool implementation to return the block to.
 * @param[in] to_free Pointer to the memory block to free.
 */
void BPLib_MEM_PoolImplFree(BPLib_MEM_PoolImpl_t* pool, void* to_free);

/**
 * @brief Get bytes in use
 * 
 * This function gets the current number of bytes in use by the memory pool.
 * 
 * @param[in] Pool Pointer to the memory pool implementation
 * 
 * @return Number of bytes in use
 */
size_t BPLib_MEM_GetBytesInUseImpl(BPLib_MEM_PoolImpl_t *Pool);

/**
 * @brief Get bytes free
 * 
 * This function gets the current number of bytes not in use by the memory pool.
 * 
 * @param[in] Pool Pointer to the memory pool implementation
 * 
 * @return Number of bytes not in use
 */
size_t BPLib_MEM_GetBytesFreeImpl(BPLib_MEM_PoolImpl_t *Pool);

#endif /* BPIB_MEM_IMPL_H */

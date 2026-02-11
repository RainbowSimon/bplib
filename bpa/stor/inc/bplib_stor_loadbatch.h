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
#ifndef BPLIB_STOR_LOADBATCH_H
#define BPLIB_STOR_LOADBATCH_H

#include "bplib_api_types.h"



typedef struct
{
    int64_t BundleRowIDs[BPLIB_STOR_LOADBATCHSIZE];
    size_t Size;
    size_t ReadIndex;
} BPLib_STOR_LoadBatch_t;

BPLib_Status_t BPLib_STOR_LoadBatch_Init(BPLib_STOR_LoadBatch_t* Batch);

BPLib_Status_t BPLib_STOR_LoadBatch_Reset(BPLib_STOR_LoadBatch_t* Batch);

bool BPLib_STOR_LoadBatch_IsEmpty(BPLib_STOR_LoadBatch_t* Batch);

bool BPLib_STOR_LoadBatch_IsConsumed(BPLib_STOR_LoadBatch_t* Batch);

BPLib_Status_t BPLib_STOR_LoadBatch_AddID(BPLib_STOR_LoadBatch_t* Batch, int64_t BundleRowID);

BPLib_Status_t BPLib_STOR_LoadBatch_PeekNextID(BPLib_STOR_LoadBatch_t* Batch, int64_t *BundleRowID);

BPLib_Status_t BPLib_STOR_LoadBatch_AdvanceReader(BPLib_STOR_LoadBatch_t* Batch);

#endif /* BPLIB_STOR_LOADBATCH_H */

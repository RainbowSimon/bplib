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
#include "bplib_stor_loadbatch.h"
#include "bplib_qm.h"

BPLib_Status_t BPLib_STOR_LoadBatch_Init(BPLib_STOR_LoadBatch_t* Batch)
{
    /* For now Init and Reset are identical. A separate function is used
    ** because many other BPLib utilities have an Init(). This is stylistic more
    ** than functional. If this is ever made thread-safe, Init also needs to
    ** do some extra one-time setup for mutexes
    */
    return BPLib_STOR_LoadBatch_Reset(Batch);
}

BPLib_Status_t BPLib_STOR_LoadBatch_Reset(BPLib_STOR_LoadBatch_t* Batch)
{
    if (Batch == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    Batch->Size = 0;
    Batch->ReadIndex = 0;
    Batch->EgressOpInProgress = false;
    return BPLIB_SUCCESS;
}

bool BPLib_STOR_LoadBatch_IsEmpty(BPLib_STOR_LoadBatch_t* Batch)
{
    if (Batch == NULL)
    {
        return true;
    }
    return (Batch->Size == 0);
}

bool BPLib_STOR_LoadBatch_IsConsumed(BPLib_STOR_LoadBatch_t* Batch)
{
    if (Batch == NULL)
    {
        return true;
    }
    return (Batch->ReadIndex >= Batch->Size);
}

BPLib_Status_t BPLib_STOR_LoadBatch_AddID(BPLib_STOR_LoadBatch_t* Batch, int64_t BundleRowID)
{
    if (Batch == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if (Batch->Size >= BPLIB_STOR_LOADBATCHSIZE)
    {
        return BPLIB_STOR_BATCH_FULL;
    }

    Batch->BundleRowIDs[Batch->Size++] = BundleRowID;
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_STOR_LoadBatch_PeekNextID(BPLib_STOR_LoadBatch_t* Batch, int64_t *BundleRowID)
{
    if ((Batch == NULL) || (BundleRowID == NULL))
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if (BPLib_STOR_LoadBatch_IsEmpty(Batch))
    {
        return BPLIB_STOR_BATCH_EMPTY;
    }

    if (Batch->ReadIndex >= Batch->Size)
    {
        return BPLIB_STOR_BATCH_CONSUMED;
    }

    *BundleRowID = Batch->BundleRowIDs[Batch->ReadIndex];
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_STOR_LoadBatch_AdvanceReader(BPLib_STOR_LoadBatch_t* Batch)
{
    if (Batch == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if (BPLib_STOR_LoadBatch_IsEmpty(Batch))
    {
        return BPLIB_STOR_BATCH_EMPTY;
    }

    if (Batch->ReadIndex >= Batch->Size)
    {
        return BPLIB_STOR_BATCH_CONSUMED;
    }

    Batch->ReadIndex++;
    return BPLIB_SUCCESS;
}

bool BPLib_STOR_LoadBatch_InProgressEgress(BPLib_Instance_t *Inst)
{
    uint32_t i;
    
    for (i = 0; i < BPLIB_MAX_NUM_CONTACTS; i++)
    {
        if (Inst->BundleStorage.ContactLoadBatches[i].EgressOpInProgress)
        {
            return true;
        }
    }

    for (i = 0; i < BPLIB_MAX_NUM_CHANNELS; i++)
    {
        if (Inst->BundleStorage.ChannelLoadBatches[i].EgressOpInProgress)
        {
            return true;
        }
    }

    return false;
}

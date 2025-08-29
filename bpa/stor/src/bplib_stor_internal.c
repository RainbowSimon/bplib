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

/* ======== */
/* Includes */
/* ======== */

#include "bplib_stor_internal.h"
#include "bplib_stor_sql_store.h"
#include "bplib_eid.h"
#include "bplib_as.h"

/* ==================== */
/* Function Definitions */
/* ==================== */

BPLib_Status_t BPLib_STOR_FlushPendingUnlocked(BPLib_Instance_t* Inst)
{
    BPLib_Status_t Status;
    BPLib_BundleCache_t* CacheInst;
    int i;
    size_t TotalBytesStored = 0;

    CacheInst = &Inst->BundleStorage;

    Status = BPLib_SQL_Store(Inst, &TotalBytesStored);

    if (Status == BPLIB_SUCCESS) 
    {
        CacheInst->BytesStorageInUse += TotalBytesStored;
        CacheInst->BundleCountStored += CacheInst->InsertBatchSize;
    }
    else if (Status == BPLIB_STOR_DB_FULL_ERR)
    {
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED, CacheInst->InsertBatchSize);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DISCARDED, CacheInst->InsertBatchSize);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED_NO_STORAGE, CacheInst->InsertBatchSize);
        BPLib_EM_SendEvent(BPLIB_STOR_DB_FULL_INF_EID, BPLib_EM_EventType_INFORMATION,
            "SQLite database is full, dropping %d bundles", CacheInst->InsertBatchSize);        
    }
    else
    {
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED, CacheInst->InsertBatchSize);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DISCARDED, CacheInst->InsertBatchSize);
        BPLib_EM_SendEvent(BPLIB_STOR_SQL_STORE_ERR_EID, BPLib_EM_EventType_ERROR,
            "BPLib_SQL_Store failed to store bundle. RC=%d", Status);
        
    }
    /* Free the bundles, as they're now persistent
    ** Note: even if the storage fails, we free everything to avoid a leak.
    */
    for (i = 0; i < CacheInst->InsertBatchSize; i++)
    {
        BPLib_MEM_BundleFree(&Inst->pool, CacheInst->InsertBatch[i]);
    }

    CacheInst->InsertBatchSize = 0;

    return Status;
}
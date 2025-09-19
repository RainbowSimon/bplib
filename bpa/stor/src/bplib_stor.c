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

#include "bplib_stor.h"
#include "bplib_qm.h"
#include "bplib_em.h"
#include "bplib_eventids.h"
#include "bplib_fwp.h"
#include "bplib_nc.h"
#include "bplib_eid.h"
#include "bplib_as.h"
#include "bplib_stor_sql.h"
#include "bplib_stor_sql_store.h"
#include "bplib_stor_sql_load.h"

#include <stdio.h>

/* ======= */
/* Globals */
/* ======= */

/* SQL query statements */
sqlite3_stmt* MarkEgressedStmt;
sqlite3_stmt* FindBlobStmt;
sqlite3_stmt* InsertBlobStmt;
sqlite3_stmt* InsertMetadataStmt;
sqlite3_stmt* ForeignKeyCheckStmt;
sqlite3_stmt* GetNumBundlesStmt;
sqlite3_stmt* TotalBytesStmt;
sqlite3_stmt* PageCntStmt;
sqlite3_stmt* DiscardExpiredStmt;
sqlite3_stmt* ExpiredBytesStmt;
sqlite3_stmt* DiscardEgressedStmt;
sqlite3_stmt* EgressedBytesStmt;

/* SQL query strings */
const char* FindForEgressID_RangeClause =
"((dest_node BETWEEN ? AND ?) AND (dest_service BETWEEN ? AND ?))";

const char* MarkEgressedSQL =
"UPDATE bundle_data SET egress_attempted = 1 WHERE id = ?;";

const char* FindBlobSQL =
"SELECT id\n"
"FROM bundle_blobs\n"
"WHERE bundle_row = ?;";

/* Insert Bundle Blob */
const char* InsertBlobSQL =
"INSERT INTO bundle_blobs (bundle_row, blob_data) VALUES (?, ?)";

/* Insert Bundle Metadata (duplicate bundle_id entries are ignored) */
const char* InsertMetadataSQL =
"INSERT INTO bundle_data (bundle_id, action_timestamp, dest_node, dest_service, bundle_bytes) VALUES (?, ?, ?, ?, ?);";

const char* GetNumBundlesSQL =
"SELECT COUNT(*) FROM bundle_data;";

const char* TotalBytesSQL =
"SELECT SUM(bundle_bytes) "
"AS TotalBytes "
"FROM bundle_data;";

const char* DiscardExpiredSQL =
"WITH to_delete AS ("
"    SELECT id FROM bundle_data "
"    WHERE (action_timestamp < ?) AND (egress_attempted = 0) "
"    LIMIT ?"
") "
"DELETE FROM bundle_data "
"WHERE id IN (SELECT id FROM to_delete);";

const char* ExpiredBytesSQL =
"WITH expired_bytes AS (\n"
"   SELECT id, bundle_bytes FROM bundle_data\n"
"   WHERE (action_timestamp < ?) AND (egress_attempted = 0)\n"
"   LIMIT ?)\n"
"SELECT SUM(bundle_bytes)\n"
"AS bytes_deleted\n"
"FROM bundle_data\n"
"WHERE id IN (SELECT id FROM expired_bytes);\n";

const char* DiscardEgressedSQL =
"WITH to_delete AS ("
"    SELECT id FROM bundle_data "
"    WHERE egress_attempted = 1 "
"    LIMIT ?"
") "
"DELETE FROM bundle_data "
"WHERE id IN (SELECT id FROM to_delete);";

const char* EgressedBytesSQL =
"WITH egressed_bytes AS (\n"
"   SELECT id, bundle_bytes FROM bundle_data\n"
"   WHERE egress_attempted = 1\n"
"   LIMIT ?)\n"
"SELECT SUM(bundle_bytes)\n"
"AS bytes_deleted\n"
"FROM bundle_data\n"
"WHERE id IN (SELECT id FROM egressed_bytes);\n";

/*
** Table and Index Creation for bundle_data and bundle_blobs
**
** This schema is designed to support efficient queries and operations on bundle metadata and associated blob data.
** The following indexes are created:
**
** 1. idx_bundle_blobs_bundle_row:
**    - Index on the 'bundle_row' column in the 'bundle_blobs' table. This index supports quick lookup of blob data
**      by its associated bundle_row in the 'bundle_data' table.
**
** 2. idx_action_timestamp:
**    - Index on 'action_timestamp' in the 'bundle_data' table. This helps with queries that need to sort or filter
**      based on the timestamp of the bundle: This is used for expiring bundles
**
** 3. idx_find_bundle (Composite Index):
**    - Composite index on the columns 'dest_node', 'dest_service', 'egress_attempted', 'action_timestamp', and 'id'.
**    - This index optimizes queries that filter by node and service ranges, filter by egress_attempted (0),
**      and sort by action_timestamp. It can also enable an index-only scan to quickly retrieve 'id'.
**    - This composite index is designed for loading egress bundles by batch for a particular EgressID (A channel or contact)
**
** 4. idx_egress_attempted:
**    - Index on the 'egress_attempted' column in the 'bundle_data' table. This index is designed to speed up
**      DELETE queries and other queries filtering by 'egress_attempted'.
**
** 5. idx_bundle_id
**    - Index on the bplib-assigned unique 'bundle_id' in the 'bundle_data' table. This is used to detect duplicate bundles
**      in storage and by Custody Transfer to request the deletion or retransmission of custodial bundles. Whether or not
**      to allow duplicate bundles in storage is toggled by the BPLIB_ALLOW_DUPLICATE_BUNDLES flag.
**/

const char* CreateTableSQL =
"CREATE TABLE IF NOT EXISTS bundle_data (\n"
"    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
#if BPLIB_ALLOW_DUPLICATE_BUNDLES == false
"    bundle_id INTEGER UNIQUE,\n"
#else
"    bundle_id INTEGER,\n"
#endif
"    action_timestamp INTEGER,\n"
"    egress_attempted INTEGER DEFAULT 0,\n"
"    dest_node INTEGER,\n"
"    dest_service INTEGER,\n"
"    bundle_bytes INTEGER\n"
");\n"
"\n"
"CREATE TABLE IF NOT EXISTS bundle_blobs (\n"
"    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
"    bundle_row INTEGER,\n"
"    blob_data BLOB,\n"
"    FOREIGN KEY (bundle_row) REFERENCES bundle_data(id) ON DELETE CASCADE\n"
");\n"
"\n"
"CREATE INDEX IF NOT EXISTS idx_bundle_blobs ON bundle_blobs (bundle_row);\n"
"CREATE INDEX IF NOT EXISTS idx_action_timestamp ON bundle_data (action_timestamp);\n"
"CREATE INDEX IF NOT EXISTS idx_bundle_id ON bundle_data (bundle_id);\n"
"\n"
"CREATE INDEX IF NOT EXISTS idx_egress_id\n"
"ON bundle_data (\n"
"    dest_node,\n"
"    dest_service,\n"
"    egress_attempted,\n"
"    action_timestamp,\n"
"    id\n"
");\n"
"\n"
"CREATE INDEX IF NOT EXISTS idx_egress_attempted\n"
"ON bundle_data (egress_attempted);\n";

BPLib_StorageHkTlm_Payload_t BPLib_STOR_StoragePayload;

/* ==================== */
/* Function Definitions */
/* ==================== */

BPLib_Status_t BPLib_STOR_Init(BPLib_Instance_t* Inst)
{
    BPLib_Status_t Status;
    size_t         i;

    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    /* Zero-out the storage housekeeping payload */
    memset((void*) &BPLib_STOR_StoragePayload, 0, sizeof(BPLib_StorageHkTlm_Payload_t));

    /* Zero-out the bundle storage */
    memset(&Inst->BundleStorage, 0, sizeof(BPLib_BundleCache_t));

    pthread_mutex_init(&Inst->BundleStorage.lock, NULL);

    for (i = 0; i < BPLIB_MAX_NUM_CHANNELS; i++)
    {
        Status = BPLib_STOR_LoadBatch_Init(&Inst->BundleStorage.ChannelLoadBatches[i]);
        if (Status != BPLIB_SUCCESS)
        {
            return Status;
        }
    }

    for (i = 0; i < BPLIB_MAX_NUM_CONTACTS; i++)
    {
        Status = BPLib_STOR_LoadBatch_Init(&Inst->BundleStorage.ContactLoadBatches[i]);
        if (Status != BPLIB_SUCCESS)
        {
            return Status;
        }
    }

    Status = BPLib_SQL_Init(Inst, (const char*) BPLIB_STOR_DBNAME);

    return Status;
}

void BPLib_STOR_Destroy(BPLib_Instance_t* Inst)
{
    if (Inst == NULL)
    {
        return;
    }

    pthread_mutex_destroy(&Inst->BundleStorage.lock);
}

/* Validate Storage table data */
BPLib_Status_t BPLib_STOR_StorageTblValidateFunc(void *TblData)
{
    BPLib_Status_t ReturnCode = BPLIB_SUCCESS;

    return ReturnCode;
}

BPLib_Status_t BPLib_STOR_StoreBundle(BPLib_Instance_t* Inst, BPLib_Bundle_t* Bundle)
{
    BPLib_Status_t       Status;
    BPLib_BundleCache_t* CacheInst;

    Status = BPLIB_SUCCESS;

    if ((Inst == NULL) || (Bundle == NULL) || (Bundle->blob == NULL))
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    CacheInst = &Inst->BundleStorage;

    pthread_mutex_lock(&CacheInst->lock);

    /* Add to the next batch */
    CacheInst->InsertBatch[CacheInst->InsertBatchSize++] = Bundle;
    if (CacheInst->InsertBatchSize == BPLIB_STOR_INSERTBATCHSIZE)
    {
        Status = BPLib_STOR_FlushPendingUnlocked(Inst);
    }

    pthread_mutex_unlock(&CacheInst->lock);

    return Status;
}

BPLib_Status_t BPLib_STOR_FlushPending(BPLib_Instance_t* Inst)
{
    BPLib_Status_t       Status;
    BPLib_BundleCache_t* CacheInst;

    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    CacheInst = &Inst->BundleStorage;

    pthread_mutex_lock(&CacheInst->lock);

    if (CacheInst->InsertBatchSize > 0)
    {
        Status = BPLib_STOR_FlushPendingUnlocked(Inst);
    }
    else
    {
        /* Don't go further if there's nothing to store */
        Status = BPLIB_SUCCESS;
    }

    pthread_mutex_unlock(&CacheInst->lock);

    return Status;
}

BPLib_Status_t BPLib_STOR_EgressForID(BPLib_Instance_t* Inst, uint32_t EgressID,
                                        bool LocalDelivery, size_t* NumEgressed)
{
    BPLib_Status_t          Status;
    BPLib_BundleCache_t*    CacheInst;
    BPLib_STOR_LoadBatch_t* LoadBatch;
    BPLib_Bundle_t*         CurrBundle;
    BPLib_EID_Pattern_t     LocalEID;
    BPLib_EID_Pattern_t*    DestEIDs;
    BPLib_QM_WaitQueue_t*   EgressQueue;
    size_t                  EgressCnt;
    int64_t                 CurrBundleID;
    size_t                  NumEIDs;

    Status     = BPLIB_SUCCESS;
    CurrBundle = NULL;
    EgressCnt  = 0;

    if ((Inst == NULL) || (NumEgressed == NULL))
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if (LocalDelivery && EgressID >= BPLIB_MAX_NUM_CHANNELS)
    {
        return BPLIB_STOR_PARAM_ERR;
    }

    if (!LocalDelivery && EgressID >= BPLIB_MAX_NUM_CONTACTS)
    {
        return BPLIB_STOR_PARAM_ERR;
    }

    if (BPLib_QM_IsIngressIdle(Inst) == false)
    {
        /* Avoid searching the DB if the unsorted jobs queue (which is the ingress queue) isn't empty.
        ** Note: this is a pretty critical performance optimization that allows bplib
        ** to use all of its CPU resources for ingress.
        */
        *NumEgressed = 0;

        return BPLIB_SUCCESS;
    }

    /* Determine which channel or contact's batch we're examining */
    BPLib_NC_ReaderLock();

    CacheInst = &Inst->BundleStorage;

    if (LocalDelivery)
    {
        LoadBatch           = &(CacheInst->ChannelLoadBatches[EgressID]);
        LocalEID.MaxNode    = BPLIB_EID_INSTANCE.Node;
        LocalEID.MinNode    = BPLIB_EID_INSTANCE.Node;
        LocalEID.MaxService = BPLib_NC_ConfigPtrs.ChanConfigPtr->Configs[EgressID].LocalServiceNumber;
        LocalEID.MinService = BPLib_NC_ConfigPtrs.ChanConfigPtr->Configs[EgressID].LocalServiceNumber;
        DestEIDs            = &LocalEID;
        NumEIDs             = 1;
        EgressQueue         = &(Inst->ChannelEgressJobs[EgressID]);
    }
    else
    {
        LoadBatch   = &(CacheInst->ContactLoadBatches[EgressID]);
        DestEIDs    = BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[EgressID].DestEIDs;
        NumEIDs     = BPLIB_MAX_CONTACT_DEST_EIDS;
        EgressQueue = &(Inst->ContactEgressJobs[EgressID]);
    }

    BPLib_NC_ReaderUnlock();

    pthread_mutex_lock(&CacheInst->lock);

    /* If the load batch is empty, try to read more from storage */
    if (BPLib_STOR_LoadBatch_IsEmpty(LoadBatch))
    {
        /* Ask SQL to load egressable bundles from the specified Destination EID */
        Status = BPLib_SQL_FindForEIDs(Inst, LoadBatch, DestEIDs, NumEIDs);
        if (Status != BPLIB_SUCCESS)
        {
            BPLib_EM_SendEvent(BPLIB_STOR_SQL_LOAD_ERR_EID,
                                BPLib_EM_EventType_ERROR,
                                "BPLib_SQL_FindForEIDs failed to load bundle. RC=%d",
                                Status);
        }
    }
    else if (BPLib_STOR_LoadBatch_IsConsumed(LoadBatch))
    { /* All of the bundles for this batch have been egressed */
        /* Mark the batch as egressed */
        Status = BPLib_SQL_MarkBatchEgressed(Inst, LoadBatch);

        /* Clear the batch */
        (void) BPLib_STOR_LoadBatch_Reset(LoadBatch);
    }
    else
    { /* There are bundles in the current batch that need to be egressed */
        while (BPLib_STOR_LoadBatch_PeekNextID(LoadBatch, &CurrBundleID) == BPLIB_SUCCESS)
        {
            /* Set the metadata EID */
            Status = BPLib_SQL_LoadBundle(Inst, CurrBundleID, &CurrBundle);
            if (Status == BPLIB_SUCCESS)
            {
                CurrBundle->Meta.EgressID = EgressID;
                if (BPLib_QM_WaitQueueTryPush(EgressQueue, &CurrBundle, QM_NO_WAIT) == false)
                {
                    /* If QM couldn't accept the bundle, free it. It will be reloaded
                    ** next time.
                    */
                    BPLib_MEM_BundleFree(&Inst->pool, CurrBundle);

                    break;
                }

                /* After the bundle made it into the destination queue, mark it consumed */
                (void) BPLib_STOR_LoadBatch_AdvanceReader(LoadBatch);
                EgressCnt++;
            }
            else if (Status == BPLIB_STOR_NO_BUNDLE_FOUND_ERR)
            {
                /* Bundle ID belongs to a bundle that is now invalid, discard it but keep going */
                (void) BPLib_STOR_LoadBatch_AdvanceReader(LoadBatch);
            }
            else
            {
                /* If LoadBundle Failed, don't keep trying. */
                break;
            }
        }
    }

    pthread_mutex_unlock(&CacheInst->lock);

    *NumEgressed = EgressCnt;
    return Status;
}

BPLib_Status_t BPLib_STOR_GarbageCollect(BPLib_Instance_t* Inst)
{
    BPLib_Status_t       Status;
    BPLib_BundleCache_t* CacheInst;
    size_t               NumDiscarded;

    NumDiscarded = 0;

    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if (BPLib_QM_IsIngressIdle(Inst) == false)
    {
        /* Avoid searching the DB if the unsorted jobs queue (which is the ingress queue) isn't empty
        ** Note: this is a pretty critical performance optimization that allows bplib
        ** to use all of its CPU resources for ingress.
        */
        return BPLIB_SUCCESS;
    }

    CacheInst = &Inst->BundleStorage;

    pthread_mutex_lock(&CacheInst->lock);

    Status = BPLib_SQL_DiscardExpired(Inst, &NumDiscarded);
    if (Status != BPLIB_SUCCESS)
    {
        BPLib_EM_SendEvent(BPLIB_STOR_SQL_GC_ERR_EID,
                            BPLib_EM_EventType_ERROR,
                            "BPLib_SQL_DiscardExpired failed. RC=%d",
                            Status);
    }
    else if (NumDiscarded > 0)
    {
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED_EXPIRED, NumDiscarded);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED, NumDiscarded);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DISCARDED, NumDiscarded);

        CacheInst->BundleCountStored -= NumDiscarded;

        BPLib_EM_SendEvent(BPLIB_STOR_EXPIRE_DBG_EID,
                            BPLib_EM_EventType_DEBUG,
                            "Discarded %d expired bundles from storage",
                            NumDiscarded);
    }

    Status = BPLib_SQL_DiscardEgressed(Inst, &NumDiscarded);
    if (Status != BPLIB_SUCCESS)
    {
        BPLib_EM_SendEvent(BPLIB_STOR_SQL_GC_ERR_EID,
                            BPLib_EM_EventType_ERROR,
                            "BPLib_SQL_DiscardEgressed failed. RC=%d",
                            Status);
    }
    else if (NumDiscarded > 0)
    {
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DISCARDED, NumDiscarded);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED, NumDiscarded);

        CacheInst->BundleCountStored -= NumDiscarded;

        BPLib_EM_SendEvent(BPLIB_STOR_DELETE_DBG_EID,
                            BPLib_EM_EventType_DEBUG,
                            "Discarded %d egressed bundles from storage",
                            NumDiscarded);
    }

    pthread_mutex_unlock(&CacheInst->lock);

    return Status;
}

void BPLib_STOR_UpdateHkPkt(BPLib_Instance_t* Inst)
{
    BPLib_Status_t Status;
    size_t         DbSize;

    Status = BPLib_SQL_GetDbSize(Inst, &DbSize);
    if (Status == BPLIB_SUCCESS)
    {
        Inst->BundleStorage.StorageSize = DbSize;
        BPLib_STOR_StoragePayload.KbStorageInUse = DbSize / 1000;
    }
    else
    {
        BPLib_EM_SendEvent(BPLIB_STOR_DB_GET_SIZE_ERR_EID,
                            BPLib_EM_EventType_ERROR,
                            "Error getting database size, RC = %d.",
                            Status);
    }

    /* Update the memory in use*/
    BPLib_STOR_StoragePayload.BytesMemInUse = ((Inst->pool.impl.num_blocks - Inst->pool.impl.num_free) * Inst->pool.impl.block_size);

    /* Update the highwater mark if needed */
    if (BPLib_STOR_StoragePayload.BytesMemInUse > BPLib_STOR_StoragePayload.BytesMemHighWater)
    {
        BPLib_STOR_StoragePayload.BytesMemHighWater = BPLib_STOR_StoragePayload.BytesMemInUse;
    }

    /* Update the free memory */
    BPLib_STOR_StoragePayload.BytesMemFree = (Inst->pool.impl.num_free * Inst->pool.impl.block_size);

    /* Update kilobytes of data in use */
    BPLib_STOR_StoragePayload.KbBundlesInStor = (Inst->BundleStorage.BytesStorageInUse / 1000);

    return;
}

BPLib_Status_t BPLib_STOR_FlushPendingUnlocked(BPLib_Instance_t* Inst)
{
    BPLib_Status_t       Status;
    BPLib_BundleCache_t* CacheInst;
    size_t               i;
    size_t               TotalBytesStored;
    size_t               DuplicateBundlesIgnored;

    CacheInst               = &Inst->BundleStorage;
    TotalBytesStored        = 0;
    DuplicateBundlesIgnored = 0;

    Status = BPLib_SQL_Store(Inst, &TotalBytesStored, &DuplicateBundlesIgnored);

    if (Status == BPLIB_SUCCESS)
    {
        CacheInst->BytesStorageInUse += TotalBytesStored;
        CacheInst->BundleCountStored += CacheInst->InsertBatchSize - DuplicateBundlesIgnored;

        if (DuplicateBundlesIgnored > 0)
        {
            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED, DuplicateBundlesIgnored);
            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DISCARDED, DuplicateBundlesIgnored);
            BPLib_EM_SendEvent(BPLIB_STOR_DUPL_DBG_EID,
                                BPLib_EM_EventType_DEBUG,
                                "Ignored %ld duplicate bundles in store batch.",
                                DuplicateBundlesIgnored);
        }

    }
    else if (Status == BPLIB_STOR_DB_FULL_ERR)
    {
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED, CacheInst->InsertBatchSize);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DISCARDED, CacheInst->InsertBatchSize);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED_NO_STORAGE, CacheInst->InsertBatchSize);

        BPLib_EM_SendEvent(BPLIB_STOR_DB_FULL_INF_EID,
                            BPLib_EM_EventType_INFORMATION,
                            "SQLite database is full, dropping %d bundles",
                            CacheInst->InsertBatchSize);
    }
    else
    {
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED, CacheInst->InsertBatchSize);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DISCARDED, CacheInst->InsertBatchSize);

        BPLib_EM_SendEvent(BPLIB_STOR_SQL_STORE_ERR_EID,
                            BPLib_EM_EventType_ERROR,
                            "BPLib_SQL_Store failed to store bundle. RC=%d",
                            Status);

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
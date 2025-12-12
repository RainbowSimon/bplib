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
#include "bplib_stor_sql_cust.h"
#include "bplib_inst.h"

#include <stdio.h>

/* ======= */
/* Globals */
/* ======= */

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

    int64_t TimeStart = BPLib_TIME_GetMonotonicTime();

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

void BPLib_STOR_SetLastActiveTime(BPLib_Instance_t* Inst)
{
    if (Inst == NULL)
    {
        return;
    }

    pthread_mutex_lock(&Inst->BundleStorage.lock);

    Inst->BundleStorage.LastActiveTime = BPLib_TIME_GetMonotonicTime();

    pthread_mutex_unlock(&Inst->BundleStorage.lock);
}

bool BPLib_STOR_IsIngressEgressActive(BPLib_Instance_t* Inst)
{
    bool IsActive = false;

    if (Inst == NULL)
    {
        return false;
    }

    if ((Inst->BundleStorage.LastActiveTime + BPLIB_STOR_MAX_IDLE_TIME) > BPLib_TIME_GetMonotonicTime())
    {
        IsActive = true;
    }

    return IsActive;
}

BPLib_Status_t BPLib_STOR_GarbageCollect(BPLib_Instance_t* Inst)
{
    BPLib_Status_t       Status = BPLIB_SUCCESS;
    BPLib_BundleCache_t* CacheInst;
    size_t               NumDiscarded;
    size_t               DbSize;

    NumDiscarded = 0;

    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    CacheInst = &Inst->BundleStorage;

    pthread_mutex_lock(&CacheInst->lock);    

    if (BPLib_STOR_IsIngressEgressActive(Inst) == false)
    {
        /* 
        ** Avoid searching the DB if any of the ingress/egress queues are not empty or
        ** storage operations are ongoing.
        ** Note: this is a pretty critical performance optimization that allows bplib
        ** to use all of its CPU resources for ingress and egress.
        */

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
    }

    /* While we're at it, update our records with the latest storage size */
    Status = BPLib_SQL_GetDbSize(Inst, &DbSize);
    if (Status == BPLIB_SUCCESS)
    {
        Inst->BundleStorage.StorageSize = DbSize;
    }
    else
    {
        BPLib_EM_SendEvent(BPLIB_STOR_DB_GET_SIZE_ERR_EID,
                            BPLib_EM_EventType_ERROR,
                            "Error getting database size, RC = %d.",
                            Status);
    }

    pthread_mutex_unlock(&CacheInst->lock);

    return Status;
}

void BPLib_STOR_UpdateHkPkt(BPLib_Instance_t* Inst)
{
    /* Update total storage size */
    BPLib_STOR_StoragePayload.KbStorageInUse = Inst->BundleStorage.StorageSize / 1000;

    /* Update the memory in use*/
    BPLib_STOR_StoragePayload.BytesMemInUse = BPLib_MEM_GetBytesInUse(&Inst->pool);

    /* Update the highwater mark if needed */
    if (BPLib_STOR_StoragePayload.BytesMemInUse > BPLib_STOR_StoragePayload.BytesMemHighWater)
    {
        BPLib_STOR_StoragePayload.BytesMemHighWater = BPLib_STOR_StoragePayload.BytesMemInUse;
    }

    /* Update the free memory */
    BPLib_STOR_StoragePayload.BytesMemFree = BPLib_MEM_GetBytesFree(&Inst->pool);

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
    size_t               CustodialBundlesStored;
    BPLib_CLA_ContactRunState_t ConState;

    CacheInst               = &Inst->BundleStorage;
    TotalBytesStored        = 0;
    DuplicateBundlesIgnored = 0;
    CustodialBundlesStored  = 0;

    Status = BPLib_SQL_Store(Inst, &TotalBytesStored, &DuplicateBundlesIgnored, &CustodialBundlesStored);

    if (Status == BPLIB_SUCCESS)
    {
        if (CustodialBundlesStored > 0)
        {
            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_IN_CUSTODY, CustodialBundlesStored);
        }

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
        /* Custodial bundles with an egress path should get sent out instead of freed */
        if (CacheInst->InsertBatch[i]->Meta.IsCustodial && 
            CacheInst->InsertBatch[i]->Meta.EgressID < BPLIB_MAX_NUM_CONTACTS)
        {
            (void) BPLib_CLA_GetContactRunState(CacheInst->InsertBatch[i]->Meta.EgressID, &ConState);

            if (ConState == BPLIB_CLA_STARTED)
            {
                BPLib_QM_WaitQueueTryPush(&(Inst->ContactEgressJobs[CacheInst->InsertBatch[i]->Meta.EgressID]), 
                                            &CacheInst->InsertBatch[i], QM_WAIT_FOREVER);
            }
        }
        else
        {
            BPLib_MEM_BundleFree(&Inst->pool, CacheInst->InsertBatch[i]);
        }
    }

    CacheInst->InsertBatchSize = 0;

    return Status;
}

BPLib_Status_t BPLib_STOR_Cleanup(BPLib_Instance_t* Inst)
{
    BPLib_Status_t Status;

    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    BPLib_EM_SendEvent(BPLIB_STOR_CLEANUP_INF_EID, BPLib_EM_EventType_INFORMATION,
            "Beginning storage cleanup. This may take a while and may interrupt any pending storage operations.");

    pthread_mutex_lock(&(Inst->BundleStorage.lock));

    Status = BPLib_SQL_Cleanup(Inst);

    pthread_mutex_unlock(&(Inst->BundleStorage.lock));
    
    return Status;
}

BPLib_Status_t BPLib_STOR_UpdateCustodialBundles(BPLib_Instance_t* Inst, BPLib_CT_CcsUpdateBatch_t *Batch)
{
    BPLib_Status_t Status;

    if (Inst == NULL || Batch == NULL || Batch->Size > BPLIB_CT_BATCH_SIZE)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    pthread_mutex_lock(&(Inst->BundleStorage.lock));

    Status = BPLib_SQL_UpdateCustodialBundles(Inst, Batch);

    pthread_mutex_unlock(&(Inst->BundleStorage.lock));

    if (Status != BPLIB_SUCCESS)
    {
        BPLib_EM_SendEvent(BPLIB_STOR_CCS_ERR_EID, BPLib_EM_EventType_ERROR,
                "Error performing CCS storage operations, Status = %d.", Status);
    }
    
    return Status;
}

BPLib_Status_t BPLib_STOR_SetNewRetransmitTrigger(BPLib_Instance_t *Inst, uint32_t ContactId)
{
    BPLib_Status_t Status;
    size_t NumUpdated;

    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if (ContactId >= BPLIB_MAX_NUM_CONTACTS)
    {
        return BPLIB_INVALID_CONT_ID_ERR;
    }

    pthread_mutex_lock(&(Inst->BundleStorage.lock));
    BPLib_NC_ReaderLock();

    Status = BPLib_SQL_SetNewRetransmitTrigger(Inst, ContactId,
                BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[ContactId].DestEIDs,
                BPLIB_MAX_CONTACT_DEST_EIDS, 
                BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[ContactId].RetransmitTimeout,
                &NumUpdated);

    BPLib_NC_ReaderUnlock();
    pthread_mutex_unlock(&(Inst->BundleStorage.lock));

    if (Status == BPLIB_SUCCESS)
    {
        BPLib_EM_SendEvent(BPLIB_STOR_RETRANSMIT_UPDATE_DBG_EID, BPLib_EM_EventType_DEBUG,
                        "Updated retransmit triggers of %ld bundles for contact %d.",
                        NumUpdated, ContactId);
    }

    /* Error event handled upstream */
    
    return Status;    
}
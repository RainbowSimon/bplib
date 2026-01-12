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

/*
** Include
*/

#include "bplib_ct.h"
#include "bplib_ct_db.h"
#include "bplib_ct_ccs.h"
#include "bplib_bblocks.h"
#include "bplib_eid.h"
#include "bplib_pdb.h"
#include "bplib_as.h"
#include "bplib_inst.h"
#include "bplib_nc.h"

/*
** Function Definitions
*/

BPLib_Status_t BPLib_CT_Init(BPLib_Instance_t *Inst)
{
    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    memset(&(Inst->Ct), 0, sizeof(BPLib_CT_Context_t));

    pthread_mutex_init(&Inst->Ct.Lock, NULL);

    BPLib_RBT_InitRoot(&(Inst->Ct.SeqTreeRoot));
    BPLib_RBT_InitRoot(&(Inst->Ct.IdTreeRoot));

    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_SetBundleId(BPLib_Bundle_t *Bundle)
{
    uint64_t UniqueIdArr[BPLIB_CT_BUNDLE_IDENTIFIER_ARRAY_LEN];

    if (Bundle == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    UniqueIdArr[0] = Bundle->blocks.PrimaryBlock.Timestamp.SequenceNumber;
    UniqueIdArr[1] = Bundle->blocks.PrimaryBlock.Timestamp.CreateTime;
    UniqueIdArr[2] = Bundle->blocks.PrimaryBlock.SrcEID.Scheme;
    UniqueIdArr[3] = Bundle->blocks.PrimaryBlock.SrcEID.IpnSspFormat;
    UniqueIdArr[4] = Bundle->blocks.PrimaryBlock.SrcEID.Allocator;
    UniqueIdArr[5] = Bundle->blocks.PrimaryBlock.SrcEID.Node;
    UniqueIdArr[6] = Bundle->blocks.PrimaryBlock.SrcEID.Service;

    /*
    ** Add any additional unique identifiers for a bundle here
    ** (and increase the array length accordingly)
    */

    /* Use a CRC-32C as a quick hash function to get a unique ID for this bundle */
    Bundle->blocks.PrimaryBlock.BundleId = BPLib_CRC_Calculate((void *) ((uintptr_t) UniqueIdArr),
                                BPLIB_CT_BUNDLE_IDENTIFIER_ARRAY_LEN * sizeof(uint64_t),
                                BPLib_CRC_Type_CRC32C);

    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_ProcessNewBundle(BPLib_Instance_t* Inst, BPLib_Bundle_t *Bundle)
{
    BPLib_Status_t Status = BPLIB_SUCCESS;
    size_t OpenCcsIdx;
    uint8_t ExtBlockIdx;
    BPLib_CustodyBlockData_t *CtebPtr;
    BPLib_CT_DbEntry_t *DbEntry = NULL;
    BPLib_CT_DispositionCode_t DispCode;

    if (Bundle == NULL || Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    /* Set bundle ID for both custodial and non-custodial bundles */
    (void) BPLib_CT_SetBundleId(Bundle);

    /* Check if there's storage left */
    if ((Inst->BundleStorage.BytesStorageInUse + Bundle->Meta.TotalBytes) >= BPLIB_MAX_STORED_BUNDLE_BYTES)
    {
        BPLib_EM_SendEvent(BPLIB_CT_NO_STOR_ERR_EID, BPLib_EM_EventType_ERROR,
                            "Cannot accept %ld byte bundle, not enough storage remaining (%ld bytes).",
                            Bundle->Meta.TotalBytes, Inst->BundleStorage.BytesStorageInUse);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED_NO_STORAGE, 1);

        /* Additional counters handled by QM job */

        Status = BPLIB_NO_STOR_ERR;
    }

    for (ExtBlockIdx = 0; ExtBlockIdx < BPLIB_MAX_NUM_EXTENSION_BLOCKS; ExtBlockIdx++)
    {
        if (Bundle->blocks.ExtBlocks[ExtBlockIdx].Header.BlockType == BPLib_BlockType_CTEB)
        {
            break;
        }
    }

    /* No CTEB was found, skip custody operations for this bundle */
    if (ExtBlockIdx >= BPLIB_MAX_NUM_EXTENSION_BLOCKS)
    {
        return Status;
    }

    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CUSTODY_REQUEST, 1);
    Bundle->Meta.IsCustodial = true;
    CtebPtr = &(Bundle->blocks.ExtBlocks[ExtBlockIdx].BlockData.CustodyBlockData);

    /* Default to refused custody */
    DispCode = BPLib_CT_CustodyRefused;

    pthread_mutex_lock(&Inst->Ct.Lock);

    /* Reject custody due to lack of storage */
    if (Status == BPLIB_NO_STOR_ERR)
    {
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DEPLETED, 1);
    }
    /* Reject custody when CTDB is full */
    else if (Inst->Ct.CurrDbSize >= BPLIB_CT_DB_MAX_ENTRIES)
    {
        BPLib_EM_SendEvent(BPLIB_CT_NO_MEM_ERR_EID, BPLib_EM_EventType_ERROR,
                            "Cannot accept bundle, CTDB is full.");
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DEPLETED, 1);

    }
    /* Reject duplicate bundles */
    else if (BPLib_CT_GetEntryFromCtdbWithId(&(Inst->Ct),
                    Bundle->blocks.PrimaryBlock.BundleId, &DbEntry) == BPLIB_SUCCESS)
    {
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_REDUNDANT, 1);
    }

    /* Custody accepted! */
    else if (BPLib_PDB_AcceptCustody(Bundle) == BPLIB_SUCCESS)
    {
        DispCode = BPLib_CT_CustodyAccepted;
    }
    /* Else PDB rejected custody */

    /* Add to an open CCS to confirm either acceptance or rejection */
    OpenCcsIdx = BPLib_CT_GetOpenCcsIdx(Inst, &(CtebPtr->BlockSrcAdminEID),
                                        CtebPtr->BundleSeqId);

    Status = BPLib_CT_AddToOpenCcs(Inst, OpenCcsIdx, Bundle->Meta.IngressID,
                                    CtebPtr, DispCode);

    if (DispCode == BPLib_CT_CustodyRefused)
    {
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_REJECTED_CUSTODY, 1);
        Status = BPLIB_CT_CUSTODY_REFUSED_ERR;
    }

    pthread_mutex_unlock(&Inst->Ct.Lock);

    return Status;
}

BPLib_Status_t BPLib_CT_UpdateBundle(BPLib_Instance_t* Inst, BPLib_Bundle_t *Bundle)
{
    BPLib_Status_t            Status = BPLIB_SUCCESS;
    BPLib_CustodyBlockData_t *CtebPtr;
    uint8_t                   ExtBlockIdx;
    uint64_t SeqId;
    BPLib_CT_DbEntry_t *DbEntry = NULL;

    if (Inst == NULL || Bundle == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    for (ExtBlockIdx = 0; ExtBlockIdx < BPLIB_MAX_NUM_EXTENSION_BLOCKS; ExtBlockIdx++)
    {
        if (Bundle->blocks.ExtBlocks[ExtBlockIdx].Header.BlockType == BPLib_BlockType_CTEB)
        {
            break;
        }
    }

    /* A CTEB was detected, do custody operations */
    if (ExtBlockIdx < BPLIB_MAX_NUM_EXTENSION_BLOCKS)
    {
        CtebPtr = &(Bundle->blocks.ExtBlocks[ExtBlockIdx].BlockData.CustodyBlockData);

        /* Update CTEB fields */
        if (Bundle->Meta.EgressID < BPLIB_MAX_NUM_CONTACTS)
        {
            pthread_mutex_lock(&Inst->Ct.Lock);

            /* Check if this is a bundle retransmission from storage or a new bundle */
            Status = BPLib_CT_GetEntryFromCtdbWithId(&(Inst->Ct),
                                Bundle->blocks.PrimaryBlock.BundleId, &DbEntry);

            if (Status == BPLIB_SUCCESS)
            {
                CtebPtr->BundleSeqId = DbEntry->SeqId;
                CtebPtr->BundleSeqNum = DbEntry->SeqNum;
                BPLib_EID_CopyEids(&(CtebPtr->BlockSrcAdminEID), BPLIB_EID_INSTANCE);

                BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CUSTODY_RE_FORWARDED, 1);
            }
            /* If new bundle, update CTEB fields to new values */
            else
            {
                SeqId = BPLib_CT_GetSequenceId(&(Inst->Ct), Bundle);
                CtebPtr->BundleSeqId = SeqId;
                CtebPtr->BundleSeqNum = BPLib_CT_GetNextSequenceNum(&(Inst->Ct), SeqId);
                BPLib_EID_CopyEids(&(CtebPtr->BlockSrcAdminEID), BPLIB_EID_INSTANCE);
                Bundle->blocks.ExtBlocks[ExtBlockIdx].Header.RequiresEncode = true;

                Status = BPLib_CT_AddToCtdb(Inst, CtebPtr->BundleSeqId, CtebPtr->BundleSeqNum,
                                            Bundle->blocks.PrimaryBlock.BundleId);
            }
        }
        else
        {
            Status = BPLIB_INVALID_CONT_ID_ERR;
            BPLib_EM_SendEvent(BPLIB_CT_CCS_CRRPTD_ERR_EID, BPLib_EM_EventType_ERROR,
                    "Bundle has an invalid egress ID %d, check for memory corruption.", Bundle->Meta.EgressID);
        }

        pthread_mutex_unlock(&Inst->Ct.Lock);

    }

    /* Do nothing for non-custodial bundles */

    return Status;
}

BPLib_Status_t BPLib_CT_ProcessCcs(BPLib_Instance_t *Inst, BPLib_CT_DeserializedCcs_t *Ccs)
{
    size_t SeqCollectIdx;
    BPLib_Status_t Status = BPLIB_SUCCESS;

    if (Inst == NULL || Ccs == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if (Ccs->NumBundleSeqCollections >= BPLIB_CT_MAX_RECVD_SEQ_COLLECTIONS)
    {
        return BPLIB_BUF_LEN_ERROR;
    }

    /* Validate CCS here? TODO */

    pthread_mutex_lock(&Inst->Ct.Lock);

    for (SeqCollectIdx = 0; SeqCollectIdx < Ccs->NumBundleSeqCollections; SeqCollectIdx++)
    {
        if (Ccs->BundleSeqCollections[SeqCollectIdx].SeqRangeLen >= BPLIB_CT_MAX_SEQ_RANGE_LEN)
        {
            Status = BPLIB_BUF_LEN_ERROR;
            break;
        }

        Status = BPLib_CT_ProcessBundleSeqCollection(Inst, &(Ccs->BundleSeqCollections[SeqCollectIdx]));
    }

    pthread_mutex_unlock(&Inst->Ct.Lock);

    return Status;
}

BPLib_Status_t BPLib_CT_AssignSeqCounter(BPLib_Instance_t *Inst, uint32_t ContactId)
{
    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if (ContactId >= BPLIB_MAX_NUM_CONTACTS)
    {
        return BPLIB_INVALID_CONT_ID_ERR;
    }

    Inst->Ct.LastSeqCounterId++;
    Inst->Ct.SeqCounters[Inst->Ct.LastSeqCounterId % BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS] = 0;
    Inst->Ct.CurrActiveSeqIds[ContactId] = Inst->Ct.LastSeqCounterId;

    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_DeleteBundleFromCtdb(BPLib_Instance_t *Inst, uint32_t BundleId)
{
    BPLib_CT_DbEntry_t *DbEntry = NULL;
    BPLib_Status_t Status = BPLIB_SUCCESS;

    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    Status = BPLib_CT_GetEntryFromCtdbWithId(&(Inst->Ct), BundleId, &DbEntry);
    if (Status == BPLIB_SUCCESS)
    {
        Status = BPLib_CT_RemoveFromCtdb(Inst, DbEntry);
    }

    return Status;
}

void BPLib_CT_BuildAndSendOpenCcs(BPLib_Instance_t* Instance, BPLib_CT_OpenCcs_t* OpenCcs)
{
    pthread_mutex_lock(&Instance->Ct.Lock);
    BPLib_CT_BuildAndSendOpenCcs_Impl(Instance, OpenCcs);
    pthread_mutex_unlock(&Instance->Ct.Lock);
}

void BPLib_CT_CheckCcsTimeout(BPLib_Instance_t* Instance)
{
    BPLib_CT_Context_t* Context;
    size_t              OpenCcsIdx;
    int64_t             TimeOpen;
    BPLib_CT_OpenCcs_t* OpenCcs;

    pthread_mutex_lock(&Instance->Ct.Lock);

    Context = &(Instance->Ct);
    for (OpenCcsIdx = 0; OpenCcsIdx < BPLIB_CT_MAX_OPEN_CCS; OpenCcsIdx++)
    {
        OpenCcs = &(Context->OpenCcss[OpenCcsIdx]);

        /* Check whether in progress CCSs exceed the time trigger */
        if (OpenCcs->InProgress == true && OpenCcs->CollectionStartTime != 0)
        {
            TimeOpen = BPLib_TIME_GetMonotonicTime() - OpenCcs->CollectionStartTime;

            /* Check if the open CCS is due to be sent */
            if (TimeOpen > OpenCcs->MaxTime)
            {
                BPLib_CT_BuildAndSendOpenCcs_Impl(Instance, OpenCcs);
            }
        }
    }

    pthread_mutex_unlock(&Instance->Ct.Lock);
}
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

    BPLib_RBT_InitRoot(&(Inst->Ct.CtdbRoot));

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
    bool   DeleteBundle = false;
    uint8_t ExtBlockIdx;
    BPLib_CustodyBlockData_t *CtebPtr;

    if (Bundle == NULL || Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    /* Set bundle ID for both custodial and non-custodial bundles */
    (void) BPLib_CT_SetBundleId(Bundle);

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
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CUSTODY_REQUEST, 1);
        Bundle->Meta.IsCustodial = true;

        CtebPtr = &(Bundle->blocks.ExtBlocks[ExtBlockIdx].BlockData.CustodyBlockData);

        OpenCcsIdx = BPLib_CT_GetOpenCcsIdx(&(Inst->Ct), &(CtebPtr->BlockSrcAdminEID),
                                                CtebPtr->BundleSeqId);

        /* Check if we can accept custody of this bundle */
        if (BPLib_PDB_AcceptCustody(Bundle) == BPLIB_SUCCESS)
        {
            /* Add to custody accepted raw CCS */
            Status = BPLib_CT_AddToOpenCcs(&(Inst->Ct.OpenCcss[OpenCcsIdx]), CtebPtr->BundleSeqNum,
                                CtebPtr->BundleSeqId, BPLib_CT_CustodyAccepted);
            if (Status != BPLIB_SUCCESS)
            {
                DeleteBundle = true;
                BPLib_EM_SendEvent(BPLIB_CT_CCS_CRRPTD_ERR_EID, BPLib_EM_EventType_ERROR,
                        "Open CCS data failed sanity checks, check for memory corruption.");

            }
            else
            {
                /*
                ** TODO this is temporary, when custodial bundles are stored,
                ** that's when this actually gets incremented. Until then, custody is
                ** technically "pending".
                */
                BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_IN_CUSTODY, 1);
            }
        }
        else
        {
            /* Add to custody rejected raw CCS and mark bundle for deletion */
            DeleteBundle = true;
            Status = BPLib_CT_AddToOpenCcs(&(Inst->Ct.OpenCcss[OpenCcsIdx]), CtebPtr->BundleSeqNum,
                                CtebPtr->BundleSeqId, BPLib_CT_CustodyRefused);
        }
    }

    /* Do nothing for non-custodial bundles */

    if (DeleteBundle)
    {
        BPLib_MEM_BundleFree(&(Inst->pool), Bundle);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_REJECTED_CUSTODY, 1);
    }

    return Status;
}

BPLib_Status_t BPLib_CT_UpdateBundle(BPLib_Instance_t* Inst, BPLib_Bundle_t *Bundle)
{
    BPLib_Status_t Status = BPLIB_SUCCESS;
    BPLib_CustodyBlockData_t *CtebPtr;
    uint8_t ExtBlockIdx;
    uint64_t SeqId;

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
            SeqId = BPLib_CT_GetSequenceId(&(Inst->Ct), Bundle);
            CtebPtr->BundleSeqId = SeqId;
            CtebPtr->BundleSeqNum = BPLib_CT_GetNextSequenceNum(&(Inst->Ct), SeqId);
            BPLib_EID_CopyEids(&(CtebPtr->BlockSrcAdminEID), BPLIB_EID_INSTANCE);
            Bundle->blocks.ExtBlocks[ExtBlockIdx].Header.RequiresEncode = true;

            Status = BPLib_CT_AddToCtdb(&(Inst->Ct), CtebPtr->BundleSeqId, CtebPtr->BundleSeqNum,
                                        Bundle->blocks.PrimaryBlock.BundleId);
        }
        else
        {
            BPLib_EM_SendEvent(BPLIB_CT_CCS_CRRPTD_ERR_EID, BPLib_EM_EventType_ERROR,
                    "Bundle has an invalid egress ID %d, check for memory corruption.", Bundle->Meta.EgressID);
        }
    }

    printf("status = %d\n", Status);

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

    for (SeqCollectIdx = 0; SeqCollectIdx < Ccs->NumBundleSeqCollections; SeqCollectIdx++)
    {
        if (Ccs->BundleSeqCollections[SeqCollectIdx].SeqRangeLen >= BPLIB_CT_MAX_SEQ_RANGE_LEN)
        {
            Status = BPLIB_BUF_LEN_ERROR;
            break;
        }

        Status = BPLib_CT_ProcessBundleSeqCollection(&(Inst->Ct), &(Ccs->BundleSeqCollections[SeqCollectIdx]));

        if (Status != BPLIB_SUCCESS)
        {
            /* TODO do something? */
            break;
        }
    }

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

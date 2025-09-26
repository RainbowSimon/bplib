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
#include "bplib_mem.h"
#include "bplib_pdb.h"
#include "bplib_qm.h"

/*
** Function Definitions
*/

BPLib_Status_t BPLib_CT_Init(BPLib_Instance_t *Inst)
{
    memset(&(Inst->Ct), 0, sizeof(BPLib_CT_Context_t));

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
    size_t RawCcsIdx;
    bool   DeleteBundle = false;
    uint8_t ExtBlockIdx;
    BPLib_CustodyBlockData_t *CtebPtr;

    if (Bundle == NULL || Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    /* Set bundle ID for both custodial and non-custodial bundles */
    (void) BPLib_CT_SetBundleId(Bundle);

    for (ExtBlockIdx = 0; ExtBlockIdx < BPLIB_MAX_NUM_CANONICAL_BLOCKS; ExtBlockIdx++)
    {
        if (Bundle->blocks.ExtBlocks[ExtBlockIdx].Header.BlockType == BPLib_BlockType_CTEB)
        {
            break;
        }
    }

    /* A CTEB was detected, do custody operations */
    if (ExtBlockIdx < BPLIB_MAX_NUM_CANONICAL_BLOCKS)
    {
        Bundle->Meta.IsCustodial = true;
        
        CtebPtr = &(Bundle->blocks.ExtBlocks[ExtBlockIdx].BlockData.CustodyBlockData);

        RawCcsIdx = BPLib_CT_GetRawCcsIdx(&(Inst->Ct), &(CtebPtr->BlockSrcAdminEID));

        /* Check if we can accept custody of this bundle */
        if (BPLib_PDB_AcceptCustody(Bundle) == BPLIB_SUCCESS)
        {            
            /* Add to custody accepted raw CCS */
            Status = BPLib_CT_AddToRawCcs(&(Inst->Ct.RawCcss[RawCcsIdx]), CtebPtr->BundleSeqNum, 
                                CtebPtr->BundleSeqId, BPLib_CT_CustodyAccepted);
            if (Status != BPLIB_SUCCESS)
            {
                DeleteBundle = true;
            }
            else
            {
                /* Check size trigger of this CCS TODO */
            }
        }
        else
        {
            /* Add to custody rejected raw CCS and mark bundle for deletion */
            DeleteBundle = true;
            Status = BPLib_CT_AddToRawCcs(&(Inst->Ct.RawCcss[RawCcsIdx]), CtebPtr->BundleSeqNum, 
                                CtebPtr->BundleSeqId, BPLib_CT_CustodyRefused);            
        }
    }

    /* Do nothing for non-custodial bundles */

    if (DeleteBundle)
    {
        BPLib_MEM_BundleFree(&(Inst->pool), Bundle);
        // TODO counters
    }

    return Status;
}

BPLib_Status_t BPLib_CT_UpdateBundle(BPLib_Instance_t* Inst, BPLib_Bundle_t *Bundle)
{
    BPLib_Status_t Status = BPLIB_SUCCESS;
    BPLib_CustodyBlockData_t *CtebPtr;
    uint8_t ExtBlockIdx;
    uint64_t SeqId;

    for (ExtBlockIdx = 0; ExtBlockIdx < BPLIB_MAX_NUM_CANONICAL_BLOCKS; ExtBlockIdx++)
    {
        if (Bundle->blocks.ExtBlocks[ExtBlockIdx].Header.BlockType == BPLib_BlockType_CTEB)
        {
            break;
        }
    }

    /* A CTEB was detected, do custody operations */
    if (ExtBlockIdx < BPLIB_MAX_NUM_CANONICAL_BLOCKS)
    {
        CtebPtr = &(Bundle->blocks.ExtBlocks[ExtBlockIdx].BlockData.CustodyBlockData);

        /* Update CTEB fields */

        Status = BPLib_CT_GetSequenceId(&(Inst->Ct), Bundle, &SeqId);

        if (Status == BPLIB_SUCCESS)
        {
            CtebPtr->BundleSeqId = SeqId;
            CtebPtr->BundleSeqNum = BPLib_CT_GetNextSequenceNum(&(Inst->Ct), SeqId);
            BPLib_EID_CopyEids(&(CtebPtr->BlockSrcAdminEID), BPLIB_EID_INSTANCE);
            Bundle->blocks.ExtBlocks[ExtBlockIdx].Header.RequiresEncode = true;

            Status = BPLib_CT_AddToCtdb(&(Inst->Ct), CtebPtr->BundleSeqId, CtebPtr->BundleSeqNum, 
                                        Bundle->blocks.PrimaryBlock.BundleId);
        }
    }

    /* Do nothing for non-custodial bundles */

    return Status;    
}

BPLib_Status_t BPLib_CT_ProcessBundleSeqCollection(BPLib_Instance_t *Inst, 
                                        BPLib_CT_BundleSeqCollection_t *SeqCollection)
{
    size_t SeqRangeIdx;
    size_t CurrSeqNum;
    size_t NextSeqNum;
    BPLib_Status_t Status = BPLIB_SUCCESS;
    BPLib_CT_DbEntry_t *DbEntry = NULL;

    CurrSeqNum =  SeqCollection->FirstSeqNum;

    for (SeqRangeIdx = 0; SeqRangeIdx < SeqCollection->SeqRangeLen; SeqRangeIdx++)
    {
        for (NextSeqNum = CurrSeqNum; NextSeqNum < CurrSeqNum + SeqCollection->SeqRange[SeqRangeIdx]; NextSeqNum++)
        {
            Status = BPLib_CT_GetEntryFromCtdb(&(Inst->Ct), SeqCollection->SeqId, 
                                                            NextSeqNum, DbEntry);

            if (Status != BPLIB_SUCCESS || DbEntry != NULL)
            {
                /* error? sequence number doesn't exist TODO */
            }

            /* Even sequence range numbers indicate sequences that are *included* */
            else if (SeqRangeIdx % 2 == 0)
            {
                /* Request bundle deletion from storage TODO */
                Status = BPLib_CT_RemoveFromCtdb(&(Inst->Ct), DbEntry);                

                /* Status checks TODO */

                /* Positive disposition code indicates custody was accepted */
                if (SeqCollection->DispositionCode > 0)
                {
                    /* Increment relevant counters TODO */
                }
                /* Negative disposition code indicates custody was rejected */
                else
                {
                    /* Increment relevant counters TODO */
                }
            }
            /* Odd sequence range numbers indicate sequences that are *excluded* */
            else
            {
                /* Request bundle retransmission from storage TODO */
            }
        }

        CurrSeqNum += SeqCollection->SeqRange[SeqRangeIdx];
    }

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

    /* Validate CCS here? TODO */

    for (SeqCollectIdx = 0; SeqCollectIdx < Ccs->NumBundleSeqCollections; SeqCollectIdx++)
    {
        Status = BPLib_CT_ProcessBundleSeqCollection(Inst, &(Ccs->BundleSeqCollections[SeqCollectIdx]));

        if (Status != BPLIB_SUCCESS)
        {
            /* TODO do something? */
            break;
        }
    }

    return Status;
}

uint32_t BPLib_CT_AssignSeqCounter(BPLib_Instance_t *Inst, uint32_t ContactId, uint64_t *SeqId)
{
    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if (ContactId >= BPLIB_MAX_NUM_CONTACTS)
    {
        return BPLIB_INVALID_CONT_ID_ERR;
    }

    *SeqId = Inst->Ct.SeqCounters[Inst->Ct.CurrSeqCounterIdx].SeqId + 1;

    Inst->Ct.CurrSeqCounterIdx++;

    if (Inst->Ct.CurrSeqCounterIdx >= BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS)
    {
        Inst->Ct.CurrSeqCounterIdx = 0;
    }

    Inst->Ct.SeqCounters[Inst->Ct.CurrSeqCounterIdx].Counter = 0;
    Inst->Ct.SeqCounters[Inst->Ct.CurrSeqCounterIdx].ContactId = ContactId;
    Inst->Ct.SeqCounters[Inst->Ct.CurrSeqCounterIdx].SeqId = *SeqId;
    Inst->Ct.SeqCounters[Inst->Ct.CurrSeqCounterIdx].Active = true;

    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_UnassignSeqCounter(BPLib_Instance_t *Inst, uint64_t SeqId)
{
    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if (Inst->Ct.CurrSeqCounterIdx < SeqId)
    {
        // TODO get better error
        return BPLIB_ERROR;
    }

    Inst->Ct.SeqCounters[SeqId % BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS].Active = false;

    return BPLIB_SUCCESS;
}

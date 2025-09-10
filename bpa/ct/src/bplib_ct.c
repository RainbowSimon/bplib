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
    size_t RawCcsIdx;

    /* Reset the raw CCS data */
    for (RawCcsIdx = 0; RawCcsIdx < BPLIB_CT_MAX_RAW_CCS; RawCcsIdx++)
    {
        BPLib_CT_ResetRawCcs(&(Inst->Ctdb.RawCcss[RawCcsIdx]));
    }

    /* Reset the pending custody transfer data */
    return BPLib_CT_InitCtdb(&(Inst->Ctdb.CtPending));
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
    // size_t RawCcsIdx;
    bool   DeleteBundle = false;

    if (Bundle == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    /* Set bundle ID for both custodial and non-custodial bundles */
    (void) BPLib_CT_SetBundleId(Bundle);

    /* if (Bundle->Meta.IsCustodial) */
    {
        // RawCcsIdx = BPLib_CT_GetRawCcsIdx(&(Inst->Ctdb), &(Bundle->blocks.Cteb.BlkAdminEid));
        
        /* Check if we can accept custody of this bundle */
        if (BPLib_PDB_AcceptCustody(Bundle) == BPLIB_SUCCESS)
        {
            /* Add to custody accepted raw CCS */
            // Status = BPLib_CT_AddToRawCcs(Inst->Ctdb.RawCcss[RawCcsIdx], Bundle->blocks.Cteb.SequenceNum, 
            //                     Bundle->blocks.Cteb.SequenceId, BPLib_CT_CustodyAccepted);
            if (Status != BPLIB_SUCCESS)
            {
                DeleteBundle = true;
            }
            else
            {
                /* Check size trigger of this CCS TODO */

                /* Add bundle to CTDB */
                // Status = BPLib_CT_AddToCtdb(Inst, Bundle->Cteb.SeqId, Bundle->Cteb.SeqNum, 
                //                                         Bundle->blocks.PrimaryBlock.BundeId)

            }
        }
        else
        {
            /* Add to custody rejected raw CCS and mark bundle for deletion */
            DeleteBundle = true;
            // Status =  BPLib_CT_AddToRawCcs(Inst->Ctdb.RawCcss[RawCcsIdx], Bundle->blocks.Cteb.SequenceNum, 
            //                     Bundle->blocks.Cteb.SequenceId, BPLib_CT_CustodyRefused);
            
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

BPLib_Status_t BPLib_CT_UpdateBundle(BPLib_Bundle_t *Bundle)
{
    BPLib_Status_t Status = BPLIB_SUCCESS;

    /* if (Bundle->Meta.IsCustodial) */
    {
        /* Update CTEB fields */

        /*
        Bundle->blocks.Cteb.SequenceId  = BPLib_CT_GetSequenceId(Bundle);
        Bundle->blocks.Cteb.SequenceNum = BPLib_CT_GetNextSequenceNum(Bundle->blocks.Cteb.SequenceId);
        BPLib_EID_CopyEids(&(Bundle->blocks.Cteb.BlkAdminEid), &BPLIB_EID_INSTANCE);
        Bundle->blocks.Cteb.RequiresEncode = true;

        BPLib_CT_AddToCtdb();
        */
    }

    /* Do nothing for non-custodial bundles */

    return Status;    
}

BPLib_Status_t BPLib_CT_ProcessBundleSeqCollection(BPLib_Instance_t *Inst, BPLib_CT_BundleSeqCollection_t *SeqCollection)
{
    size_t SeqRangeIdx;
    size_t CurrSeqNum;
    size_t NextSeqNum;
    BPLib_Status_t Status = BPLIB_SUCCESS;
    uint32_t BundleId;

    CurrSeqNum =  SeqCollection->FirstSeqNum;

    for (SeqRangeIdx = 0; SeqRangeIdx < SeqCollection->SeqRangeLen; SeqRangeIdx++)
    {
        for (NextSeqNum = CurrSeqNum; NextSeqNum < CurrSeqNum + SeqCollection->SeqRange[SeqRangeIdx]; NextSeqNum++)
        {
            /* Even sequence range numbers indicate sequences that are *included* */
            if (SeqRangeIdx % 2 == 0)
            {
                Status = BPLib_CT_RemoveFromCtdb(Inst, SeqCollection->SeqId, NextSeqNum, &BundleId);

                /* Request bundle deletion from storage TODO */

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


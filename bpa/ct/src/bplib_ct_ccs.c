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
#include "bplib_ct_ccs.h"
#include "bplib_ct_db.h"
#include "bplib_bblocks.h"
#include "bplib_eid.h"
#include "bplib_mem.h"
#include "bplib_pdb.h"
#include "bplib_as.h"
#include "bplib_em.h"

/*
** Function Definitions
*/

void BPLib_CT_ResetOpenCcs(BPLib_CT_OpenCcs_t *OpenCcs)
{
    uint8_t i;

    for (i = 0; i < BPLIB_CT_MAX_SEQ_COLLECTIONS; i++)
    {
        OpenCcs->BundleSeqCollections[i].SeqRangeLen = 0;
    }

    OpenCcs->InProgress = false;
    OpenCcs->Size = 0;

    return;
}

BPLib_Status_t BPLib_CT_AddToOpenCcs(BPLib_CT_OpenCcs_t *OpenCcs, uint64_t SequenceNum, 
                          uint64_t SequenceId, BPLib_CT_DispositionCode_t DispositionCode)
{
    BPLib_CT_BundleSeqCollection_t *Collection;

    if (DispositionCode == BPLib_CT_CustodyAccepted)
    {
        Collection = &(OpenCcs->BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx]);
    }
    else
    {
        Collection = &(OpenCcs->BundleSeqCollections[BPLib_CT_CustodyRefused_Idx]);
    }

    /* Sanity checks */
    if ((Collection->SeqRangeLen != 0 && Collection->SeqRangeLen % 2 != 1) ||
        Collection->SeqRangeLen >= (BPLIB_CT_MAX_SEQ_RANGE_LEN - 1) ||
        SequenceNum < Collection->LastSeqNumAdded)
    {
        return BPLIB_ERROR;
    }

    /* If OpenCcs is empty, add first sequence number */
    if (Collection->SeqRangeLen == 0)
    {
        Collection->SeqId = SequenceId;
        Collection->FirstSeqNum = SequenceNum;
        Collection->SeqRange[0] = 1;
        Collection->SeqRangeLen = 1;

        /* Update full CCS size accordingly */
        OpenCcs->Size += 1;
    }
    else
    {
        /* If we received the previous sequence number, increment the relevant sequence range value */
        if (SequenceNum == (Collection->LastSeqNumAdded + 1))
        {
            Collection->SeqRange[Collection->SeqRangeLen - 1]++;
        }
        /* If a gap in sequence numbers is detected, record missing sequence length */
        else
        {
            Collection->SeqRange[Collection->SeqRangeLen] = SequenceNum - Collection->LastSeqNumAdded - 1;
            Collection->SeqRange[Collection->SeqRangeLen + 1] = 1;
            Collection->SeqRangeLen += 2;

            /* Update full CCS size accordingly */
            OpenCcs->Size += 2;
        }
    }

    Collection->LastSeqNumAdded = SequenceNum;

    return BPLIB_SUCCESS;
}

size_t BPLib_CT_GetOpenCcsIdx(BPLib_CT_Context_t *Context, BPLib_EID_t *SourceAdminEID, uint64_t SequenceId)
{
    size_t OpenCcsIdx;
    size_t FirstUnusedCcs = BPLIB_CT_MAX_RAW_CCS;
    size_t MaxCcsSize = 0;
    size_t LargestCcsIdx = BPLIB_CT_MAX_RAW_CCS;
    size_t RetCcsIdx;

    for (OpenCcsIdx = 0; OpenCcsIdx < BPLIB_CT_MAX_RAW_CCS; OpenCcsIdx++)
    {
        /* See if there's already an in progress CCS with the right EID */
        if (Context->OpenCcss[OpenCcsIdx].InProgress == true &&
            Context->OpenCcss[OpenCcsIdx].BundleSeqCollections->SeqId == SequenceId &&
            BPLib_EID_IsMatch(&(Context->OpenCcss[OpenCcsIdx].SourceAdminEid), SourceAdminEID))
        {
            break;
        }
        /* Find the first unused CCS */
        else if (FirstUnusedCcs == BPLIB_CT_MAX_RAW_CCS && 
                 Context->OpenCcss[OpenCcsIdx].InProgress == false) 
        {
            FirstUnusedCcs = OpenCcsIdx;
        }
        /* Find the largest CCS */
        else if (MaxCcsSize < Context->OpenCcss[OpenCcsIdx].Size)
        {
            MaxCcsSize = Context->OpenCcss[OpenCcsIdx].Size;
            LargestCcsIdx = OpenCcsIdx;
        }
    }
    
    /* Found an in progress raw CCS with a matching EID */
    if (OpenCcsIdx < BPLIB_CT_MAX_RAW_CCS)
    {
        RetCcsIdx = OpenCcsIdx; 
    }
    /* Found an unused CCS */
    else if (FirstUnusedCcs != BPLIB_CT_MAX_RAW_CCS)
    {
        Context->OpenCcss[FirstUnusedCcs].InProgress = true;
        BPLib_EID_CopyEids(SourceAdminEID, Context->OpenCcss[FirstUnusedCcs].SourceAdminEid);
        
        RetCcsIdx = FirstUnusedCcs;
    }
    /* No CCSs were available, send the largest one and wipe it to use */
    else
    {
        BPLib_CT_BuildAndSendOpenCcs(&(Context->OpenCcss[LargestCcsIdx]));
        
        RetCcsIdx = LargestCcsIdx;
    }

    return RetCcsIdx;
}

void BPLib_CT_BuildAndSendOpenCcs(BPLib_CT_OpenCcs_t *OpenCcs)
{
    /* Have ARP build CCS and send it TODO */

    BPLib_CT_ResetOpenCcs(OpenCcs);

    return;
}

BPLib_Status_t BPLib_CT_ProcessBundleSeqCollection(BPLib_CT_Context_t *Context, 
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
            Status = BPLib_CT_GetEntryFromCtdb(Context, SeqCollection->SeqId, 
                                                            NextSeqNum, &DbEntry);

            if (Status != BPLIB_SUCCESS || DbEntry == NULL)
            {
                BPLib_EM_SendEvent(BPLIB_CT_INV_SEQ_NUM_ERR_EID, BPLib_EM_EventType_ERROR,
                    "Error, bundle sequence number %ld with sequence ID %ld does not exist in CTDB.",
                    SeqCollection->SeqId, NextSeqNum);
            }

            /* Even sequence range numbers indicate sequences that are *included* */
            else if (SeqRangeIdx % 2 == 0)
            {
                Status = BPLib_CT_RemoveFromCtdb(Context, DbEntry);   
                             
                /* Request bundle deletion from storage TODO */

                if (Status != BPLIB_SUCCESS)
                {
                    BPLib_EM_SendEvent(BPLIB_CT_BUNDLE_DLT_ERR_EID, BPLib_EM_EventType_ERROR,
                        "Error deleting custodial bundle sequence number %ld with sequence ID %ld.",
                        SeqCollection->SeqId, NextSeqNum);
                }

                /* Positive disposition code indicates custody was accepted */
                if (SeqCollection->DispositionCode > 0)
                {
                    BPLib_AS_Decrement(BPLIB_EID_INSTANCE, BUNDLE_COUNT_IN_CUSTODY, 1);
                    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CUSTODY_TRANSFERRED, 1);
                }
                /* Negative disposition code indicates custody was rejected */
                else
                {
                    BPLib_AS_Decrement(BPLIB_EID_INSTANCE, BUNDLE_COUNT_IN_CUSTODY, 1);
                    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CUSTODY_REJECTED, 1);
                }
            }
            /* Odd sequence range numbers indicate sequences that are *excluded* */
            else
            {
                /* Request bundle retransmission from storage TODO */
                BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CUSTODY_RE_FORWARDED, 1);
            }
        }

        CurrSeqNum += SeqCollection->SeqRange[SeqRangeIdx];
    }

    return Status;
}

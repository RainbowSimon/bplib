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
#include "bplib_arp.h"
#include "bplib_stor.h"
#include "bplib_nc.h"
#include "bplib_inst.h"

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

BPLib_Status_t BPLib_CT_AddToOpenCcs(BPLib_Instance_t* Instance, size_t OpenCcsIdx,
                                        uint32_t ContactId,
                                        BPLib_CustodyBlockData_t* CtebPtr,
                                        BPLib_CT_DispositionCode_t DispositionCode)
{
    BPLib_CT_BundleSeqCollection_t *Collection;
    BPLib_CT_SeqCollectionIdx_t     DispCodeIdx;
    BPLib_CT_OpenCcs_t*             OpenCcs;

    OpenCcs     = &(Instance->Ct.OpenCcss[OpenCcsIdx]);
    DispCodeIdx = BPLib_ARP_GetDispCodeIdx(DispositionCode);
    Collection  = &(OpenCcs->BundleSeqCollections[DispCodeIdx]);

    /* Sanity checks */
    if ((Collection->SeqRangeLen != 0 && Collection->SeqRangeLen % 2 != 1) ||
        Collection->SeqRangeLen >= (BPLIB_CT_MAX_SEQ_RANGE_LEN - 1) ||
        CtebPtr->BundleSeqNum < Collection->LastSeqNumAdded)
    {
        BPLib_EM_SendEvent(BPLIB_CT_CCS_CRRPTD_ERR_EID, BPLib_EM_EventType_ERROR,
                "Open CCS data failed sanity checks, check for memory corruption.");
        return BPLIB_CT_CUSTODY_REFUSED_ERR;
    }

    /* If OpenCcs is empty, add first sequence number */
    if (Collection->SeqRangeLen == 0)
    {
        Collection->SeqId       = CtebPtr->BundleSeqId;
        Collection->FirstSeqNum = CtebPtr->BundleSeqNum;
        Collection->SeqRange[0] = 1;
        Collection->SeqRangeLen = 1;

        /* Update full CCS size accordingly */
        OpenCcs->Size += 1;

        /* Set the collection start time for a time trigger */
        OpenCcs->CollectionStartTime = BPLib_TIME_GetMonotonicTime();

        /* Set the maximum collection size for a size trigger */
        BPLib_NC_ReaderLock();
        OpenCcs->MaxSize = BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[ContactId].CSSizeTrigger;
        BPLib_NC_ReaderUnlock();
    }
    else
    {
        /* If we received the previous sequence number, increment the relevant sequence range value */
        if (CtebPtr->BundleSeqNum == (Collection->LastSeqNumAdded + 1))
        {
            Collection->SeqRange[Collection->SeqRangeLen - 1]++;
        }
        /* If a gap in sequence numbers is detected, record missing sequence length */
        else
        {
            Collection->SeqRange[Collection->SeqRangeLen] = CtebPtr->BundleSeqNum - (Collection->LastSeqNumAdded + 1);
            Collection->SeqRange[Collection->SeqRangeLen + 1] = 1;
            Collection->SeqRangeLen += 2;

            /* Update full CCS size accordingly */
            OpenCcs->Size += 2;
        }
    }

    /* Trigger CCS generation based on size */
    if (OpenCcs->Size >= OpenCcs->MaxSize)
    {
        BPLib_CT_BuildAndSendOpenCcs(OpenCcs, &(Instance->pool));
    }

    Collection->LastSeqNumAdded = CtebPtr->BundleSeqNum;

    return BPLIB_SUCCESS;
}

size_t BPLib_CT_GetOpenCcsIdx(BPLib_CT_Context_t *Context, BPLib_MEM_Pool_t* Pool,
                                BPLib_EID_t *SourceAdminEID, uint64_t SequenceId)
{
    size_t OpenCcsIdx;
    size_t FirstUnusedCcs = BPLIB_CT_MAX_OPEN_CCS;
    size_t MaxCcsSize = 0;
    size_t LargestCcsIdx = BPLIB_CT_MAX_OPEN_CCS;
    size_t RetCcsIdx;

    for (OpenCcsIdx = 0; OpenCcsIdx < BPLIB_CT_MAX_OPEN_CCS; OpenCcsIdx++)
    {
        /* See if there's already an in progress CCS with the right EID */
        if (Context->OpenCcss[OpenCcsIdx].InProgress == true &&
            Context->OpenCcss[OpenCcsIdx].BundleSeqCollections->SeqId == SequenceId &&
            BPLib_EID_IsMatch(&(Context->OpenCcss[OpenCcsIdx].SourceAdminEid), SourceAdminEID))
        {
            break;
        }
        /* Find the first unused CCS */
        else if (FirstUnusedCcs == BPLIB_CT_MAX_OPEN_CCS &&
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
    if (OpenCcsIdx < BPLIB_CT_MAX_OPEN_CCS)
    {
        RetCcsIdx = OpenCcsIdx;
    }
    /* Found an unused CCS */
    else if (FirstUnusedCcs != BPLIB_CT_MAX_OPEN_CCS)
    {
        Context->OpenCcss[FirstUnusedCcs].InProgress = true;
        BPLib_EID_CopyEids(SourceAdminEID, Context->OpenCcss[FirstUnusedCcs].SourceAdminEid);

        RetCcsIdx = FirstUnusedCcs;
    }
    /* No CCSs were available, send the largest one and wipe it to use */
    else
    {
        BPLib_CT_BuildAndSendOpenCcs(&(Context->OpenCcss[LargestCcsIdx]), Pool);

        RetCcsIdx = LargestCcsIdx;
    }

    return RetCcsIdx;
}

void BPLib_CT_BuildAndSendOpenCcs(BPLib_CT_OpenCcs_t *OpenCcs, BPLib_MEM_Pool_t* Pool)
{
    /* Have ARP build CCS and send the open CCS */
    BPLib_ARP_ProcessInProgressCcs(OpenCcs, Pool);

    BPLib_CT_ResetOpenCcs(OpenCcs);

    return;
}

BPLib_Status_t BPLib_CT_ProcessBundleSeqCollection(BPLib_Instance_t *Inst, 
            BPLib_CT_Context_t *Context, BPLib_CT_BundleSeqCollection_t *SeqCollection)
{
    size_t SeqRangeIdx;
    size_t CurrSeqNum;
    size_t NextSeqNum;
    BPLib_Status_t Status = BPLIB_SUCCESS;
    BPLib_CT_DbEntry_t *DbEntry = NULL;
    BPLib_CT_CcsUpdateBatch_t CcsStorBatch;

    CcsStorBatch.Size = 0;
    CurrSeqNum =  SeqCollection->FirstSeqNum;

    for (SeqRangeIdx = 0; SeqRangeIdx < SeqCollection->SeqRangeLen; SeqRangeIdx++)
    {
        for (NextSeqNum = CurrSeqNum; NextSeqNum < CurrSeqNum + SeqCollection->SeqRange[SeqRangeIdx]; NextSeqNum++)
        {
            Status = BPLib_CT_GetEntryFromCtdbWithSeq(Context, SeqCollection->SeqId,
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
                /* Positive disposition code indicates custody was accepted */
                if (SeqCollection->DispositionCode > 0)
                {
                    Status = BPLib_CT_RemoveFromCtdb(Context, DbEntry);   

                    if (Status == BPLIB_SUCCESS)
                    {
                        /* Request bundle deletion from storage */
                        CcsStorBatch.BundleIDs[CcsStorBatch.Size] = DbEntry->BundleId;
                        CcsStorBatch.Ops[CcsStorBatch.Size] = BPLIB_CT_MARK_DELETE;
                        CcsStorBatch.Size++;

                        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CUSTODY_TRANSFERRED, 1);
                        BPLib_AS_Decrement(BPLIB_EID_INSTANCE, BUNDLE_COUNT_IN_CUSTODY, 1);
                    }                                
                    else
                    {
                        BPLib_EM_SendEvent(BPLIB_CT_BUNDLE_DLT_ERR_EID, BPLib_EM_EventType_ERROR,
                            "Error deleting custodial bundle sequence number %ld with sequence ID %ld. Status = %d.",
                            SeqCollection->SeqId, NextSeqNum, Status);
                    }
                }
                /* Negative disposition code indicates custody was rejected */
                else
                {
                    /* Request storage turn retransmit timer off */
                    CcsStorBatch.BundleIDs[CcsStorBatch.Size] = DbEntry->BundleId;
                    CcsStorBatch.Ops[CcsStorBatch.Size] = BPLIB_CT_STOP_RETRANSMIT;
                    CcsStorBatch.Size++;

                    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CUSTODY_REJECTED, 1);
                }
            }
            /* Odd sequence range numbers indicate sequences that are *excluded* */
            else
            {
                /* Request bundle retransmission from storage */
                CcsStorBatch.BundleIDs[CcsStorBatch.Size] = DbEntry->BundleId;
                CcsStorBatch.Ops[CcsStorBatch.Size] = BPLIB_CT_START_RETRANSMIT;
                CcsStorBatch.Size++;
            }

            /* If a batch of bundle IDs is reached, do storage operation */
            if (CcsStorBatch.Size >= BPLIB_CT_BATCH_SIZE)
            {
                Status = BPLib_STOR_UpdateCustodialBundles(Inst, &CcsStorBatch);

                /* Ignore return code, event message handled internally */

                CcsStorBatch.Size = 0;
            }
        }

        CurrSeqNum += SeqCollection->SeqRange[SeqRangeIdx];
    }

    /* Do remaining batch of storage operations */
    Status = BPLib_STOR_UpdateCustodialBundles(Inst, &CcsStorBatch);

    return Status;
}

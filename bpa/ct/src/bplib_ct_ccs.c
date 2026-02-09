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
#include "bplib_inst.h"
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
        OpenCcs->BundleSeqCollections[i].SeqId = 0;
    }

    OpenCcs->InProgress          = false;
    OpenCcs->Size                = 0;
    OpenCcs->CollectionStartTime = 0;
    OpenCcs->BundlesInCcs        = 0;

    return;
}

void BPLib_CT_InsertOldSeqNumToOpenCcs(BPLib_CT_BundleSeqCollection_t *Collection, 
                                            uint64_t BundleSeqNum, size_t *CcsSizePtr)
{
    uint64_t LastIncludeRangeNum;
    uint64_t OldSeqRange[BPLIB_CT_MAX_SEQ_RANGE_LEN];
    size_t OrigRangeIdx;
    size_t LastNewRangeIdx;
    bool InsertionComplete = false;
    size_t OldExcIdx;
    size_t NewIncIdx;
    size_t CondenseCount;

    memcpy(OldSeqRange, Collection->SeqRange, sizeof(Collection->SeqRange));

    /* If new bundle sequence number predates first sequence number entirely */
    if (BundleSeqNum < Collection->FirstSeqNum)
    {
        if (BundleSeqNum + 1 == Collection->FirstSeqNum)
        {
            Collection->SeqRange[0] = OldSeqRange[0] + 1;

            LastNewRangeIdx = 0;
        }
        else
        {
            Collection->SeqRange[0] = 1;
            Collection->SeqRange[1] = (Collection->FirstSeqNum - (BundleSeqNum + 1));
            Collection->SeqRange[2] = OldSeqRange[0];
            Collection->SeqRangeLen += 2;
            (*CcsSizePtr) += 2;

            LastNewRangeIdx = 2;
        }

        LastIncludeRangeNum = Collection->FirstSeqNum + OldSeqRange[0];
        Collection->FirstSeqNum = BundleSeqNum;

        InsertionComplete = true;
    }
    else
    {
        Collection->SeqRange[0] = OldSeqRange[0];
    
        LastIncludeRangeNum = Collection->FirstSeqNum + Collection->SeqRange[0];
        LastNewRangeIdx = 0;
    }
    
    /* 
    ** Iterate through sequence range two at a time [ExcludedRange, IncludedRange] and 
    ** insert new bundle sequence number. For example given the following BSC:
    **      FirstSeqNum: 10, SeqRange: [3, 4, 5]
    **          - Included: 10-12, 17-21
    **          - Excluded: 13-16
    ** If a bundle sequence number of 13 is received, update the BSC as follows:
    **          [3, 0, 1, 3, 5]
    ** The SeqRange will be condensed in the next step
    */
    for (OrigRangeIdx = 1; OrigRangeIdx < Collection->SeqRangeLen; OrigRangeIdx += 2)
    {
        /* Sequence number is within current excluded range */
        if (InsertionComplete == false && 
                 BundleSeqNum < (LastIncludeRangeNum + OldSeqRange[OrigRangeIdx]))
        {
            Collection->SeqRange[LastNewRangeIdx + 1] = BundleSeqNum - LastIncludeRangeNum;
            Collection->SeqRange[LastNewRangeIdx + 2] = 1;
            Collection->SeqRange[LastNewRangeIdx + 3] = (LastIncludeRangeNum + OldSeqRange[OrigRangeIdx]) - (BundleSeqNum + 1);
            Collection->SeqRange[LastNewRangeIdx + 4] = OldSeqRange[OrigRangeIdx + 1];

            InsertionComplete = true;
            LastNewRangeIdx += 4;
        }
        /* Else just copy over current exclude and include ranges */
        else
        {
            Collection->SeqRange[LastNewRangeIdx + 1] = OldSeqRange[OrigRangeIdx];
            Collection->SeqRange[LastNewRangeIdx + 2] = OldSeqRange[OrigRangeIdx + 1];

            LastNewRangeIdx += 2;
        }

        /* Jump to next "include" range */
        LastIncludeRangeNum += OldSeqRange[OrigRangeIdx];
        LastIncludeRangeNum += OldSeqRange[OrigRangeIdx + 1];
    }

    if (InsertionComplete)
    {
        Collection->SeqRangeLen += 2;
        (*CcsSizePtr) += 2;
    }

    /*
    ** The previous operation may have left some exclude ranges set to 0, we need to
    ** condense some include ranges now. For example, the following sequence range would
    ** be transformed as follows: [3, 0, 1, 0, 2, 2, 2] -> [6, 2, 2]
    */
    OldExcIdx = 1;
    NewIncIdx = 0;
    CondenseCount = 0;

    while (OldExcIdx < Collection->SeqRangeLen)
    {
        /* Exclude range is 0, need to condense array */
        if (Collection->SeqRange[OldExcIdx] == 0)
        {
            Collection->SeqRange[NewIncIdx] += Collection->SeqRange[OldExcIdx + 1];

            CondenseCount++;
        }
        /* No 0 detected, just copy values down */
        else
        {
            Collection->SeqRange[NewIncIdx + 1] = Collection->SeqRange[OldExcIdx];
            Collection->SeqRange[NewIncIdx + 2] = Collection->SeqRange[OldExcIdx + 1];

            NewIncIdx += 2;
        }

        OldExcIdx += 2;
    }

    Collection->SeqRangeLen -= (CondenseCount * 2);
    (*CcsSizePtr) -= (CondenseCount * 2);

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
        Collection->SeqRangeLen >= (BPLIB_CT_MAX_SEQ_RANGE_LEN - 1))
    {
        BPLib_EM_SendEvent(BPLIB_CT_CCS_CRRPTD_ERR_EID, BPLib_EM_EventType_ERROR,
                "Open CCS data failed sanity checks, check for memory corruption.");

        return BPLIB_CT_CUSTODY_REFUSED_ERR;
    }

    /* If this collection is empty, add first sequence number */
    if (Collection->SeqRangeLen == 0)
    {
        Collection->SeqId       = CtebPtr->BundleSeqId;
        Collection->FirstSeqNum = CtebPtr->BundleSeqNum;
        Collection->SeqRange[0] = 1;
        Collection->SeqRangeLen = 1;
        Collection->LastSeqNumAdded = CtebPtr->BundleSeqNum;
        Collection->DispositionCode = DispositionCode;

        /* Update full CCS size accordingly */
        OpenCcs->Size += 1;

        /* Set the collection start time for a time trigger */
        OpenCcs->CollectionStartTime = BPLib_TIME_GetMonotonicTime();

        /* Set the contact ID to track for contact-stop operations */
        OpenCcs->ContactId = ContactId;

        /* Set the trigger values */
        BPLib_NC_ReaderLock();
        OpenCcs->MaxSize = BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[ContactId].CSSizeTrigger;
        OpenCcs->MaxTime = BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[ContactId].CSTimeTrigger;
        BPLib_NC_ReaderUnlock();
    }
    /* Older bundle was received, gotta update older CCS records */
    else if (CtebPtr->BundleSeqNum < Collection->LastSeqNumAdded)
    {
        BPLib_CT_InsertOldSeqNumToOpenCcs(Collection, CtebPtr->BundleSeqNum, &(OpenCcs->Size));
    }
    /* Sequence number comes after last sequence number received */
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

        Collection->LastSeqNumAdded = CtebPtr->BundleSeqNum;
    }

    OpenCcs->BundlesInCcs++;

    /* Trigger CCS generation based on size */
    if (OpenCcs->Size >= OpenCcs->MaxSize || Collection->SeqRangeLen == BPLIB_CT_MAX_SEQ_RANGE_LEN)
    {
        BPLib_CT_BuildAndSendOpenCcs_Impl(Instance, OpenCcs);
    }

    return BPLIB_SUCCESS;
}

size_t BPLib_CT_GetOpenCcsIdx(BPLib_Instance_t* Instance, BPLib_EID_t *SourceAdminEID,
                                uint64_t SequenceId)
{
    size_t              OpenCcsIdx;
    size_t              FirstUnusedCcs = BPLIB_CT_MAX_OPEN_CCS;
    size_t              MaxCcsSize     = 0;
    size_t              LargestCcsIdx  = BPLIB_CT_MAX_OPEN_CCS;
    size_t              RetCcsIdx;
    BPLib_CT_Context_t* Context;

    Context = &(Instance->Ct);

    for (OpenCcsIdx = 0; OpenCcsIdx < BPLIB_CT_MAX_OPEN_CCS; OpenCcsIdx++)
    {
        /* See if there's already an in progress CCS with the right EID */
        if (Context->OpenCcss[OpenCcsIdx].InProgress == true &&
            Context->OpenCcss[OpenCcsIdx].BundleSeqCollections[0].SeqId == SequenceId &&
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
        Context->OpenCcss[FirstUnusedCcs].Size = BPLIB_MINIMUM_ENCODED_CCS_LEN;
        Context->OpenCcss[FirstUnusedCcs].BundleSeqCollections[0].SeqId = SequenceId;
        BPLib_EID_CopyEids(&(Context->OpenCcss[FirstUnusedCcs].SourceAdminEid), *SourceAdminEID);

        RetCcsIdx = FirstUnusedCcs;
    }
    /* No CCSs were available, send the largest one and wipe it to use */
    else
    {
        BPLib_CT_BuildAndSendOpenCcs_Impl(Instance, &(Context->OpenCcss[LargestCcsIdx]));
        RetCcsIdx = LargestCcsIdx;

        Context->OpenCcss[RetCcsIdx].InProgress = true;
        Context->OpenCcss[RetCcsIdx].Size = BPLIB_MINIMUM_ENCODED_CCS_LEN;
        BPLib_EID_CopyEids(&(Context->OpenCcss[RetCcsIdx].SourceAdminEid), *SourceAdminEID);
    }

    return RetCcsIdx;
}

void BPLib_CT_BuildAndSendOpenCcs_Impl(BPLib_Instance_t* Instance, BPLib_CT_OpenCcs_t* OpenCcs)
{
    /* Have ARP build CCS and send the open CCS */
    BPLib_ARP_ProcessInProgressCcs(Instance, OpenCcs);
    BPLib_CT_ResetOpenCcs(OpenCcs);

    return;
}

BPLib_Status_t BPLib_CT_ProcessBundleSeqCollection(BPLib_Instance_t *Inst,
                                                    BPLib_CT_BundleSeqCollection_t *SeqCollection)
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
            Status = BPLib_CT_GetEntryFromCtdbWithSeq(&Inst->Ct, SeqCollection->SeqId,
                                                            NextSeqNum, &DbEntry);

            if (Status != BPLIB_SUCCESS || DbEntry == NULL)
            {
                BPLib_EM_SendEvent(BPLIB_CT_INV_SEQ_NUM_ERR_EID, BPLib_EM_EventType_ERROR,
                    "Error, bundle sequence number %ld with sequence ID %ld does not exist in CTDB.",
                    NextSeqNum, SeqCollection->SeqId);
            }

            /* Even sequence range numbers indicate sequences that are *included* */
            else if (SeqRangeIdx % 2 == 0)
            {
                /* Positive disposition code indicates custody was accepted */
                if (SeqCollection->DispositionCode > 0)
                {
                    Status = BPLib_CT_RemoveFromCtdb(Inst, DbEntry);   

                    if (Status == BPLIB_SUCCESS)
                    {
                        /* Request bundle deletion from storage */
                        CcsStorBatch.BundleIDs[CcsStorBatch.Size] = DbEntry->BundleId;
                        CcsStorBatch.Ops[CcsStorBatch.Size] = BPLIB_CT_MARK_DELETE;
                        CcsStorBatch.Size++;

                        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CUSTODY_TRANSFERRED, 1);
                        Inst->Ct.BundleCountInCustody--;
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

                BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_CUSTODY_SIGNAL, 1);
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
    if (CcsStorBatch.Size > 0)
    {
        Status = BPLib_STOR_UpdateCustodialBundles(Inst, &CcsStorBatch);
    }

    return Status;
}
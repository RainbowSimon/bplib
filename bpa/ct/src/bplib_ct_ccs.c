/*
 * NASA Docket No. GSC-19,559-1, and identified as "Delay/Disruption Tolerant Networking 
 * (DTN) Bundle Protocol (BP) v7 Core Flight System (cFS) Application Build 7.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this 
 * file except in compliance with the License. You may obtain a copy of the License at 
 *
 * http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software distributed under 
 * the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF 
 * ANY KIND, either express or implied. See the License for the specific language 
 * governing permissions and limitations under the License. The copyright notice to be 
 * included in the software is as follows: 
 *
 * Copyright 2025 United States Government as represented by the Administrator of the 
 * National Aeronautics and Space Administration. All Rights Reserved.
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
    memset(OpenCcs, 0, sizeof(BPLib_CT_OpenCcs_t));

    return;
}

BPLib_Status_t BPLib_CT_InsertOldSeqNumToOpenCcs(BPLib_CT_BundleSeqCollection_t *Collection, 
                                            uint64_t BundleSeqNum, size_t *CcsSizePtr)
{
    uint64_t FirstExcludeRangeNum;
    uint64_t NewSeqRange[BPLIB_CT_MAX_SEQ_RANGE_LEN + 2];
    size_t OrigRangeIdx;
    size_t LastNewRangeIdx;
    bool InsertionComplete = false;
    size_t OldExcIdx;
    size_t NewIncIdx;
    size_t CondenseCount;
    int64_t ChangeInSeqLen = 0;
    uint64_t FirstSeqNum = Collection->FirstSeqNum;

    memset(NewSeqRange, 0, sizeof(NewSeqRange));

    LastNewRangeIdx = 0;

    /* If new bundle sequence number predates first sequence number entirely */
    if (BundleSeqNum < FirstSeqNum)
    {
        if (BundleSeqNum + 1 == FirstSeqNum)
        {
            NewSeqRange[0] = Collection->SeqRange[0] + 1;
        }
        else
        {
            NewSeqRange[0] = 1;
            NewSeqRange[1] = (FirstSeqNum - (BundleSeqNum + 1));
            NewSeqRange[2] = Collection->SeqRange[0];
            ChangeInSeqLen += 2;

            LastNewRangeIdx = 2;
        }

        
        FirstSeqNum = BundleSeqNum;

        InsertionComplete = true;
    }
    /* Nothing to do if the number is already included in this CCS */
    else if (BundleSeqNum == FirstSeqNum)
    {
        NewSeqRange[0] = Collection->SeqRange[0];
        InsertionComplete = true;
    }
    else
    {
        NewSeqRange[0] = Collection->SeqRange[0];
    }

    FirstExcludeRangeNum = FirstSeqNum + Collection->SeqRange[0];
    
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
        if (InsertionComplete == false && BundleSeqNum >= FirstExcludeRangeNum &&
                 BundleSeqNum < (FirstExcludeRangeNum + Collection->SeqRange[OrigRangeIdx]))
        {
            if (LastNewRangeIdx + 4 > (BPLIB_CT_MAX_SEQ_RANGE_LEN + 2))
            {
                return BPLIB_BUF_LEN_ERROR;
            }

            NewSeqRange[LastNewRangeIdx + 1] = BundleSeqNum - FirstExcludeRangeNum;
            NewSeqRange[LastNewRangeIdx + 2] = 1;
            NewSeqRange[LastNewRangeIdx + 3] = (FirstExcludeRangeNum + Collection->SeqRange[OrigRangeIdx]) - (BundleSeqNum + 1);
            NewSeqRange[LastNewRangeIdx + 4] = Collection->SeqRange[OrigRangeIdx + 1];

            InsertionComplete = true;
            LastNewRangeIdx += 4;
            ChangeInSeqLen += 2;
        }
        /* Else just copy over current exclude and include ranges */
        else
        {
            if (LastNewRangeIdx + 2 > (BPLIB_CT_MAX_SEQ_RANGE_LEN + 2))
            {
                return BPLIB_BUF_LEN_ERROR;
            }

            NewSeqRange[LastNewRangeIdx + 1] = Collection->SeqRange[OrigRangeIdx];
            NewSeqRange[LastNewRangeIdx + 2] = Collection->SeqRange[OrigRangeIdx + 1];

            LastNewRangeIdx += 2;
        }

        /* Jump to next "exclude" range */
        FirstExcludeRangeNum += Collection->SeqRange[OrigRangeIdx];
        FirstExcludeRangeNum += Collection->SeqRange[OrigRangeIdx + 1];
    }

    /*
    ** The previous operation may have left some exclude ranges set to 0, we need to
    ** condense some include ranges now. For example, the following sequence range would
    ** be transformed as follows: [3, 0, 1, 0, 2, 2, 2] -> [6, 2, 2]
    */
    OldExcIdx = 1;
    NewIncIdx = 0;
    CondenseCount = 0;

    while (OldExcIdx < (Collection->SeqRangeLen + ChangeInSeqLen))
    {
        /* Exclude range is 0, need to condense array */
        if (NewSeqRange[OldExcIdx] == 0)
        {
            NewSeqRange[NewIncIdx] += NewSeqRange[OldExcIdx + 1];

            CondenseCount++;
        }
        /* No 0 detected, just copy values down */
        else
        {
            NewSeqRange[NewIncIdx + 1] = NewSeqRange[OldExcIdx];
            NewSeqRange[NewIncIdx + 2] = NewSeqRange[OldExcIdx + 1];

            NewIncIdx += 2;
        }

        OldExcIdx += 2;
    }

    ChangeInSeqLen -= (CondenseCount * 2);

    if (Collection->SeqRangeLen + ChangeInSeqLen > BPLIB_CT_MAX_SEQ_RANGE_LEN)
    {
        return BPLIB_BUF_LEN_ERROR;
    }

    memcpy(Collection->SeqRange, NewSeqRange, sizeof(Collection->SeqRange));
    Collection->FirstSeqNum = FirstSeqNum;
    Collection->SeqRangeLen += ChangeInSeqLen;
    (*CcsSizePtr) += ChangeInSeqLen;

    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_AddToOpenCcs(BPLib_Instance_t* Instance, size_t OpenCcsIdx,
                                        uint32_t ContactId,
                                        BPLib_CustodyBlockData_t* CtebPtr,
                                        BPLib_CT_DispositionCode_t DispositionCode)
{
    BPLib_CT_BundleSeqCollection_t *Collection;
    BPLib_CT_SeqCollectionIdx_t     DispCodeIdx;
    BPLib_CT_OpenCcs_t*             OpenCcs;
    BPLib_Status_t                  Status = BPLIB_SUCCESS;

    OpenCcs     = &(Instance->Ct.OpenCcss[OpenCcsIdx]);
    DispCodeIdx = BPLib_ARP_GetDispCodeIdx(DispositionCode);
    Collection  = &(OpenCcs->BundleSeqCollections[DispCodeIdx]);

    /* Sanity checks */
    if ((Collection->SeqRangeLen != 0 && Collection->SeqRangeLen % 2 != 1) ||
        Collection->SeqRangeLen >= (BPLIB_CT_MAX_SEQ_RANGE_LEN - 1))
    {
        BPLib_EM_SendEvent(BPLIB_CT_CCS_CRRPTD_ERR_EID, BPLib_EM_EventType_ERROR,
                "Open CCS data failed sanity checks, check for memory corruption. Sequence range length = %ld.",
                Collection->SeqRangeLen);

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
        Status = BPLib_CT_InsertOldSeqNumToOpenCcs(Collection, CtebPtr->BundleSeqNum, &(OpenCcs->Size));

        if (Status != BPLIB_SUCCESS)
        {
            /* 
            ** Couldn't insert this bundle number for some reason, just go ahead and send
            ** the existing CCS and throw this bundle sequence number out, it will get
            ** added to the next CCS upon bundle retransmission
            */
            BPLib_EM_SendEvent(BPLIB_CT_CCS_INSERT_ERR_EID, BPLib_EM_EventType_ERROR, 
                    "Error inserting bundle sequence number %ld in CCS with sequence ID %ld starting at sequence number %ld, Status = %d",
                CtebPtr->BundleSeqNum, CtebPtr->BundleSeqId, Collection->FirstSeqNum, Status);

            BPLib_CT_BuildAndSendOpenCcs_Impl(Instance, OpenCcs);

            return Status;
        }
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
        else if (CtebPtr->BundleSeqNum > (Collection->LastSeqNumAdded + 1))
        {
            Collection->SeqRange[Collection->SeqRangeLen] = CtebPtr->BundleSeqNum - (Collection->LastSeqNumAdded + 1);
            Collection->SeqRange[Collection->SeqRangeLen + 1] = 1;
            Collection->SeqRangeLen += 2;

            /* Update full CCS size accordingly */
            OpenCcs->Size += 2;
        }
        /* else CtebPtr->BundleSeqNum == Collection->LastSeqNumAdded: do nothing */

        Collection->LastSeqNumAdded = CtebPtr->BundleSeqNum;
    }

    OpenCcs->BundlesInCcs++;

    /* Trigger CCS generation based on size */
    if (OpenCcs->Size >= OpenCcs->MaxSize || Collection->SeqRangeLen >= BPLIB_CT_MAX_SEQ_RANGE_LEN)
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
    BPLib_Status_t TempStatus = BPLIB_SUCCESS;
    BPLib_Status_t Status = BPLIB_SUCCESS;
    BPLib_CT_DbEntry_t *DbEntry = NULL;
    bool NotFoundErr = false;

    CurrSeqNum = SeqCollection->FirstSeqNum;

    /* 
    ** Normally we'd lock CT outside of this function but taking both the CT and STOR
    ** locks at the same time introduces the potential for deadlocks. Each thread should
    ** only ever have one of each lock at a time.
    */
    pthread_mutex_lock(&Inst->Ct.Lock);

    for (SeqRangeIdx = 0; SeqRangeIdx < SeqCollection->SeqRangeLen; SeqRangeIdx++)
    {
        for (NextSeqNum = CurrSeqNum; NextSeqNum < CurrSeqNum + SeqCollection->SeqRange[SeqRangeIdx]; NextSeqNum++)
        {
            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_CUSTODY_SIGNAL, 1);
            TempStatus = BPLib_CT_GetEntryFromCtdbWithSeq(&Inst->Ct, SeqCollection->SeqId,
                                                            NextSeqNum, &DbEntry);
            if (TempStatus != BPLIB_SUCCESS || DbEntry == NULL)
            {
                /* Excluded sequences can be ignored */
                if (SeqRangeIdx % 2 != 0)
                {
                    TempStatus = BPLIB_SUCCESS;
                }
                else
                {
                    NotFoundErr = true;
                    Status = TempStatus;
                }
            }

            /* Even sequence range numbers indicate sequences that are *included* */
            else if (SeqRangeIdx % 2 == 0)
            {
                /* Positive disposition code indicates custody was accepted */
                if (SeqCollection->DispositionCode > 0)
                {
                    TempStatus = BPLib_CT_RemoveFromCtdb(Inst, DbEntry);

                    if (TempStatus == BPLIB_SUCCESS)
                    {
                        /* Request bundle deletion from storage */
                        pthread_mutex_unlock(&Inst->Ct.Lock);
                        BPLib_STOR_AddToCustodialUpdateBatch(Inst, DbEntry->BundleId, BPLIB_CT_MARK_DELETE);
                        pthread_mutex_lock(&Inst->Ct.Lock);

                        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CUSTODY_TRANSFERRED, 1);
                        Inst->Ct.BundleCountInCustody--;
                    }                                
                    else
                    {
                        Status = TempStatus;
                        BPLib_EM_SendEvent(BPLIB_CT_BUNDLE_DLT_ERR_EID, BPLib_EM_EventType_ERROR,
                            "Error deleting custodial bundle sequence number %ld with sequence ID %ld. Status = %d.",
                            SeqCollection->SeqId, NextSeqNum, Status);
                    }
                }
                /* Negative disposition code indicates custody was rejected */
                else
                {
                    /* Request storage turn retransmit timer off */
                    pthread_mutex_unlock(&Inst->Ct.Lock);
                    BPLib_STOR_AddToCustodialUpdateBatch(Inst, DbEntry->BundleId, BPLIB_CT_STOP_RETRANSMIT);
                    pthread_mutex_lock(&Inst->Ct.Lock);

                    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CUSTODY_REJECTED, 1);
                }
            }
            /* Odd sequence range numbers indicate sequences that are *excluded* */
            else
            {
                /* Request bundle retransmission from storage */
                pthread_mutex_unlock(&Inst->Ct.Lock);
                BPLib_STOR_AddToCustodialUpdateBatch(Inst, DbEntry->BundleId, BPLIB_CT_START_RETRANSMIT);
                pthread_mutex_lock(&Inst->Ct.Lock);
            }
        }

        if (NotFoundErr)
        {
            BPLib_EM_SendEvent(BPLIB_CT_INV_SEQ_NUM_DBG_EID, BPLib_EM_EventType_DEBUG,
                    "Error, at least one bundle with sequence ID %ld in sequence number range [%ld-%ld] could not be found in the CTDB.",
                    SeqCollection->SeqId, CurrSeqNum, 
                    CurrSeqNum + SeqCollection->SeqRange[SeqRangeIdx] - 1);

            NotFoundErr = false;
        }
        
        /* Output on the CTEB-sending node when bundles have been rejected */
        if (SeqRangeIdx % 2 == 0 && SeqCollection->DispositionCode < 0)
        {
            BPLib_EM_SendEvent(BPLIB_CT_REJECTED_DEBG_EID, BPLib_EM_EventType_DEBUG,
                                    "Bundles with sequence ID %lu and sequence numbers [%lu-%lu] were rejected by downstream node",
                                    SeqCollection->SeqId,
                                    CurrSeqNum,
                                    (CurrSeqNum + SeqCollection->SeqRange[SeqRangeIdx]) - 1);
        }

        CurrSeqNum += SeqCollection->SeqRange[SeqRangeIdx];
    }

    pthread_mutex_unlock(&Inst->Ct.Lock);

    /* Do remaining batch of storage operations */
    BPLib_STOR_UpdateCustodialBundles(Inst);

    return Status;
}
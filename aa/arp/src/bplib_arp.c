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

/* ======== */
/* Includes */
/* ======== */

#include "bplib_arp.h"
#include "bplib_as.h"
#include "bplib_inst.h"
#include "bplib_nc.h"
#include "bplib_eid.h"

/* ==================== */
/* Function Definitions */
/* ==================== */

BPLib_Status_t BPLib_ARP_Init(void)
{
    return BPLIB_SUCCESS;
}

/* Validate CRS table data */
BPLib_Status_t BPLib_ARP_CRSTblValidateFunc(void *TblData)
{
    BPLib_Status_t        ReturnCode = BPLIB_SUCCESS;
    BPLib_ARP_CRSTable_t *TblDataPtr = (BPLib_ARP_CRSTable_t *)TblData;

    /* Validate data values are within allowed range */
    if (TblDataPtr[0].CRS_Set->SizeTrigger <= 0)
    {
        /* element is out of range, return an appropriate error code */
        ReturnCode = BPLIB_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;
}

BPLib_CT_SeqCollectionIdx_t BPLib_ARP_GetDispCodeIdx(BPLib_CT_DispositionCode_t DispositionCode)
{
    DispositionCode *= -1; /* Make negative/refusal codes go towards the end of the collection */
    DispositionCode -= (DispositionCode > 0); /* Shift refusal codes down to make codes contiguous */
    DispositionCode -= BPLib_CT_LastRefuseDispCode; /* Make 0 smallest index */

    return (BPLib_CT_SeqCollectionIdx_t) DispositionCode;
}

void BPLib_ARP_ProcessBsr(BPLib_ARP_AdminRecord_t* AdminRecord, BPLib_Bundle_t* Bundle)
{
    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD, 1);
}

void BPLib_ARP_ProcessCrs(BPLib_ARP_AdminRecord_t* AdminRecord, BPLib_Bundle_t* Bundle)
{
    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD, 1);
}

void BPLib_ARP_ProcessNewCcs(BPLib_ARP_AdminRecord_t* AdminRecord)
{
    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD, 1);
    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CCS_RECEIVED, 1);
}

void BPLib_ARP_ProcessInProgressCcs(BPLib_Instance_t* Instance, BPLib_CT_OpenCcs_t* InProgressCcs)
{
    BPLib_ARP_AdminRecord_t     CcsAdminRecord;
    BPLib_CT_DeserializedCcs_t* AdminRecordCcs;
    BPLib_Bundle_t*             Bundle;
    BPLib_Status_t              Status;
    uint8_t                     DispCodeIdx;

    /* Set the Administrative Record type */
    CcsAdminRecord.AdminRecordType = BPLib_CT_CcsRecordTypeCode;

    /* === Build the Administrative Record's CCS === */

    AdminRecordCcs = &(CcsAdminRecord.AdminRecordBody.CCS);

    /* Shove the bundle sequence collections into the CCS */
    AdminRecordCcs->NumBundleSeqCollections = 0;
    for (DispCodeIdx = 0; DispCodeIdx < BPLIB_CT_MAX_SEQ_COLLECTIONS; DispCodeIdx++)
    {
        if (InProgressCcs->BundleSeqCollections[DispCodeIdx].SeqRangeLen != 0)
        { /* Only encode bundle sequence collections with at least 1 sequence range element */
            memcpy(&(AdminRecordCcs->BundleSeqCollections[AdminRecordCcs->NumBundleSeqCollections++]),
                    &(InProgressCcs->BundleSeqCollections[DispCodeIdx]),
                    sizeof(BPLib_CT_BundleSeqCollection_t));

        #ifdef BPLIB_ARP_DEBUG_PRINTS_ENABLED
            fprintf(stderr, "Sending BSC with disp_code %d, first_seq_num %ld: [", 
                        InProgressCcs->BundleSeqCollections[DispCodeIdx].DispositionCode,
                        InProgressCcs->BundleSeqCollections[DispCodeIdx].FirstSeqNum);

            for (size_t i = 0; i < InProgressCcs->BundleSeqCollections[DispCodeIdx].SeqRangeLen; i++)
            {
                fprintf(stderr, "%ld ", InProgressCcs->BundleSeqCollections[DispCodeIdx].SeqRange[i]);
            }
            fprintf(stderr, "]\n");
        #endif
        }
    }

    /* === Generate the bundle with a CCS Administrative Record as the payload === */

    Bundle = BPLib_MEM_BundleAlloc(&(Instance->pool), &CcsAdminRecord, sizeof(BPLib_ARP_AdminRecord_t));
    if (Bundle != NULL)
    {
        Bundle->Meta.LocalBundle = true;
        Bundle->Meta.IsCustodial = false;

        /* === Set up the primary block === */
        Bundle->blocks.PrimaryBlock.RequiresEncode = true;

        /* Set primary block fields */
        Bundle->blocks.PrimaryBlock.BundleProcFlags = BPLIB_BUNDLE_PROC_ADMIN_RECORD_FLAG;
        Bundle->blocks.PrimaryBlock.CrcType = BPLIB_ADMIN_RECORD_CRC_TYPE;
        Bundle->blocks.PrimaryBlock.Lifetime = BPLIB_ADMIN_RECORD_LIFETIME;

        /* Set routing destination of bundle */
        BPLib_EID_CopyEids(&(Bundle->blocks.PrimaryBlock.DestEID),
                            InProgressCcs->SourceAdminEid);

        /* Identify source node of CCS */
        BPLib_EID_CopyEids(&(Bundle->blocks.PrimaryBlock.SrcEID),
                            BPLIB_EID_INSTANCE);

        BPLib_EID_CopyEids(&(Bundle->blocks.PrimaryBlock.ReportToEID),
                            BPLIB_EID_IPN_NONE_2D);

        /* Set timestamp for bundle creation */
        Bundle->blocks.PrimaryBlock.MonoTime.Time        = BPLib_TIME_GetMonotonicTime();
        Bundle->blocks.PrimaryBlock.MonoTime.BootEra     = BPLib_TIME_GetBootEra();
        Bundle->blocks.PrimaryBlock.Timestamp.CreateTime = BPLib_TIME_GetDtnTime(Bundle->blocks.PrimaryBlock.MonoTime);
        Bundle->blocks.PrimaryBlock.Timestamp.SequenceNumber = Instance->Arp.SequenceNum++;

        BPLib_NC_ReaderLock();
        if (Instance->Arp.SequenceNum > BPLib_NC_ConfigPtrs.MibPnConfigPtr->Configs[PARAM_SET_MAX_SEQUENCE_NUM])
        {
            Instance->Arp.SequenceNum = 0;
        }
        BPLib_NC_ReaderUnlock();

        /* Add an age block if needed - no other extension blocks allowed in admin records */
        if (Bundle->blocks.PrimaryBlock.Timestamp.CreateTime == 0)
        {
            Bundle->blocks.ExtBlocks[0].Header.BlockType = BPLib_BlockType_Age;
            Bundle->blocks.ExtBlocks[0].Header.CrcType = BPLIB_ADMIN_RECORD_CRC_TYPE;
            Bundle->blocks.ExtBlocks[0].Header.BlockNum = BPLIB_ADMIN_RECORD_AGE_BLOCK_NUM;
            Bundle->blocks.ExtBlocks[0].Header.BlockProcFlags = BPLIB_ADMIN_RECORD_BLOCK_FLAGS;          

            Bundle->blocks.ExtBlocks[0].Header.RequiresEncode = true;
        }

        /* === Set up payload === */
        Bundle->blocks.PayloadHeader.RequiresEncode = true;

        /* Configure payload block header */
        Bundle->blocks.PayloadHeader.BlockType       = BPLib_BlockType_Payload;
        Bundle->blocks.PayloadHeader.BlockNum        = 1;
        Bundle->blocks.PayloadHeader.BlockProcFlags  = BPLIB_ADMIN_RECORD_BLOCK_FLAGS;
        Bundle->blocks.PayloadHeader.CrcType         = BPLIB_ADMIN_RECORD_CRC_TYPE;

        /* Set payload size for CRC operations */
        Bundle->blocks.PayloadHeader.BlockOffsetStart = 0;
        Bundle->blocks.PayloadHeader.BlockOffsetEnd   = sizeof(BPLib_ARP_AdminRecord_t) - 1;

        /* === Put the constructed bundle on the job queue === */

        Status = BPLib_QM_CreateJob(Instance, Bundle, CHANNEL_IN_PI_TO_EBP, QM_PRI_NORMAL, QM_STANDARD_WAIT);
        if (Status != BPLIB_SUCCESS)
        { /* If something failed, cease bundle processing and free memory */
            BPLib_MEM_BundleFree(&(Instance->pool), Bundle);

            BPLib_EM_SendEvent(BPLIB_ARP_CREATE_JOB_ERR_EID,
                                BPLib_EM_EventType_ERROR,
                                "Error putting an in-progress CCS on the job queue, RC = %d",
                                Status);

            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DISCARDED, 1);
            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED, 1);
            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_GENERATED_REJECTED, 1);
        }
        else
        {
            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_GENERATED_ACCEPTED, 1);
            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_GENERATED_CCS, 1);
            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_GENERATED_CUSTODY_SIGNAL, 
                                                                InProgressCcs->BundlesInCcs);
        }
    }
    else
    { /* BPLib_MEM_BundleAlloc returned NULL */
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_GENERATED_REJECTED, 1);
        BPLib_EM_SendEvent(BPLIB_ARP_NULL_BUNDLE_ERR_EID,
                            BPLib_EM_EventType_ERROR,
                            "Could not be allocated a bundle while processing an in-progress CCS");
    }

    return;
}

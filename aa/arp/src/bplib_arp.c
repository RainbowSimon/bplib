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

#include "bplib_arp.h"
#include "bplib_as.h"
#include "bplib_inst.h"

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

BPLib_CT_DispositionCode_t BPLib_ARP_GetDispCode(BPLib_CT_SeqCollectionIdx_t DispositionCodeIdx)
{
    DispositionCodeIdx -= BPLib_CT_FirstAcceptDispCode; /* Push the disposition code to a polar maximum */
    DispositionCodeIdx += (DispositionCodeIdx > 0);     /* Shift refusal codes down so 0 is skipped */
    DispositionCodeIdx *= -1;                           /* Flip the poles so accepts are positive and refusals are negative */

    return (BPLib_CT_DispositionCode_t) DispositionCodeIdx;
}

void BPLib_ARP_ProcessBsr(BPLib_ARP_AdminRecord_t* AdminRecord, BPLib_Bundle_t* Bundle)
{
    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD, 1);
}

void BPLib_ARP_ProcessCrs(BPLib_ARP_AdminRecord_t* AdminRecord, BPLib_Bundle_t* Bundle)
{
    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD, 1);
}

void BPLib_ARP_ProcessNewCcs(BPLib_ARP_AdminRecord_t* AdminRecord, BPLib_Bundle_t* Bundle)
{
    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD, 1);
    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_CUSTODY_SIGNAL, 1);

    /* As of right now, administrative records are 264 bytes; < 512 bytes for user_data */
    memcpy((void*) &(Bundle->blob->user_data), AdminRecord, sizeof(BPLib_ARP_AdminRecord_t));
}

void BPLib_ARP_ProcessInProgressCcs(BPLib_Instance_t* Instance, BPLib_CT_OpenCcs_t* InProgressCcs)
{
    BPLib_ARP_AdminRecord_t    CcsAdminRecord;
    uint8_t                    ExtBlockIdx;
    BPLib_Bundle_t*            Bundle;
    BPLib_Status_t             Status;
    uint8_t                    DispCodeIdx;
    BPLib_CT_DispositionCode_t DispCode;

    /* === Build the Administrative Record's CCS === */

    /* Set the Administrative Record type */
    CcsAdminRecord.AdminRecordType = BPLib_CT_CcsRecordTypeCode;

    /* Copy over source administrative EID */
    BPLib_EID_CopyEids(&(CcsAdminRecord.AdminRecordBody.CCS.SourceAdminEid),
                        InProgressCcs->SourceAdminEid);

    /* Shove the bundle sequence collections into the CCS */
    memcpy(CcsAdminRecord.AdminRecordBody.CCS.BundleSeqCollections,
            InProgressCcs->BundleSeqCollections,
            sizeof(BPLib_CT_BundleSeqCollection_t) * BPLIB_CT_MAX_SEQ_COLLECTIONS);

    // TODO: CcsAdminRecord->AdminRecordBody.CCS.NumBundleSeqCollections

    /* === Generate the bundle with a CCS Administrative Record as the payload === */

    Bundle = BPLib_MEM_BundleAlloc(&(Instance->pool), &CcsAdminRecord, sizeof(BPLib_ARP_AdminRecord_t));
    if (Bundle != NULL)
    {
        /* === Configure the bundle for egressing === */

        /* Mark every block as requiring encoding */
        Bundle->blocks.PrimaryBlock.RequiresEncode  = true;
        Bundle->blocks.PayloadHeader.RequiresEncode = true;

        for(ExtBlockIdx = 0; ExtBlockIdx < BPLIB_MAX_NUM_EXTENSION_BLOCKS; ExtBlockIdx++)
        {
            Bundle->blocks.ExtBlocks[ExtBlockIdx].Header.RequiresEncode = true;
        }

        /* Set the administrative record flag */
        Bundle->blocks.PrimaryBlock.BundleProcFlags |= BPLIB_BUNDLE_PROC_ADMIN_RECORD_FLAG;

        /* Set routing destination of bundle */
        BPLib_EID_CopyEids(&(Bundle->blocks.PrimaryBlock.DestEID),
                            InProgressCcs->SourceAdminEid);

        /* === Put the constructed bundle on the job queue === */

        Status = BPLib_QM_CreateJob(Instance, Bundle, CONTACT_IN_CT_TO_STOR, QM_PRI_NORMAL, QM_WAIT_FOREVER);
        if (Status != BPLIB_SUCCESS)
        { /* If something failed, cease bundle processing and free memory */
            BPLib_MEM_BundleFree(&(Instance->pool), Bundle);

            BPLib_EM_SendEvent(BPLIB_ARP_CREATE_JOB_ERR,
                                BPLib_EM_EventType_INFORMATION,
                                "Error putting an in-progress CCS on the job queue, RC = %d",
                                Status);

            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DISCARDED, 1);
            BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED, 1);
        }
    }
    else
    { /* BPLib_MEM_BundleAlloc returned NULL */
        BPLib_EM_SendEvent(BPLIB_ARP_NULL_BUNDLE_ERR,
                            BPLib_EM_EventType_ERROR,
                            "Could not be allocated a bundle while processing an in-progress CCS");
    }

    return;
}
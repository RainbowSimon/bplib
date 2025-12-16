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

void BPLib_ARP_ProcessCcs(BPLib_ARP_AdminRecord_t* AdminRecord)
{
    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD, 1);
    BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_CCS_RECEIVED, 1);
}

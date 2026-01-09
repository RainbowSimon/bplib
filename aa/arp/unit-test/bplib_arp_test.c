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

#include "bplib_arp_test_utils.h"
#include "bplib_arp_handlers.h"
#include "bplib_inst.h"
#include "bplib_eid.h"

/* ==================== */
/* Function Definitions */
/* ==================== */

void Test_BPLib_ARP_Init(void)
{
    UtAssert_INT32_EQ(BPLib_ARP_Init(), BPLIB_SUCCESS);
}

void Test_BPLib_ARP_CRSTblValidateFunc_Nominal(void)
{
    BPLib_ARP_CRSTable_t TestTblData;
    memset(&TestTblData, 0, sizeof(TestTblData));
    TestTblData.CRS_Set[0].SizeTrigger = 10;
    UtAssert_INT32_EQ((int32) BPLib_ARP_CRSTblValidateFunc(&TestTblData), (int32) BPLIB_SUCCESS);
}

void Test_BPLib_ARP_CRSTblValidateFunc_Invalid(void)
{
    BPLib_ARP_CRSTable_t TestTblData;
    memset(&TestTblData, 0, sizeof(TestTblData));

    /* Error case should return BPLIB_TABLE_OUT_OF_RANGE_ERR_CODE */
    TestTblData.CRS_Set[0].SizeTrigger = 0;

    UtAssert_INT32_EQ(BPLib_ARP_CRSTblValidateFunc(&TestTblData),
                                                BPLIB_TABLE_OUT_OF_RANGE_ERR_CODE);
}

void Test_BPLib_ARP_GetDispCodeIdx_Pos(void)
{
    UtAssert_EQ(BPLib_CT_DispositionCode_t, BPLib_ARP_GetDispCodeIdx(BPLib_CT_CustodyAccepted), BPLib_CT_CustodyAccepted_Idx);
}

void Test_BPLib_ARP_GetDispCodeIdx_Neg(void)
{
    UtAssert_EQ(BPLib_CT_DispositionCode_t, BPLib_ARP_GetDispCodeIdx(BPLib_CT_CustodyRefused), BPLib_CT_CustodyRefused_Idx);
}

void Test_BPLib_ARP_ProcessBsr_Nominal(void)
{
    BPLib_Bundle_t    Bundle;
    BPLib_MEM_Block_t Block;

    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    memset(&Block, 0, sizeof(BPLib_MEM_Block_t));
    Bundle.blob = &Block;

    BPLib_ARP_ProcessBsr(&AdminRecord, &Bundle);

    BPLib_ARP_Test_VerifyIncrement(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD, 1, 1);
}

void Test_BPLib_ARP_ProcessCrs_Nominal(void)
{
    BPLib_Bundle_t    Bundle;
    BPLib_MEM_Block_t Block;

    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    memset(&Block, 0, sizeof(BPLib_MEM_Block_t));
    Bundle.blob = &Block;

    BPLib_ARP_ProcessBsr(&AdminRecord, &Bundle);

    BPLib_ARP_Test_VerifyIncrement(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD, 1, 1);
}

void Test_BPLib_ARP_ProcessNewCcs_Nominal(void)
{
    BPLib_Bundle_t          Bundle;
    BPLib_MEM_Block_t       Block;
    BPLib_ARP_AdminRecord_t BlobAsAdminRecord;
    uint8_t                 BundleSeqIdx;
    uint8_t                 SeqRangeIdx;

    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    memset(&Block, 0, sizeof(BPLib_MEM_Block_t));
    Bundle.blob = &Block;

    BPLib_ARP_ProcessNewCcs(&AdminRecord, &Bundle);

    BPLib_ARP_Test_VerifyIncrement(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD,   1, 1);
    BPLib_ARP_Test_VerifyIncrement(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_CUSTODY_SIGNAL, 1, 2);

    UtAssert_STUB_COUNT(BPLib_AS_Increment, 2);

    memcpy((void*) &BlobAsAdminRecord, (void*) Bundle.blob->user_data.BigData, sizeof(BPLib_ARP_AdminRecord_t));

    UtAssert_EQ(BPLib_ARP_AdminRecordTypeCode_t,
                BlobAsAdminRecord.AdminRecordType,
                AdminRecord.AdminRecordType);

    for (BundleSeqIdx = 0; BundleSeqIdx < BPLIB_CT_MAX_SEQ_COLLECTIONS; BundleSeqIdx++)
    {
        UtAssert_EQ(uint64_t,
                    BlobAsAdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].SeqId,
                    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].SeqId);

        UtAssert_EQ(uint64_t,
                    BlobAsAdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].FirstSeqNum,
                    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].FirstSeqNum);

        for (SeqRangeIdx = 0; SeqRangeIdx < BPLIB_CT_MAX_SEQ_RANGE_LEN; SeqRangeIdx++)
        {
            UtAssert_EQ(uint64_t,
                        BlobAsAdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].SeqRange[SeqRangeIdx],
                        AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].SeqRange[SeqRangeIdx]);
        }

        UtAssert_EQ(size_t,
                    BlobAsAdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].SeqRangeLen,
                    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].SeqRangeLen);

        UtAssert_EQ(size_t,
                    BlobAsAdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].LastSeqNumAdded,
                    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].LastSeqNumAdded);

        UtAssert_EQ(BPLib_CT_DispositionCode_t,
                    BlobAsAdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].DispositionCode,
                    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BundleSeqIdx].DispositionCode);
    }

    UtAssert_EQ(size_t,
                BlobAsAdminRecord.AdminRecordBody.CCS.NumBundleSeqCollections,
                AdminRecord.AdminRecordBody.CCS.NumBundleSeqCollections);
}

void Test_BPLib_ARP_ProcessInProgressCcs_Nominal(void)
{
    BPLib_Instance_t        Instance;
    BPLib_CT_OpenCcs_t      InProgressCcs;
    uint8_t                 SequenceCollection;
    uint64_t                AcceptedSeqRange[3] = {1, 3, 5};
    uint64_t                RefusedSeqRange[3]  = {2, 4, 6};
    BPLib_ARP_AdminRecord_t BundleAdminRecord;
    BPLib_Bundle_t          AllocBundle;
    BPLib_MEM_Block_t       BundleBlob;

    /* Set up for test */
    AllocBundle.blob = &BundleBlob;
    UT_SetHandlerFunction(UT_KEY(BPLib_MEM_BundleAlloc), UT_Handler_BPLib_MEM_BundleAlloc, &AllocBundle);
    UT_SetHandlerFunction(UT_KEY(BPLib_EID_CopyEids), UT_Handler_BPLib_EID_CopyEids, NULL);
    memset(&Instance, 0, sizeof(BPLib_Instance_t));

    /* Initialize in-process CCS's source admin EID to evaluate later */

    InProgressCcs.SourceAdminEid.Scheme       = 2;
    InProgressCcs.SourceAdminEid.IpnSspFormat = 2;
    InProgressCcs.SourceAdminEid.Allocator    = 3;
    InProgressCcs.SourceAdminEid.Node         = 100;
    InProgressCcs.SourceAdminEid.Service      = 4;

    /* Initialize the custody accepted bundle sequence collection to evaluate later */

    InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqId           = 42;
    InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].FirstSeqNum     = 1;

    memcpy(InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange,
            AcceptedSeqRange,
            sizeof(uint64_t) * BPLIB_CT_MAX_SEQ_RANGE_LEN);

    InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRangeLen     = 3;
    InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].LastSeqNumAdded = 5;
    InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].DispositionCode = BPLib_CT_CustodyAccepted;

    /* Initialize the custody refused bundle sequence collection to evaluate later */

    InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqId           = 24;
    InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].FirstSeqNum     = 2;

    memcpy(InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRange,
            RefusedSeqRange,
            sizeof(uint64_t) * BPLIB_CT_MAX_SEQ_RANGE_LEN);

    InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRangeLen     = 3;
    InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].LastSeqNumAdded = 6;
    InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].DispositionCode = BPLib_CT_CustodyRefused;

    BPLib_ARP_ProcessInProgressCcs(&Instance, &InProgressCcs);

    /* Evaluate constructed bundle */
    UtAssert_BOOL_TRUE(AllocBundle.blocks.PrimaryBlock.RequiresEncode);
    UtAssert_BOOL_TRUE(AllocBundle.blocks.PayloadHeader.RequiresEncode);

    UtAssert_EQ(uint64_t, AllocBundle.blocks.PrimaryBlock.BundleProcFlags, BPLIB_BUNDLE_PROC_ADMIN_RECORD_FLAG);
    BPLib_ARP_Test_EIDs_EQ(&(AllocBundle.blocks.PrimaryBlock.DestEID), &(InProgressCcs.SourceAdminEid));

    /* Evaluate constructed bundle payload */
    memcpy(&BundleAdminRecord, AllocBundle.blob->user_data.BigData, sizeof(BPLib_ARP_AdminRecord_t));

    UtAssert_EQ(BPLib_ARP_AdminRecordTypeCode_t, BundleAdminRecord.AdminRecordType, BPLib_CT_CcsRecordTypeCode);

    UtAssert_EQ(uint64_t,
                BundleAdminRecord.AdminRecordBody.CCS.NumBundleSeqCollections,
                (InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRangeLen > 0) +
                    (InProgressCcs.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRangeLen > 0));

    for (SequenceCollection = 0; SequenceCollection < BundleAdminRecord.AdminRecordBody.CCS.NumBundleSeqCollections; SequenceCollection++)
    {
        BPLIB_ARP_Test_BundleSequenceCollections_EQ(&(BundleAdminRecord.AdminRecordBody.CCS.BundleSeqCollections[SequenceCollection]),
                                                    &(InProgressCcs.BundleSeqCollections[SequenceCollection]));
    }

    /* Show that no error event was issued */
    UtAssert_STUB_COUNT(BPLib_EM_SendEvent, 0);
}

void Test_BPLib_ARP_ProcessInProgressCcs_NullBundleErr(void)
{
    // TODO
}

void Test_BPLib_ARP_ProcessInProgressCcs_CreateJobErr(void)
{
    // TODO
}

void TestBplibArp_Register(void)
{
    ADD_TEST(Test_BPLib_ARP_Init);
    ADD_TEST(Test_BPLib_ARP_CRSTblValidateFunc_Nominal);
    ADD_TEST(Test_BPLib_ARP_CRSTblValidateFunc_Invalid);
    ADD_TEST(Test_BPLib_ARP_GetDispCodeIdx_Pos);
    ADD_TEST(Test_BPLib_ARP_GetDispCodeIdx_Neg);
    ADD_TEST(Test_BPLib_ARP_ProcessBsr_Nominal);
    ADD_TEST(Test_BPLib_ARP_ProcessCrs_Nominal);
    ADD_TEST(Test_BPLib_ARP_ProcessNewCcs_Nominal);
    ADD_TEST(Test_BPLib_ARP_ProcessInProgressCcs_Nominal);
    ADD_TEST(Test_BPLib_ARP_ProcessInProgressCcs_NullBundleErr);
    ADD_TEST(Test_BPLib_ARP_ProcessInProgressCcs_CreateJobErr);
}

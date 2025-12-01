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
#include "bplib_as_handlers.h"
#include "bplib_mem.h"

/* ======= */
/* Globals */
/* ======= */

BPLib_ARP_AdminRecord_t AdminRecord;

/* ==================== */
/* Function Definitions */
/* ==================== */

void BPLib_ARP_Test_VerifyIncrement(BPLib_EID_t EID, BPLib_AS_Counter_t Counter, uint32_t Amount, int16_t CallNum)
{
    UtAssert_EQ(uint64_t, EID.Scheme,       Context_BPLib_AS_Increment[CallNum - 1].EID.Scheme);
    UtAssert_EQ(uint64_t, EID.IpnSspFormat, Context_BPLib_AS_Increment[CallNum - 1].EID.IpnSspFormat);
    UtAssert_EQ(uint64_t, EID.Allocator,    Context_BPLib_AS_Increment[CallNum - 1].EID.Allocator);
    UtAssert_EQ(uint64_t, EID.Node,         Context_BPLib_AS_Increment[CallNum - 1].EID.Node);
    UtAssert_EQ(uint64_t, EID.Service,      Context_BPLib_AS_Increment[CallNum - 1].EID.Service);

    if (Counter != -1)
    {
        UtAssert_EQ(BPLib_AS_Counter_t, Counter, Context_BPLib_AS_Increment[CallNum - 1].Counter);
    }

    if (Amount != 0xFFFFFFFF)
    {
        UtAssert_EQ(uint32_t, Amount, Context_BPLib_AS_Increment[CallNum - 1].Amount);
    }
}

void BPLib_ARP_Test_EIDs_EQ(BPLib_EID_t* Actual, BPLib_EID_t* Reference)
{
    UtAssert_EQ(uint64_t , Actual->Scheme, Reference->Scheme);
    UtAssert_EQ(uint64_t , Actual->IpnSspFormat, Reference->IpnSspFormat);
    UtAssert_EQ(uint64_t , Actual->Allocator, Reference->Allocator);
    UtAssert_EQ(uint64_t , Actual->Node, Reference->Node);
    UtAssert_EQ(uint64_t , Actual->Service, Reference->Service);
}

void BPLIB_ARP_Test_BundleSequenceCollections_EQ(BPLib_CT_BundleSeqCollection_t* Actual, BPLib_CT_BundleSeqCollection_t* Reference)
{
    uint8_t SeqRangeEntry;

    /* Assess accepted custody values */
    UtAssert_EQ(uint64_t, Actual[BPLib_CT_CustodyAccepted_Idx].SeqId, Reference[BPLib_CT_CustodyAccepted_Idx].SeqId);
    UtAssert_EQ(uint64_t, Actual[BPLib_CT_CustodyAccepted_Idx].FirstSeqNum, Reference[BPLib_CT_CustodyAccepted_Idx].FirstSeqNum);
    UtAssert_EQ(size_t, Actual[BPLib_CT_CustodyAccepted_Idx].SeqRangeLen, Reference[BPLib_CT_CustodyAccepted_Idx].SeqRangeLen);

    for (SeqRangeEntry = 0; SeqRangeEntry < Reference[BPLib_CT_CustodyAccepted_Idx].SeqRangeLen; SeqRangeEntry++)
    {
        UtAssert_EQ(uint64_t,
                    Actual[BPLib_CT_CustodyAccepted_Idx].SeqRange[SeqRangeEntry],
                    Reference[BPLib_CT_CustodyAccepted_Idx].SeqRange[SeqRangeEntry]);
    }

    UtAssert_EQ(size_t, Actual[BPLib_CT_CustodyAccepted_Idx].LastSeqNumAdded, Reference[BPLib_CT_CustodyAccepted_Idx].LastSeqNumAdded);
    UtAssert_EQ(BPLib_CT_DispositionCode_t, Actual[BPLib_CT_CustodyAccepted_Idx].DispositionCode, Reference[BPLib_CT_CustodyAccepted_Idx].DispositionCode);

    /* Assess refused custody values */
    UtAssert_EQ(uint64_t, Actual[BPLib_CT_CustodyRefused_Idx].SeqId, Reference[BPLib_CT_CustodyRefused_Idx].SeqId);
    UtAssert_EQ(uint64_t, Actual[BPLib_CT_CustodyRefused_Idx].FirstSeqNum, Reference[BPLib_CT_CustodyRefused_Idx].FirstSeqNum);
    UtAssert_EQ(size_t, Actual[BPLib_CT_CustodyRefused_Idx].SeqRangeLen, Reference[BPLib_CT_CustodyRefused_Idx].SeqRangeLen);

    for (SeqRangeEntry = 0; SeqRangeEntry < Reference[BPLib_CT_CustodyRefused_Idx].SeqRangeLen; SeqRangeEntry++)
    {
        UtAssert_EQ(uint64_t,
                    Actual[BPLib_CT_CustodyRefused_Idx].SeqRange[SeqRangeEntry],
                    Reference[BPLib_CT_CustodyRefused_Idx].SeqRange[SeqRangeEntry]);
    }

    UtAssert_EQ(size_t, Actual[BPLib_CT_CustodyRefused_Idx].LastSeqNumAdded, Reference[BPLib_CT_CustodyRefused_Idx].LastSeqNumAdded);
    UtAssert_EQ(BPLib_CT_DispositionCode_t, Actual[BPLib_CT_CustodyRefused_Idx].DispositionCode, Reference[BPLib_CT_CustodyRefused_Idx].DispositionCode);
}

void BPLib_ARP_Test_Setup(void)
{
    /* Initialize test environment to default state for every test */
    UT_ResetState(0);

    /* Initialize the CCS Administrative Record */
    AdminRecord.AdminRecordType = BPLib_CT_CcsRecordTypeCode;

    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqId           = 1;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].FirstSeqNum     = 12;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[0]     = 2;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[1]     = 3;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[2]     = 4;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[3]     = 5;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRangeLen     = 4;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].LastSeqNumAdded = 5;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].DispositionCode = BPLib_CT_CustodyAccepted;

    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqId           = 1;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].FirstSeqNum     = 12;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRange[0]     = 2;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRange[1]     = 3;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRangeLen     = 2;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].LastSeqNumAdded = 3;
    AdminRecord.AdminRecordBody.CCS.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].DispositionCode = BPLib_CT_CustodyRefused;

    UT_SetHandlerFunction(UT_KEY(BPLib_AS_Increment), UT_Handler_BPLib_AS_Increment, NULL);
}

void BPLib_ARP_Test_Teardown(void)
{
    /* Clean up test environment */
}

void UtTest_Setup(void)
{
    TestBplibArp_Register();
}

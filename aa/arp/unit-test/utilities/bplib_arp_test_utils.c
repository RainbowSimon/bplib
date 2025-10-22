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

void BPLib_ARP_Test_Setup(void)
{
    /* Initialize test environment to default state for every test */
    UT_ResetState(0);

    AdminRecord.AdminRecordType = BPLib_CT_CcsRecordTypeCode;

    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqId           = 1;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].FirstSeqNum     = 12;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[0]     = 2;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[1]     = 3;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[2]     = 4;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[3]     = 5;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRangeLen     = 4;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].LastSeqNumAdded = 5;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].DispositionCode = BPLib_CT_CustodyAccepted;

    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqId           = 1;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].FirstSeqNum     = 12;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRange[0]     = 2;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRange[1]     = 3;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRangeLen     = 2;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].LastSeqNumAdded = 3;
    AdminRecord.BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].DispositionCode = BPLib_CT_CustodyRefused;

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

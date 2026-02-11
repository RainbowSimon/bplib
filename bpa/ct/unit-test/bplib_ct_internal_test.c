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
 * Include
 */

#include "bplib_ct_test_utils.h"
#include "bplib_arp.h"
#include "bplib_nc.h"

void Test_BPLib_CT_InsertOldSeqNum_BeforeFirstGap(void)
{
    BPLib_CT_BundleSeqCollection_t Collection;
    uint64_t BundleSeqNum;
    size_t CcsSize;

    Collection.LastSeqNumAdded = 32;
    Collection.SeqRangeLen = 3;
    Collection.SeqRange[0] = 2;
    Collection.SeqRange[1] = 3;
    Collection.SeqRange[2] = 2;
    Collection.FirstSeqNum = 26;

    BundleSeqNum = 24;
    CcsSize = 3;

    /*
    ** CCS data explanation: [2, 3, 2]
    ** Sequence numbers 26-27 were received
    ** Sequence numbers 28-30 were not
    ** Sequence numbers 31-32 were received
    ** New received sequence number is 24, so the new CCS should be:
    **      [1, 1, 2, 3, 2]
    */

    UtAssert_VOIDCALL(BPLib_CT_InsertOldSeqNumToOpenCcs(&Collection, BundleSeqNum, &CcsSize));
    UtAssert_EQ(uint64_t, Collection.FirstSeqNum, BundleSeqNum);
    UtAssert_EQ(uint64_t, Collection.SeqRangeLen, 5);
    UtAssert_EQ(uint64_t, Collection.SeqRange[0], 1);
    UtAssert_EQ(uint64_t, Collection.SeqRange[1], 1);
    UtAssert_EQ(uint64_t, Collection.SeqRange[2], 2);
    UtAssert_EQ(uint64_t, Collection.SeqRange[3], 3);
    UtAssert_EQ(uint64_t, Collection.SeqRange[4], 2);
    UtAssert_EQ(uint64_t, CcsSize, 5);
}

void Test_BPLib_CT_InsertOldSeqNum_BeforeFirst(void)
{
    BPLib_CT_BundleSeqCollection_t Collection;
    uint64_t BundleSeqNum;
    size_t CcsSize;

    Collection.LastSeqNumAdded = 32;
    Collection.SeqRangeLen = 3;
    Collection.SeqRange[0] = 2;
    Collection.SeqRange[1] = 3;
    Collection.SeqRange[2] = 2;
    Collection.FirstSeqNum = 26;

    BundleSeqNum = 25;
    CcsSize = 3;

    /*
    ** CCS data explanation: [2, 3, 2]
    ** Sequence numbers 26-27 were received
    ** Sequence numbers 28-30 were not
    ** Sequence numbers 31-32 were received
    ** New received sequence number is 25, so the new CCS should be:
    **      [3, 3, 2]
    */

    UtAssert_VOIDCALL(BPLib_CT_InsertOldSeqNumToOpenCcs(&Collection, BundleSeqNum, &CcsSize));
    UtAssert_EQ(uint64_t, Collection.FirstSeqNum, BundleSeqNum);
    UtAssert_EQ(uint64_t, Collection.SeqRangeLen, 3);
    UtAssert_EQ(uint64_t, Collection.SeqRange[0], 3);
    UtAssert_EQ(uint64_t, Collection.SeqRange[1], 3);
    UtAssert_EQ(uint64_t, Collection.SeqRange[2], 2);
    UtAssert_EQ(uint64_t, CcsSize, 3);
}

void Test_BPLib_CT_InsertOldSeqNum_EdgeOfGap(void)
{
    BPLib_CT_BundleSeqCollection_t Collection;
    uint64_t BundleSeqNum;
    size_t CcsSize;

    Collection.LastSeqNumAdded = 32;
    Collection.SeqRangeLen = 3;
    Collection.SeqRange[0] = 2;
    Collection.SeqRange[1] = 3;
    Collection.SeqRange[2] = 2;
    Collection.FirstSeqNum = 26;

    BundleSeqNum = 30;
    CcsSize = 3;

    /*
    ** CCS data explanation: [2, 3, 2]
    ** Sequence numbers 26-27 were received
    ** Sequence numbers 28-30 were not
    ** Sequence numbers 31-32 were received
    ** New received sequence number is 30, so the new CCS should be:
    **      [2, 2, 3]
    */

    UtAssert_VOIDCALL(BPLib_CT_InsertOldSeqNumToOpenCcs(&Collection, BundleSeqNum, &CcsSize));
    UtAssert_EQ(uint64_t, Collection.FirstSeqNum, 26);
    UtAssert_EQ(uint64_t, Collection.SeqRangeLen, 3);
    UtAssert_EQ(uint64_t, Collection.SeqRange[0], 2);
    UtAssert_EQ(uint64_t, Collection.SeqRange[1], 2);
    UtAssert_EQ(uint64_t, Collection.SeqRange[2], 3);
    UtAssert_EQ(uint64_t, CcsSize, 3);
}

void TestBplibCtInternal_Register(void)
{
    ADD_TEST(Test_BPLib_CT_InsertOldSeqNum_BeforeFirstGap);
    ADD_TEST(Test_BPLib_CT_InsertOldSeqNum_BeforeFirst);
    ADD_TEST(Test_BPLib_CT_InsertOldSeqNum_EdgeOfGap);
    
}
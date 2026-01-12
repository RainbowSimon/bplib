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
 * Include
 */

#include "bplib_ct_test_utils.h"
#include "bplib_arp.h"
#include "bplib_nc.h"

/* 
 * Global Data
 */

extern BPLib_Instance_t BplibInst;

/* Test null case for BPLib_CT_Init */
void Test_BPLib_CT_Init_Null(void)
{
    UtAssert_INT32_EQ(BPLib_CT_Init(NULL), BPLIB_NULL_PTR_ERROR);
}


/* Test nominal case for BPLib_CT_Init */
void Test_BPLib_CT_Init_Nominal(void)
{
    UtAssert_INT32_EQ(BPLib_CT_Init(&BplibInst), BPLIB_SUCCESS);
}

/* Test nominal case for BPLib_CT_SetBundleId */
void Test_BPLib_CT_SetBundleId_Nominal(void)
{
    BPLib_Bundle_t Bundle;

    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    UT_SetDefaultReturnValue(UT_KEY(BPLib_CRC_Calculate), 0xdeadbeef);

    UtAssert_INT32_EQ(BPLib_CT_SetBundleId(&Bundle), BPLIB_SUCCESS);

    UtAssert_STUB_COUNT(BPLib_CRC_Calculate, 1);
    UtAssert_UINT32_EQ(Bundle.blocks.PrimaryBlock.BundleId, 0xdeadbeef);
}

/* Test null case for BPLib_CT_SetBundleId */
void Test_BPLib_CT_SetBundleId_Null(void)
{
    UtAssert_INT32_EQ(BPLib_CT_SetBundleId(NULL), BPLIB_NULL_PTR_ERROR);

    UtAssert_STUB_COUNT(BPLib_CRC_Calculate, 0);
}

void Test_BPLib_CT_ProcessNewBundle_Null(void)
{
    BPLib_Bundle_t Bundle;

    UtAssert_INT32_EQ(BPLib_CT_ProcessNewBundle(NULL, &Bundle), BPLIB_NULL_PTR_ERROR);
    UtAssert_INT32_EQ(BPLib_CT_ProcessNewBundle(&BplibInst, NULL), BPLIB_NULL_PTR_ERROR);
}

void Test_BPLib_CT_ProcessNewBundle_AcceptCustody(void)
{
    BPLib_Bundle_t Bundle;

    /* Set up custodial bundle */
    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    Bundle.blocks.ExtBlocks[0].Header.BlockType = BPLib_BlockType_CTEB;
    Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqId = 12;
    Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqNum = 33;

    /* Set up an in progress CCS */
    BplibInst.Ct.OpenCcss[0].InProgress = true;
    BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqId = 12;
    BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].LastSeqNumAdded = 31;
    BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRangeLen = 3;
    BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[0] = 2;
    BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[1] = 2;
    BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[2] = 2;
    BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].FirstSeqNum = 28;
    BplibInst.Ct.OpenCcss[0].MaxSize = 100;

    /*
    ** CCS data explanation:
    ** Sequence numbers 26-27 were received
    ** Sequence numbers 28-29 were not
    ** Sequence numbers 30-31 were received
    ** New received sequence number is 33, so the new CCS should be:
    **      [2, 2, 2, 1, 1]
    ** to indicate the missing sequence number of 32
    */

    UT_SetDefaultReturnValue(UT_KEY(BPLib_EID_IsMatch), true);
    UT_SetDefaultReturnValue(UT_KEY(BPLib_PDB_AcceptCustody), BPLIB_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(BPLib_ARP_GetDispCodeIdx), BPLib_CT_CustodyAccepted_Idx);

    UtAssert_INT32_EQ(BPLib_CT_ProcessNewBundle(&BplibInst, &Bundle), BPLIB_SUCCESS);
    UtAssert_EQ(size_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRangeLen, 5);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqId, 12);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[0], 2);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[1], 2);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[2], 2);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[3], 1);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].SeqRange[4], 1);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].FirstSeqNum, 28);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx].LastSeqNumAdded, 33);
    UtAssert_EQ(bool, BplibInst.Ct.OpenCcss[0].InProgress, true);
}

void Test_BPLib_CT_ProcessNewBundle_RejectCustody(void)
{
    BPLib_Bundle_t Bundle;

    /* Set up custodial bundle */
    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    Bundle.blocks.ExtBlocks[BPLIB_MAX_NUM_EXTENSION_BLOCKS - 1].Header.BlockType = BPLib_BlockType_CTEB;
    Bundle.blocks.ExtBlocks[BPLIB_MAX_NUM_EXTENSION_BLOCKS - 1].BlockData.CustodyBlockData.BundleSeqId = 12;
    Bundle.blocks.ExtBlocks[BPLIB_MAX_NUM_EXTENSION_BLOCKS - 1].BlockData.CustodyBlockData.BundleSeqNum = 33;

    /*
    ** CCS data explanation:
    ** This is the first sequence number received for this CCS so it should just be:
    **  [1]
    */

    /* Avoid open CCS max size trigger */
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[0].CSSizeTrigger = 100;

    UT_SetDefaultReturnValue(UT_KEY(BPLib_EID_IsMatch), false);
    UT_SetDefaultReturnValue(UT_KEY(BPLib_PDB_AcceptCustody), BPLIB_ERROR);
    UT_SetDefaultReturnValue(UT_KEY(BPLib_ARP_GetDispCodeIdx), BPLib_CT_CustodyRefused_Idx);

    UtAssert_INT32_EQ(BPLib_CT_ProcessNewBundle(&BplibInst, &Bundle), BPLIB_CT_CUSTODY_REFUSED_ERR);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqId, 12);
    UtAssert_EQ(size_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRangeLen, 1);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRange[0], 1);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].FirstSeqNum, 33);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].LastSeqNumAdded, 33);
    UtAssert_EQ(bool, BplibInst.Ct.OpenCcss[0].InProgress, true);
}

void Test_BPLib_CT_ProcessNewBundle_Noncustodial(void)
{
    BPLib_Bundle_t Bundle;

    /* Set up non-custodial bundle */
    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));


    UtAssert_INT32_EQ(BPLib_CT_ProcessNewBundle(&BplibInst, &Bundle), BPLIB_SUCCESS);
    UtAssert_STUB_COUNT(BPLib_PDB_AcceptCustody, 0);
}

void Test_BPLib_CT_ProcessNewBundle_StorFull(void)
{
    BPLib_Bundle_t Bundle;

    /* Set up custodial bundle */
    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    Bundle.blocks.ExtBlocks[BPLIB_MAX_NUM_EXTENSION_BLOCKS - 1].Header.BlockType = BPLib_BlockType_CTEB;
    Bundle.blocks.ExtBlocks[BPLIB_MAX_NUM_EXTENSION_BLOCKS - 1].BlockData.CustodyBlockData.BundleSeqId = 12;
    Bundle.blocks.ExtBlocks[BPLIB_MAX_NUM_EXTENSION_BLOCKS - 1].BlockData.CustodyBlockData.BundleSeqNum = 33;

    /* Make the disposition returned from BPLib_ARP_GetDispCode an expected value */
    UT_SetDefaultReturnValue(UT_KEY(BPLib_ARP_GetDispCodeIdx), BPLib_CT_CustodyRefused_Idx);

    /* Avoid open CCS max size trigger */
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[0].CSSizeTrigger = 100;

    /* Set storage to full */
    BplibInst.BundleStorage.BytesStorageInUse = BPLIB_MAX_STORED_BUNDLE_BYTES;

    /*
    ** CCS data explanation:
    ** This is the first sequence number received for this CCS so it should just be:
    **  [1]
    */

    UtAssert_INT32_EQ(BPLib_CT_ProcessNewBundle(&BplibInst, &Bundle), BPLIB_CT_CUSTODY_REFUSED_ERR);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqId, 12);
    UtAssert_EQ(size_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRangeLen, 1);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRange[0], 1);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].FirstSeqNum, 33);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].LastSeqNumAdded, 33);
    UtAssert_EQ(bool, BplibInst.Ct.OpenCcss[0].InProgress, true);
    UtAssert_STUB_COUNT(BPLib_EM_SendEvent, 2);
    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_DELETED_NO_STORAGE, 
                                            Context_BPLib_AS_Increment[0].Counter);
    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_CUSTODY_REQUEST, 
                                            Context_BPLib_AS_Increment[1].Counter);
    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_DEPLETED, 
                                            Context_BPLib_AS_Increment[2].Counter);
}

void Test_BPLib_CT_ProcessNewBundle_Dupl(void)
{
    BPLib_Bundle_t Bundle;
    BPLib_CT_DbEntry_t Entry1;  /* Existing DB entry for a duplicate bundle */

    /* Set up custodial bundle */
    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    Bundle.blocks.ExtBlocks[BPLIB_MAX_NUM_EXTENSION_BLOCKS - 1].Header.BlockType = BPLib_BlockType_CTEB;
    Bundle.blocks.ExtBlocks[BPLIB_MAX_NUM_EXTENSION_BLOCKS - 1].BlockData.CustodyBlockData.BundleSeqId = 12;
    Bundle.blocks.ExtBlocks[BPLIB_MAX_NUM_EXTENSION_BLOCKS - 1].BlockData.CustodyBlockData.BundleSeqNum = 33;

    /* Return link with duplicate bundle */
    UT_SetDefaultReturnValue(UT_KEY(BPLib_RBT_SearchGeneric), (UT_IntReturn_t) &(Entry1.IdRbtLink));

    /* Make the disposition returned from BPLib_ARP_GetDispCode an expected value */
    UT_SetDefaultReturnValue(UT_KEY(BPLib_ARP_GetDispCodeIdx), BPLib_CT_CustodyRefused_Idx);

    /* Avoid open CCS max size trigger */
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[0].CSSizeTrigger = 100;

    /*
    ** CCS data explanation:
    ** This is the first sequence number received for this CCS so it should just be:
    **  [1]
    */

    UtAssert_INT32_EQ(BPLib_CT_ProcessNewBundle(&BplibInst, &Bundle), BPLIB_CT_CUSTODY_REFUSED_ERR);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqId, 12);
    UtAssert_EQ(size_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRangeLen, 1);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].SeqRange[0], 1);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].FirstSeqNum, 33);
    UtAssert_EQ(uint64_t, BplibInst.Ct.OpenCcss[0].BundleSeqCollections[BPLib_CT_CustodyRefused_Idx].LastSeqNumAdded, 33);
    UtAssert_EQ(bool, BplibInst.Ct.OpenCcss[0].InProgress, true);
    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_CUSTODY_REQUEST, 
                                            Context_BPLib_AS_Increment[0].Counter);
    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_REDUNDANT, 
                                            Context_BPLib_AS_Increment[1].Counter);
}

void Test_BPLib_CT_UpdateBundle_Null(void)
{
    BPLib_Bundle_t Bundle;

    UtAssert_INT32_EQ(BPLib_CT_UpdateBundle(NULL, &Bundle), BPLIB_NULL_PTR_ERROR);
    UtAssert_INT32_EQ(BPLib_CT_UpdateBundle(&BplibInst, NULL), BPLIB_NULL_PTR_ERROR);    
}

void Test_BPLib_CT_UpdateBundle_Custodial(void)
{
    BPLib_Bundle_t Bundle;
    BPLib_MEM_Block_t MemBlk;
    BPLib_CT_DbEntry_t *NewDbEntry;

    /* Set up custodial bundle */
    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    Bundle.blocks.ExtBlocks[0].Header.BlockType = BPLib_BlockType_CTEB;
    Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqId = 12;
    Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqNum = 33;
    Bundle.blocks.PrimaryBlock.BundleId = 0xdead;
    Bundle.Meta.EgressID = 0;

    /* Set up counter context */
    BplibInst.Ct.CurrActiveSeqIds[0] = 56;
    BplibInst.Ct.SeqCounters[56 % BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS] = 69;

    /* Set up CTDB context */
    UT_SetDeferredRetcode(UT_KEY(BPLib_MEM_BlockAlloc), 1, (UT_IntReturn_t) &MemBlk);
    NewDbEntry = &MemBlk.user_data.DbEntry;
    BplibInst.Ct.CurrDbSize = 10;
    
    UtAssert_INT32_EQ(BPLib_CT_UpdateBundle(&BplibInst, &Bundle), BPLIB_SUCCESS);
    
    UtAssert_EQ(uint64_t, Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqId, 56);
    UtAssert_EQ(uint64_t, Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqNum, 69);
    UtAssert_STUB_COUNT(BPLib_EID_CopyEids, 1);
    UtAssert_EQ(uint32_t, NewDbEntry->BundleId, 0xdead);
    UtAssert_EQ(uint64_t, NewDbEntry->SeqId, 56);
    UtAssert_EQ(uint64_t, NewDbEntry->SeqNum, 69);
    UtAssert_EQ(size_t, BplibInst.Ct.CurrDbSize, 11);
    UtAssert_EQ(uint64_t, BplibInst.Ct.SeqCounters[56 % BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS], 70);
}

void Test_BPLib_CT_UpdateBundle_Noncustodial(void)
{
    BPLib_Bundle_t Bundle;

    /* Set up non-custodial bundle */
    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));

    UtAssert_INT32_EQ(BPLib_CT_UpdateBundle(&BplibInst, &Bundle), BPLIB_SUCCESS);
    
    UtAssert_STUB_COUNT(BPLib_EID_CopyEids, 0);
}

void Test_BPLib_CT_UpdateBundle_BadEgressId(void)
{
    BPLib_Bundle_t Bundle;

    /* Set up custodial bundle */
    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    Bundle.blocks.ExtBlocks[0].Header.BlockType = BPLib_BlockType_CTEB;
    Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqId = 12;
    Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqNum = 33;
    Bundle.blocks.PrimaryBlock.BundleId = 0xdead;
    Bundle.Meta.EgressID = BPLIB_MAX_NUM_CONTACTS;

    UtAssert_INT32_EQ(BPLib_CT_UpdateBundle(&BplibInst, &Bundle), BPLIB_INVALID_CONT_ID_ERR);
    
    UtAssert_STUB_COUNT(BPLib_EID_CopyEids, 0);
    UtAssert_STUB_COUNT(BPLib_EM_SendEvent, 1);
}

void Test_BPLib_CT_UpdateBundle_Retransmit(void)
{
    BPLib_Bundle_t Bundle;
    BPLib_MEM_Block_t MemBlk;
    BPLib_CT_DbEntry_t *ExistingDbEntry;

    /* Set up custodial bundle */
    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    Bundle.blocks.ExtBlocks[0].Header.BlockType = BPLib_BlockType_CTEB;
    Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqId = 12;
    Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqNum = 33;
    Bundle.blocks.PrimaryBlock.BundleId = 0xdead;
    Bundle.Meta.EgressID = 0;

    /* Set up counter context */
    BplibInst.Ct.CurrActiveSeqIds[0] = 56;
    BplibInst.Ct.SeqCounters[56 % BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS] = 69;

    /* Set up CTDB context */
    UT_SetDeferredRetcode(UT_KEY(BPLib_MEM_BlockAlloc), 1, (UT_IntReturn_t) &MemBlk);
    ExistingDbEntry = &MemBlk.user_data.DbEntry;
    ExistingDbEntry->SeqId = 12;
    ExistingDbEntry->SeqNum = 34;
    ExistingDbEntry->BundleId = 0xdead;
    BplibInst.Ct.CurrDbSize = 10;    
    
    /* Find bundle in CTDB */
    UT_SetDefaultReturnValue(UT_KEY(BPLib_RBT_SearchGeneric), (UT_IntReturn_t) &(ExistingDbEntry->IdRbtLink));

    UtAssert_INT32_EQ(BPLib_CT_UpdateBundle(&BplibInst, &Bundle), BPLIB_SUCCESS);
    
    UtAssert_EQ(uint64_t, Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqId, 12);
    UtAssert_EQ(uint64_t, Bundle.blocks.ExtBlocks[0].BlockData.CustodyBlockData.BundleSeqNum, 34);
    UtAssert_STUB_COUNT(BPLib_EID_CopyEids, 1);
    UtAssert_EQ(size_t, BplibInst.Ct.CurrDbSize, 10);
    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_CUSTODY_RE_FORWARDED, 
                                            Context_BPLib_AS_Increment[0].Counter);
}

void Test_BPLib_CT_AssignSeqCounter_Nominal(void)
{
    uint32_t ContId = 0;

    /* Set up counter context */
    BplibInst.Ct.CurrActiveSeqIds[0] = BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS - 1;
    BplibInst.Ct.LastSeqCounterId = BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS - 1;
    BplibInst.Ct.SeqCounters[0] = 69;


    UtAssert_INT32_EQ(BPLib_CT_AssignSeqCounter(&BplibInst, ContId), BPLIB_SUCCESS);

    UtAssert_EQ(uint64_t, BplibInst.Ct.SeqCounters[0], 0);
    UtAssert_EQ(uint64_t, BplibInst.Ct.LastSeqCounterId, BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS);
    UtAssert_EQ(uint64_t, BplibInst.Ct.CurrActiveSeqIds[0], BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS);
}

void Test_BPLib_CT_AssignSeqCounter_InputErr(void)
{
    UtAssert_INT32_EQ(BPLib_CT_AssignSeqCounter(NULL, 0), BPLIB_NULL_PTR_ERROR);
    UtAssert_INT32_EQ(BPLib_CT_AssignSeqCounter(&BplibInst, BPLIB_MAX_NUM_CONTACTS), BPLIB_INVALID_CONT_ID_ERR);
}

void Test_BPLib_CT_ProcessCcs_Nominal(void)
{
    BPLib_CT_DeserializedCcs_t Ccs;
    size_t ExpCtdbSize;

    memset(&Ccs, 0, sizeof(BPLib_CT_DeserializedCcs_t));

    Ccs.NumBundleSeqCollections = 2;

    /*
    ** Bundle sequence collection #0, accepted sequences for sequence ID 10:
    **  - Sequence 20-21 received
    **  - Sequence 22 not received
    **  - Sequence 23-25 received
    **  - Sequence 26-27 not received
    **  - Sequence 28-31 received
    */

    Ccs.BundleSeqCollections[0].DispositionCode = BPLib_CT_CustodyAccepted;
    Ccs.BundleSeqCollections[0].FirstSeqNum = 20;
    Ccs.BundleSeqCollections[0].SeqId = 10;
    Ccs.BundleSeqCollections[0].SeqRangeLen = 5;
    Ccs.BundleSeqCollections[0].SeqRange[0] = 2;
    Ccs.BundleSeqCollections[0].SeqRange[1] = 1;
    Ccs.BundleSeqCollections[0].SeqRange[2] = 3;
    Ccs.BundleSeqCollections[0].SeqRange[3] = 2;
    Ccs.BundleSeqCollections[0].SeqRange[4] = 4;

    /*
    ** Bundle sequence collection #1, refused sequences for sequence ID 10:
    **  - Sequence 32-33 received
    **  - Sequence 34 not received
    **  - Sequence 34-36 received
    */    

    Ccs.BundleSeqCollections[1].DispositionCode = BPLib_CT_CustodyRefused;
    Ccs.BundleSeqCollections[1].FirstSeqNum = 32;
    Ccs.BundleSeqCollections[1].SeqId = 10;
    Ccs.BundleSeqCollections[1].SeqRangeLen = 3;
    Ccs.BundleSeqCollections[1].SeqRange[0] = 2;
    Ccs.BundleSeqCollections[1].SeqRange[1] = 1;
    Ccs.BundleSeqCollections[1].SeqRange[2] = 3;

    BplibInst.Ct.CurrDbSize = BPLIB_CT_DB_MAX_ENTRIES;
    ExpCtdbSize = BPLIB_CT_DB_MAX_ENTRIES - 9; /* CCS should remove only the 9 custody accepted entries from CTDB */

    /* Set RBT to always return same link (for simplicity) */
    UT_SetDefaultReturnValue(UT_KEY(BPLib_RBT_SearchGeneric), (UT_IntReturn_t) &(BplibInst.Ct.SeqTreeRoot));

    UtAssert_INT32_EQ(BPLib_CT_ProcessCcs(&BplibInst, &Ccs), BPLIB_SUCCESS);
    UtAssert_EQ(size_t, BplibInst.Ct.CurrDbSize, ExpCtdbSize);
}

void Test_BPLib_CT_ProcessCcs_Null(void)
{
    BPLib_CT_DeserializedCcs_t Ccs;

    UtAssert_INT32_EQ(BPLib_CT_ProcessCcs(NULL, &Ccs), BPLIB_NULL_PTR_ERROR);
    UtAssert_INT32_EQ(BPLib_CT_ProcessCcs(&BplibInst, NULL), BPLIB_NULL_PTR_ERROR);
}

void Test_BPLib_CT_ProcessCcs_BufErr(void)
{
    BPLib_CT_DeserializedCcs_t Ccs;

    memset(&Ccs, 0, sizeof(Ccs));

    Ccs.NumBundleSeqCollections = BPLIB_CT_MAX_RECVD_SEQ_COLLECTIONS;
    UtAssert_INT32_EQ(BPLib_CT_ProcessCcs(&BplibInst, &Ccs), BPLIB_BUF_LEN_ERROR);

    Ccs.NumBundleSeqCollections = 1;
    Ccs.BundleSeqCollections[0].SeqRangeLen = BPLIB_CT_MAX_SEQ_RANGE_LEN;
    UtAssert_INT32_EQ(BPLib_CT_ProcessCcs(&BplibInst, &Ccs), BPLIB_BUF_LEN_ERROR);
}

void TestBplibCt_Register(void)
{
    ADD_TEST(Test_BPLib_CT_Init_Nominal);
    ADD_TEST(Test_BPLib_CT_Init_Null);

    ADD_TEST(Test_BPLib_CT_SetBundleId_Nominal);
    ADD_TEST(Test_BPLib_CT_SetBundleId_Null);

    ADD_TEST(Test_BPLib_CT_ProcessNewBundle_Null);
    ADD_TEST(Test_BPLib_CT_ProcessNewBundle_AcceptCustody);
    ADD_TEST(Test_BPLib_CT_ProcessNewBundle_RejectCustody);
    ADD_TEST(Test_BPLib_CT_ProcessNewBundle_Noncustodial);
    ADD_TEST(Test_BPLib_CT_ProcessNewBundle_StorFull);
    ADD_TEST(Test_BPLib_CT_ProcessNewBundle_Dupl);

    ADD_TEST(Test_BPLib_CT_UpdateBundle_Null);
    ADD_TEST(Test_BPLib_CT_UpdateBundle_Custodial);
    ADD_TEST(Test_BPLib_CT_UpdateBundle_Noncustodial);
    ADD_TEST(Test_BPLib_CT_UpdateBundle_BadEgressId);
    ADD_TEST(Test_BPLib_CT_UpdateBundle_Retransmit);

    ADD_TEST(Test_BPLib_CT_AssignSeqCounter_Nominal);
    ADD_TEST(Test_BPLib_CT_AssignSeqCounter_InputErr);

    ADD_TEST(Test_BPLib_CT_ProcessCcs_Nominal);
    ADD_TEST(Test_BPLib_CT_ProcessCcs_Null);
    ADD_TEST(Test_BPLib_CT_ProcessCcs_BufErr);
}

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
#include "bplib_stor_test_utils.h"
#include "bplib_nc.h"
#include "bplib_qm_handlers.h"

#include <stdlib.h>

/*
** Test function for
** int BPLib_STOR_Init()
*/
void Test_BPLib_STOR_Init(void)
{
    UtAssert_INT32_EQ(BPLib_STOR_Init(&BplibInst), BPLIB_SUCCESS);
}

void Test_BPLib_STOR_InitNullInst(void)
{
    UtAssert_INT32_EQ(BPLib_STOR_Init(NULL), BPLIB_NULL_PTR_ERROR);
}

/* Test the STOR Correctly initializes the stored bundle count */
void Test_BPLib_STOR_InitStoredCount(void)
{
    /* The fixture for this function pre-stores one bundle. therefore the check is simply that the
    ** stored count is already 1.
    */
    UtAssert_EQ(uint32_t, BplibInst.BundleStorage.BundleCountStored, 1);
}

/* Test Destroy runs without segfault */
void Test_BPLib_STOR_Destroy(void)
{
    BPLib_STOR_Destroy(&BplibInst);
    BPLib_STOR_Destroy(NULL);
}

/* Test Table Validation returns SUCCESS in Nominal case */
void Test_BPLib_STOR_StorageTblValidateFunc_Nominal(void)
{
    BPLib_STOR_StorageTable_t TestTblData;
    memset(&TestTblData, 0, sizeof(TestTblData));
    UtAssert_INT32_EQ((int32) BPLib_STOR_StorageTblValidateFunc(&TestTblData), (int32) BPLIB_SUCCESS);     
}


/*******************************************************************************
** GarbageCollect Tests
*/

/* Test STOR_GarbageCollect guards against Null Params */
void Test_BPLib_STOR_GarbageCollect_NullParams(void)
{
    UtAssert_INT32_EQ(BPLib_STOR_GarbageCollect(NULL), BPLIB_NULL_PTR_ERROR);
}

/* Test STOR_GarbageCollect discards a bundle. */
void Test_BPLib_STOR_GarbageCollect_NominalExpired(void)
{
    UtAssert_EQ(uint32_t, BplibInst.BundleStorage.BundleCountStored, 1);

    /* Nothing should be discarded if current time is before bundle expiration time */
    UT_SetDeferredRetcode(UT_KEY(BPLib_TIME_GetMonotonicTime), 1, 797186475264); /* 797186475264 is creation time for TestBundle */
    UtAssert_INT32_EQ(BPLib_STOR_GarbageCollect(&BplibInst), BPLIB_SUCCESS);

    UtAssert_STUB_COUNT(BPLib_AS_Increment, 0);
    UtAssert_EQ(uint32_t, BplibInst.BundleStorage.BundleCountStored, 1);

    /* Skip counters 3,4 which are for DiscardEgressed */

    /* Storage should be discarded because bundle timestamp is expired */
    UT_SetDeferredRetcode(UT_KEY(BPLib_TIME_GetMonotonicTime), 1, INT64_MAX);
    UtAssert_INT32_EQ(BPLib_STOR_GarbageCollect(&BplibInst), BPLIB_SUCCESS);
    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_DELETED_EXPIRED, 
                                                Context_BPLib_AS_Increment[0].Counter);
    UtAssert_INT32_EQ(1, Context_BPLib_AS_Increment[0].Amount);
    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_DELETED, 
                                                Context_BPLib_AS_Increment[1].Counter);
    UtAssert_INT32_EQ(1, Context_BPLib_AS_Increment[1].Amount);
    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_DISCARDED, 
                                                Context_BPLib_AS_Increment[2].Counter);
    UtAssert_INT32_EQ(1, Context_BPLib_AS_Increment[2].Amount);
    UtAssert_EQ(uint32_t, BplibInst.BundleStorage.BundleCountStored, 0);
}

/* Test STOR_GarbageCollect handles a SQL Failure */
void Test_BPLib_STOR_GarbageCollect_SQLFail(void)
{
    /* Force sql statements to fail by passing a NULL instance */
    BplibInst.BundleStorage.db = NULL;
    UtAssert_INT32_EQ(BPLib_STOR_GarbageCollect(&BplibInst), BPLIB_STOR_SQL_DISCARD_ERR);

    /* Look for event message */
    UtAssert_STUB_COUNT(BPLib_EM_SendEvent, 2);
    UtAssert_INT32_EQ(context_BPLib_EM_SendEvent[0].EventID, BPLIB_STOR_SQL_GC_ERR_EID);
    UtAssert_INT32_EQ(context_BPLib_EM_SendEvent[1].EventID, BPLIB_STOR_SQL_GC_ERR_EID);
}

void Test_BPLib_STOR_UpdateHkPkt_Nominal(void)
{
    size_t ExpectedBytesMemInUse;
    size_t ExpectedBytesMemHighWater;
    size_t ExpectedBytesMemFree;
    size_t ExpectedKbBundlesInStor;

    memset((void*) &BPLib_STOR_StoragePayload, 0, sizeof(BPLib_StorageHkTlm_Payload_t));

    BplibInst.pool.impl.num_blocks = 10;
    BplibInst.pool.impl.num_free   = 20;
    BplibInst.pool.impl.block_size = 30;

    ExpectedBytesMemInUse     = ((BplibInst.pool.impl.num_blocks - BplibInst.pool.impl.num_free) * BplibInst.pool.impl.block_size);
    ExpectedBytesMemHighWater = ExpectedBytesMemInUse;
    ExpectedBytesMemFree      = (BplibInst.pool.impl.num_free * BplibInst.pool.impl.block_size);
    ExpectedKbBundlesInStor    = (BplibInst.BundleStorage.BytesStorageInUse / 1000);

    BPLib_STOR_UpdateHkPkt(&BplibInst);

    UtAssert_EQ(size_t, BPLib_STOR_StoragePayload.BytesMemInUse,     ExpectedBytesMemInUse);
    UtAssert_EQ(size_t, BPLib_STOR_StoragePayload.BytesMemHighWater, ExpectedBytesMemHighWater);
    UtAssert_EQ(size_t, BPLib_STOR_StoragePayload.BytesMemFree,      ExpectedBytesMemFree);
    UtAssert_EQ(size_t, BPLib_STOR_StoragePayload.KbBundlesInStor,    ExpectedKbBundlesInStor);
}

void Test_BPLib_STOR_Cleanup_Nominal(void)
{
    BPLib_Status_t Status;

    Status = BPLib_STOR_Cleanup(&BplibInst);

    UtAssert_INT32_EQ(Status, BPLIB_SUCCESS);
    UtAssert_INT32_EQ(context_BPLib_EM_SendEvent[0].EventID, BPLIB_STOR_CLEANUP_INF_EID);
}

void Test_BPLib_STOR_UpdateCustodialBundles_Nominal(void)
{
    BPLib_CT_CcsUpdateBatch_t Batch;
    size_t BundlesPerOp = BPLIB_CT_BATCH_SIZE / 3;
    size_t NumBundles = BPLIB_CT_BATCH_SIZE;
    BPLib_Bundle_t Bundles[BPLIB_CT_BATCH_SIZE];
    size_t i;
    size_t NumEgressed;
    uint64_t RetransmissionTime = 12345;

    /*
    ** Test setup: Put a bunch of unique bundles into storage
    */
    UT_SetDefaultReturnValue(UT_KEY(BPLib_TIME_GetCurrentDtnTime), 797186475264);
    UT_SetDefaultReturnValue(UT_KEY(BPLib_TIME_GetMonotonicTime), 797186475264);

    for (i = 0; i < NumBundles; i++)
    {
        BPLib_STOR_Test_CreateTestBundle(&Bundles[i], i);
        Bundles[i].Meta.IsCustodial = true;
        Bundles[i].Meta.RetransmitTime = 100;
        BPLib_STOR_StoreBundle(&BplibInst, &Bundles[i]);

        Batch.BundleIDs[i] = i;
        
        if (i < BundlesPerOp)
        {
            Batch.Ops[i] = BPLIB_CT_MARK_DELETE;
        }
        else if (i < (BundlesPerOp * 2))
        {
            Batch.Ops[i] = BPLIB_CT_START_RETRANSMIT;
        }
        else if (i < (BundlesPerOp * 3))
        {
            Batch.Ops[i] = BPLIB_CT_STOP_RETRANSMIT;
        }
    }

    Batch.Size = NumBundles;
    BPLib_STOR_FlushPending(&BplibInst);

    /*
    ** Run test: Perform the following batch operations on the database and then
    ** test that the expected updates took effect
    */
    UT_SetDefaultReturnValue(UT_KEY(BPLib_TIME_GetMonotonicTime), RetransmissionTime);
    UtAssert_EQ(BPLib_Status_t, BPLib_STOR_UpdateCustodialBundles(&BplibInst, &Batch), BPLIB_SUCCESS);

    /* Garbage collect all bundles marked for expiration */
    UT_SetDefaultReturnValue(UT_KEY(BPLib_TIME_GetMonotonicTime), 0);
    UtAssert_INT32_EQ(BPLib_STOR_GarbageCollect(&BplibInst), BPLIB_SUCCESS);
    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_IN_CUSTODY, 
                                                Context_BPLib_AS_Increment[0].Counter);
    UtAssert_INT32_EQ(NumBundles, Context_BPLib_AS_Increment[0].Amount);

    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_DISCARDED, 
                                                Context_BPLib_AS_Increment[1].Counter);
    UtAssert_INT32_EQ(BundlesPerOp, Context_BPLib_AS_Increment[1].Amount);
    UtAssert_EQ(BPLib_AS_Counter_t, BUNDLE_COUNT_DELETED, 
                                                Context_BPLib_AS_Increment[2].Counter);
    UtAssert_INT32_EQ(BundlesPerOp, Context_BPLib_AS_Increment[2].Amount);
    UtAssert_EQ(uint32_t, BplibInst.BundleStorage.BundleCountStored, NumBundles - BundlesPerOp);

    /* Try to egress all bundles marked for retransmission */
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[0].DestEIDs[0].MaxNode = 100;
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[0].DestEIDs[0].MinNode = 100;
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[0].DestEIDs[0].MaxService = 1;
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[0].DestEIDs[0].MinService = 1;
    UT_SetDefaultReturnValue(UT_KEY(BPLib_TIME_GetMonotonicTime), RetransmissionTime);

    UtAssert_EQ(BPLib_Status_t, BPLib_STOR_EgressForID(&BplibInst, 0, false, &NumEgressed), BPLIB_SUCCESS);
    UtAssert_INT32_EQ(BplibInst.BundleStorage.ContactLoadBatches[0].Size, BundlesPerOp);
    
    /*
    ** Test teardown: free memory
    */    
   for (i = 0; i < NumBundles; i++)
   {
        BPLib_STOR_Test_FreeTestBundle(&Bundles[i]);
   }
}

void Test_BPLib_STOR_UpdateCustodialBundles_Null(void)
{
    BPLib_CT_CcsUpdateBatch_t Batch;

    UtAssert_EQ(BPLib_Status_t, BPLib_STOR_UpdateCustodialBundles(NULL, &Batch), BPLIB_NULL_PTR_ERROR);
    UtAssert_EQ(BPLib_Status_t, BPLib_STOR_UpdateCustodialBundles(&BplibInst, NULL), BPLIB_NULL_PTR_ERROR);

    Batch.Size = BPLIB_CT_BATCH_SIZE + 1;
    UtAssert_EQ(BPLib_Status_t, BPLib_STOR_UpdateCustodialBundles(&BplibInst, &Batch), BPLIB_NULL_PTR_ERROR);
}

void Test_BPLib_STOR_UpdateCustodialBundles_Err(void)
{
    BPLib_CT_CcsUpdateBatch_t Batch;

    BplibInst.BundleStorage.db = NULL;
    Batch.Size = 0;

    UtAssert_EQ(BPLib_Status_t, BPLib_STOR_UpdateCustodialBundles(&BplibInst, &Batch), BPLIB_SQL_CUSTODY_UPDATE_ERR);
    UtAssert_STUB_COUNT(BPLib_EM_SendEvent, 1);
}

void Test_BPLib_STOR_SetNewRetransmitTrigger_Nominal(void)
{
    size_t NumBundles = BPLIB_CT_BATCH_SIZE;
    BPLib_Bundle_t Bundles[BPLIB_CT_BATCH_SIZE];
    size_t i;
    uint32_t OldTrigger = 100;
    uint32_t NewTrigger = 500;
    uint32_t ContId = 0;

    /*
    ** Test setup: Put a bunch of unique bundles into storage
    */
    UT_SetDefaultReturnValue(UT_KEY(BPLib_TIME_GetCurrentDtnTime), 797186475264);
    UT_SetDefaultReturnValue(UT_KEY(BPLib_TIME_GetMonotonicTime), 797186475264);

    for (i = 0; i < NumBundles; i++)
    {
        BPLib_STOR_Test_CreateTestBundle(&Bundles[i], i);
        Bundles[i].Meta.IsCustodial = true;
        Bundles[i].Meta.RetransmitTime = OldTrigger;
        BPLib_STOR_StoreBundle(&BplibInst, &Bundles[i]);
    }

    BPLib_STOR_FlushPending(&BplibInst);

    /*
    ** Run test: Perform the following batch operations on the database and then
    ** test that the expected updates took effect
    */
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[ContId].DestEIDs[0].MaxNode = 100;
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[ContId].DestEIDs[0].MinNode = 100;
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[ContId].DestEIDs[0].MaxService = 1;
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[ContId].DestEIDs[0].MinService = 1;
    BPLib_NC_ConfigPtrs.ContactsConfigPtr->ContactSet[ContId].RetransmitTimeout = NewTrigger;
    UtAssert_EQ(BPLib_Status_t, BPLib_STOR_SetNewRetransmitTrigger(&BplibInst, ContId), BPLIB_SUCCESS);

    /*
    ** Test teardown: free memory
    */    
   for (i = 0; i < NumBundles; i++)
   {
        BPLib_STOR_Test_FreeTestBundle(&Bundles[i]);
   }
}

void Test_BPLib_STOR_SetNewRetransmitTrigger_BadInput(void)
{
    UtAssert_INT32_EQ(BPLib_STOR_SetNewRetransmitTrigger(NULL, 0), BPLIB_NULL_PTR_ERROR);
    UtAssert_INT32_EQ(BPLib_STOR_SetNewRetransmitTrigger(&BplibInst, BPLIB_MAX_NUM_CONTACTS), BPLIB_INVALID_CONT_ID_ERR);
}

void Test_BPLib_STOR_SetNewRetransmitTrigger_Err(void)
{
    BplibInst.BundleStorage.db = NULL;

    UtAssert_INT32_EQ(BPLib_STOR_SetNewRetransmitTrigger(&BplibInst, 0), BPLIB_STOR_SQL_NEW_RETRANSMIT_ERR);
}

void TestBplib_STOR_Register(void)
{
   ADD_TEST(Test_BPLib_STOR_Init);
   ADD_TEST(Test_BPLib_STOR_Init);
   ADD_TEST(Test_BPLib_STOR_Destroy);
   ADD_TEST_ONE_BUNDLE(Test_BPLib_STOR_InitStoredCount);

   ADD_TEST(Test_BPLib_STOR_StorageTblValidateFunc_Nominal);

   ADD_TEST_ONE_BUNDLE(Test_BPLib_STOR_GarbageCollect_NullParams);
   ADD_TEST_ONE_BUNDLE(Test_BPLib_STOR_GarbageCollect_NominalExpired);
   ADD_TEST(Test_BPLib_STOR_GarbageCollect_SQLFail);

   ADD_TEST(Test_BPLib_STOR_UpdateHkPkt_Nominal);

   ADD_TEST(Test_BPLib_STOR_Cleanup_Nominal);

   ADD_TEST(Test_BPLib_STOR_UpdateCustodialBundles_Nominal);
   ADD_TEST(Test_BPLib_STOR_UpdateCustodialBundles_Null);
   ADD_TEST(Test_BPLib_STOR_UpdateCustodialBundles_Err);

   ADD_TEST(Test_BPLib_STOR_SetNewRetransmitTrigger_Nominal);
   ADD_TEST(Test_BPLib_STOR_SetNewRetransmitTrigger_BadInput);
   ADD_TEST(Test_BPLib_STOR_SetNewRetransmitTrigger_Err);
}

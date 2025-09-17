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

#include "bplib_stor_test_utils.h"
#include "bplib_stor_sql_store.h"

/* ============== */
/* Test Functions */
/* ============== */

void Test_BPLib_SQL_StoreMetadata_ValidCreateTimeValidDtnTime(void)
{
    BPLib_Bundle_t      Bundle;
    BPLib_BundleCache_t BundleCache;
    uint64_t            CurrDtnTime;
    int64_t             MonoTime;
    SQL_Status_t        SQL_Status;

    /* Initialize values needed for testing */
    BPLib_STOR_Test_CreateTestBundle(&Bundle);
    CurrDtnTime = Bundle.blocks.PrimaryBlock.Timestamp.CreateTime;
    MonoTime    = 0;

    /* Feed values to functions that will determine expiration time */
    UT_SetDataBuffer(UT_KEY(BPLib_TIME_GetCurrentDtnTime), (void*) &CurrDtnTime, sizeof(uint64_t), false);
    UT_SetDataBuffer(UT_KEY(BPLib_TIME_GetMonotonicTime), (void*) &MonoTime, sizeof(int64_t), false);

    /* Run the function under test */
    SQL_Status = BPLib_SQL_StoreMetadata(&Bundle, &BundleCache);

    /* Show that the function was run successfully */
    UtAssert_EQ(SQL_Status_t, SQL_Status, SQLITE_DONE);

    /* Show that the correct branch was tested */
    UtAssert_STUB_COUNT(BPLib_TIME_GetCurrentDtnTime, 1);
    UtAssert_STUB_COUNT(BPLib_TIME_GetMonotonicTime, 1);

    /* Free memory used for bundle */
    BPLib_STOR_Test_FreeTestBundle(&Bundle);
}

void Test_BPLib_SQL_StoreMetadata_ValidCreateTimeInvalidDtnTime(void)
{
    BPLib_Bundle_t      Bundle;
    BPLib_BundleCache_t BundleCache;
    SQL_Status_t        SQL_Status;

    /* Initialize values needed for testing */
    BPLib_STOR_Test_CreateTestBundle(&Bundle);

    /* Run the function under test */
    SQL_Status = BPLib_SQL_StoreMetadata(&Bundle, &BundleCache);

    /* Show that the function was run successfully */
    UtAssert_EQ(SQL_Status_t, SQL_Status, SQLITE_DONE);

    /* Show that the correct branch was tested */
    UtAssert_STUB_COUNT(BPLib_TIME_GetCurrentDtnTime, 0);
    UtAssert_STUB_COUNT(BPLib_TIME_GetMonotonicTime, 0);

    /* Free memory used for bundle */
    BPLib_STOR_Test_FreeTestBundle(&Bundle);
}

void Test_BPLib_SQL_StoreMetadata_InvalidCreateTime(void)
{
    BPLib_Bundle_t      Bundle;
    BPLib_BundleCache_t BundleCache;
    SQL_Status_t        SQL_Status;

    /* Initialize values needed for testing */
    BPLib_STOR_Test_CreateTestBundle(&Bundle);
    Bundle.blocks.PrimaryBlock.Timestamp.CreateTime = 0;
    
    /* Initialize the age block */
    Bundle.blocks.ExtBlocks[0].Header.BlockType           = BPLib_BlockType_Age;
    Bundle.blocks.ExtBlocks[0].BlockData.AgeBlockData.Age = 1000;

    /* Run the function under test */
    SQL_Status = BPLib_SQL_StoreMetadata(&Bundle, &BundleCache);

    /* Show that the function was run successfully */
    UtAssert_EQ(SQL_Status_t, SQL_Status, SQLITE_DONE);

    /* Show that the correct branch was tested */
    UtAssert_STUB_COUNT(BPLib_TIME_GetCurrentDtnTime, 0);
    UtAssert_STUB_COUNT(BPLib_TIME_GetMonotonicTime, 0);

    /* Free memory used for bundle */
    BPLib_STOR_Test_FreeTestBundle(&Bundle);
}

void Test_BPLib_SQL_StoreMetadata_Err(void)
{
    /* TBD */
}

void Test_BPLib_SQL_StoreChunk_Nominal(void)
{
    /* TBD */
}

void Test_BPLib_SQL_StoreBundle_Nominal(void)
{
    /* TBD */
}

void Test_BPLib_SQL_StoreBundle_Err(void)
{
    /* TBD */
}

void Test_BPLib_SQL_StoreImpl_Nominal(void)
{
    /* TBD */
}

void Test_BPLib_SQL_StoreImpl_Err(void)
{
    /* TBD */
}

void Test_BPLib_SQL_Store_Nominal(void)
{
    /* TBD */
}

void Test_BPLib_SQL_Store_Err(void)
{
    /* TBD */
}

void TestBplib_STOR_SQL_Store_Register(void)
{
    UtTest_Add(Test_BPLib_SQL_StoreMetadata_ValidCreateTimeValidDtnTime, BPLib_STOR_Test_Setup, BPLib_STOR_Test_Teardown, "Test_BPLib_SQL_StoreMetadata_ValidCreateTimeValidDtnTime");
    UtTest_Add(Test_BPLib_SQL_StoreMetadata_ValidCreateTimeInvalidDtnTime, BPLib_STOR_Test_Setup, BPLib_STOR_Test_Teardown, "Test_BPLib_SQL_StoreMetadata_ValidCreateTimeInvalidDtnTime");
    UtTest_Add(Test_BPLib_SQL_StoreMetadata_InvalidCreateTime, BPLib_STOR_Test_Setup, BPLib_STOR_Test_Teardown, "Test_BPLib_SQL_StoreMetadata_InvalidCreateTime");
    UtTest_Add(Test_BPLib_SQL_StoreMetadata_Err, BPLib_STOR_Test_Setup, BPLib_STOR_Test_Teardown, "Test_BPLib_SQL_StoreMetadata_Err");
    UtTest_Add(Test_BPLib_SQL_StoreChunk_Nominal, BPLib_STOR_Test_Setup, BPLib_STOR_Test_Teardown, "Test_BPLib_SQL_StoreChunk_Nominal");
    UtTest_Add(Test_BPLib_SQL_StoreBundle_Nominal, BPLib_STOR_Test_Setup, BPLib_STOR_Test_Teardown, "Test_BPLib_SQL_StoreBundle_Nominal");
    UtTest_Add(Test_BPLib_SQL_StoreBundle_Err, BPLib_STOR_Test_Setup, BPLib_STOR_Test_Teardown, "Test_BPLib_SQL_StoreBundle_Err");
    UtTest_Add(Test_BPLib_SQL_StoreImpl_Nominal, BPLib_STOR_Test_Setup, BPLib_STOR_Test_Teardown, "Test_BPLib_SQL_StoreImpl_Nominal");
    UtTest_Add(Test_BPLib_SQL_StoreImpl_Err, BPLib_STOR_Test_Setup, BPLib_STOR_Test_Teardown, "Test_BPLib_SQL_StoreImpl_Err");
    UtTest_Add(Test_BPLib_SQL_Store_Nominal, BPLib_STOR_Test_Setup, BPLib_STOR_Test_Teardown, "Test_BPLib_SQL_Store_Nominal");
    UtTest_Add(Test_BPLib_SQL_Store_Err, BPLib_STOR_Test_Setup, BPLib_STOR_Test_Teardown, "Test_BPLib_SQL_Store_Err");
}

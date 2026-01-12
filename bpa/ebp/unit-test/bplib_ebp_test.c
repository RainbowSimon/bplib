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
 * Includes
 */
#include "bplib_ebp_test_utils.h"


/* Test that block initialization fails when the bundle is null */
void Test_BPLib_EBP_InitBlocks_Null(void)
{
    BPLib_Status_t Status;


    Status = BPLib_EBP_InitializeExtensionBlocks(&Inst, NULL, 0);

    UtAssert_INT32_EQ(Status, BPLIB_NULL_PTR_ERROR);
}

/* Test that block initialization fails when channel ID is invalid */
void Test_BPLib_EBP_InitBlocks_ChanIdErr(void)
{
    BPLib_Status_t Status;
    BPLib_Bundle_t DeserializedBundle;
    uint32_t ChanId = BPLIB_MAX_NUM_CHANNELS;


    Status = BPLib_EBP_InitializeExtensionBlocks(&Inst, &DeserializedBundle, ChanId);

    UtAssert_INT32_EQ(Status, BPLIB_INVALID_CHAN_ID_ERR);
}

/* Test that block initialization succeeds for previous node blocks */
void Test_BPLib_EBP_InitBlocks_PrevNode(void)
{
    BPLib_Status_t Status;
    BPLib_Bundle_t DeserializedBundle;
    uint32_t ChanId = 0;

    memset(&DeserializedBundle, 0, sizeof(DeserializedBundle));

    DeserializedBundle.blocks.PrimaryBlock.Timestamp.CreateTime = 1000;

    Inst.ChanCtxt[ChanId].Config.PrevNodeBlkConfig.IncludeBlock = true;
    Inst.ChanCtxt[ChanId].Config.PrevNodeBlkConfig.CrcType = BPLib_CRC_Type_CRC16;
    Inst.ChanCtxt[ChanId].Config.PrevNodeBlkConfig.BlockNum = 1;
    Inst.ChanCtxt[ChanId].Config.PrevNodeBlkConfig.BlockProcFlags = 4;

    Status = BPLib_EBP_InitializeExtensionBlocks(&Inst, &DeserializedBundle, ChanId);

    UtAssert_INT32_EQ(Status, BPLIB_SUCCESS);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockType, 
                            BPLib_BlockType_PrevNode);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.CrcType,
                            Inst.ChanCtxt[ChanId].Config.PrevNodeBlkConfig.CrcType);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockNum,
                            Inst.ChanCtxt[ChanId].Config.PrevNodeBlkConfig.BlockNum);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockProcFlags,
                            Inst.ChanCtxt[ChanId].Config.PrevNodeBlkConfig.BlockProcFlags);
    UtAssert_BOOL_TRUE(DeserializedBundle.blocks.ExtBlocks[0].Header.RequiresEncode);
}

/* Test that block initialization succeeds for age blocks when configured for them */
void Test_BPLib_EBP_InitBlocks_AgeCfg(void)
{
    BPLib_Status_t Status;
    BPLib_Bundle_t DeserializedBundle;
    uint32_t ChanId = 0;

    memset(&DeserializedBundle, 0, sizeof(DeserializedBundle));

    Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.IncludeBlock = true;
    Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.CrcType = BPLib_CRC_Type_CRC16;
    Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.BlockNum = 1;
    Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.BlockProcFlags = 4;

    Status = BPLib_EBP_InitializeExtensionBlocks(&Inst, &DeserializedBundle, ChanId);

    UtAssert_INT32_EQ(Status, BPLIB_SUCCESS);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockType, 
                            BPLib_BlockType_Age);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.CrcType,
                            Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.CrcType);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockNum,
                            Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.BlockNum);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockProcFlags,
                            Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.BlockProcFlags);
    UtAssert_BOOL_TRUE(DeserializedBundle.blocks.ExtBlocks[0].Header.RequiresEncode);
}

/* Test that block initialization succeeds for age blocks when no valid time detected */
void Test_BPLib_EBP_InitBlocks_AgeAuto(void)
{
    BPLib_Status_t Status;
    BPLib_Bundle_t DeserializedBundle;
    uint32_t ChanId = 0;

    memset(&DeserializedBundle, 0, sizeof(DeserializedBundle));

    DeserializedBundle.blocks.PrimaryBlock.Timestamp.CreateTime = 0;
    Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.IncludeBlock = false;
    Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.CrcType = BPLib_CRC_Type_CRC16;
    Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.BlockNum = 1;
    Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.BlockProcFlags = 4;

    Status = BPLib_EBP_InitializeExtensionBlocks(&Inst, &DeserializedBundle, ChanId);

    UtAssert_INT32_EQ(Status, BPLIB_SUCCESS);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockType, 
                            BPLib_BlockType_Age);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.CrcType,
                            Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.CrcType);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockNum,
                            Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.BlockNum);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockProcFlags,
                            Inst.ChanCtxt[ChanId].Config.AgeBlkConfig.BlockProcFlags);
    UtAssert_BOOL_TRUE(DeserializedBundle.blocks.ExtBlocks[0].Header.RequiresEncode);
}

/* Test that block initialization succeeds for hop count blocks */
void Test_BPLib_EBP_InitBlocks_HopCount(void)
{
    BPLib_Status_t Status;
    BPLib_Bundle_t DeserializedBundle;
    uint32_t ChanId = 0;

    memset(&DeserializedBundle, 0, sizeof(DeserializedBundle));

    DeserializedBundle.blocks.PrimaryBlock.Timestamp.CreateTime = 1000;

    Inst.ChanCtxt[ChanId].Config.HopCountBlkConfig.IncludeBlock = true;
    Inst.ChanCtxt[ChanId].Config.HopCountBlkConfig.CrcType = BPLib_CRC_Type_CRC16;
    Inst.ChanCtxt[ChanId].Config.HopCountBlkConfig.BlockNum = 1;
    Inst.ChanCtxt[ChanId].Config.HopCountBlkConfig.BlockProcFlags = 4;
    Inst.ChanCtxt[ChanId].Config.HopLimit = 10;

    Status = BPLib_EBP_InitializeExtensionBlocks(&Inst, &DeserializedBundle, ChanId);

    UtAssert_INT32_EQ(Status, BPLIB_SUCCESS);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockType, 
                            BPLib_BlockType_HopCount);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.CrcType,
                            Inst.ChanCtxt[ChanId].Config.HopCountBlkConfig.CrcType);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockNum,
                            Inst.ChanCtxt[ChanId].Config.HopCountBlkConfig.BlockNum);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].Header.BlockProcFlags,
                            Inst.ChanCtxt[ChanId].Config.HopCountBlkConfig.BlockProcFlags);
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[0].BlockData.HopCountData.HopLimit,
                            Inst.ChanCtxt[ChanId].Config.HopLimit);    
    UtAssert_BOOL_TRUE(DeserializedBundle.blocks.ExtBlocks[0].Header.RequiresEncode);
}

/* Test that block updates fail for null bundle */
void Test_BPLib_EBP_UpdateBlocks_Null(void)
{
    BPLib_Status_t Status;

    Status = BPLib_EBP_UpdateExtensionBlocks(NULL);

    UtAssert_INT32_EQ(Status, BPLIB_NULL_PTR_ERROR);    
}

/* Test that block updates succeed for hop count blocks */
void Test_BPLib_EBP_UpdateBlocks_HopCount(void)
{
    BPLib_Status_t Status;
    BPLib_Bundle_t DeserializedBundle;
    uint64_t InitialHopCount = 1;

    memset(&DeserializedBundle, 0, sizeof(DeserializedBundle));

    DeserializedBundle.blocks.ExtBlocks[2].Header.BlockType = BPLib_BlockType_HopCount;
    DeserializedBundle.blocks.ExtBlocks[2].BlockData.HopCountData.HopCount = InitialHopCount;

    Status = BPLib_EBP_UpdateExtensionBlocks(&DeserializedBundle);

    UtAssert_INT32_EQ(Status, BPLIB_SUCCESS);    
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[2].BlockData.HopCountData.HopCount,
                            InitialHopCount + 1);
    UtAssert_BOOL_TRUE(DeserializedBundle.blocks.ExtBlocks[2].Header.RequiresEncode);
}

/* Test that block updates succeed for age blocks */
void Test_BPLib_EBP_UpdateBlocks_Age(void)
{
    BPLib_Status_t Status;
    BPLib_Bundle_t DeserializedBundle;
    uint64_t NewAge;
    int64_t TimeElapsed = 10;

    memset(&DeserializedBundle, 0, sizeof(DeserializedBundle));

    DeserializedBundle.blocks.ExtBlocks[1].Header.BlockType = BPLib_BlockType_Age;
    DeserializedBundle.blocks.ExtBlocks[1].BlockData.AgeBlockData.Age = 10;

    UT_SetDataBuffer(UT_KEY(BPLib_TIME_GetTimeDelta), &TimeElapsed, sizeof(TimeElapsed), false);

    NewAge = DeserializedBundle.blocks.ExtBlocks[1].BlockData.AgeBlockData.Age + TimeElapsed;

    Status = BPLib_EBP_UpdateExtensionBlocks(&DeserializedBundle);

    UtAssert_INT32_EQ(Status, BPLIB_SUCCESS);    
    UtAssert_EQ(uint64_t, DeserializedBundle.blocks.ExtBlocks[1].BlockData.AgeBlockData.Age,
                            NewAge);
    UtAssert_BOOL_TRUE(DeserializedBundle.blocks.ExtBlocks[1].Header.RequiresEncode);
}

/* Test that block updates succeed for previous node blocks */
void Test_BPLib_EBP_UpdateBlocks_PrevNode(void)
{
    BPLib_Status_t Status;
    BPLib_Bundle_t DeserializedBundle;

    memset(&DeserializedBundle, 0, sizeof(DeserializedBundle));

    DeserializedBundle.blocks.ExtBlocks[0].Header.BlockType = BPLib_BlockType_PrevNode;

    Status = BPLib_EBP_UpdateExtensionBlocks(&DeserializedBundle);

    UtAssert_INT32_EQ(Status, BPLIB_SUCCESS);
    UtAssert_STUB_COUNT(BPLib_EID_CopyEids, 1);
    UtAssert_BOOL_TRUE(DeserializedBundle.blocks.ExtBlocks[0].Header.RequiresEncode);
}

void TestBplibEbp_Register(void)
{
    ADD_TEST(Test_BPLib_EBP_InitBlocks_Null);
    ADD_TEST(Test_BPLib_EBP_InitBlocks_ChanIdErr);
    ADD_TEST(Test_BPLib_EBP_InitBlocks_PrevNode);
    ADD_TEST(Test_BPLib_EBP_InitBlocks_AgeCfg);
    ADD_TEST(Test_BPLib_EBP_InitBlocks_AgeAuto);
    ADD_TEST(Test_BPLib_EBP_InitBlocks_HopCount);

    ADD_TEST(Test_BPLib_EBP_UpdateBlocks_Null);
    ADD_TEST(Test_BPLib_EBP_UpdateBlocks_HopCount);
    ADD_TEST(Test_BPLib_EBP_UpdateBlocks_Age);
    ADD_TEST(Test_BPLib_EBP_UpdateBlocks_PrevNode);
}

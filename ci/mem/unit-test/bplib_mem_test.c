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
#include "bplib_mem_test_utils.h"

void Test_BPLib_MEM_PoolInit_Nominal(void)
{
    BPLib_MEM_Pool_t Pool;
    uint8_t MemBuff[80];
    size_t InitSize = 80;

    UtAssert_INT32_EQ(BPLib_MEM_PoolInit(&Pool, MemBuff, InitSize), BPLIB_SUCCESS);
}

void Test_BPLib_MEM_PoolInit_Null(void)
{
    BPLib_MEM_Pool_t Pool;
    uint8_t MemBuff[80];
    size_t InitSize = 80;

    UtAssert_INT32_EQ(BPLib_MEM_PoolInit(NULL, MemBuff, InitSize), BPLIB_NULL_PTR_ERROR);
    UtAssert_INT32_EQ(BPLib_MEM_PoolInit(&Pool, NULL, InitSize), BPLIB_NULL_PTR_ERROR);
    UtAssert_INT32_EQ(BPLib_MEM_PoolInit(&Pool, MemBuff, 0), BPLIB_NULL_PTR_ERROR);
}

void Test_BPLib_MEM_PoolDestroy_Nominal(void)
{
    BPLib_MEM_Pool_t Pool;

    UtAssert_VOIDCALL(BPLib_MEM_PoolDestroy(&Pool));
}

void Test_BPLib_MEM_PoolDestroy_Null(void)
{
    UtAssert_VOIDCALL(BPLib_MEM_PoolDestroy(NULL));
}

void Test_BPLib_MEM_BlockAlloc_Nominal(void)
{
    BPLib_MEM_Block_t *Block;
    BPLib_MEM_Pool_t Pool;

    memset(&Pool, 0, sizeof(Pool));

    Block = BPLib_MEM_BlockAlloc(&Pool, BPLIB_MEM_BIG_BLK_SIZE);

    UtAssert_NOT_NULL(Block);
    UtAssert_EQ(size_t, Block->used_len, 0);
    UtAssert_NULL(Block->next);

    UtAssert_VOIDCALL(BPLib_MEM_BlockFree(&Pool, Block));

    Block = BPLib_MEM_BlockAlloc(&Pool, BPLIB_MEM_SMALL_BLK_SIZE);

    UtAssert_NOT_NULL(Block);
    UtAssert_EQ(size_t, Block->used_len, 0);
    UtAssert_NULL(Block->next);

    UtAssert_VOIDCALL(BPLib_MEM_BlockFree(&Pool, Block));    
}

void Test_BPLib_MEM_BlockFree_Null(void)
{
    BPLib_MEM_Block_t Block;
    BPLib_MEM_Pool_t Pool;

    memset(&Pool, 0, sizeof(Pool));

    UtAssert_VOIDCALL(BPLib_MEM_BlockFree(NULL, &Block));    
    UtAssert_VOIDCALL(BPLib_MEM_BlockFree(&Pool, NULL));    
}

void Test_BPLib_MEM_BlockListAlloc_Nominal(void)
{
    BPLib_MEM_Block_t *HeadBlk;
    BPLib_MEM_Pool_t Pool;
    BPLib_MEM_Block_t Blk1, Blk2;

    memset(&Pool, 0, sizeof(Pool));

    #ifdef BPLIB_MEM_CFE_IMPL
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetPoolBuf), &Blk1, BPLIB_MEM_BIG_BLK_SIZE, false);
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetPoolBuf), &Blk2, BPLIB_MEM_BIG_BLK_SIZE, false);
    #endif

    HeadBlk = BPLib_MEM_BlockListAlloc(&Pool, BPLIB_MEM_BIG_BLK_DATA_SIZE * 2, BPLIB_MEM_BIG_BLK_SIZE);

    #ifdef BPLIB_MEM_CFE_IMPL
    UtAssert_ADDRESS_EQ(HeadBlk, &Blk1);
    UtAssert_ADDRESS_EQ(HeadBlk->next, &Blk2);
    #endif

    UtAssert_VOIDCALL(BPLib_MEM_BlockListFree(&Pool, HeadBlk));

    #ifdef BPLIB_MEM_CFE_IMPL
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetPoolBuf), &Blk1, BPLIB_MEM_SMALL_BLK_SIZE, false);
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetPoolBuf), &Blk2, BPLIB_MEM_SMALL_BLK_SIZE, false);
    #endif

    HeadBlk = BPLib_MEM_BlockListAlloc(&Pool, BPLIB_MEM_SMALL_BLK_SIZE * 2, BPLIB_MEM_SMALL_BLK_SIZE);

    #ifdef BPLIB_MEM_CFE_IMPL
    UtAssert_ADDRESS_EQ(HeadBlk, &Blk1);
    UtAssert_ADDRESS_EQ(HeadBlk->next, &Blk2);
    #endif

    UtAssert_VOIDCALL(BPLib_MEM_BlockListFree(&Pool, HeadBlk));    
}

void Test_BPLib_MEM_BlockListFree_Null(void)
{
    BPLib_MEM_Block_t Blk;
    BPLib_MEM_Pool_t Pool;

    memset(&Pool, 0, sizeof(Pool));

    UtAssert_VOIDCALL(BPLib_MEM_BlockListFree(NULL, &Blk));
    UtAssert_VOIDCALL(BPLib_MEM_BlockListFree(&Pool, NULL));
}

void Test_BPLib_MEM_BundleAlloc_Nominal(void)
{
    BPLib_MEM_Pool_t Pool;
    BPLib_MEM_Block_t Blk1, Blk2;
    BPLib_Bundle_t *Bundle;
    uint8_t Buff[BPLIB_MEM_BIG_BLK_DATA_SIZE];

    memset(&Pool, 0, sizeof(Pool));

    #ifdef BPLIB_MEM_CFE_IMPL
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetPoolBuf), &Blk1, sizeof(BPLib_MEM_Block_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetPoolBuf), &Blk2, sizeof(BPLib_MEM_Block_t), false);
    #endif

    Bundle = BPLib_MEM_BundleAlloc(&Pool, Buff, BPLIB_MEM_BIG_BLK_DATA_SIZE);

    UtAssert_ADDRESS_EQ(Bundle, Blk1.user_data.BigData);

    UtAssert_VOIDCALL(BPLib_MEM_BundleFree(&Pool, Bundle));
}

void Test_BPLib_MEM_GetBytesInUse_Nominal(void)
{
    BPLib_MEM_Pool_t Pool;

    memset(&Pool, 0, sizeof(Pool));
    
    UtAssert_EQ(size_t, BPLib_MEM_GetBytesInUse(&Pool), 0);
}

void Test_BPLib_MEM_GetBytesFree_Nominal(void)
{
    BPLib_MEM_Pool_t Pool;

    memset(&Pool, 0, sizeof(Pool));
    
    UtAssert_EQ(size_t, BPLib_MEM_GetBytesFree(&Pool), 0);
}

void Test_BPLib_MEM_GetBlockFromUserData_Nominal(void)
{
    BPLib_MEM_Block_t Block;

    UtAssert_ADDRESS_EQ(&Block, BPLib_MEM_GetBlockFromUserData(Block.user_data.BigData));
    UtAssert_ADDRESS_EQ(&Block, BPLib_MEM_GetBlockFromUserData(&Block.user_data.Bundle));
    UtAssert_ADDRESS_EQ(&Block, BPLib_MEM_GetBlockFromUserData(Block.user_data.SmallData));
    UtAssert_ADDRESS_EQ(&Block, BPLib_MEM_GetBlockFromUserData(&Block.user_data.DbEntry));
}


void Test_BPLib_MEM_CopyOutFromOffset_NullInputErrors(void)
{
    BPLib_MEM_Block_t FirstBlock;
    BPLib_Bundle_t Bundle;
    char OutputBuffer[2048];
    memset(&Bundle, 0, sizeof(Bundle));
    
    /* all three null */
    UtAssert_INT32_EQ(BPLib_MEM_CopyOutFromOffset(NULL, 0, 0, NULL, 0), BPLIB_NULL_PTR_ERROR);

    /* output buffer null */
    Bundle.blob = &FirstBlock;
    UtAssert_INT32_EQ(BPLib_MEM_CopyOutFromOffset(&Bundle, 0, 0, NULL, 0), BPLIB_NULL_PTR_ERROR);
    
    /* blob null */
    Bundle.blob = NULL;
    UtAssert_INT32_EQ(BPLib_MEM_CopyOutFromOffset(&Bundle, 0, 0, OutputBuffer, 0), BPLIB_NULL_PTR_ERROR);
}

void Test_BPLib_MEM_CopyOutFromOffset_BadSize(void)
{
    BPLib_MEM_Block_t FirstBlock;
    BPLib_Bundle_t Bundle;
    char OutputBuffer[2048];
    uint64_t NumBytesToCopy = 100;
    size_t OutputBufferSize = 10;
    memset(&Bundle, 0, sizeof(Bundle));

    Bundle.blob = &FirstBlock;
    UtAssert_INT32_EQ(BPLib_MEM_CopyOutFromOffset(&Bundle, 0, NumBytesToCopy, OutputBuffer, OutputBufferSize), BPLIB_BUF_LEN_ERROR);
}

void TestBplibMem_Register(void)
{
    ADD_TEST(Test_BPLib_MEM_PoolInit_Nominal);
    ADD_TEST(Test_BPLib_MEM_PoolInit_Null);

    ADD_TEST(Test_BPLib_MEM_PoolDestroy_Nominal);
    ADD_TEST(Test_BPLib_MEM_PoolDestroy_Null);

    ADD_TEST(Test_BPLib_MEM_BlockAlloc_Nominal);

    ADD_TEST(Test_BPLib_MEM_BlockFree_Null);

    ADD_TEST(Test_BPLib_MEM_BlockListAlloc_Nominal);

    ADD_TEST(Test_BPLib_MEM_BlockListFree_Null);

    ADD_TEST(Test_BPLib_MEM_BundleAlloc_Nominal);

    ADD_TEST(Test_BPLib_MEM_GetBytesInUse_Nominal);

    ADD_TEST(Test_BPLib_MEM_GetBytesFree_Nominal);

    ADD_TEST(Test_BPLib_MEM_GetBlockFromUserData_Nominal);

    ADD_TEST(Test_BPLib_MEM_CopyOutFromOffset_NullInputErrors);
    ADD_TEST(Test_BPLib_MEM_CopyOutFromOffset_BadSize);
}

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

void TestBplibCt_Register(void)
{
    ADD_TEST(Test_BPLib_CT_SetBundleId_Nominal);
    ADD_TEST(Test_BPLib_CT_SetBundleId_Null);
}

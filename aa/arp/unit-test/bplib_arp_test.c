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
** Include
*/

#include "bplib_arp_test_utils.h"

/*
** Test function for
** int BPLib_ARP_Init()
*/
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

void Test_BPLib_ARP_ProcessBsr(void)
{
    BPLib_Bundle_t    Bundle;
    BPLib_MEM_Block_t Block;

    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    memset(&Block, 0, sizeof(BPLib_MEM_Block_t));
    Bundle.blob = &Block;

    BPLib_ARP_ProcessBsr(&AdminRecord, &Bundle);

    BPLib_ARP_Test_VerifyIncrement(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD, 1, 1);
}

void Test_BPLib_ARP_ProcessCrs(void)
{
    BPLib_Bundle_t    Bundle;
    BPLib_MEM_Block_t Block;

    memset(&Bundle, 0, sizeof(BPLib_Bundle_t));
    memset(&Block, 0, sizeof(BPLib_MEM_Block_t));
    Bundle.blob = &Block;

    BPLib_ARP_ProcessBsr(&AdminRecord, &Bundle);

    BPLib_ARP_Test_VerifyIncrement(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD, 1, 1);
}

void Test_BPLib_ARP_ProcessCcs(void)
{
    BPLib_ARP_ProcessCcs(&AdminRecord);

    BPLib_ARP_Test_VerifyIncrement(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_ADMIN_RECORD,   1, 1);
    BPLib_ARP_Test_VerifyIncrement(BPLIB_EID_INSTANCE, BUNDLE_COUNT_RECEIVED_CUSTODY_SIGNAL, 1, 2);

    UtAssert_STUB_COUNT(BPLib_AS_Increment, 2);
}

void TestBplibArp_Register(void)
{
    ADD_TEST(Test_BPLib_ARP_Init);
    ADD_TEST(Test_BPLib_ARP_CRSTblValidateFunc_Nominal);
    ADD_TEST(Test_BPLib_ARP_CRSTblValidateFunc_Invalid);
    ADD_TEST(Test_BPLib_ARP_GetDispCodeIdx_Pos);
    ADD_TEST(Test_BPLib_ARP_GetDispCodeIdx_Neg);
    ADD_TEST(Test_BPLib_ARP_ProcessBsr);
    ADD_TEST(Test_BPLib_ARP_ProcessCrs);
    ADD_TEST(Test_BPLib_ARP_ProcessCcs);
}

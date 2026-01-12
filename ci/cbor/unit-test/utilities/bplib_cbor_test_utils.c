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

#include "bplib_cbor_test_utils.h"
#include "bplib_nc.h"

/* =========== */
/* Global Data */
/* =========== */

BPLib_NC_MibPerNodeConfig_t TestMibConfigPnTbl;
BPLib_Instance_t BplibInst;


/* ======== */
/* Handlers */
/* ======== */

void UT_Handler_BPLib_MEM_CopyOutFromOffset(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    void*    OutputBuffer;
    uint64_t NumBytesToCopy;
    
    OutputBuffer   = UT_Hook_GetArgValueByName(Context, "OutputBuffer", void*);
    NumBytesToCopy = UT_Hook_GetArgValueByName(Context, "NumBytesToCopy", uint64_t);

    memcpy(OutputBuffer, UserObj, NumBytesToCopy);
}

/* ==================== */
/* Function Definitions */
/* ==================== */

void BPLib_CBOR_Test_Setup(void)
{
    /* Initialize test environment to default state for every test */
    UT_ResetState(0);

    BPLib_NC_ConfigPtrs.MibPnConfigPtr = &TestMibConfigPnTbl;

    /* Set default max length to something excessively high for most tests */
    TestMibConfigPnTbl.Configs[PARAM_SET_MAX_BUNDLE_LENGTH] = 1000000;

    memset(&BplibInst, 0, sizeof(BplibInst));
}

void BPLib_CBOR_Test_Teardown(void)
{
    /* Clean up test environment */
}

void UtTest_Setup(void)
{
    TestBplibCborDecode_Register();
    TestBplibCborDecodeInternal_Register();
    
    TestBplibCborEncode_Register();
    TestBplibCborEncodePrevNode_Register();
    TestBplibCborEncodeInternal_Register();
}

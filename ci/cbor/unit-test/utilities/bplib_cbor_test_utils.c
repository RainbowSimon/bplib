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

/* ======== */
/* Includes */
/* ======== */

#include "bplib_cbor_test_utils.h"
#include "bplib_nc.h"

/* =========== */
/* Global Data */
/* =========== */

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

    /* Set default max length to something excessively high for most tests */
    UT_SetDefaultReturnValue(UT_KEY(BPLib_NC_GetNodeConfigValue), 1000000);

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

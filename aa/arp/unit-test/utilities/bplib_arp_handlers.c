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

#include "bplib_arp_handlers.h"
#include "bplib_mem.h"

/* ==================== */
/* Function Definitions */
/* ==================== */

void UT_Handler_BPLib_MEM_BundleAlloc(void *UserObj, UT_EntryKey_t FuncKey,
                                        const UT_StubContext_t *Context)
{
    void* Payload;

    Payload = UT_Hook_GetArgValueByName(Context, "blob_data", void*);
    memcpy(((BPLib_Bundle_t*) UserObj)->blob->user_data.BigData, Payload, sizeof(BPLib_MEM_UserData_t));

    UT_Stub_SetReturnValue(UT_KEY(BPLib_MEM_BundleAlloc), UserObj);
}

void UT_Handler_BPLib_EID_CopyEids(void *UserObj, UT_EntryKey_t FuncKey,
                                        const UT_StubContext_t *Context)
{
    BPLib_EID_t* EID_Destination;
    BPLib_EID_t  EID_Source;

    EID_Destination = UT_Hook_GetArgValueByName(Context, "EID_Destination", BPLib_EID_t*);
    EID_Source      = UT_Hook_GetArgValueByName(Context, "EID_Source", BPLib_EID_t);

    EID_Destination->Scheme       = EID_Source.Scheme;
    EID_Destination->IpnSspFormat = EID_Source.IpnSspFormat;
    EID_Destination->Allocator    = EID_Source.Allocator;
    EID_Destination->Node         = EID_Source.Node;
    EID_Destination->Service      = EID_Source.Service;
}
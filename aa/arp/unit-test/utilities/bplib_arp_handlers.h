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

#include "utstubs.h"
#include "bplib_mem.h"

/* ==================== */
/* Function Definitions */
/* ==================== */

/**
 * \brief Make BPLib_MEM_BundleAlloc return a bundle that can be evaluated in a
 *        unit test
 */
void UT_Handler_BPLib_MEM_BundleAlloc(void *UserObj, UT_EntryKey_t FuncKey, 
                                        const UT_StubContext_t *Context);

/**
 * \brief Actually copy EIDs so the unit tests can evaluate that the bundle has
 *        the admin record in the payload
 */
void UT_Handler_BPLib_EID_CopyEids(void *UserObj, UT_EntryKey_t FuncKey,
                                        const UT_StubContext_t *Context);
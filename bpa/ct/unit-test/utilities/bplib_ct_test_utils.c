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

/*
** Include
*/

#include "bplib_ct_test_utils.h"
#include "bplib_nc.h"

/*
** Global Data
*/

BPLib_Instance_t BplibInst;


/*
** Function Definitions
*/

void BPLib_CT_Test_Setup(void)
{
    BPLib_CLA_ContactsTable_t ContactsTable;

    /* Initialize the CCS size trigger */
    memset(&ContactsTable, 0, sizeof(BPLib_CLA_ContactsTable_t));
    ContactsTable.ContactSet[0].CSSizeTrigger = 40;
    ContactsTable.ContactSet[1].CSSizeTrigger = 30;
    ContactsTable.ContactSet[2].CSSizeTrigger = 20;

    BPLib_NC_ConfigPtrs.ContactsConfigPtr = &ContactsTable;

    /* Initialize test environment to default state for every test */
    UT_ResetState(0);

    memset(&BplibInst, 0, sizeof(BPLib_Instance_t));
    UT_SetHandlerFunction(UT_KEY(BPLib_AS_Increment), UT_Handler_BPLib_AS_Increment, NULL);
    UT_SetHandlerFunction(UT_KEY(BPLib_AS_Decrement), UT_Handler_BPLib_AS_Decrement, NULL);

}

void BPLib_CT_Test_Teardown(void)
{
    /* Clean up test environment */
}

void UtTest_Setup(void)
{
    TestBplibCt_Register();
    TestBplibCtInternal_Register();
}

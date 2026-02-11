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

/**
 * @file
 *
 * Auto-Generated stub implementations for functions defined in bplib_ct header
 */

#include "bplib_ct.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for BPLib_CT_AssignSeqCounter()
 * ----------------------------------------------------
 */
BPLib_Status_t BPLib_CT_AssignSeqCounter(BPLib_Instance_t *Inst, uint32_t ContactId)
{
    UT_GenStub_SetupReturnBuffer(BPLib_CT_AssignSeqCounter, BPLib_Status_t);

    UT_GenStub_AddParam(BPLib_CT_AssignSeqCounter, BPLib_Instance_t *, Inst);
    UT_GenStub_AddParam(BPLib_CT_AssignSeqCounter, uint32_t, ContactId);

    UT_GenStub_Execute(BPLib_CT_AssignSeqCounter, Basic, NULL);

    return UT_GenStub_GetReturnValue(BPLib_CT_AssignSeqCounter, BPLib_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for BPLib_CT_BuildAndSendOpenCcs()
 * ----------------------------------------------------
 */
void BPLib_CT_BuildAndSendOpenCcs(BPLib_Instance_t *Instance, BPLib_CT_OpenCcs_t *OpenCcs)
{
    UT_GenStub_AddParam(BPLib_CT_BuildAndSendOpenCcs, BPLib_Instance_t *, Instance);
    UT_GenStub_AddParam(BPLib_CT_BuildAndSendOpenCcs, BPLib_CT_OpenCcs_t *, OpenCcs);

    UT_GenStub_Execute(BPLib_CT_BuildAndSendOpenCcs, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for BPLib_CT_CheckCcsTimeout()
 * ----------------------------------------------------
 */
void BPLib_CT_CheckCcsTimeout(BPLib_Instance_t *Instance)
{
    UT_GenStub_AddParam(BPLib_CT_CheckCcsTimeout, BPLib_Instance_t *, Instance);

    UT_GenStub_Execute(BPLib_CT_CheckCcsTimeout, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for BPLib_CT_DeleteBundleFromCtdb()
 * ----------------------------------------------------
 */
void BPLib_CT_DeleteBundleFromCtdb(BPLib_Instance_t *Inst, uint32_t BundleId)
{
    UT_GenStub_AddParam(BPLib_CT_DeleteBundleFromCtdb, BPLib_Instance_t *, Inst);
    UT_GenStub_AddParam(BPLib_CT_DeleteBundleFromCtdb, uint32_t, BundleId);

    UT_GenStub_Execute(BPLib_CT_DeleteBundleFromCtdb, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for BPLib_CT_Init()
 * ----------------------------------------------------
 */
BPLib_Status_t BPLib_CT_Init(BPLib_Instance_t *Inst)
{
    UT_GenStub_SetupReturnBuffer(BPLib_CT_Init, BPLib_Status_t);

    UT_GenStub_AddParam(BPLib_CT_Init, BPLib_Instance_t *, Inst);

    UT_GenStub_Execute(BPLib_CT_Init, Basic, NULL);

    return UT_GenStub_GetReturnValue(BPLib_CT_Init, BPLib_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for BPLib_CT_ProcessCcs()
 * ----------------------------------------------------
 */
BPLib_Status_t BPLib_CT_ProcessCcs(BPLib_Instance_t *Inst, BPLib_CT_DeserializedCcs_t *Ccs)
{
    UT_GenStub_SetupReturnBuffer(BPLib_CT_ProcessCcs, BPLib_Status_t);

    UT_GenStub_AddParam(BPLib_CT_ProcessCcs, BPLib_Instance_t *, Inst);
    UT_GenStub_AddParam(BPLib_CT_ProcessCcs, BPLib_CT_DeserializedCcs_t *, Ccs);

    UT_GenStub_Execute(BPLib_CT_ProcessCcs, Basic, NULL);

    return UT_GenStub_GetReturnValue(BPLib_CT_ProcessCcs, BPLib_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for BPLib_CT_ProcessNewBundle()
 * ----------------------------------------------------
 */
BPLib_Status_t BPLib_CT_ProcessNewBundle(BPLib_Instance_t *Inst, BPLib_Bundle_t *Bundle)
{
    UT_GenStub_SetupReturnBuffer(BPLib_CT_ProcessNewBundle, BPLib_Status_t);

    UT_GenStub_AddParam(BPLib_CT_ProcessNewBundle, BPLib_Instance_t *, Inst);
    UT_GenStub_AddParam(BPLib_CT_ProcessNewBundle, BPLib_Bundle_t *, Bundle);

    UT_GenStub_Execute(BPLib_CT_ProcessNewBundle, Basic, NULL);

    return UT_GenStub_GetReturnValue(BPLib_CT_ProcessNewBundle, BPLib_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for BPLib_CT_SetBundleId()
 * ----------------------------------------------------
 */
BPLib_Status_t BPLib_CT_SetBundleId(BPLib_Bundle_t *Bundle)
{
    UT_GenStub_SetupReturnBuffer(BPLib_CT_SetBundleId, BPLib_Status_t);

    UT_GenStub_AddParam(BPLib_CT_SetBundleId, BPLib_Bundle_t *, Bundle);

    UT_GenStub_Execute(BPLib_CT_SetBundleId, Basic, NULL);

    return UT_GenStub_GetReturnValue(BPLib_CT_SetBundleId, BPLib_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for BPLib_CT_SignalCustody()
 * ----------------------------------------------------
 */
BPLib_Status_t BPLib_CT_SignalCustody(BPLib_Instance_t *Inst, BPLib_Bundle_t *Bundle,
                                      BPLib_CT_DispositionCode_t DispCode, bool IsDuplicate)
{
    UT_GenStub_SetupReturnBuffer(BPLib_CT_SignalCustody, BPLib_Status_t);

    UT_GenStub_AddParam(BPLib_CT_SignalCustody, BPLib_Instance_t *, Inst);
    UT_GenStub_AddParam(BPLib_CT_SignalCustody, BPLib_Bundle_t *, Bundle);
    UT_GenStub_AddParam(BPLib_CT_SignalCustody, BPLib_CT_DispositionCode_t, DispCode);
    UT_GenStub_AddParam(BPLib_CT_SignalCustody, bool, IsDuplicate);

    UT_GenStub_Execute(BPLib_CT_SignalCustody, Basic, NULL);

    return UT_GenStub_GetReturnValue(BPLib_CT_SignalCustody, BPLib_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for BPLib_CT_UpdateBundle()
 * ----------------------------------------------------
 */
BPLib_Status_t BPLib_CT_UpdateBundle(BPLib_Instance_t *Inst, BPLib_Bundle_t *Bundle)
{
    UT_GenStub_SetupReturnBuffer(BPLib_CT_UpdateBundle, BPLib_Status_t);

    UT_GenStub_AddParam(BPLib_CT_UpdateBundle, BPLib_Instance_t *, Inst);
    UT_GenStub_AddParam(BPLib_CT_UpdateBundle, BPLib_Bundle_t *, Bundle);

    UT_GenStub_Execute(BPLib_CT_UpdateBundle, Basic, NULL);

    return UT_GenStub_GetReturnValue(BPLib_CT_UpdateBundle, BPLib_Status_t);
}

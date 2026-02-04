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

#include "bplib_as_internal.h"
#include "bplib_as.h"
#include "bplib_version.h"

/* ================= */
/* Telemetry Packets */
/* ================= */

BPLib_NodeMibCountersHkTlm_Payload_t   BPLib_AS_NodeCountersPayload;   /** \brief Global node MIB counter payload */
BPLib_SourceMibCountersHkTlm_Payload_t BPLib_AS_SourceCountersPayload; /** \brief Global source MIB counter payload */
BPLib_NodeMibReportsHkTlm_Payload_t    BPLib_AS_NodeReportsPayload;    /** \brief Global node MIB reports payload */

/* =============== */
/* Mutex Variables */
/* =============== */
mutex_t BPLib_AS_Mutex;
BPLib_TIME_MonotonicTime_t InitTime;    /** \brief Time the node was initialized */

/* ==================== */
/* Function Definitions */
/* ==================== */

uint32_t BPLib_AS_GetCounterImpl(BPLib_EID_t *EID, BPLib_AS_Counter_t Counter)
{
    if (BPLib_EID_IsMatch(EID, &BPLIB_EID_INSTANCE))
    {
        return BPLib_AS_NodeCountersPayload.NodeCounters[Counter];
    }
    else
    {
        /* Source counters are unimplemented */
        return 0;
    }
}

BPLib_Status_t BPLib_AS_SetCounter(BPLib_EID_t EID, BPLib_AS_Counter_t Counter, uint32_t Value)
{
    BPLib_Status_t Status;

    Status = BPLIB_SUCCESS;

    if (BPLib_EID_IsValid(&EID))
    {
        if (BPLib_EID_IsMatch(&EID, &BPLIB_EID_INSTANCE))
        { /* EID denotes a node counter manipulation */
            if (Counter < BPLIB_AS_NUM_NODE_CNTRS)
            { // Counter is within range
                BPLib_AS_NodeCountersPayload.NodeCounters[Counter] = Value;
            }
            else
            { // Counter is out of valid range
                Status = BPLIB_AS_UNKNOWN_NODE_CNTR;
            }
        }
        // else
        // { /* EID denotes a source counter manipulation */
        //     if (Counter > -1 && Counter < BPLIB_AS_NUM_SOURCE_CNTRS)
        //     { // Counter is within range
        //         BPLib_AS_SourceCountersPayload.MibArray[SourceEid].SourceCounters[Counter] = Value;
        //     }
        //     else
        //     { // Counter is out of valid range
        //         Status = BPLIB_AS_UNKNOWN_SRC_CNTR;
        //     }
        // }
    }
    else
    {
        Status = BPLIB_INVALID_EID;
    }

    return Status;
}

BPLib_Status_t BPLib_AS_InitMutex(void)
{
    /* Instantiate a mutex for AS counters */
    mutex_init(&BPLib_AS_Mutex);

    /* Always return success as this cannot fail in RIOT */
    return BPLIB_SUCCESS;
}

void BPLib_AS_LockCounters(void)
{
    mutex_lock(&BPLib_AS_Mutex);
}

void BPLib_AS_UnlockCounters(void)
{
    mutex_unlock(&BPLib_AS_Mutex);
}

/* Initialize the static MIB reports telemetry values */
void BPLib_AS_InitializeReportsHkTlm(void)
{
    InitTime.Time = BPLib_TIME_GetMonotonicTime();
    InitTime.BootEra = BPLib_TIME_GetBootEra();

    memset(&BPLib_AS_NodeReportsPayload, 0, sizeof(BPLib_AS_NodeReportsPayload));

    BPLib_AS_NodeReportsPayload.NodeStartupCounter = InitTime.BootEra;

    BPLib_AS_NodeReportsPayload.BundleAgentAvailableStorage = BPLIB_MAX_STORED_BUNDLE_BYTES / 1000;

    strncpy(BPLib_AS_NodeReportsPayload.SystemNodeName, 
                                BPLIB_SYSTEM_NODE_NAME, BPLIB_MAX_STR_LENGTH);
    strncpy(BPLib_AS_NodeReportsPayload.SystemNodeOwner, 
                                BPLIB_SYSTEM_NODE_OWNER, BPLIB_MAX_STR_LENGTH);
    strncpy(BPLib_AS_NodeReportsPayload.SystemSoftwareExec, 
                                BPLIB_SYSTEM_SOFWARE_EXEC, BPLIB_MAX_STR_LENGTH);
    strncpy(BPLib_AS_NodeReportsPayload.SystemSoftwareExecVersion, 
                                BPLIB_SYSTEM_SOFTWARE_EXEC_VERSION, BPLIB_MAX_STR_LENGTH);
    strncpy(BPLib_AS_NodeReportsPayload.ParamSupportedCLAs, 
                                BPLIB_SUPPORTED_CLAS, BPLIB_MAX_STR_LENGTH);

    snprintf(BPLib_AS_NodeReportsPayload.BundleAgentSoftwareVersion, 
                                BPLIB_MAX_STR_LENGTH, "v%u.%u.%u-sprint-%u",
                                BPLIB_MAJOR_VERSION, BPLIB_MINOR_VERSION, 
                                BPLIB_REVISION, BPLIB_BUILD_NUMBER);
}

void BPLib_AS_UpdateReportsHkTlm(BPLib_Instance_t *Inst)
{
    BPLib_AS_NodeReportsPayload.SystemNodeUpTime = BPLib_TIME_GetMonotonicTime() - InitTime.Time;
    BPLib_AS_NodeReportsPayload.BundleCountStored = Inst->BundleStorage.BundleCountStored;
    BPLib_AS_NodeReportsPayload.KbytesCountStorageAvailable = (BPLIB_MAX_STORED_BUNDLE_BYTES - Inst->BundleStorage.BytesStorageInUse) / 1000;
}

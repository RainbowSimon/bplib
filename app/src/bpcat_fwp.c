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
#include "bpcat_fwp.h"
#include "bplib.h"

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>

int64_t BPA_TIMEP_GetMonotonicTime(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    {
        perror("clock_gettime MONOTONIC");
        return 0;
    }
    return (int64_t)(ts.tv_sec) * 1000 + (int64_t)(ts.tv_nsec) / 1000000;
}

int64_t BPA_TIMEP_GetHostTime(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) == -1) {
        perror("gettimeofday");
        return 0;
    }

    return (int64_t)(tv.tv_sec) * 1000 + (int64_t)(tv.tv_usec) / 1000;
}

void BPA_TIMEP_GetHostEpoch(BPLib_TIME_Epoch_t *Epoch)
{
    if (Epoch != NULL)
    {
        Epoch->Year   = 1970;
        Epoch->Day    = 1;
        Epoch->Hour   = 1;
        Epoch->Minute = 0;
        Epoch->Second = 0;
        Epoch->Msec   = 0;
    }
}

BPLib_TIME_ClockState_t BPA_TIMEP_GetHostClockState(void)
{
    return BPLIB_TIME_CLOCK_VALID;
}

void BPA_PERFLOGP_Entry(uint32_t PerfLogID)
{
    return;
}

void BPA_PERFLOGP_Exit(uint32_t PerfLogID)
{
    return;
}

BPLib_Status_t BPA_TABLEP_TableInit(void)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_TABLEP_TableUpdate(uint8_t TableType, void** TblPtr)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_EVP_Init()
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_EVP_SendEvent(uint16_t EventID, BPLib_EM_EventType_t EventType, char const* Spec)
{
    printf("Event Type: %d, Event ID: %d, Event Text: %s\n", EventID, EventType, Spec);
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_TLMP_SendNodeMibConfigPkt(BPLib_NodeMibConfigHkTlm_Payload_t* NodeMIBConfigTlmPayload)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_TLMP_SendPerSourceMibConfigPkt(BPLib_SourceMibConfigHkTlm_Payload_t* SrcMIBConfigTlmPayload)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_TLMP_SendNodeMibReportsPkt(BPLib_NodeMibReportsHkTlm_Payload_t* NodeMIBReportsTlmPayload)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_TLMP_SendNodeMibCounterPkt(BPLib_NodeMibCountersHkTlm_Payload_t* NodeMIBCounterTlmPayload)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_TLMP_SendPerSourceMibCounterPkt(BPLib_SourceMibCountersHkTlm_Payload_t* SrcMIBCounterTlmPayload)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_TLMP_SendChannelContactPkt(BPLib_ChannelContactStatHkTlm_Payload_t* ChannelContactTlmPayload)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_TLMP_SendStoragePkt(BPLib_StorageHkTlm_Payload_t* StorTlmPayload)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_ADUP_AddApplication(uint32_t ChanId)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_ADUP_StartApplication(uint32_t ChanId)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_ADUP_StopApplication(uint32_t ChanId)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_ADUP_RemoveApplication(uint32_t ChanId)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_CLAP_ContactSetup(uint32_t ContactId)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_CLAP_ContactStart(uint32_t ContactId)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPA_CLAP_ContactStop(uint32_t ContactId)
{
    return BPLIB_SUCCESS;
}

void BPA_CLAP_ContactTeardown(uint32_t ContactId)
{
    return;
}

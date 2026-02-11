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

/*******************************************************************************
** Includes
*/
#include "bpcat_types.h"
#include "bpcat_task.h"
#include "bpcat_cla.h"
#include "bpcat_nc.h"
#include "bplib.h"
#include "bpcat_fwp.h"

#include "osapi.h"
#include <unistd.h>

/*******************************************************************************
** Configuration Definitions
*/
#define BPCAT_NUM_GEN_WORKER            1
#define BPCAT_GEN_WORKER_TIMEOUT        100u
#define BPCAT_MEMPOOL_LEN               8000000u
#define BPCAT_QM_MAX_JOBS               1024u
#define BPCAT_JOBS_PER_CYCLE            100

/*******************************************************************************
** Global State
*/
BPCat_AppData_t AppData;
static BPCat_Task_t CLAOutTask;
static BPCat_Task_t CLAInTask;
static BPCat_Task_t GenWorkers[BPCAT_NUM_GEN_WORKER];

/*******************************************************************************
** Generic Worker Task Functions
*/
static BPLib_Status_t BPCat_GenWorkerTaskSetup()
{
    /* Generic Worker does not need any pre-task setup */
    printf("BPLib generic-worker reporting for duty\n");
    return BPCAT_SUCCESS;
}

static BPLib_Status_t BPCat_GenWorkerTaskTeardown()
{
    /* Generic Worker does not need any post-task teardown */
    return BPCAT_SUCCESS;
}

static void* BPCat_GenWorkerTaskFunc(BPCat_AppData_t* gAppData)
{
    int WorkerID; // DO NOT MERGE THIS: THIS ONLY WORKS IF THERE'S ONE WORKER ID!!!
    if (BPLib_QM_RegisterWorker(&gAppData->BPLibInst, &WorkerID) != BPLIB_SUCCESS)
    {
        return NULL;
    }

    while (gAppData->Running)
    {
        BPLib_QM_WorkerRunJob(&gAppData->BPLibInst, 0, BPCAT_GEN_WORKER_TIMEOUT);
    }
    return NULL;
}

/*******************************************************************************
** Task Start/Stop
*/
static BPCat_Status_t BPCat_StartTasks()
{
    int i;
    BPCat_Status_t Status;

    /* Genworkers TaskInit */
    for (i = 0; i < BPCAT_NUM_GEN_WORKER; i++)
    {
        GenWorkers[i].TaskSetup = BPCat_GenWorkerTaskSetup;
        GenWorkers[i].TaskTeardown = BPCat_GenWorkerTaskTeardown;
        GenWorkers[i].TaskFunc = BPCat_GenWorkerTaskFunc;
        Status = BPCat_TaskInit(&GenWorkers[i]);
        if (Status != BPCAT_SUCCESS)
        {
            fprintf(stderr, "Failed to initialize Generic Worker Task\n");
            return Status;
        }
    }

    /* CLA TaskInit */
    CLAOutTask.TaskSetup = BPCat_CLAOutSetup;
    CLAOutTask.TaskTeardown = BPCat_CLAOutTeardown;
    CLAOutTask.TaskFunc = BPCat_CLAOutTaskFunc;
    CLAOutTask.TaskId = 0;
    Status = BPCat_TaskInit(&CLAOutTask);
    if (Status != BPCAT_SUCCESS)
    {
        fprintf(stderr, "Failed to initialize CLA-Egress Task\n");
        return Status;
    }
    CLAInTask.TaskSetup = BPCat_CLAInSetup;
    CLAInTask.TaskTeardown = BPCat_CLAOutTeardown;
    CLAInTask.TaskFunc = BPCat_CLAInTaskFunc;
    CLAInTask.TaskId = 0;
    Status = BPCat_TaskInit(&CLAInTask);
    if (Status != BPCAT_SUCCESS)
    {
        fprintf(stderr, "Failed to initialize CLA-Ingress Task\n");
        return Status;
    }

    /* Start the generic workers first so BPLib is ready to do work */
    for (i = 0; i < BPCAT_NUM_GEN_WORKER; i++)
    {
        Status = BPCat_TaskStart(&GenWorkers[i], &AppData);
        if (Status != BPCAT_SUCCESS)
        {
            fprintf(stderr, "Failed to start Generic Worker Task\n");
            return Status;
        }
    }

    /* Start the CLAs */
    Status = BPCat_TaskStart(&CLAOutTask, &AppData);
    if (Status != BPCAT_SUCCESS)
    {
        fprintf(stderr, "Failed to start CLA-Egress Task\n");
        return Status;
    }
    Status = BPCat_TaskStart(&CLAInTask, &AppData);
    if (Status != BPCAT_SUCCESS)
    {
        fprintf(stderr, "Failed to start CLA-Ingress Task\n");
        return Status;
    }

    return BPCAT_SUCCESS;
}

static void BPCat_StopTasks()
{
    int i;
    BPCat_Status_t Status;

    /* Stop CLA Tasks */
    Status = BPCat_TaskStop(&CLAOutTask);
    if (Status != BPCAT_SUCCESS)
    {
        fprintf(stderr, "Failed to stop CLA-Egress Task\n");
    }
    Status = BPCat_TaskStop(&CLAInTask);
    if (Status != BPCAT_SUCCESS)
    {
        fprintf(stderr, "Failed to stop CLA-Ingress Task\n");
    }

    /* Stop Generic Workers */
    for (i = 0; i < BPCAT_NUM_GEN_WORKER; i++)
    {
        Status = BPCat_TaskStop(&GenWorkers[i]);
        if (Status != BPCAT_SUCCESS)
        {
            fprintf(stderr, "Failed to stop generic worker\n");
        }
    }

    return;
}

/*******************************************************************************
** Main
**  Note: Because BPLib is dependent on OSAL, we have to use OS_Application_Startup()
**  to enter into this function 
*/
void BPCat_Main()
{
    BPLib_Status_t BPLibStatus;
    BPCat_Status_t Status;

    BPLib_FWP_ProxyCallbacks_t Callbacks = {
        /* Time Proxy */
        .BPA_TIMEP_GetMonotonicTime          = BPA_TIMEP_GetMonotonicTime,
        .BPA_TIMEP_GetHostEpoch              = BPA_TIMEP_GetHostEpoch,
        .BPA_TIMEP_GetHostClockState         = BPA_TIMEP_GetHostClockState,
        .BPA_TIMEP_GetHostTime               = BPA_TIMEP_GetHostTime,
        /* Perf Log Proxy */
        .BPA_PERFLOGP_Entry                  = BPA_PERFLOGP_Entry,
        .BPA_PERFLOGP_Exit                   = BPA_PERFLOGP_Exit,
        /* Table Proxy */
        .BPA_TABLEP_TableInit                = BPA_TABLEP_TableInit,
        .BPA_TABLEP_TableUpdate              = BPA_TABLEP_TableUpdate,
        /* Event Proxy */
        .BPA_EVP_Init                        = BPA_EVP_Init,
        .BPA_EVP_SendEvent                   = BPA_EVP_SendEvent,
        /* ADU Proxy */
        .BPA_ADUP_AddApplication             = BPA_ADUP_AddApplication,
        .BPA_ADUP_StartApplication           = BPA_ADUP_StartApplication,
        .BPA_ADUP_StopApplication            = BPA_ADUP_StopApplication,
        .BPA_ADUP_RemoveApplication          = BPA_ADUP_RemoveApplication,
        /* Telemetry Proxy */
        .BPA_TLMP_SendNodeMibConfigPkt       = BPA_TLMP_SendNodeMibConfigPkt,
        .BPA_TLMP_SendPerSourceMibConfigPkt  = BPA_TLMP_SendPerSourceMibConfigPkt,
        .BPA_TLMP_SendNodeMibCounterPkt      = BPA_TLMP_SendNodeMibCounterPkt,
        .BPA_TLMP_SendPerSourceMibCounterPkt = BPA_TLMP_SendPerSourceMibCounterPkt,
        .BPA_TLMP_SendNodeMibReportsPkt      = BPA_TLMP_SendNodeMibReportsPkt,
        .BPA_TLMP_SendChannelContactPkt      = BPA_TLMP_SendChannelContactPkt,
        .BPA_TLMP_SendStoragePkt             = BPA_TLMP_SendStoragePkt,
        /* CLA Proxy */
        .BPA_CLAP_ContactSetup               = BPA_CLAP_ContactSetup,
        .BPA_CLAP_ContactStart               = BPA_CLAP_ContactStart,
        .BPA_CLAP_ContactStop                = BPA_CLAP_ContactStop,
        .BPA_CLAP_ContactTeardown            = BPA_CLAP_ContactTeardown,
    };

    /* MEM */
    AppData.PoolMem = calloc(BPCAT_MEMPOOL_LEN, 1);
    if (AppData.PoolMem == NULL)
    {
        fprintf(stderr, "Failed to calloc() memory for the BPLib Memory Pool\n");
        return;
    }

    /* Node Config */
    Status = BPCat_NC_Init(&AppData.ConfigPtrs, (void*) &Callbacks, &(AppData.BPLibInst), BPCAT_QM_MAX_JOBS, AppData.PoolMem, (size_t) BPCAT_MEMPOOL_LEN);
    if (Status != BPCAT_SUCCESS)
    {
        fprintf(stderr, "Failed to init NC\n");
        return;
    }

    /* Start CLAs and Gen Workers */
    Status = BPCat_StartTasks();
    if (Status != BPCAT_SUCCESS)
    {
        fprintf(stderr, "Failed to start BPCat tasks\n");
        return;
    }

    /* Enable Contacts */
    if (BPLib_CLA_ContactSetup(&AppData.BPLibInst, 0) != BPLIB_SUCCESS || 
        BPLib_CLA_ContactStart(&AppData.BPLibInst, 0) != BPLIB_SUCCESS)
    {
        fprintf(stderr, "Failed to setup and start contact 0\n");
    }

    /* Run until a SIGINT (CTRL-C) sets AppData.Running to 0 */
    while (AppData.Running)
    {
        sleep(BPCAT_CYCLE_TIME_SECS);

        BPLibStatus = BPLib_STOR_FlushPending(&AppData.BPLibInst);

        if (BPLibStatus != BPLIB_SUCCESS)
        {
            fprintf(stderr, "Error flushing storage\n");
        }
        
        BPLibStatus = BPLib_STOR_GarbageCollect(&AppData.BPLibInst);
        
        if (BPLibStatus != BPLIB_SUCCESS)
        {
            fprintf(stderr, "Error garbage collecting\n");
        }
    }   

    /* Exit Signal Received */

    /* Cleanup */
    BPCat_StopTasks();
    BPLib_QM_QueueTableDestroy(&AppData.BPLibInst);
    free(AppData.PoolMem);
}

void SigHandler(int signo)
{
    if (signo == SIGINT)
    {
        AppData.Running = 0;
    }
}

void OS_Application_Startup()
{
    OS_API_Init();

    AppData.Running = 1;
    if (signal(SIGINT, SigHandler) == SIG_ERR) {
        fprintf(stderr, "Failed to register signal handler for SIGINT.\n");
        return;
    }

    BPCat_Main();
}

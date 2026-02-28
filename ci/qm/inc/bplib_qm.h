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

#ifndef BPLIB_QM_H
#define BPLIB_QM_H

#include "bplib_api_types.h"
#include "bplib_qm_waitqueue.h"
#include "bplib_qm_job.h"

#define QM_STANDARD_WAIT    100L /**< Standard wait of 100msec */
#define QM_NO_WAIT          0L   /**< Constant representing no wait */
#define QM_WAIT_FOREVER    -1L   /**< Constant representing an indefinite wait */
#define QM_MAX_GEN_WORKERS  8L   /**< Constant representing maximum allowed generic workers */


typedef struct BPLib_QM_WorkerState
{
    BPLib_QM_Job_t CurrJob;
} BPLib_QM_WorkerState_t;

/**
 * @brief Initializes the Queue Table for a specific instance.
 * 
 * This function initializes the memory and queues for the Queue Manager instance.
 * 
 * @param[in] inst The instance to be initialized.
 * @param[in] MaxJobs The maximum number of jobs that can be queued.
 * 
 * @return Status of the initialization (success or failure).
 */
BPLib_Status_t BPLib_QM_QueueTableInit(BPLib_Instance_t* inst, size_t MaxJobs);

/**
 * @brief Destroys the Queue Table for a specific instance.
 * 
 * This function cleans up and frees the memory used by the instance's job queues.
 * 
 * @param[in] inst The instance to be destroyed.
 */
void BPLib_QM_QueueTableDestroy(BPLib_Instance_t* inst);

/**
 * @brief Executes a single job in the Queue Manager.
 * 
 * This function runs a job, potentially blocking until the specified timeout.
 * This function is intended to be called from a generic worker thread.
 * 
 * @param[in] inst The instance where the job is to be run.
 * @param[out] WorkerID The returned ID of this worker, which should be passed to WorkerRunJob
 * 
 * @return Status of worker registration
 */
BPLib_Status_t BPLib_QM_RegisterWorker(BPLib_Instance_t* inst, int32_t* WorkerID);

/**
 * @brief Executes a single job in the Queue Manager.
 * 
 * This function runs a job, potentially blocking until the specified timeout.
 * This function is intended to be called from a generic worker thread.
 * 
 * @param[in] inst The instance where the job is to be run.
 * @param[in] WorkerID The ID of the worker, give at init by BPLIB_QM_RegisterWorker();
 * @param[in] TimeoutMs Timeout in milliseconds to wait for a new job to be available.
 * 
 * @return Status of running the job
 */
BPLib_Status_t BPLib_QM_WorkerRunJob(BPLib_Instance_t* inst, int32_t WorkerID, int TimeoutMs);

/**
 * @brief Check if system is idle
 * 
 * This function checks if any of the ingress or egress queues are active and only returns
 * true if they are all inactive.
 * 
 * @param[in] Inst The bplib instance
 * 
 * @return Whether the system is idle or not
 */
bool BPLib_QM_IsSystemIdle(BPLib_Instance_t* Inst);

/**
 * @brief Check if ingress is idle
 * 
 * This function checks if the jobs queue (effectively the ingress queue) is empty and if
 * so, returns true
 * 
 * @param[in] Inst The bplib instance
 * 
 * @return Whether ingress is idle or not
 */
bool BPLib_QM_IsIngressIdle(BPLib_Instance_t* Inst);

/**
 * @brief Check if a duct is active
 * 
 * This function checks if a given egress duct is set to a channel_started/contact_started
 * state and if so, if the duct contains anything. If both cases are true, the duct is
 * active
 * 
 * @param[in] Inst The bplib instance
 * @param[in] EgressID Corresponds to either a contact or channel ID for an egress duct
 * @param[in] LocalDelivery Identifies the duct as either a channel duct (local) or a 
 *                          contact duct (remote)
 * 
 * @return Whether the duct is active or not
 */
bool BPLib_QM_IsDuctActive(BPLib_Instance_t* Inst, uint32_t EgressID, bool LocalDelivery);


BPLib_Status_t BPLib_QM_DuctPull(BPLib_Instance_t* Inst, uint32_t EgressID, bool LocalDelivery,
    int TimeoutMs, BPLib_Bundle_t** RetBundle);

/**
 * @brief Adds a job to the queue.
 * 
 * This function adds a job to the job queue, with the specified state 
 * and priority, and a timeout for processing.
 * 
 * @param[in] inst The instance to which the job is to be added.
 * @param[in] bundle The bundle associated with the job.
 * @param[in] state The initial state of the job.
 * @param[in] priority The priority of the job.
 * @param[in] TimeoutMs Timeout in milliseconds for adding the job.
 * 
 * @return Status of the job addition (success or failure).
 */
BPLib_Status_t BPLib_QM_CreateJob(BPLib_Instance_t* inst, BPLib_Bundle_t* bundle,
    BPLib_QM_JobState_t state, BPLib_QM_Priority_t priority, int TimeoutMs);


#endif /* BPLIB_QM_H */

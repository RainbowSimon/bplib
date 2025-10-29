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

#ifndef BPLIB_INST_H
#define BPLIB_INST_H

#include "bplib_api_types.h"
#include "bplib_cfg.h"
#include "bplib_qm.h"
#include "bplib_as.h"
#include "bplib_mem.h"
#include "bplib_ct.h"
#include "bplib_stor.h"


/*
** Types
*/

/**
 * @struct BPLib_Instance
 * @brief Represents an instance of bplib
 * 
 * This structure holds the necessary data that bplib needs to know its current state.
 * There's still work to go to make multiple instances of bplib possible but eventually
 * all state information will be held in here.
 */
struct BPLib_Instance
{
    BPLib_MEM_Pool_t pool; /**< Memory pool for this BPLib Instance */

    /* Worker Management */
    pthread_mutex_t RegisteredWorkersLock; // Move to bplib_os
    BPLib_QM_WorkerState_t RegisteredWorkers[QM_MAX_GEN_WORKERS];
    size_t NumWorkers;

    /* Queues */
    BPLib_QM_WaitQueue_t GenericWorkerJobs; /**< Queue of jobs */
    BPLib_QM_WaitQueue_t BundleCacheList; /**< Queue of bundles in cache */
    BPLib_QM_WaitQueue_t ContactEgressJobs[BPLIB_MAX_NUM_CONTACTS]; /**< Queue of contact egress jobs */
    BPLib_QM_WaitQueue_t ChannelEgressJobs[BPLIB_MAX_NUM_CHANNELS]; /**< Queue of channel egress jobs */

    /* Bundle Storage */
    BPLib_BundleCache_t BundleStorage;

    /* Admin statistics context */
    BLib_AS_Context_t As;

    /* Custody transfer context */
    BPLib_CT_Context_t Ct;
};

#endif /* BPLIB_INST_H */

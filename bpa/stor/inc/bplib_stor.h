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

#ifndef BPLIB_STOR_H
#define BPLIB_STOR_H

/* ======== */
/* Includes */
/* ======== */

#include "bplib_api_types.h"
#include "bplib_cfg.h"
#include "bplib_eid.h"
#include "bplib_mem.h"
#include "bplib_stor_loadbatch.h"
#include "bplib_stor_sql.h"
#include "bplib_ct.h"

/* ====== */
/* Macros */
/* ====== */

#ifndef BPLIB_STOR_INSERTBATCHSIZE
#define BPLIB_STOR_INSERTBATCHSIZE 100
#endif

#ifndef BPLIB_STOR_DISCARDBATCHSIZE
#define BPLIB_STOR_DISCARDBATCHSIZE 25000
#endif

/* We conditionally allow this to be defined by a compile time variable
** so that the unit tests can pass in :memory: here and avoid using the disk
*/
#ifndef BPLIB_STOR_DBNAME
#define BPLIB_STOR_DBNAME       "bplib-storage.db"
#endif

#define BPLIB_STOR_MAX_IDLE_TIME        (4000u)

/* ======== */
/* Typedefs */
/* ======== */

struct BPLib_BundleCache
{
    pthread_mutex_t        lock;
    sqlite3*               db;
    int64_t                BundleRowsToExpire[BPLIB_STOR_DISCARDBATCHSIZE];
    BPLib_Bundle_t*        InsertBatch[BPLIB_STOR_INSERTBATCHSIZE];
    size_t                 InsertBatchSize;
    BPLib_STOR_LoadBatch_t ChannelLoadBatches[BPLIB_MAX_NUM_CHANNELS];
    BPLib_STOR_LoadBatch_t ContactLoadBatches[BPLIB_MAX_NUM_CONTACTS];
    int64_t                LastActiveTime;
    size_t                 BundleCountNotEgressed;  /* Number of bundles in storage that have not been marked as egressed */

    /* Storage-related MIB reports */
    uint32_t BundleCountStored;
    size_t   BytesStorageInUse;
    size_t   StorageSize;
};

/**
 * \brief Storage housekeeping payload
 */

typedef struct BPLib_StorageHkTlm_Payload BPLib_StorageHkTlm_Payload_t;

struct BPLib_StorageHkTlm_Payload
{
    size_t  BytesMemInUse;     /** \brief Bytes in memory that are in use */
    size_t  BytesMemFree;      /** \brief Number of bytes free */
    size_t  BytesMemHighWater; /** \brief Memory high water mark in bytes */
    size_t  KbStorageInUse;    /** \brief Kilobytes of storage currently in use */
    size_t  KbBundlesInStor;   /** \brief Kilobytes of storage currently occupied by bundles */
    int64_t MonotonicTime;     /** \brief Monotonic Time Counter */
    int64_t CorrelationFactor; /** \brief Time Correlation Factor */
};

/*
** Storage Policy Table
*/
typedef struct
{
    BPLib_EID_Pattern_t SrcEIDs[BPLIB_MAX_NUM_STORE_EIDS];
    uint32_t            StorageSize;
} BPLib_STOR_StorageSet_t;

typedef struct
{
    BPLib_STOR_StorageSet_t StorageSet[BPLIB_MAX_NUM_STORE_SET];
} BPLib_STOR_StorageTable_t;

/*
** Externs
*/

extern BPLib_StorageHkTlm_Payload_t BPLib_STOR_StoragePayload;

/*
** Exported Functions
*/

/**
 * \brief Storage initialization
 *
 *  \par Description
 *       STOR initialization function
 *
 *  \par Assumptions, External Events, and Notes:
 *
 *  \param[in] Inst Pointer to BPLib Instance, which contains cache instance within
 *
 *  \return Execution status
 *  \retval BPLIB_SUCCESS Initialization was successful
 */
BPLib_Status_t BPLib_STOR_Init(BPLib_Instance_t* Inst);


void BPLib_STOR_Destroy(BPLib_Instance_t* Inst);

/**
 * \brief Validate Storage Table configurations
 *
 *  \par Description
 *       Validate configuration table parameters
 *
 *  \par Assumptions, External Events, and Notes:
 *       - This function is called by whatever external task handles table management.
 *         Every time a new Storage table is loaded, this function should be called to
 *         validate its parameters.
 *
 *  \param[in] TblData Pointer to the config table
 *
 *  \return Execution status
 *  \retval BPLIB_SUCCESS Validation was successful
 */
BPLib_Status_t BPLib_STOR_StorageTblValidateFunc(void *TblData);

BPLib_Status_t BPLib_STOR_StoreBundle(BPLib_Instance_t* Inst, BPLib_Bundle_t* Bundle);

BPLib_Status_t BPLib_STOR_FlushPending(BPLib_Instance_t* Inst);

BPLib_Status_t BPLib_STOR_EgressForID(BPLib_Instance_t* Inst, uint32_t EgressID,
                                        bool LocalDelivery, size_t* NumEgressed);

BPLib_Status_t BPLib_STOR_GarbageCollect(BPLib_Instance_t* Inst);

/**
 * \brief Update values in the STOR housekeeping packet with values of the
 *        BPLib_Instance_t representing the current iteration of FSW
 * \param[in] Inst Instance variable that represents the current system
 * \return void
 */
void BPLib_STOR_UpdateHkPkt(BPLib_Instance_t* Inst);

BPLib_Status_t BPLib_STOR_FlushPendingUnlocked(BPLib_Instance_t* Inst);

BPLib_Status_t BPLib_STOR_Cleanup(BPLib_Instance_t* Inst);

BPLib_Status_t BPLib_STOR_UpdateCustodialBundles(BPLib_Instance_t* Inst, BPLib_CT_CcsUpdateBatch_t *Batch);

BPLib_Status_t BPLib_STOR_SetNewRetransmitTrigger(BPLib_Instance_t *Inst, uint32_t ContactId);

void BPLib_STOR_SetLastActiveTime(BPLib_Instance_t* Inst);

bool BPLib_STOR_IsIngressEgressActive(BPLib_Instance_t* Inst);


/**
 * \brief Egress bundles from storage and into the relevant egress queue in node 
 *        memory. This is a multi-step operation that expects this function to be 
 *        called at a regular wakeup cadence, and different operations are performed
 *        each time. The operations are:
 *            - Load a batch of bundle IDs that match a viable contact/channel into memory
 *            - Load the bundle ID's full bundle into memory and place it on the relevant
 *              queue. 
 *            - Once a load batch has been fully loaded into memory, mark the bundles
 *              in storage as egressed for future deletion
 * \param[in] Inst Instance variable that represents the current system
 * \param[in] MaxBundles Maximum number of bundles that can be egressed per call
 *
 * \return Execution status
 * \retval BPLIB_SUCCESS Validation was successful
 */
BPLib_Status_t BPLib_STOR_Egress(BPLib_Instance_t *Instance, size_t MaxBundles);

#endif /* BPLIB_STOR_H */

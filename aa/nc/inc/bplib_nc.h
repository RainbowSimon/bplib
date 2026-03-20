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

#ifndef BPLIB_NC_H
#define BPLIB_NC_H

/* ======== */
/* Includes */
/* ======== */

#include "bplib_api_types.h"
#include "bplib_nc_payloads.h"
#include "bplib_pi.h"
#include "bplib_cla.h"
#include "bplib_arp.h"
#include "bplib_pdb.h"
#include "bplib_stor.h"
#include "bplib_nc_directives.h"

/* ======== */
/* Typedefs */
/* ======== */

typedef struct
{
    BPLib_PI_ChannelTable_t*     ChanConfigPtr;
    BPLib_CLA_ContactsTable_t*   ContactsConfigPtr;
    BPLib_ARP_CRSTable_t*        CrsConfigPtr;
    BPLib_PDB_CustodianTable_t*  CustodianConfigPtr;
    BPLib_PDB_CustodyTable_t*    CustodyConfigPtr;
    BPLib_NC_MibPerNodeConfig_t* MibPnConfigPtr;
    BPLib_NC_MIBConfigPSTable_t* MibPsConfigPtr;
    BPLib_PDB_ReportToTable_t*   ReportConfigPtr;
    BPLib_PDB_SrcAuthTable_t*    AuthConfigPtr;
    BPLib_PDB_SrcLatencyTable_t* LatConfigPtr;
    BPLib_STOR_StorageTable_t*   StorConfigPtr;
} BPLib_NC_ConfigPtrs_t;

/**
  * \brief Channel application state
  */
typedef enum
{
    BPLIB_NC_APP_STATE_REMOVED = 0,
    BPLIB_NC_APP_STATE_STOPPED = 1,
    BPLIB_NC_APP_STATE_ADDED   = 2,
    BPLIB_NC_APP_STATE_STARTED = 3,
} BPLib_NC_ApplicationState_t;

/**
  * \brief Indicator for type of configuration
  */
typedef enum
{
    BPLIB_CHANNEL                   =  0, /* Channel configuration */
    BPLIB_CONTACTS                  =  1, /* Contacts configuration */
    BPLIB_COMPRESSED_REPORTING      =  2, /* Compressed Reporting configuration */
    BPLIB_CUSTODIAN_AUTH_POLICY     =  3, /* Custodian Authorization Policy configuration */
    BPLIB_CUSTODY_AUTH_POLICY       =  4, /* Custody Authorization Policy configuration */
    BPLIB_MIB_PER_NODE              =  5, /* MIB per Node configuration */
    BPLIB_MIB_PER_SRC               =  6, /* MIB per Source configuration */
    BPLIB_REPORT_TO_EID_AUTH_POLICY =  7, /* Report-to-EID Authorization Policy configuration */
    BPLIB_SRC_AUTH_POLICY           =  8, /* Source Authorization Policy configuration */
    BPLIB_SRC_LATENCY_POLICY        =  9, /* Source Latency Policy configuration */
    BPLIB_STORAGE                   = 10, /* Storage configuration */
    BPLIB_ADU_PROXY                 = 11, /* FWP's ADU Proxy configuration; confined to BPNode */
} BPLib_NC_ConfigType_t;

/**
  * \brief Generic validation function for MIB config items
  */
typedef bool (*MibConfigValidateFunc_t)(BPLib_NC_MibPerNodeConfig_t *TblDataPtr);

/* =========== */
/* Global Data */
/* =========== */

extern BPLib_NC_ConfigPtrs_t BPLib_NC_ConfigPtrs;

/* =================== */
/* Function Prototypes */
/* =================== */

/**
 * \brief      Initialize the NC module
 * \details    Initialize source and node MIB configuration payloads, the channel
 *             and contacts statistics packet payload. Initialize the RW locks
 *             used in configuration management. Capture the configuration
 *             pointers passed in from BPNode. Save the instance EID in a place
 *             that is easily accessible. Initialize the CRC table. Initialize AS
 * \param[in]  ConfigPtrs Pointer to configurations for BPLib populated by BPNode
 * \param[in]  Inst Pointer to bplib instance
 * 
 * \return     Execution status
 * \retval     BPLIB_SUCCESS: Successful initialization of the NC module
 * \retval     BPLIB_FWP_CONFIG_PTRS_INIT_ERROR: At least one passed in configuration is NULL
 */
BPLib_Status_t BPLib_NC_InitImpl(BPLib_Instance_t* Instance, BPLib_NC_ConfigPtrs_t* ConfigPtrs);

/**
  * \brief      Initialize all BPLib subsystems
  * \details    Initialize FWP, EM, TIME, NC, QM, and MEM
  * \note       Callbacks is a void* because bplib_nc.h include bplib_fwp.h
  *             which includes bplib_nc.h. This cycle creates a scenario where
  *             BPLib_FWP_ProxyCallbacks_t is not defined. Since bplib.fwp.h needs
  *             bplib_nc.h prototypes for proxies and bplib_nc.h needs bplib_fwp.h
  *             for the proxy struct, Callbacks was chosen to become a void* by
  *             pulling a rabbit from a hat and selecting based on the fur color
  * \param[in]  ConfigPtrs      Pointer to configurations for BPLib populated by BPNode
  * \param[in]  Callbacks       Pointer to callback functions for BPLib, populated by BPNode
  * \param[out] Instance        The instance to be initialized
  * \param[in]  MaxUnsortedJobs The maximum number of jobs that can be queued
  * \param[out] PoolMem         Pointer to the initial memory for the pool
  * \param[in]  PoolMemLen      Size of the initial memory
  * \return     Execution status
  * \retval     BPLIB_SUCCESS: Initialization of subsystems was successful
  * \retval     BPLIB_NC_FWP_INIT_ERR: FWP initialization error
  * \retval     BPLIB_NC_EM_INIT_ERR: EM initialization error
  * \retval     BPLIB_NC_TIME_INIT_ERR: TIME initialization error
  * \retval     BPLIB_NC_INIT_ERR: NC initialization error
  * \retval     BPLIB_NC_AS_INIT_ERR: AS initialization error
  * \retval     BPLIB_NC_QM_INIT_ERR: QM initialization error
  * \retval     BPLIB_NC_MEM_INIT_ERR: MEM initialization error
  */
BPLib_Status_t BPLib_NC_Init(BPLib_NC_ConfigPtrs_t* ConfigPtrs, void* Callbacks,
                                BPLib_Instance_t* Instance, uint16_t MaxUnsortedJobs,
                                void* PoolMem, size_t PoolMemLen);

/**
 * \brief Acquires the reader lock on the node configuration.
 *
 * \details This function acquires the reader lock on the node configuration,
 * allowing multiple readers to access the configuration data simultaneously
 * as long as no writer holds the lock.
 */
void BPLib_NC_ReaderLock(void);

/**
 * \brief Releases the reader lock on the node configuration.
 *
 * \details This function releases the reader lock on the node configuration,
 * allowing other threads (readers or writers) to acquire the lock as needed.
 */
void BPLib_NC_ReaderUnlock(void);

/**
 * \brief Validate MIB Per Node Configuration Table configurations
 *
 * \par Description
 *       Validate configuration table parameters
 *
 * \par Assumptions, External Events, and Notes:
 *       - This function is called by whatever external task handles table management.
 *         Every time a new MIB Configuration Per Node table is loaded, this function should be called to
 *         validate its parameters.
 *
 * \param[in] TblData Pointer to the config table
 *
 * \return Execution status
 * \retval BPLIB_SUCCESS Validation was successful
 * \retval BPLIB_TABLE_OUT_OF_RANGE_ERR_CODE: table parameters are out of range
*/
BPLib_Status_t BPLib_NC_MIBConfigPNTblValidateFunc(void *TblData);


/**
 * \brief Validate MIB Per Source Table configurations
 *
 * \par Description
 *      Validate configuration table parameters
 *
 * \par Assumptions, External Events, and Notes:
 *      - This function is called by whatever external task handles table management.
 *        Every time a new MIB Configuration Per Source table is loaded, this function should be called to
 *        validate its parameters.
 *
 * \param[in] TblData Pointer to the config table
 *
 * \return Execution status
 * \retval BPLIB_SUCCESS Validation was successful
 * \retval BPLIB_TABLE_OUT_OF_RANGE_ERR_CODE: table parameters are out of range
 */
BPLib_Status_t BPLib_NC_MIBConfigPSTblValidateFunc(void *TblData);

/**
 * \brief Set MIB Node Configuration
 *
 * \par Description
 *      Set the given Node MIB configuration to the given value after validating both.
 *      The node MIB configuration telemetry is also updated.
 *
 * \param[in] MibItem An enumeration of the MIB configuration item
 * \param[in] Value The new value to set the configuration to
 *
 * \return Execution status
 * \retval BPLIB_SUCCESS Node configuration update was successful
 */
BPLib_Status_t BPLib_NC_SetMibNodeConfig(uint32_t MibItem, uint32_t Value);

/**
 * \brief Set MIB Source Configuration
 *
 * \par Description
 *      Set the given Source MIB configuration to the given value after validating both.
 *      The Source MIB configuration telemetry is also updated.
 *
 * \param[in] EidPattern The EIDs to update the source configuration for
 * \param[in] MibItem An enumeration of the MIB configuration item
 * \param[in] Value The new value to set the configuration to
 *
 * \return Execution status
 * \retval BPLIB_SUCCESS Source configuration update was successful
 */
BPLib_Status_t BPLib_NC_SetMibSourceConfig(const BPLib_EID_Pattern_t *EidPattern, 
                                                      uint32_t MibItem, uint32_t Value);

/**
  * \brief     Mutator of the application state of a channel
  * \note      This is primarily used in ADU operations
  * \param[in] ChanId (uint8) ID of the channel whose application state is to be modified
  * \param[in] State (BPLib_NC_ApplicationState_t) Application state to set the channel to
  * \return    void
  */
void BPLib_NC_SetAppState(uint8_t ChanId, BPLib_NC_ApplicationState_t State);

/**
  * \brief     Accessor to the application state of a channel
  * \note      This is primarily used in ADU operations
  * \param[in] ChanId (uint8) ID of the channel whose application state is requested
  * \return    Application State of the Channel
  * \retval    See BPLib_NC_ApplicationState_t for specific states
  */
BPLib_NC_ApplicationState_t BPLib_NC_GetAppState(uint8_t ChanId);

/**
  * \brief     Pass updated configurations to corresponding modules
  * \details   Uses FWP to update the BPLib configuration pointers, then pass those updated pointer
  *            configurations to the module that controls that configuration
  * \note      As of right now, the API calls to modules that will update configurations are commented out, since
  *            some of those functions are not implemented yet
  * \param[in] Inst Pointer to bplib instance
  * \return    Execution status
  * \retval    BPLIB_SUCCESS: Successful execution without updates to configurations
  * \retval    BPLIB_TBL_UPDATED: Successful execution with configuration updates
  * \retval    BPLIB_ERROR: An error occured while attempting to refresh/update configurations
  */
BPLib_Status_t BPLib_NC_ConfigUpdate(BPLib_Instance_t *Inst);

/**
  * \brief     Run regular system maintenance activities
  * \details   Runs regular Time Management, Storage, and Custody Transfer maintenance
  *            activities. The calling task sets how frequently these activities should be
  *            run but no faster than a 1Hz cadence is recommended. Note that the storage
  *            garbage collection function will only actually run if it does not detect
  *            any active ingress or egress operations from other tasks.
  * \param[in] Inst Pointer to bplib instance
  * \return    void
  */
void BPLib_NC_RunMaintenanceActivities(BPLib_Instance_t *Inst);

/**
  * \brief     Get MIB per node configuration value
  * \param[in] Config Enumeration of configuration to get
  * \return    MIB Configuration value (or 0 if an error occured)
  */
uint32_t BPLib_NC_GetNodeConfigValue(BPLib_NC_Config_t Config);

#endif // BPLIB_NC_H
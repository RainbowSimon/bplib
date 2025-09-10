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

#ifndef BPLIB_CT_H
#define BPLIB_CT_H

/*
** Include
*/

#include "bplib_api_types.h"
#include "bplib_mem.h"


/*
** Macros
*/

#define BPLIB_CT_BUNDLE_IDENTIFIER_ARRAY_LEN        (7u)

/**
 * \brief Maximum sequence range length. Must always be an odd number
 */
#define BPLIB_CT_MAX_SEQ_RANGE_LEN                  (11u)

#define BPLIB_CT_MAX_RAW_CCS                        (10u)
#define BPLIB_CT_MAX_SEQ_COLLECTIONS                (2u)
#define BPLIB_CT_MAX_RECVD_SEQ_COLLECTIONS          (10u)

/*
** Type Definitions
*/

typedef enum
{
    BPLib_CT_CustodyAccepted = 1,
    BPLib_CT_CustodyRefused = -1
} BPLib_CT_DispositionCode_t;

typedef enum
{
    BPLib_CT_CustodyAccepted_Idx = 0,
    BPLib_CT_CustodyRefused_Idx = -1
} BPLib_CT_SeqCollectionIdx_t;

typedef struct
{
    uint64_t SeqId;
    uint64_t FirstSeqNum;
    uint64_t SeqRange[BPLIB_CT_MAX_SEQ_RANGE_LEN];
    size_t   SeqRangeLen;
    size_t   LastSeqNumAdded;
    BPLib_CT_DispositionCode_t DispositionCode;
} BPLib_CT_BundleSeqCollection_t;

typedef struct
{
    bool                           InProgress;
    size_t                         Size;
    BPLib_EID_t                    SourceAdminEid;
    BPLib_CT_BundleSeqCollection_t BundleSeqCollections[BPLIB_CT_MAX_SEQ_COLLECTIONS];
} BPLib_CT_RawCcs_t;

typedef struct
{
    BPLib_EID_t                    SourceAdminEid;
    BPLib_CT_BundleSeqCollection_t BundleSeqCollections[BPLIB_CT_MAX_RECVD_SEQ_COLLECTIONS];
    size_t                         NumBundleSeqCollections;
} BPLib_CT_DeserializedCcs_t;

typedef struct 
{
    uint32_t Temp;
} BPLib_CT_PendingTransfers_t;


typedef struct 
{
    BPLib_CT_RawCcs_t RawCcss[BPLIB_CT_MAX_RAW_CCS];
    BPLib_CT_PendingTransfers_t CtPending;
} BPLib_CT_Database_t;


/*
** Exported Functions
*/

BPLib_Status_t BPLib_CT_Init(BPLib_Instance_t *Inst);

/**
 * \brief Set bundle ID
 *
 *  \par Description
 *       Calculate and set a bundle ID for the provided bundle. The bundle ID is calculated
 *       with a CRC-32C calculation done over the bundle's unique identifiers: source
 *       EID, creation time, and sequence number.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 * 
 *  \param[in] Bundle Pointer to the bundle
 *
 *  \return Execution status
 *  \retval BPLIB_SUCCESS Initialization was successful
 *  \retval BPLIB_NULL_PTR_ERR The bundle was null
 */
BPLib_Status_t BPLib_CT_SetBundleId(BPLib_Bundle_t *Bundle);

BPLib_Status_t BPLib_CT_ProcessNewBundle(BPLib_Instance_t* Inst, BPLib_Bundle_t *Bundle);

BPLib_Status_t BPLib_CT_UpdateBundle(BPLib_Bundle_t *Bundle);

BPLib_Status_t BPLib_CT_ProcessCcs(BPLib_Instance_t *Inst, BPLib_CT_DeserializedCcs_t *Ccs);

#endif /* BPLIB_CT_H */

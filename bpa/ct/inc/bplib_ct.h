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
#include "bplib_rbt.h"

/*
** Macros
*/

/**
 * \brief Length of unique bundle identifier array in \ref BPLib_CT_SetBundleId
 */
#define BPLIB_CT_BUNDLE_IDENTIFIER_ARRAY_LEN        (7u)

/**
 * \brief Maximum supported sequence range length. Must always be an odd number
 */
#define BPLIB_CT_MAX_SEQ_RANGE_LEN                  (11u)

/**
 * \brief Maximum number of open CCSs at any given time
 */
#define BPLIB_CT_MAX_OPEN_CCS                        (10u)

/**
 * \brief Maximum number of bundle sequence collections in a locally built CCS.
 *        Only two disposition codes are currently supported at this point so no
 *        more than two bundle sequence collections are needed.
 */
#define BPLIB_CT_MAX_SEQ_COLLECTIONS                (2u)

/**
 * \brief Maximum number of bundle sequence collections allowed in a CCS received from
 *        another node.
 */
#define BPLIB_CT_MAX_RECVD_SEQ_COLLECTIONS          (10u)

/**
 * \brief Maximum number of independent sequence counters. Should be greater than or 
 *        equal to the \ref BPLIB_MAX_NUM_CONTACTS, since each open contact will have its
 *        own assigned sequence counter.
 */
#define BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS           (BPLIB_MAX_NUM_CONTACTS * 3)

/**
 * \brief Maximum number of entries allowed in the Custody Transfer Database (CTDB)
 */
#define BPLIB_CT_DB_MAX_ENTRIES                     (100u)


/*
** Type Definitions
*/

/**
 * \brief Disposition codes relating custody information of a bundle sequence collection
 *        in a CCS. A positive disposition code indicates custody acceptance, a negative
 *        disposition code indicates custody refusal.
 */
typedef enum
{
    BPLib_CT_CustodyAccepted     = 1,
    BPLib_CT_CustodyRefused      = -1,
    BPLib_CT_FirstAcceptDispCode = BPLib_CT_CustodyAccepted,
    BPLib_CT_LastRefuseDispCode  = BPLib_CT_CustodyRefused,
} BPLib_CT_DispositionCode_t;

/**
 * \brief Mapping of a disposition code to its index in the bundle sequence collections of
 *        the open CCSs.
 */
typedef enum
{
    BPLib_CT_CustodyAccepted_Idx = 0,
    BPLib_CT_CustodyRefused_Idx = 1
} BPLib_CT_SeqCollectionIdx_t;

/**
 * \brief A bundle sequence collection and relevant information for building one.
 */
typedef struct
{
    uint64_t SeqId;
    uint64_t FirstSeqNum;
    uint64_t SeqRange[BPLIB_CT_MAX_SEQ_RANGE_LEN];

    /* Additional information */
    size_t   SeqRangeLen;
    size_t   LastSeqNumAdded;
    BPLib_CT_DispositionCode_t DispositionCode;
} BPLib_CT_BundleSeqCollection_t;

/**
 * \brief The raw data needed to build a new CCS administrative record. 
 */
typedef struct
{
    bool                           InProgress;
    size_t                         Size;
    BPLib_EID_t                    SourceAdminEid;
    BPLib_CT_BundleSeqCollection_t BundleSeqCollections[BPLIB_CT_MAX_SEQ_COLLECTIONS];
} BPLib_CT_OpenCcs_t;

/**
 * \brief The deserialized administrative record data from a received CCS.
 */
typedef struct
{
    BPLib_EID_t                    SourceAdminEid;
    BPLib_CT_BundleSeqCollection_t BundleSeqCollections[BPLIB_CT_MAX_RECVD_SEQ_COLLECTIONS];
    size_t                         NumBundleSeqCollections;
} BPLib_CT_DeserializedCcs_t;

/**
 * \brief An entry in the Custody Transfer Database (CTDB)
 */
typedef struct
{
    BPLib_RBT_Link_t RbtLink;
    uint64_t SeqId;
    uint64_t SeqNum;
    uint32_t BundleId;
    bool     Free;
} BPLib_CT_DbEntry_t;

/**
 * \brief Sequence counter
 */
typedef struct
{
    uint64_t Counter;
} BPLib_SeqCounter_t;

/**
 * \brief The context information for performing custody transfer
 */
typedef struct 
{
    BPLib_CT_OpenCcs_t OpenCcss[BPLIB_CT_MAX_OPEN_CCS];

    uint64_t SeqCounters[BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS];
    uint64_t CurrActiveSeqIds[BPLIB_MAX_NUM_CONTACTS];
    uint64_t LastSeqCounterId;

    BPLib_CT_DbEntry_t Ctdb[BPLIB_CT_DB_MAX_ENTRIES];
    size_t CurrDbSize;
    size_t LastDbEntry;
    BPLib_RBT_Root_t CtdbRoot;
} BPLib_CT_Context_t;


/*
** Exported Functions
*/

/**
 * \brief Initialize custody transfer
 *
 *  \par Description
 *       Initializes custody transfer context to an empty starting state
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 * 
 *  \param[in] Inst Pointer to the BPLib instance
 *
 *  \return Execution status
 *  \retval BPLIB_SUCCESS Initialization was successful
 *  \retval BPLIB_NULL_PTR_ERR The instance was null
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
 *  \retval BPLIB_SUCCESS Operation was successful
 *  \retval BPLIB_NULL_PTR_ERR The bundle was null
 */
BPLib_Status_t BPLib_CT_SetBundleId(BPLib_Bundle_t *Bundle);

/**
 * \brief Process a new bundle
 *
 *  \par Description
 *       Sets the bundle ID and if a bundle is custodial, check if PDB can accept
 *       custody and if so, mark it as custody accepted in the relevant open CCS, 
 *       and if not, mark it as custody refused in the relevant open CCS.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 * 
 *  \param[in] Bundle Pointer to the bundle
 *  \param[in] Inst Pointer to the BPLib instance
 *
 *  \return Execution status
 *  \retval BPLIB_SUCCESS Operation was successful
 */
BPLib_Status_t BPLib_CT_ProcessNewBundle(BPLib_Instance_t* Inst, BPLib_Bundle_t *Bundle);

/**
 * \brief Update a bundle on egress
 *
 *  \par Description
 *       If an egressing bundle has a CTEB, give it a new sequence number, update its CTEB
 *       fields, and add it to the CTDB.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 * 
 *  \param[in] Bundle Pointer to the bundle
 *  \param[in] Inst Pointer to the BPLib instance
 *
 *  \return Execution status
 *  \retval BPLIB_SUCCESS Operation was successful
 */
BPLib_Status_t BPLib_CT_UpdateBundle(BPLib_Instance_t* Inst, BPLib_Bundle_t *Bundle);

/**
 * \brief Process a new CCS
 *
 *  \par Description
 *       Process a new CCS by validating it, removing any included sequence numbers from
 *       the CTDB and requesting Storage delete them, requesting Storage retransmit any
 *       missing sequence numbers, and incrementing all relevant counters.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 * 
 *  \param[in] CCS Pointer to the received CCS
 *  \param[in] Inst Pointer to the BPLib instance
 *
 *  \return Execution status
 *  \retval BPLIB_SUCCESS Operation was successful
 */
BPLib_Status_t BPLib_CT_ProcessCcs(BPLib_Instance_t *Inst, BPLib_CT_DeserializedCcs_t *Ccs);

/**
 * \brief Assign a new sequence counter
 *
 *  \par Description
 *       Assign a new sequence counter to a new contact.
 *
 *  \par Assumptions, External Events, and Notes:
 *       This is done by CLA on the contact-start directive
 * 
 *  \param[in] ContactId ID of the contact requesting a new sequence counter
 *  \param[in] Inst Pointer to the BPLib instance
 *
 *  \return Execution status
 *  \retval BPLIB_SUCCESS Operation was successful
 */
BPLib_Status_t BPLib_CT_AssignSeqCounter(BPLib_Instance_t *Inst, uint32_t ContactId);

#endif /* BPLIB_CT_H */

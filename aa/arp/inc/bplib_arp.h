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

#ifndef BPLIB_ARP_H
#define BPLIB_ARP_H

/*
** Includes
*/

#include "bplib_api_types.h"
#include "bplib_cfg.h"
#include "bplib_eid.h"
#include "bplib_ct.h"

/* ======== */
/* Typedefs */
/* ======== */

/**
 * \brief CRS configuration member
 */
typedef struct
{
    BPLib_EID_t DestEID;
    uint32_t    TimeTrigger;
    uint32_t    SizeTrigger;
} BPLib_ARP_CRSSet_t;

/**
 * \brief CRS configuration
 */
typedef struct
{
    BPLib_ARP_CRSSet_t CRS_Set[BPLIB_MAX_NUM_CRS];
} BPLib_ARP_CRSTable_t;

/**
 * \brief Identifiers for standard administrative record ADUs that are used in
 *        providing some of the features of the Bundle Protocol
 */
typedef enum
{
    BPLib_CT_BsrRecordTypeCode = 1,
    /* BPLib_CT_???RecordTypeCode = 2, */
    BPLib_CT_CrsRecordTypeCode = 3,
    BPLib_CT_CcsRecordTypeCode = 4,

    /* Others here when applicable */
} BPLib_ARP_AdminRecordTypeCode_t;

/**
 * \brief Standard and typically small ADUs that are used in providing some of the
 *        features of the Bundle Protocol as a response to bundle transmission
 *        requests presented by nodes' application agents.
 */
typedef struct
{
    BPLib_ARP_AdminRecordTypeCode_t AdminRecordType; /** \brief Administrative record type */

    /* Administrative record content map */

    /**
     * \brief The value to the disposition key, bundle sequence collection value
     *        CBOR map that represents a CCS. The actual key (disposition code) is
     *        inside the BPLib_CT_BundleSeqCollection_t struct
     */
    BPLib_CT_BundleSeqCollection_t BundleSeqCollections[BPLIB_CT_MAX_SEQ_COLLECTIONS];
} BPLib_ARP_AdminRecord_t;

/* =================== */
/* Function Prototypes */
/* =================== */

/**
 * \brief Admin Records Processor initialization
 *
 *  \par Description
 *       ARP initialization function
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \return Execution status
 *  \retval BPLIB_SUCCESS Initialization was successful
 */
int BPLib_ARP_Init(void);

/**
 * \brief Validate CRS Table Table configurations
 *
 *  \par Description
 *       Validate configuration table parameters
 *
 *  \par Assumptions, External Events, and Notes:
 *       - This function is called by whatever external task handles table management.
 *         Every time a new CRS table is loaded, this function should be called to
 *         validate its parameters.
 *
 *  \param[in] TblData Pointer to the config table
 *
 *  \return Execution status
 *  \retval BPLIB_SUCCESS Validation was successful
 * \retval    BPLIB_TABLE_OUT_OF_RANGE_ERR_CODE: table parameters are out of range
 */
BPLib_Status_t BPLib_ARP_CRSTblValidateFunc(void *TblData);

/**
 * \brief     Convert the disposition code into an index into a bundle sequence collection array
 * \note      This only works if disposition codes are contiguous
 * \details   In order to turn a semi-contiguous list of disposition codes into an
 *            index we just need to make the most negative number into 0. This
 *            doesn't take into account the fact that 0 is skipped though, so we
 *            must check if the number is positive and, if so, shift the value back
 *            by 1 to make the index contiguous.
 * \param[in] DispositionCode The dispoisition code of a custody signal
 * \return    Index into a Bundle Sequence Collection array that maps the
 *            disposition code value to the Bundle Sequence Collection associated
 *            with that disposition code
 */
BPLib_CT_SeqCollectionIdx_t BPLib_ARP_GetDispCodeIdx(BPLib_CT_DispositionCode_t DispositionCode);

/**
 * \brief     Counts and discards the Bundle Status Report
 * \param[in] AdminRecord Decoded Bundle Status Report administrative record
 * \return    Execution status
 * \retval    BPLIB_SUCCESS: Successful execution
 * TODO: Other return values
 */
BPLib_Status_t BPLib_ARP_ProcessBsr(BPLib_ARP_AdminRecord_t* AdminRecord);

/**
 * \brief     Counts and discards the Compressed Reporting Signal
 * \param[in] AdminRecord Decoded Compressed Reporting Signal administrative record
 * \return    Execution status
 * \retval    BPLIB_SUCCESS: Successful execution
 * TODO: Other return values
 */
BPLib_Status_t BPLib_ARP_ProcessCrs(BPLib_ARP_AdminRecord_t* AdminRecord);

/**
 * \brief     Passthrough to BPLib_CT_ProcessCcs
 * \param[in] AdminRecord Decoded Compressed Custody Signal administrative record
 * \return    Execution status of BPLib_CT_ProcessCcs
 */
BPLib_Status_t BPLib_ARP_ProcessCcs(BPLib_ARP_AdminRecord_t* AdminRecord);

#endif /* BPLIB_ARP_H */

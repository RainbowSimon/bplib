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

#ifndef BPLIB_CT_CCS_H
#define BPLIB_CT_CCS_H

/*
** Include
*/

#include "bplib_ct.h"


/*
** Exported Functions
*/

void BPLib_CT_ResetOpenCcs(BPLib_CT_OpenCcs_t *OpenCcs);

/**
 * \brief     Add values from a CTEB to an open CCS's suite of information. This 
 *            function also checks the size of a CCS against the limit and starts
 *            the process of sending the CCS off if it's at or beyond that limit
 * \param[in] Instance        The struct that holds the currently open CCSs, one
 *                            of which we need to add to, and the memory pool from
 *                            which bundles can be generated from if the open CCS
 *                            reaches the size limit
 * \param[in] OpenCcsIdx      Indicates which open CCS in Instances's CT context
 *                            to add to
 * \param[in] ContactId       Contact that the bundle came in on. This is used to
 *                            determine the size limit of an open CCS
 * \param[in] CtebPtr         CTEB information to store in the Open CCS's bundle
 *                            sequence collection
 * \param[in] DispositionCode The open CCS's bundle sequence collection to add
 *                            CTEB information to
 * \return    Execution status
 * \retval    BPLIB_SUCCESS: Successful execution
 * \retval    BPLIB_CT_CUSTODY_REFUSED_ERR: Open CCS data failed sanity checks
 */
BPLib_Status_t BPLib_CT_AddToOpenCcs(BPLib_Instance_t* Instance, size_t OpenCcsIdx,
                                        uint32_t ContactId,
                                        BPLib_CustodyBlockData_t* CtebPtr,
                                        BPLib_CT_DispositionCode_t DispositionCode);

size_t BPLib_CT_GetOpenCcsIdx(BPLib_Instance_t* Instance, BPLib_EID_t *SourceAdminEID,
                                uint64_t SequenceId);

/**
 * \brief     After an open CCS has hit the size, time, or open CCS limit, this
 *            function will send the CCS and memory pool off to ARP to build the
 *            CCS into a bundle. The open CCS is then reset for continued use
 * \param[in] Instance Abstraction of the node that will be used for putting the
 *                     bundle with a CCS in the payload on the job queue. Instance
 *                     also contains the memory pool used to create the bundle
 * \param[in] OpenCcs  An open CCS that has reached a configured limit
 * \return    void
 */
void BPLib_CT_BuildAndSendOpenCcs(BPLib_Instance_t* Instance, BPLib_CT_OpenCcs_t* OpenCcs);

BPLib_Status_t BPLib_CT_ProcessBundleSeqCollection(BPLib_Instance_t *Inst, BPLib_CT_BundleSeqCollection_t *SeqCollection);

#endif /* BPLIB_CT_CCS_H */

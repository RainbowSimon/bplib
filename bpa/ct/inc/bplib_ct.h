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


/*
** Exported Functions
*/

/**
 * \brief Custody Transfer initialization
 *
 *  \par Description
 *       CT initialization function
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \return Execution status
 *  \retval BPLIB_SUCCESS Initialization was successful
 */
int BPLib_CT_Init(void);

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

#endif /* BPLIB_CT_H */

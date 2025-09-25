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

#ifndef BPLIB_CT_DB_H
#define BPLIB_CT_DB_H

/*
** Include
*/

#include "bplib_api_types.h"
#include "bplib_mem.h"


/*
** Macro Definitions
*/



/*
** Type Definitions 
*/



/*
** Exported Functions
*/

BPLib_Status_t BPLib_CT_InitCtdb(BPLib_CT_Database_t *Ctdb);

BPLib_Status_t BPLib_CT_AddToCtdb(BPLib_Instance_t *Inst, uint64_t SeqNum, 
                                                    uint64_t SeqId, uint32_t BundleId);

BPLib_Status_t BPLib_CT_RemoveFromCtdb(BPLib_Instance_t *Inst, uint64_t SeqNum, 
                                                    uint64_t SeqId, uint32_t *BundleId);

BPLib_Status_t BPLib_CT_GetSequenceId(BPLib_Instance_t *Inst, BPLib_Bundle_t *Bundle, uint64_t *SeqId);

uint64_t BPLib_CT_GetNextSequenceNum(BPLib_Instance_t *Inst, uint64_t SeqId);

#endif /* BPLIB_CT_DB_H */

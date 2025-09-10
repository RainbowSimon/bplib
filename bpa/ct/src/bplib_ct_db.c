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

/*
** Include
*/

#include "bplib_ct.h"
#include "bplib_ct_db.h"
#include "bplib_bblocks.h"
#include "bplib_eid.h"
#include "bplib_mem.h"
#include "bplib_pdb.h"

/*
** Function Definitions
*/

BPLib_Status_t BPLib_CT_InitCtdb(BPLib_CT_PendingTransfers_t *CtPending)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_AddToCtdb(BPLib_Instance_t *Inst, uint64_t SeqId, 
                                                    uint64_t SeqNum, uint32_t *BundeId)
{
    /* Add bundle info to CTDB */
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_RemoveFromCtdb(BPLib_Instance_t *Inst, uint64_t SeqId, 
                                                    uint64_t SeqNum, uint32_t *BundeId)
{
    /* Remove bundle info from CTDB */
    return BPLIB_SUCCESS;
}

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
#include "bplib_qm.h"
#include "bplib_pdb.h"

/*
** Function Definitions
*/

BPLib_Status_t BPLib_CT_InitCtdb(BPLib_CT_Database_t *Ctdb)
{
    memset(Ctdb, 0, sizeof(BPLib_CT_Database_t));

    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_AddToCtdb(BPLib_Instance_t *Inst, uint64_t SeqNum, 
                                                    uint64_t SeqId, uint32_t BundleId)
{
    /* Add bundle info to CTDB */
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_RemoveFromCtdb(BPLib_Instance_t *Inst, uint64_t SeqNum, 
                                                    uint64_t SeqId, uint32_t *BundleId)
{
    /* Remove bundle info from CTDB */
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_GetSequenceId(BPLib_Instance_t *Inst, BPLib_Bundle_t *Bundle, uint64_t *SeqId)
{
    uint32_t CurrSeqId;

    for (CurrSeqId = 0; CurrSeqId < BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS; CurrSeqId++)
    {
        if (Inst->Ctdb.SeqCounters[CurrSeqId].ContactId == Bundle->Meta.EgressID &&
            Inst->Ctdb.SeqCounters[CurrSeqId].Active == true)
        {
            *SeqId = CurrSeqId;

            return BPLIB_SUCCESS;
        }
    }

    return BPLIB_ERROR; /* TODO better error */
}

uint64_t BPLib_CT_GetNextSequenceNum(BPLib_Instance_t *Inst, uint64_t SeqId)
{
    return Inst->Ctdb.SeqCounters[SeqId % BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS].Counter++;
}

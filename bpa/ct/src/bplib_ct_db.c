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

BPLib_CT_DbEntry_t *BPLib_CT_GetDbEntryFromRbt(const BPLib_RBT_Link_t *Node)
{
    /* Get a pointer to the DB entry */
    return (BPLib_CT_DbEntry_t *)(void *)((uint8_t *) Node - offsetof(BPLib_CT_DbEntry_t, RbtLink));
}

int BPLib_CT_CompareDbEntries(const BPLib_RBT_Link_t *Node, void *Arg)
{
    BPLib_CT_DbEntry_t *DbEntry;
    uint64_t *SeqNum; 

    SeqNum = (uint64_t *) Arg;

    DbEntry = BPLib_CT_GetDbEntryFromRbt(Node);

    if (*SeqNum == DbEntry->SeqNum)
    {
        return 0;
    }
    else if (*SeqNum > DbEntry->SeqNum)
    {
        return 1;
    }

    return -1;
}

BPLib_Status_t BPLib_CT_AddToCtdb(BPLib_CT_Context_t *Context, uint64_t SeqId, 
                                                    uint64_t SeqNum, uint32_t BundleId)
{
    size_t CurrDbEntry;

    if (Context->CurrDbSize == BPLIB_CT_DB_MAX_ENTRIES)
    {
        return BPLIB_CT_FULL_DB_ERR;
    }

    /* Get a CTDB entry to use */
    /* TODO THIS IS NOT A GOOD ALGORITHM. FIND A MORE EFFICIENT IDEA */
    CurrDbEntry = Context->LastDbEntry;
    do
    {
        CurrDbEntry = (CurrDbEntry + 1) % BPLIB_CT_DB_MAX_ENTRIES;

        if (Context->Ctdb[CurrDbEntry].Free)
        {
            break;
        }
    } while (CurrDbEntry != Context->LastDbEntry);
    
    /* Set CTDB entry values */
    Context->Ctdb[CurrDbEntry].BundleId = BundleId;
    Context->Ctdb[CurrDbEntry].SeqId = SeqId;
    Context->Ctdb[CurrDbEntry].SeqNum = SeqNum;
    Context->Ctdb[CurrDbEntry].Free = false;

    Context->CurrDbSize++;
    Context->LastDbEntry = CurrDbEntry;
    
    BPLib_RBT_InsertValueGeneric(SeqId, &(Context->CtdbRoot), &(Context->Ctdb[CurrDbEntry].RbtLink),
                            BPLib_CT_CompareDbEntries, &SeqNum);

    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_GetEntryFromCtdb(BPLib_CT_Context_t *Context, uint64_t SeqId, 
                                            uint64_t SeqNum, BPLib_CT_DbEntry_t *DbEntry)
{
    BPLib_Status_t Status = BPLIB_NOT_FOUND_ERR;
    BPLib_RBT_Link_t *RbtLink;

    RbtLink = BPLib_RBT_SearchGeneric(SeqId, &Context->CtdbRoot, 
                                                    BPLib_CT_CompareDbEntries, &SeqNum);

    if (RbtLink != NULL)
    {
        DbEntry = BPLib_CT_GetDbEntryFromRbt(RbtLink);
        Status = BPLIB_SUCCESS; 
    }

    return Status;
}

BPLib_Status_t BPLib_CT_RemoveFromCtdb(BPLib_CT_Context_t *Context, BPLib_CT_DbEntry_t *DbEntry)
{
    BPLib_Status_t Status;

    Status = BPLib_RBT_ExtractNode(&Context->CtdbRoot, &DbEntry->RbtLink);
    if (Status == BPLIB_SUCCESS)
    {
        DbEntry->Free = true;

        Context->CurrDbSize--;
    }

    return Status;
}

uint64_t BPLib_CT_GetSequenceId(BPLib_CT_Context_t *Context, BPLib_Bundle_t *Bundle)
{
    return Context->CurrActiveSeqIds[Bundle->Meta.EgressID];
}

uint64_t BPLib_CT_GetNextSequenceNum(BPLib_CT_Context_t *Context, uint64_t SeqId)
{
    return Context->SeqCounters[SeqId % BPLIB_CT_DB_MAX_SEQUENCE_COUNTERS]++;
}

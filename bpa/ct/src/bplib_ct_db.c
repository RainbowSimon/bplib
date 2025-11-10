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
#include <stdio.h>

/*
** Function Definitions
*/

BPLib_CT_DbEntry_t *BPLib_CT_GetDbEntryFromSeqRbt(const BPLib_RBT_Link_t *Node)
{
    /* Get a pointer to the DB entry */
    return (BPLib_CT_DbEntry_t *)(void *)((uint8_t *) Node - offsetof(BPLib_CT_DbEntry_t, SeqRbtLink));
}

BPLib_CT_DbEntry_t *BPLib_CT_GetDbEntryFromIdRbt(const BPLib_RBT_Link_t *Node)
{
    /* Get a pointer to the DB entry */
    return (BPLib_CT_DbEntry_t *)(void *)((uint8_t *) Node - offsetof(BPLib_CT_DbEntry_t, IdRbtLink));
}


int BPLib_CT_CompareDbEntries(const BPLib_RBT_Link_t *Node, void *Arg)
{
    BPLib_CT_DbEntry_t *DbEntry;
    uint64_t *SeqNum; 

    SeqNum = (uint64_t *) Arg;

    DbEntry = BPLib_CT_GetDbEntryFromSeqRbt(Node);

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
    BPLib_Status_t Status;
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

        if (Context->Ctdb[CurrDbEntry].Used == false)
        {
            break;
        }
    } while (CurrDbEntry != Context->LastDbEntry);
    
    /* Set CTDB entry values */
    Context->Ctdb[CurrDbEntry].BundleId = BundleId;
    Context->Ctdb[CurrDbEntry].SeqId = SeqId;
    Context->Ctdb[CurrDbEntry].SeqNum = SeqNum;
    Context->Ctdb[CurrDbEntry].Used = true;

    Context->CurrDbSize++;
    Context->LastDbEntry = CurrDbEntry;
    
    Status = BPLib_RBT_InsertValueGeneric(SeqId, &(Context->SeqTreeRoot), 
                                        &(Context->Ctdb[CurrDbEntry].SeqRbtLink),
                                        BPLib_CT_CompareDbEntries, &SeqNum);

    if (Status == BPLIB_SUCCESS)
    {
        Status = BPLib_RBT_InsertValueUnique(BundleId, &(Context->IdTreeRoot), 
                                            &(Context->Ctdb[CurrDbEntry].IdRbtLink));
    }

    return Status;
}

BPLib_Status_t BPLib_CT_GetEntryFromCtdbWithSeq(BPLib_CT_Context_t *Context, uint64_t SeqId, 
                                            uint64_t SeqNum, BPLib_CT_DbEntry_t **DbEntry)
{
    BPLib_Status_t Status = BPLIB_NOT_FOUND_ERR;
    BPLib_RBT_Link_t *RbtLink;

    RbtLink = BPLib_RBT_SearchGeneric(SeqId, &Context->SeqTreeRoot, 
                                                    BPLib_CT_CompareDbEntries, &SeqNum);

    if (RbtLink != NULL)
    {
        *DbEntry = BPLib_CT_GetDbEntryFromSeqRbt(RbtLink);
        Status = BPLIB_SUCCESS; 
    }

    return Status;
}

BPLib_Status_t BPLib_CT_GetEntryFromCtdbWithId(BPLib_CT_Context_t *Context, 
                                        uint32_t BundleId, BPLib_CT_DbEntry_t **DbEntry)
{
    BPLib_Status_t Status = BPLIB_NOT_FOUND_ERR;
    BPLib_RBT_Link_t *RbtLink;

    RbtLink = BPLib_RBT_SearchUnique(BundleId, &Context->IdTreeRoot);

    if (RbtLink != NULL)
    {
        *DbEntry = BPLib_CT_GetDbEntryFromIdRbt(RbtLink);
        Status = BPLIB_SUCCESS; 
    }

    return Status;
}

BPLib_Status_t BPLib_CT_RemoveFromCtdb(BPLib_CT_Context_t *Context, BPLib_CT_DbEntry_t *DbEntry)
{
    BPLib_Status_t Status;

    Status = BPLib_RBT_ExtractNode(&Context->SeqTreeRoot, &DbEntry->SeqRbtLink);
    if (Status == BPLIB_SUCCESS)
    {
        Status = BPLib_RBT_ExtractNode(&Context->IdTreeRoot, &DbEntry->IdRbtLink);
        if (Status == BPLIB_SUCCESS)
        {
            DbEntry->Used = false;
            Context->CurrDbSize--;
        }
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

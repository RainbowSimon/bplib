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
#include "bplib_bblocks.h"
#include "bplib_eid.h"
#include "bplib_mem.h"
#include "bplib_pdb.h"

/*
** Function Definitions
*/

void BPLib_CT_ResetRawCcs(BPLib_CT_RawCcs_t *RawCcs)
{
    uint8_t i;

    for (i = 0; i < BPLIB_CT_MAX_SEQ_COLLECTIONS; i++)
    {
        RawCcs->BundleSeqCollections[i].SeqRangeLen = 0;
    }

    return;
}

BPLib_Status_t BPLib_CT_AddToRawCcs(BPLib_CT_RawCcs_t *RawCcs, uint64_t SequenceNum, 
                          uint64_t SequenceId, BPLib_CT_DispositionCode_t DispositionCode)
{
    BPLib_CT_BundleSeqCollection_t *Collection;
    uint64_t MissingSeqNumLen;

    if (DispositionCode == BPLib_CT_CustodyAccepted)
    {
        Collection = &(RawCcs->BundleSeqCollections[BPLib_CT_CustodyAccepted_Idx]);
    }
    else
    {
        Collection = &(RawCcs->BundleSeqCollections[BPLib_CT_CustodyRefused_Idx]);
    }

    /* Sanity checks */
    if ((Collection->SeqRangeLen != 0 && Collection->SeqRangeLen % 2 != 1) ||
        Collection->SeqRangeLen >= (BPLIB_CT_MAX_SEQ_RANGE_LEN - 1) ||
        SequenceNum < Collection->LastSeqNumAdded)
    {
        return BPLIB_ERROR;
    }

    /* If RawCcs is empty, add first sequence number */
    if (Collection->SeqRangeLen == 0)
    {
        Collection->SeqId = SequenceId;
        Collection->FirstSeqNum = SequenceNum;
        Collection->SeqRange[0] = 1;
        Collection->SeqRangeLen = 1;
    }
    else
    {
        /* If we received the previous sequence number, increment the relevant sequence range value */
        if (SequenceNum == (Collection->LastSeqNumAdded + 1))
        {
            Collection->SeqRange[Collection->SeqRangeLen - 1]++;
        }
        /* If a gap in sequence numbers is detected, record missing sequence length */
        else
        {
            Collection->SeqRange[Collection->SeqRangeLen] = SequenceNum - Collection->LastSeqNumAdded;
            Collection->SeqRange[Collection->SeqRangeLen + 1] = 1;
            Collection->SeqRangeLen += 2;
        }
    }

    Collection->LastSeqNumAdded = SequenceNum;

    return BPLIB_SUCCESS;
}

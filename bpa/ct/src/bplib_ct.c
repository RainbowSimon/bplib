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

BPLib_Status_t BPLib_CT_SetBundleId(BPLib_Bundle_t *Bundle)
{
    uint64_t UniqueIdArr[BPLIB_CT_BUNDLE_IDENTIFIER_ARRAY_LEN];

    if (Bundle == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    UniqueIdArr[0] = Bundle->blocks.PrimaryBlock.Timestamp.SequenceNumber;
    UniqueIdArr[1] = Bundle->blocks.PrimaryBlock.Timestamp.CreateTime;
    UniqueIdArr[2] = Bundle->blocks.PrimaryBlock.SrcEID.Scheme;
    UniqueIdArr[3] = Bundle->blocks.PrimaryBlock.SrcEID.IpnSspFormat;
    UniqueIdArr[4] = Bundle->blocks.PrimaryBlock.SrcEID.Allocator;
    UniqueIdArr[5] = Bundle->blocks.PrimaryBlock.SrcEID.Node;
    UniqueIdArr[6] = Bundle->blocks.PrimaryBlock.SrcEID.Service;

    /* 
    ** Add any additional unique identifiers for a bundle here 
    ** (and increase the array length accordingly) 
    */

    /* Use a CRC-32C as a quick hash function to get a unique ID for this bundle */
    Bundle->blocks.PrimaryBlock.BundleId = BPLib_CRC_Calculate((void *) ((uintptr_t) UniqueIdArr), 
                                BPLIB_CT_BUNDLE_IDENTIFIER_ARRAY_LEN * sizeof(uint64_t), 
                                BPLib_CRC_Type_CRC32C);

    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_AddBundleToCcs(BPLib_Bundle_t *Bundle, bool CustodyAccepted)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CT_ProcessNewBundle(BPLib_Instance_t* Inst, BPLib_Bundle_t *Bundle)
{
    BPLib_Status_t Status = BPLIB_SUCCESS;

    if (Bundle == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    /* Set bundle ID for both custodial and non-custodial bundles */
    (void) BPLib_CT_SetBundleId(Bundle);

    if (Bundle->Meta.IsCustodial)
    {
        if (BPLib_PDB_CanCustodyBeAccepted(Bundle))
        {
            Status = BPLib_CT_AddBundleToCcs(Bundle, true);
        }
        else
        {
            Status = BPLib_CT_AddBundleToCcs(Bundle, false);
        }
    }

    /* Do nothing for non-custodial bundles */

    return Status;
}

BPLib_Status_t BPLib_CT_UpdateBundle(BPLib_Bundle_t *Bundle)
{
    BPLib_Status_t Status = BPLIB_SUCCESS;

    if (Bundle->Meta.IsCustodial)
    {
        /* Update CTEB fields */

        /*
        Bundle->blocks.Cteb.SequenceId  = BPLib_CT_GetSequenceId(Bundle);
        Bundle->blocks.Cteb.SequenceNum = BPLib_CT_GetNextSequenceNum(Bundle->blocks.Cteb.SequenceId);
        BPLib_EID_CopyEids(&(Bundle->blocks.Cteb.BlkAdminEid), &BPLIB_EID_INSTANCE);
        Bundle->blocks.Cteb.RequiresEncode = true;
        */
        

    }

    /* Do nothing for non-custodial bundles */

    return Status;    
}
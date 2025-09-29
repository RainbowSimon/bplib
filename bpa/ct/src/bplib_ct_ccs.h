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

BPLib_Status_t BPLib_CT_AddToOpenCcs(BPLib_CT_OpenCcs_t *OpenCcs, uint64_t SequenceNum, 
                          uint64_t SequenceId, BPLib_CT_DispositionCode_t DispositionCode);

size_t BPLib_CT_GetOpenCcsIdx(BPLib_CT_Context_t *Context, BPLib_EID_t *SourceAdminEID, uint64_t SequenceId);

size_t BPLib_CT_GetOpenCcsSize(BPLib_CT_OpenCcs_t *OpenCcs);

void BPLib_CT_BuildAndSendOpenCcs(BPLib_CT_OpenCcs_t *OpenCcs);

#endif /* BPLIB_CT_CCS_H */

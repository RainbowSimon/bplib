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

#ifndef BPLIB_STOR_SQL_LOAD_H
#define BPLIB_STOR_SQL_LOAD_H

/* ======== */
/* Includes */
/* ======== */

#include "bplib_stor_sql.h"

/* ======= */
/* Globals */
/* ======= */

/* SQL query statements */
extern sqlite3_stmt* FindBlobStmt;
extern sqlite3_stmt* FindForEgressIDStmt;
extern sqlite3_stmt* MarkEgressedStmt;

/* SQL query strings */

extern const char* FindBlobSQL;
extern       char  FindForEgressIdSQL[BPLIB_SQL_MAX_STRLEN];
extern const char* MarkEgressedSQL;

/* =================== */
/* Function Prototypes */
/* =================== */

BPLib_Status_t BPLib_SQL_FindForEIDs(BPLib_Instance_t* Inst, BPLib_STOR_LoadBatch_t* Batch,
                                        BPLib_EID_Pattern_t* DestEIDs, size_t NumEIDs);

SQL_Status_t BPLib_SQL_FindForEIDsImpl(BPLib_Instance_t* Inst, BPLib_STOR_LoadBatch_t* Batch,
                                        BPLib_EID_Pattern_t* DestEIDs, size_t NumEIDs,
                                        size_t MaxBundles);

BPLib_Status_t BPLib_SQL_MarkBatchEgressed(BPLib_Instance_t* Inst, BPLib_STOR_LoadBatch_t* Batch);

SQL_Status_t BPLib_SQL_MarkBatchEgressedImpl(BPLib_Instance_t* Inst, BPLib_STOR_LoadBatch_t* Batch);

BPLib_Status_t BPLib_SQL_LoadBundle(BPLib_Instance_t* Inst, int64_t BundleRowID,
                                    BPLib_Bundle_t** Bundle);

SQL_Status_t BPLib_SQL_LoadBundleImpl(BPLib_Instance_t* Inst, int64_t BundleRowID,
                                        BPLib_Bundle_t** Bundle);

#endif
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

#ifndef BPLIB_STOR_SQL_STORE_H
#define BPLIB_STOR_SQL_STORE_H

/* ======== */
/* Includes */
/* ======== */

/* =================== */
/* Function Prototypes */
/* =================== */

SQL_Status_t BPLib_SQL_StoreMetadata(BPLib_Bundle_t* Bundle, BPLib_BundleCache_t* BundleCache);

SQL_Status_t BPLib_SQL_StoreChunk(int64_t BundleRowID, const void* Chunk, size_t ChunkSize);

SQL_Status_t BPLib_SQL_StoreBundle(sqlite3* db, BPLib_Bundle_t* Bundle, BPLib_BundleCache_t* BundleCache);

SQL_Status_t BPLib_SQL_StoreImpl(BPLib_Instance_t* Inst, size_t *TotalBytesStored);

BPLib_Status_t BPLib_SQL_Store(BPLib_Instance_t* Inst, size_t *TotalBytesStored);

#endif /* BPLIB_STOR_SQL_STORE_H */
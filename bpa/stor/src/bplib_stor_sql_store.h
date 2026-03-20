/*
 * NASA Docket No. GSC-19,559-1, and identified as "Delay/Disruption Tolerant Networking 
 * (DTN) Bundle Protocol (BP) v7 Core Flight System (cFS) Application Build 7.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this 
 * file except in compliance with the License. You may obtain a copy of the License at 
 *
 * http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software distributed under 
 * the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF 
 * ANY KIND, either express or implied. See the License for the specific language 
 * governing permissions and limitations under the License. The copyright notice to be 
 * included in the software is as follows: 
 *
 * Copyright 2025 United States Government as represented by the Administrator of the 
 * National Aeronautics and Space Administration. All Rights Reserved.
 *
 */

#ifndef BPLIB_STOR_SQL_STORE_H
#define BPLIB_STOR_SQL_STORE_H

/* ======== */
/* Includes */
/* ======== */

#include "bplib_stor_sql.h"

/* ======= */
/* Globals */
/* ======= */

/* SQL query statements */

extern sqlite3_stmt* InsertBlobStmt;
extern sqlite3_stmt* InsertMetadataStmt;

/* SQL query strings */

extern const char* InsertBlobSQL;
extern const char* InsertMetadataSQL;

/* =================== */
/* Function Prototypes */
/* =================== */

SQL_Status_t BPLib_SQL_StoreMetadata(BPLib_Bundle_t* Bundle, BPLib_BundleCache_t* BundleCache);

SQL_Status_t BPLib_SQL_StoreChunk(int64_t BundleRowID, const void* Chunk, size_t ChunkSize);

SQL_Status_t BPLib_SQL_StoreBundle(sqlite3* db, BPLib_Bundle_t* Bundle, BPLib_BundleCache_t* BundleCache);

SQL_Status_t BPLib_SQL_StoreImpl(BPLib_Instance_t* Inst, size_t *TotalBytesStored,
                                size_t *DuplicateBundlesIgnored, size_t *CustodialBundles);

BPLib_Status_t BPLib_SQL_Store(BPLib_Instance_t* Inst, size_t *TotalBytesStored,
                                size_t *DuplicateBundlesIgnored, size_t *CustodialBundles);

#endif /* BPLIB_STOR_SQL_STORE_H */
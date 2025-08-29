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

#ifndef BPLIB_STOR_INTERNAL_H
#define BPLIB_STOR_INTERNAL_H

/* ======== */
/* Includes */
/* ======== */

#include "bplib_api_types.h"
#include "bplib_stor_sql.h"

/* ====== */
/* Macros */
/* ====== */

/* We conditionally allow this to be defined by a compile time variable
** so that the unit tests can pass in :memory: here and avoid using the disk
*/
#ifndef BPLIB_STOR_DBNAME
#define BPLIB_STOR_DBNAME       "bplib-storage.db"
#endif

/* ======= */
/* Globals */
/* ======= */

/* SQL query statements */
extern sqlite3_stmt* FindForEgressIDStmt;
extern sqlite3_stmt* MarkEgressedStmt;
extern sqlite3_stmt* FindBlobStmt;
extern sqlite3_stmt* InsertBlobStmt;
extern sqlite3_stmt* InsertMetadataStmt;
extern sqlite3_stmt* ForeignKeyCheckStmt;
extern sqlite3_stmt* GetNumBundlesStmt;
extern sqlite3_stmt* TotalBytesStmt;
extern sqlite3_stmt* PageCntStmt;
extern sqlite3_stmt* DiscardExpiredStmt;
extern sqlite3_stmt* ExpiredBytesStmt;
extern sqlite3_stmt* DiscardEgressedStmt;
extern sqlite3_stmt* EgressedBytesStmt;

/* SQL query strings */
extern       char  WhereClause[BPLIB_SQL_MAX_STRLEN / 2];
extern       char  FindForEgressIdSQL[BPLIB_SQL_MAX_STRLEN];
extern const char* FindForEgressID_RangeClause;
extern const char* MarkEgressedSQL;
extern const char* FindBlobSQL;
extern const char* InsertBlobSQL;
extern const char* InsertMetadataSQL;
extern const char* GetNumBundlesSQL;
extern const char* TotalBytesSQL;
extern const char* DiscardExpiredSQL;
extern const char* ExpiredBytesSQL;
extern const char* DiscardEgressedSQL;
extern const char* EgressedBytesSQL;
extern const char* CreateTableSQL;

/* =================== */
/* Function Prototypes */
/* =================== */

BPLib_Status_t BPLib_STOR_FlushPendingUnlocked(BPLib_Instance_t* Inst);

#endif /* BPLIB_STOR_INTERNAL_H */
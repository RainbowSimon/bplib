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
#ifndef BPLIB_STOR_SQL_H
#define BPLIB_STOR_SQL_H

/* ======== */
/* Includes */
/* ======== */

#include "bplib_api_types.h"
#include "bplib_mem.h"
#include "bplib_eid.h"
#include "bplib_stor_loadbatch.h"

/* ====== */
/* Macros */
/* ====== */

#define BPLIB_SQL_MAX_STRLEN 4096

/* This is a temporary define denoting milliseconds between POSIX and DTN time
** It is used until a more mature boot-era solution is implemented in BPLIB_TIME
*/
#define BPLIB_STOR_EPOCHOFFSET 946684800000

/* ======== */
/* Typedefs */
/* ======== */

/*
** Result codes of SQLite executions:
** SQLITE_OK         =  0  Successful result
** SQLITE_ERROR      =  1  Generic error
** SQLITE_INTERNAL   =  2  Internal logic error in SQLite
** SQLITE_PERM       =  3  Access permission denied
** SQLITE_ABORT      =  4  Callback routine requested an abort
** SQLITE_BUSY       =  5  The database file is locked
** SQLITE_LOCKED     =  6  A table in the database is locked
** SQLITE_NOMEM      =  7  A malloc() failed
** SQLITE_READONLY   =  8  Attempt to write a readonly database
** SQLITE_INTERRUPT  =  9  Operation terminated by sqlite3_interrupt(
** SQLITE_IOERR      = 10  Some kind of disk I/O error occurred
** SQLITE_CORRUPT    = 11  The database disk image is malformed
** SQLITE_NOTFOUND   = 12  Unknown opcode in sqlite3_file_control()
** SQLITE_FULL       = 13  Insertion failed because database is full
** SQLITE_CANTOPEN   = 14  Unable to open the database file
** SQLITE_PROTOCOL   = 15  Database lock protocol error
** SQLITE_EMPTY      = 16  Internal use only
** SQLITE_SCHEMA     = 17  The database schema changed
** SQLITE_TOOBIG     = 18  String or BLOB exceeds size limit
** SQLITE_CONSTRAINT = 19  Abort due to constraint violation
** SQLITE_MISMATCH   = 20  Data type mismatch
** SQLITE_MISUSE     = 21  Library used incorrectly
** SQLITE_NOLFS      = 22  Uses OS features not supported on host
** SQLITE_AUTH       = 23  Authorization denied
** SQLITE_FORMAT     = 24  Not used
** SQLITE_RANGE      = 25  2nd parameter to sqlite3_bind out of range
** SQLITE_NOTADB     = 26  File opened that is not a database file
** SQLITE_NOTICE     = 27  Notifications from sqlite3_log()
** SQLITE_WARNING    = 28  Warnings from sqlite3_log()
** SQLITE_ROW        = 100 sqlite3_step() has another row ready
** SQLITE_DONE       = 101 sqlite3_step() has finished executing
*/
typedef uint8_t SQL_Status_t;

/* =================== */
/* Function Prototypes */
/* =================== */

BPLib_Status_t BPLib_SQL_Init(BPLib_Instance_t* Inst, const char* DbName);

BPLib_Status_t BPLib_SQL_Store(BPLib_Instance_t* Inst, size_t *TotalBytesStored);

BPLib_Status_t BPLib_SQL_DiscardExpired(BPLib_Instance_t* Inst, size_t* NumDiscarded);

BPLib_Status_t BPLib_SQL_DiscardEgressed(BPLib_Instance_t* Inst, size_t* NumDiscarded);

BPLib_Status_t BPLib_SQL_FindForEIDs(BPLib_Instance_t* Inst, BPLib_STOR_LoadBatch_t* Batch,
    BPLib_EID_Pattern_t *DestEIDs, size_t NumEIDs);

BPLib_Status_t BPLib_SQL_MarkBatchEgressed(BPLib_Instance_t* Inst, BPLib_STOR_LoadBatch_t* Batch);

BPLib_Status_t BPLib_SQL_LoadBundle(BPLib_Instance_t* Inst, int64_t BundleID, BPLib_Bundle_t** Bundle);

BPLib_Status_t BPLib_SQL_GetDbSize(BPLib_Instance_t *Inst, size_t *DbSize);

#endif /* BPLIB_STOR_SQL_H */

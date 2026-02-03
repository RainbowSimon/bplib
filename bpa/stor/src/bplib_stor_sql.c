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

/* ======== */
/* Includes */
/* ======== */

#include "bplib_stor_sql.h"
#include "bplib_time.h"
#include "bplib_fwp.h"
#include "bplib_inst.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/* ======= */
/* Globals */
/* ======= */

/* SQL query statements */

sqlite3_stmt* GetNumBundlesStmt;
sqlite3_stmt* GetNumEgressedStmt;
sqlite3_stmt* TotalBytesStmt;
sqlite3_stmt* DiscardEgressedStmt;
sqlite3_stmt* EgressedBytesStmt;
sqlite3_stmt* GetExpiredStmt;
sqlite3_stmt* DeleteExpiredStmt;

/* SQL query strings */

/*
** Table and Index Creation for bundle_data and bundle_blobs
**
** This schema is designed to support efficient queries and operations on bundle metadata and associated blob data.
** The following indexes are created:
**
** 1. idx_bundle_blobs_bundle_row:
**    - Index on the 'bundle_row' column in the 'bundle_blobs' table. This index supports quick lookup of blob data
**      by its associated bundle_row in the 'bundle_data' table.
**
** 2. idx_action_timestamp:
**    - Index on 'action_timestamp' in the 'bundle_data' table. This helps with queries that need to sort or filter
**      based on the timestamp of the bundle: This is used for expiring bundles
**
** 3. idx_egress_id (Composite Index):
**    - Composite index on the columns 'dest_node', 'dest_service', 'egress_attempted', 'action_timestamp', and 'id'.
**    - This index optimizes queries that filter by node and service ranges, filter by egress_attempted (0),
**      and sort by action_timestamp. It can also enable an index-only scan to quickly retrieve 'id'.
**    - This composite index is designed for loading egress bundles by batch for a particular EgressID (A channel or contact)
**
** 4. idx_egress_attempted:
**    - Index on the 'egress_attempted' column in the 'bundle_data' table. This index is designed to speed up
**      DELETE queries and other queries filtering by 'egress_attempted'.
**
** 5. idx_bundle_id
**    - Index on the bplib-assigned unique 'bundle_id' in the 'bundle_data' table. This is used to detect duplicate bundles
**      in storage and by Custody Transfer to request the deletion or retransmission of custodial bundles. Whether or not
**      to allow duplicate bundles in storage is toggled by the BPLIB_ALLOW_DUPLICATE_BUNDLES flag.
**/

const char* CreateTableSQL =
"CREATE TABLE IF NOT EXISTS bundle_data (\n"
"    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
#if BPLIB_ALLOW_DUPLICATE_BUNDLES == false
"    bundle_id INTEGER UNIQUE,\n"
#else
"    bundle_id INTEGER,\n"
#endif
"    action_timestamp INTEGER,\n"
"    retransmit_timestamp INTEGER,\n"
"    retransmit_trigger INTEGER,\n"
"    egress_attempted INTEGER DEFAULT 0,\n"
"    dest_node INTEGER,\n"
"    dest_service INTEGER,\n"
"    is_custodial INTEGER,\n"
"    bundle_bytes INTEGER\n"
");\n"
"\n"
"CREATE TABLE IF NOT EXISTS bundle_blobs (\n"
"    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
"    bundle_row INTEGER,\n"
"    blob_data BLOB,\n"
"    FOREIGN KEY (bundle_row) REFERENCES bundle_data(id) ON DELETE CASCADE\n"
");\n"
"\n"
"CREATE INDEX IF NOT EXISTS idx_bundle_blobs ON bundle_blobs (bundle_row);\n"
"CREATE INDEX IF NOT EXISTS idx_action_timestamp ON bundle_data (action_timestamp);\n"
#if BPLIB_ALLOW_DUPLICATE_BUNDLES == false
"CREATE UNIQUE INDEX IF NOT EXISTS idx_bundle_id ON bundle_data (bundle_id);\n"
#else
"CREATE INDEX IF NOT EXISTS idx_bundle_id ON bundle_data (bundle_id);\n"
#endif
"\n"
"CREATE INDEX IF NOT EXISTS idx_egress_id\n"
"ON bundle_data (\n"
"    dest_node,\n"
"    dest_service,\n"
"    egress_attempted,\n"
"    action_timestamp,\n"
"    is_custodial,\n"
"    id\n"
");\n"
"\n"
"CREATE INDEX IF NOT EXISTS idx_egress_attempted\n"
"ON bundle_data (egress_attempted);\n";


const char* GetNumBundlesSQL =
"SELECT COUNT(*) FROM bundle_data;";

const char* GetNumEgressedSQL =
"SELECT SUM(egress_attempted) "
"As NumEgressed "
"FROM bundle_data;";

const char* TotalBytesSQL =
"SELECT SUM(bundle_bytes) "
"AS TotalBytes "
"FROM bundle_data;";

const char* GetExpiredSQL = 
"SELECT id, bundle_bytes, bundle_id, is_custodial FROM bundle_data "
"WHERE (action_timestamp < ?) AND (egress_attempted = 0) "
"LIMIT ?;";

const char* DeleteExpiredSQL =
"DELETE FROM bundle_data WHERE id = ?;";

const char* DiscardEgressedSQL =
"WITH to_delete AS ("
"    SELECT id FROM bundle_data "
"    WHERE egress_attempted = 1 "
"    LIMIT ?"
") "
"DELETE FROM bundle_data "
"WHERE id IN (SELECT id FROM to_delete);";

const char* EgressedBytesSQL =
"WITH egressed_bytes AS (\n"
"   SELECT id, bundle_bytes FROM bundle_data\n"
"   WHERE egress_attempted = 1\n"
"   LIMIT ?)\n"
"SELECT SUM(bundle_bytes)\n"
"AS bytes_deleted\n"
"FROM bundle_data\n"
"WHERE id IN (SELECT id FROM egressed_bytes);\n";


/* ==================== */
/* Function Definitions */
/* ==================== */

SQL_Status_t BPLib_SQL_InitDb(const char* DbName, sqlite3** ActiveDbPtr)
{
    SQL_Status_t  SQLStatus;
    sqlite3*      ActiveDb;
    uint8_t       ForeignKeysEnabled;
    sqlite3_stmt* ForeignKeyCheckStmt;

    SQLStatus = sqlite3_open(DbName, ActiveDbPtr);
    if (SQLStatus != SQLITE_OK)
    {
        return SQLStatus;
    }

    ActiveDb = *ActiveDbPtr;

    /* Set the atomic commit and rollback method to write-ahead log */
    SQLStatus = sqlite3_exec(ActiveDb, "PRAGMA journal_mode=WAL;", 0, 0, NULL);
    if (SQLStatus != SQLITE_OK)
    {
        return SQLStatus;
    }

    /* Enable foreign key support */
    SQLStatus = sqlite3_exec(ActiveDb, "PRAGMA foreign_keys=ON;", 0, 0, NULL);
    if (SQLStatus != SQLITE_OK)
    {
        return SQLStatus;
    }

    /* Disable synchronization */
    SQLStatus = sqlite3_exec(ActiveDb, "PRAGMA synchronous=OFF;", 0, 0, NULL);
    if (SQLStatus != SQLITE_OK)
    {
        return SQLStatus;
    }

    /* Page size should already be 4096 by default, this just enforces it */
    SQLStatus = sqlite3_exec(ActiveDb, "PRAGMA page_size=4096;", 0, 0, NULL);
    if (SQLStatus != SQLITE_OK)
    {
        return SQLStatus;
    }

    /* Note: Apparently SQLite3 can have foreign_keys=ON fail SILENTLY if
    ** libsqlite3.so wasn't compiled with foreign key support. We have to manually
    ** check if foreign keys were enabled by reading the setting back.
    */
    ForeignKeysEnabled = 0;
    SQLStatus = sqlite3_prepare_v2(ActiveDb, "PRAGMA foreign_keys;", -1, &ForeignKeyCheckStmt, NULL);
    if (SQLStatus != SQLITE_OK)
    {
        return SQLStatus;
    }

    /* Check if the execution of foreign key enabling succeeded */
    if (sqlite3_step(ForeignKeyCheckStmt) == SQLITE_ROW)
    {
        ForeignKeysEnabled = sqlite3_column_int(ForeignKeyCheckStmt, 0);
    }

    sqlite3_finalize(ForeignKeyCheckStmt);
    if (ForeignKeysEnabled != 1)
    {
        fprintf(stderr, "Please use a SQLite3 compiled with Foreign Key Support.\n");
        return SQLITE_MISUSE;
    }

    return SQLITE_OK;
}

SQL_Status_t BPLib_SQL_InitTable(BPLib_Instance_t* Inst)
{
    SQL_Status_t SQLStatus;
    uint32_t     NumStoredBundles;
    uint64_t     TotalBundleBytes;
    size_t       NumEgressed;

    NumStoredBundles = 0;
    TotalBundleBytes = 0;
    NumEgressed      = 0;

    /* Create the table if it doesn't already exist */
    SQLStatus = sqlite3_exec(Inst->BundleStorage.db, CreateTableSQL, 0, 0, NULL);
    if (SQLStatus != SQLITE_OK)
    {
        return SQLStatus;
    }

    /* Determine how many bundles are presently in storage, and set the stored counter to this value */
    SQLStatus = BPLib_SQL_GetNumStoredBundles(Inst->BundleStorage.db, &NumStoredBundles);
    if (SQLStatus != SQLITE_OK)
    {
        return SQLStatus;
    }
    
    Inst->BundleStorage.BundleCountStored = NumStoredBundles;

    SQLStatus = BPLib_SQL_GetNumEgressed(Inst->BundleStorage.db, &NumEgressed);
    if (SQLStatus != SQLITE_OK)
    {
        return SQLStatus;
    }

    Inst->BundleStorage.BundleCountNotEgressed = NumStoredBundles - NumEgressed;

    /* Find the total number of bytes of bundles stored */
    SQLStatus = BPLib_SQL_GetTotalBundleBytes(Inst->BundleStorage.db, &TotalBundleBytes);
    if (SQLStatus == SQLITE_OK)
    {
        Inst->BundleStorage.BytesStorageInUse = TotalBundleBytes;
    }

    return SQLStatus;
}

BPLib_Status_t BPLib_SQL_Init(BPLib_Instance_t* Inst, const char* DbName)
{
    BPLib_Status_t Status;
    SQL_Status_t   SQLStatus;

    Status = BPLIB_SUCCESS;

    SQLStatus = BPLib_SQL_InitDb(DbName, &Inst->BundleStorage.db);
    if (SQLStatus == SQLITE_OK)
    {
        SQLStatus = BPLib_SQL_InitTable(Inst);
    }

    if (SQLStatus != SQLITE_OK)
    {
        Status = BPLIB_STOR_SQL_INIT_ERR;
    }

    return Status;
}

SQL_Status_t BPLib_SQL_GetNumStoredBundles(sqlite3 *db, uint32_t *BundleCnt)
{
    SQL_Status_t SQLStatus;

    SQLStatus = sqlite3_prepare_v2(db, GetNumBundlesSQL, -1, &GetNumBundlesStmt, NULL);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    SQLStatus = sqlite3_step(GetNumBundlesStmt);
    if (SQLStatus != SQLITE_ROW)
    {
        return SQLStatus;
    }

    *BundleCnt = sqlite3_column_int(GetNumBundlesStmt, 0);

    sqlite3_finalize(GetNumBundlesStmt);

    return SQLITE_OK;
}

SQL_Status_t BPLib_SQL_GetNumEgressed(sqlite3* db, size_t *EgressCnt)
{
    SQL_Status_t SQLStatus;

    SQLStatus = sqlite3_prepare_v2(db, GetNumEgressedSQL, -1, &GetNumEgressedStmt, NULL);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    SQLStatus = sqlite3_step(GetNumEgressedStmt);
    if (SQLStatus != SQLITE_ROW)
    {
        return SQLStatus;
    }

    *EgressCnt = sqlite3_column_int(GetNumEgressedStmt, 0);

    sqlite3_finalize(GetNumEgressedStmt);
    
    return SQLITE_OK;
}

SQL_Status_t BPLib_SQL_GetTotalBundleBytes(sqlite3* db, uint64_t* TotalBytes)
{
    SQL_Status_t SQLStatus;

    /* Load up the SQL command */
    SQLStatus = sqlite3_prepare_v2(db, TotalBytesSQL, -1, &TotalBytesStmt, NULL);
    if (SQLStatus == SQLITE_OK)
    {
        /* Evaluate the command */
        SQLStatus = sqlite3_step(TotalBytesStmt);
        if (SQLStatus == SQLITE_ROW)
        {
            /* Assign the result of the query to TotalBytes */
            *TotalBytes = sqlite3_column_int64(TotalBytesStmt, 0);
            sqlite3_finalize(TotalBytesStmt);

            /* Set the status to a success value */
            SQLStatus = SQLITE_OK;
        }
        else
        {
            fprintf(stderr, "Error code %s received while evaluating the SQL statement: %s\n",
                    sqlite3_errmsg(db),
                    TotalBytesSQL);
        }
    }
    else
    {
        fprintf(stderr, "Error code %s, received while preparing SQL statement: %s\n",
                sqlite3_errmsg(db),
                TotalBytesSQL);
    }

    return SQLStatus;
}

BPLib_Status_t BPLib_SQL_GetDbSize(BPLib_Instance_t *Inst, size_t *DbSize)
{
    SQL_Status_t  SQLStatus;
    size_t        PageCnt;
    sqlite3_stmt* PageCntStmt;

    *DbSize = 0;
    PageCnt = 0;

    SQLStatus = sqlite3_prepare_v2(Inst->BundleStorage.db, "PRAGMA page_count;", -1, &PageCntStmt, NULL);
    if (SQLStatus != SQLITE_OK)
    {
        return BPLIB_ERROR;
    }

    if (sqlite3_step(PageCntStmt) == SQLITE_ROW)
    {
        PageCnt = sqlite3_column_int(PageCntStmt, 0);
    }

    sqlite3_finalize(PageCntStmt);

    *DbSize = PageCnt * 4096;

    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_SQL_DiscardExpired(BPLib_Instance_t* Inst, size_t* NumDiscarded)
{
    BPLib_Status_t Status;
    SQL_Status_t   SQLStatus;
    sqlite3*       db;

    Status = BPLIB_SUCCESS;
    db     = Inst->BundleStorage.db;

    SQLStatus = sqlite3_prepare_v2(db, GetExpiredSQL, -1, &GetExpiredStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prep: %s\n", sqlite3_errmsg(db));
        return BPLIB_STOR_SQL_DISCARD_ERR;
    }

    SQLStatus = sqlite3_prepare_v2(db, DeleteExpiredSQL, -1, &DeleteExpiredStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prep: %s\n", sqlite3_errmsg(db));
        return BPLIB_STOR_SQL_DISCARD_ERR;
    }

    SQLStatus = BPLib_SQL_DiscardExpiredImpl(db, NumDiscarded, Inst);
    if (SQLStatus != SQLITE_OK)
    {
        Status = BPLIB_STOR_SQL_DISCARD_ERR;
    }

    /* Finalize the statement */
    sqlite3_finalize(GetExpiredStmt);
    sqlite3_finalize(DeleteExpiredStmt);

    return Status;
}

SQL_Status_t BPLib_SQL_DiscardExpiredImpl(sqlite3* db, size_t* NumDiscarded, BPLib_Instance_t* Inst)
{
    SQL_Status_t SQLStatus;
    uint64_t     MonoTime;
    size_t       ExpiredBytes;
    uint32_t     CurrBundleId;
    int64_t      BundleRow;
    int64_t      IsCustodial;
    size_t       NumToDiscard;

   *NumDiscarded = 0;
    ExpiredBytes = 0;
    NumToDiscard = 0;

    /* Get DTN Time */
    MonoTime = BPLib_TIME_GetMonotonicTime();

    /* Create a batch query */
    SQLStatus = sqlite3_exec(db, "BEGIN;", 0, 0, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to start transaction: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    sqlite3_reset(GetExpiredStmt);

    /* Get all bundle rows to expire */

    SQLStatus = sqlite3_bind_int64(GetExpiredStmt, 1, (int64_t)MonoTime);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind action_timestamp: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    SQLStatus = sqlite3_bind_int64(GetExpiredStmt, 2, BPLIB_STOR_DISCARDBATCHSIZE);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind LIMIT: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    SQLStatus = sqlite3_step(GetExpiredStmt);

    /* Step through expired bundle rows */
    while (SQLStatus == SQLITE_ROW)
    {
        /* Get information about bundle to delete */
        Inst->BundleStorage.BundleRowsToExpire[NumToDiscard] = sqlite3_column_int64(GetExpiredStmt, 0);
        ExpiredBytes += sqlite3_column_int64(GetExpiredStmt, 1);
        CurrBundleId = sqlite3_column_int64(GetExpiredStmt, 2);
        IsCustodial = sqlite3_column_int64(GetExpiredStmt, 3);

        if (IsCustodial != 0)
        {
            Inst->BundleStorage.BundleCountInCustody--;
            BPLib_CT_DeleteBundleFromCtdb(Inst, CurrBundleId);
        }

        SQLStatus = sqlite3_step(GetExpiredStmt);

        NumToDiscard++;
    }

    for (BundleRow = 0; BundleRow < NumToDiscard; BundleRow++)
    {
        sqlite3_reset(DeleteExpiredStmt);
     
        /* Delete bundle from storage */
        SQLStatus = sqlite3_bind_int64(DeleteExpiredStmt, 1,
                                    Inst->BundleStorage.BundleRowsToExpire[BundleRow]); 
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to bind BundleRow: %s\n", sqlite3_errmsg(db));
            return SQLStatus;
        }

        SQLStatus = sqlite3_step(DeleteExpiredStmt);
        if (SQLStatus != SQLITE_DONE)
        {
            fprintf(stderr, "Failed to delete expired bundle: %s\n", sqlite3_errmsg(db));
            return SQLStatus;            
        }
    }

    /* If there have been no errors so far commit the delete */
    if (SQLStatus == SQLITE_DONE)
    {
        SQLStatus = sqlite3_exec(db, "COMMIT;", 0, 0, 0);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to commit transaction\n");
        }
    }

    /* The batch commit was not successful, ROLLBACK to prevent DB corruption */
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Attempting ROLLBACK\n");

        SQLStatus = sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to rollback transaction, RC=%d\n", SQLStatus);
        }
    }

    /* Determine how many changes were made to the database, which is the number
    ** of discarded bundles.
    */
    *NumDiscarded = NumToDiscard;

    /* Decrement that counter that tracks bytes of storage used */
    Inst->BundleStorage.BytesStorageInUse -= ExpiredBytes;    

    return SQLITE_OK;
}

BPLib_Status_t BPLib_SQL_DiscardEgressed(BPLib_Instance_t* Inst, size_t* NumDiscarded)
{
    BPLib_Status_t Status;
    SQL_Status_t   SQLStatus;
    sqlite3*       db;

    Status = BPLIB_SUCCESS;
    db     = Inst->BundleStorage.db;

    SQLStatus = sqlite3_prepare_v2(db, DiscardEgressedSQL, -1, &DiscardEgressedStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prep: %s\n", sqlite3_errmsg(db));
        Status = BPLIB_STOR_SQL_DISCARD_ERR;
    }

    if (Status == BPLIB_SUCCESS)
    {
        SQLStatus = BPLib_SQL_DiscardEgressedImpl(db, NumDiscarded, &(Inst->BundleStorage));
        if (SQLStatus != SQLITE_OK)
        {
            Status = BPLIB_STOR_SQL_DISCARD_ERR;
        }
    }

    /* Finalize the statement */
    sqlite3_finalize(DiscardEgressedStmt);

    return Status;
}

SQL_Status_t BPLib_SQL_DiscardEgressedImpl(sqlite3* db, size_t* NumDiscarded, BPLib_BundleCache_t* BundleCache)
{
    SQL_Status_t SQLStatus;
    size_t       EgressedBytes;

    *NumDiscarded = 0;
    EgressedBytes = 0;

    /* Collect the size of the bundles to be discarded */
    /* Load up the SQL command */
    SQLStatus = sqlite3_prepare_v2(db, EgressedBytesSQL, -1, &EgressedBytesStmt, NULL);
    if (SQLStatus == SQLITE_OK)
    {
        SQLStatus = sqlite3_bind_int64(EgressedBytesStmt, 1, BPLIB_STOR_DISCARDBATCHSIZE);
        if (SQLStatus == SQLITE_OK)
        {
            /* Evaluate the command */
            SQLStatus = sqlite3_step(EgressedBytesStmt);
            if (SQLStatus == SQLITE_ROW)
            {
                /* Assign the result of the query to EgressedBytes */
                EgressedBytes = sqlite3_column_int64(EgressedBytesStmt, 0);

                /* Amount is decremented when the command to discard is successful */
                sqlite3_finalize(EgressedBytesStmt);
            }
            else
            {
                fprintf(stderr, "Error code %s received while evaluating the SQL statement: %s\n",
                        sqlite3_errmsg(db),
                        EgressedBytesSQL);
            }
        }
        else
        {
            fprintf(stderr, "Failed to bind LIMIT: %s\n", sqlite3_errmsg(db));
        }
    }
    else
    {
        fprintf(stderr, "Error code %s, received while preparing SQL statement: %s\n",
                sqlite3_errmsg(db),
                EgressedBytesSQL);
    }

    /* Create a batch query */
    SQLStatus = sqlite3_exec(db, "BEGIN;", 0, 0, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to start transaction: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    sqlite3_reset(DiscardEgressedStmt);

    SQLStatus = sqlite3_bind_int64(DiscardEgressedStmt, 1, BPLIB_STOR_DISCARDBATCHSIZE);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind LIMIT: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    /* Run the query */
    SQLStatus = sqlite3_step(DiscardEgressedStmt);
    if (SQLStatus != SQLITE_DONE)
    {
        fprintf(stderr, "Failed to discard egressed bundles: %s\n", sqlite3_errmsg(db));  
        return SQLStatus;
    }

    /* If there have been no errors so far commit the delete  */
    if (SQLStatus == SQLITE_DONE)
    {
        SQLStatus = sqlite3_exec(db, "COMMIT;", 0, 0, 0);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to commit transaction\n");
        }
    }

    /* The batch commit was not successful, ROLLBACK to prevent DB corruption */
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Attempting ROLLBACK\n");
        SQLStatus = sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to rollback transaction, RC=%d\n", SQLStatus);
        }
    }

    /* Determine how many changes were made to the database, which is the number
    ** of discarded bundles.
    */
    *NumDiscarded = sqlite3_changes(db);

    /* Decrement that counter that tracks bytes of storage used */
    BundleCache->BytesStorageInUse -= EgressedBytes;

    return SQLITE_OK;
}

BPLib_Status_t BPLib_SQL_Cleanup(BPLib_Instance_t* Inst)
{
    BPLib_Status_t Status = BPLIB_SUCCESS;
    int SQLStatus;

    SQLStatus = sqlite3_exec(Inst->BundleStorage.db, "VACUUM;", 0, 0, NULL);

    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to vacuum: %s\n", sqlite3_errmsg(Inst->BundleStorage.db));
        Status = BPLIB_STOR_CLEANUP_ERR;
    }    

    return Status;
}

BPLib_Status_t BPLib_SQL_GetDestEidWhereClause(BPLib_EID_Pattern_t* DestEIDs, size_t NumEIDs, 
                                                                        char *WhereClause)
{
    size_t         MaxWhereLen;
    size_t         CurrWhereLen;
    size_t         i;
    const char*    FindForDestNodeSql_RangeClause = "((dest_node BETWEEN ? AND ?) AND";
    const char*    FindForDestNodeSql_EqualityClause = "((dest_node = ?) AND";
    const char*    FindForDestServSql_RangeClause = " (dest_service BETWEEN ? AND ?))";
    const char*    FindForDestServSql_EqalityClause = " (dest_service = ?))";
    const char**   FindClausePtr;

    MaxWhereLen = BPLIB_SQL_MAX_STRLEN - 180; /* size of final query minus the non-where clause stuff */

    /* To keep search as efficient possible, we generate one combined query that contains all
    ** the DestEID patterns.
    **
    ** NOTE:
    ** This bit is tricky to understand from inspection. Basically, for each destination
    ** eid pattern node number or service number it checks if the pattern is a true pattern
    ** depicting a range, or if the minimum value equals the maximum value. In the former 
    ** case, it checks if either dest_node or dest_service is "BETWEEN ? AND ?". In the
    ** latter case it does an exact value check, so whether dest_node or dest_service "= ?".
    ** This is a minor optimization since exact queries are a bit faster in sqlite than 
    ** range queries. The final output should look something like 
    ** "((dest_node BETWEEN ? AND ?)) AND (dest_service = ?)) OR 
    **  ((dest_node = ?) AND (dest_service BETWEEN ? AND ?)) OR
    **  ((dest_node BETWEEN ? AND ?) AND (dest_service BETWEEN ? AND ?))",
    ** or any additional combinations of the sort, depending on the DestEIDs.
    */

    CurrWhereLen = 0;

    for (i = 0; i < NumEIDs; i++)
    {
        if (DestEIDs[i].MaxNode == 0 && DestEIDs[i].MaxService == 0)
        {
            /* dtn:none detected, skip this */
            break;
        }
        /* If maxNode == minNode, do an exact query, otherwise do a range query */
        if (DestEIDs[i].MaxNode == DestEIDs[i].MinNode)
        {
            CurrWhereLen += strlen(FindForDestNodeSql_EqualityClause);
            FindClausePtr = &FindForDestNodeSql_EqualityClause;
        }
        else
        {
            CurrWhereLen += strlen(FindForDestNodeSql_RangeClause);
            FindClausePtr = &FindForDestNodeSql_RangeClause;
        }

        if (CurrWhereLen >= MaxWhereLen)
        {
            fprintf(stderr, "Programming Error: Node WHERE clause too long\n");
            return BPLIB_STOR_SQL_OVERFLOW_ERR;       
        }
        else
        {
            /* Add the node query to the where clause */
            strncat(WhereClause, *FindClausePtr, MaxWhereLen - strlen(WhereClause));
        }

        /* If MaxService == MinService, do an exact query, otherwise do a range query */
        if (DestEIDs[i].MaxService == DestEIDs[i].MinService)
        {
            CurrWhereLen += strlen(FindForDestServSql_EqalityClause);
            FindClausePtr = &FindForDestServSql_EqalityClause;
        }
        else
        {
            CurrWhereLen += strlen(FindForDestServSql_RangeClause);
            FindClausePtr = &FindForDestServSql_RangeClause;
        }

        if (CurrWhereLen >= MaxWhereLen)
        {
            fprintf(stderr, "Programming Error: Node WHERE clause too long\n");
            return BPLIB_STOR_SQL_OVERFLOW_ERR;
        }
        else
        {
            /* Add the service query to the where clause */
            strncat(WhereClause, *FindClausePtr, MaxWhereLen - strlen(WhereClause));
        }

        /* Link multiple EID queries with an OR, unless this is the last EID */
        if (i != (NumEIDs - 1) && !(DestEIDs[i + 1].MaxNode == 0 && DestEIDs[i + 1].MaxService == 0))
        {
            if (CurrWhereLen + strlen(" OR ") >= MaxWhereLen)
            {
                fprintf(stderr, "Programming Error: OR WHERE clause too long\n");
                return BPLIB_STOR_SQL_OVERFLOW_ERR;
            }
            else
            {
                strncat(WhereClause, " OR ", MaxWhereLen - CurrWhereLen);
                CurrWhereLen += strlen(" OR ");
            }
        }
    }

    WhereClause[strlen(WhereClause)] = '\0';  
    
    return BPLIB_SUCCESS;
}

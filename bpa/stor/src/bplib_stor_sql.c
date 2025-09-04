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
#include "bplib_qm.h"
#include "bplib_time.h"
#include "bplib_fwp.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/* ==================== */
/* Function Definitions */
/* ==================== */

SQL_Status_t BPLib_SQL_InitDb(const char* DbName, sqlite3** ActiveDbPtr)
{
    SQL_Status_t SQLStatus;
    sqlite3*     ActiveDb;
    uint8_t      ForeignKeysEnabled;

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
    
    NumStoredBundles = 0;
    TotalBundleBytes = 0;

    /* Create the table if it doesn't already exist */
    SQLStatus = sqlite3_exec(Inst->BundleStorage.db, CreateTableSQL, 0, 0, NULL);
    if (SQLStatus != SQLITE_OK)
    {
        return SQLStatus;
    }

    /* Determine how many bundles are presently in storage, and set the stored counter to this value */
    if (BPLib_SQL_GetNumStoredBundles(Inst->BundleStorage.db, &NumStoredBundles) != SQLITE_OK)
    {
        return SQLStatus;
    }
    
    Inst->BundleStorage.BundleCountStored = NumStoredBundles;

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
    SQL_Status_t SQLStatus;
    size_t       PageCnt;

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

    SQLStatus = sqlite3_prepare_v2(db, DiscardExpiredSQL, -1, &DiscardExpiredStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prep: %s\n", sqlite3_errmsg(db));
        Status = BPLIB_STOR_SQL_DISCARD_ERR;
    }

    if (Status == BPLIB_SUCCESS)
    {
        SQLStatus = BPLib_SQL_DiscardExpiredImpl(db, NumDiscarded, &(Inst->BundleStorage));
        if (SQLStatus != SQLITE_OK)
        {
            Status = BPLIB_STOR_SQL_DISCARD_ERR;
        }
    }

    /* Finalize the statement */
    sqlite3_finalize(DiscardExpiredStmt);

    return Status;
}

SQL_Status_t BPLib_SQL_DiscardExpiredImpl(sqlite3* db, size_t* NumDiscarded, BPLib_BundleCache_t* BundleCache)
{
    //BPLib_TIME_MonotonicTime_t DtnMonotonicTime;
    SQL_Status_t SQLStatus;
    uint64_t     DtnMonoTime;
    size_t       ExpiredBytes;

    *NumDiscarded = 0;
    ExpiredBytes  = 0;

    /* Get DTN Time */
    DtnMonoTime = BPLib_TIME_GetMonotonicTime();

    /* Collect the size of the bundles to be discarded */
    /* Load up the SQL command */
    SQLStatus = sqlite3_prepare_v2(db, ExpiredBytesSQL, -1, &ExpiredBytesStmt, NULL);
    if (SQLStatus == SQLITE_OK)
    {
        SQLStatus = sqlite3_bind_int64(ExpiredBytesStmt, 1, (int64_t) DtnMonoTime);
        if (SQLStatus == SQLITE_OK)
        {
            SQLStatus = sqlite3_bind_int64(ExpiredBytesStmt, 2, BPLIB_STOR_DISCARDBATCHSIZE);
            if (SQLStatus == SQLITE_OK)
            {
                /* Evaluate the command */
                SQLStatus = sqlite3_step(ExpiredBytesStmt);
                if (SQLStatus == SQLITE_ROW)
                {
                    /* Assign the result of the query to EgressedBytes */
                    ExpiredBytes = sqlite3_column_int64(ExpiredBytesStmt, 0);

                    /* Amount is decremented when the command to discard is successful */
                    sqlite3_finalize(ExpiredBytesStmt);
                }
                else
                {
                    fprintf(stderr, "Error code %s received while evaluating the SQL statement: %s\n",
                            sqlite3_errmsg(db),
                            ExpiredBytesSQL);
                }
            }
            else
            {
                fprintf(stderr, "Failed to bind LIMIT: %s\n", sqlite3_errmsg(db));
            }
        }
        else
        {
            fprintf(stderr, "Failed to bind action_timestamp: %s\n", sqlite3_errmsg(db));
        }
    }
    else
    {
        fprintf(stderr, "Error code %s, received while preparing SQL statement: %s\n",
                sqlite3_errmsg(db),
                ExpiredBytesSQL);
    }

    /* Create a batch query */
    SQLStatus = sqlite3_exec(db, "BEGIN;", 0, 0, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to start transaction: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    sqlite3_reset(DiscardExpiredStmt);

    SQLStatus = sqlite3_bind_int64(DiscardExpiredStmt, 1, (int64_t)DtnMonoTime);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind action_timestamp: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    SQLStatus = sqlite3_bind_int64(DiscardExpiredStmt, 2, BPLIB_STOR_DISCARDBATCHSIZE);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind LIMIT: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    /* Run the query */
    SQLStatus = sqlite3_step(DiscardExpiredStmt);
    if (SQLStatus != SQLITE_DONE)
    {
        fprintf(stderr, "Failed to discard expired bundles: %s\n", sqlite3_errmsg(db));  
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
    BundleCache->BytesStorageInUse -= ExpiredBytes;    

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
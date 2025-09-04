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

#include "bplib_stor_sql.h"
#include "bplib_stor_sql_load.h"
#include "bplib_qm.h"
#include "bplib_stor.h"

#include <stdio.h>

/* ==================== */
/* Function Definitions */
/* ==================== */

BPLib_Status_t BPLib_SQL_FindForEIDs(BPLib_Instance_t* Inst, BPLib_STOR_LoadBatch_t* Batch,
                                        BPLib_EID_Pattern_t* DestEIDs, size_t NumEIDs)
{
    BPLib_Status_t Status;
    SQL_Status_t   SQLStatus;
    uint8_t        i;
    sqlite3*       db;
    size_t         MaxWhereLen;
    sqlite3_stmt*  FindForEgressIDStmt;
    char           WhereClause[BPLIB_SQL_MAX_STRLEN]        = {0};
    char           FindForEgressIdSQL[BPLIB_SQL_MAX_STRLEN] = {0};

    Status      = BPLIB_SUCCESS;
    db          = Inst->BundleStorage.db;
    MaxWhereLen = BPLIB_SQL_MAX_STRLEN - 102; /* size of final query minus the non-where clause stuff */

    if ((Inst == NULL) || (Batch == NULL) || (DestEIDs == NULL))
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if ((NumEIDs == 0) || (NumEIDs > BPLIB_MAX_CONTACT_DEST_EIDS))
    {
        return BPLIB_STOR_PARAM_ERR;
    }

    /* To keep search as efficient possible, we generate one combined query that contains all
    ** the DestEID patterns.
    **
    ** NOTE:
    ** This bit is tricky to understand from inspection. It is just taking the
    ** "((dest_node BETWEEN ? AND ?) AND (dest_service BETWEEN ? AND ?))" and appending it once
    **  for each DestEID in the EgressID array.
    */

    /*
    ** There's no risk of overflow here since the size of
    ** FindForEgressID_RangeClause is known and much less than the buffer size
    */
    strncat(WhereClause, FindForEgressID_RangeClause, MaxWhereLen);

    for (i = 1; i < NumEIDs; i++)
    {
        if ((strlen(WhereClause) + strlen(" OR ")) < MaxWhereLen)
        {
            strncat(WhereClause, " OR ", MaxWhereLen - strlen(WhereClause));
        }
        else
        {
            fprintf(stderr, "Programming Error: WHERE clause too long\n");
            return BPLIB_STOR_SQL_OVERFLOW_ERR;
        }

        if ((strlen(WhereClause) + strlen(FindForEgressID_RangeClause)) < MaxWhereLen)
        {
            strncat(WhereClause, FindForEgressID_RangeClause, MaxWhereLen - strlen(WhereClause));
        }
        else
        {
            fprintf(stderr, "Programming Error: WHERE clause too long\n");
            return BPLIB_STOR_SQL_OVERFLOW_ERR;
        }
    }

    WhereClause[strlen(WhereClause)] = '\0';

    /* Build the final query */
    snprintf(FindForEgressIdSQL, BPLIB_SQL_MAX_STRLEN,
            "SELECT id FROM bundle_data WHERE (%s) AND egress_attempted = 0 ORDER BY action_timestamp ASC LIMIT ?;",
            WhereClause);

    FindForEgressIdSQL[strlen(FindForEgressIdSQL)] = '\0';

    /* Prepare Search Statements needed for this batch query */
    SQLStatus = sqlite3_prepare_v2(db, FindForEgressIdSQL, -1, &FindForEgressIDStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Programming Error: FindForEgressIdSQL prepare failed, error=%s\n", sqlite3_errmsg(db));
        Status = BPLIB_STOR_SQL_LOAD_IDS_ERR;
    }

    if (Status == BPLIB_SUCCESS)
    {
        /* Run Batch Load Logic */
        SQLStatus = BPLib_SQL_FindForEIDsImpl(Inst, &FindForEgressIDStmt, Batch, DestEIDs, NumEIDs, BPLIB_STOR_LOADBATCHSIZE);

        if (SQLStatus != SQLITE_OK)
        {
            Status = BPLIB_STOR_SQL_LOAD_IDS_ERR;
        }
    }

    /* Cleanup/Finalize */
    sqlite3_finalize(FindForEgressIDStmt);

    return Status;
}

SQL_Status_t BPLib_SQL_FindForEIDsImpl(BPLib_Instance_t* Inst, sqlite3_stmt** FindForEgressIDStmt,
                                        BPLib_STOR_LoadBatch_t* Batch, BPLib_EID_Pattern_t* DestEIDs,
                                        size_t NumEIDs, size_t MaxBundles)
{
    SQL_Status_t  SQLStatus;
    sqlite3*      db;
    uint8_t       CurrBundleRowID;
    uint8_t       i;
    uint8_t       BindIndex;
    sqlite3_stmt* FindStmt;

    db       = Inst->BundleStorage.db;
    FindStmt = *FindForEgressIDStmt;

    /* Bind parameters for metadata query */
    sqlite3_reset(FindStmt);

    BindIndex = 1;
    for (i = 0; i < NumEIDs; i++)
    {
        SQLStatus = sqlite3_bind_int64(FindStmt, BindIndex++, DestEIDs[i].MinNode);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to bind dest_node min: %s\n", sqlite3_errmsg(db));
            return SQLStatus;
        }

        SQLStatus = sqlite3_bind_int64(FindStmt, BindIndex++, DestEIDs[i].MaxNode);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to bind dest_node max: %s\n", sqlite3_errmsg(db));
            return SQLStatus;
        }

        SQLStatus = sqlite3_bind_int64(FindStmt, BindIndex++, DestEIDs[i].MinService);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to bind dest_node min: %s\n", sqlite3_errmsg(db));
            return SQLStatus;
        }

        SQLStatus = sqlite3_bind_int64(FindStmt, BindIndex++, DestEIDs[i].MaxService);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to bind dest_node max: %s\n", sqlite3_errmsg(db));
            return SQLStatus;
        }
    }

    SQLStatus = sqlite3_bind_int64(FindStmt, BindIndex++, MaxBundles);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind limit: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    /* For every bundle found, place it's ID in the load batch for the EgressID */
    SQLStatus = sqlite3_step(FindStmt);
    while (SQLStatus == SQLITE_ROW)
    {
        /* Load a single bundle from storage that matches the query */
        CurrBundleRowID = sqlite3_column_int64(FindStmt, 0);
        if (BPLib_STOR_LoadBatch_AddID(Batch, CurrBundleRowID) != BPLIB_SUCCESS)
        {
            break;
        }

        /* Go to the next row, which corresponds to the next bundle ID */
        SQLStatus = sqlite3_step(FindStmt);
    }

    if (SQLStatus == SQLITE_DONE)
    {
        /* For consistency with other helpers, convert DONE to OK */
        SQLStatus = SQLITE_OK;
    }

    /* Expecting SQLITE_OK */
    return SQLStatus;
}

BPLib_Status_t BPLib_SQL_MarkBatchEgressed(BPLib_Instance_t* Inst, BPLib_STOR_LoadBatch_t* Batch)
{
    BPLib_Status_t Status;
    SQL_Status_t   SQLStatus;
    sqlite3*       db;

    Status = BPLIB_SUCCESS;
    db     = Inst->BundleStorage.db;

    if (Inst == NULL)
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    SQLStatus = sqlite3_prepare_v2(db, MarkEgressedSQL, -1, &MarkEgressedStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Programming Error: MarkEgressedSQL prepare failed, error=%s\n", sqlite3_errmsg(db));
        Status = BPLIB_STOR_SQL_MARK_EGRESSED_ERR;
    }

    if (Status == BPLIB_SUCCESS)
    {
        SQLStatus = BPLib_SQL_MarkBatchEgressedImpl(Inst, Batch);
    }

    sqlite3_finalize(MarkEgressedStmt);

    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Programming Error: MarkEgressedSQL finalize failed, error=%s\n", sqlite3_errmsg(db));
        return BPLIB_STOR_SQL_MARK_EGRESSED_ERR;
    }

    return BPLIB_SUCCESS;
}

SQL_Status_t BPLib_SQL_MarkBatchEgressedImpl(BPLib_Instance_t* Inst, BPLib_STOR_LoadBatch_t* Batch)
{
    SQL_Status_t SQLStatus;
    sqlite3*     db;
    uint8_t      i;

    db = Inst->BundleStorage.db;

    /* Create a batch query */
    SQLStatus = sqlite3_exec(db, "BEGIN;", 0, 0, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to start transaction\n");
        return SQLStatus;
    }

    /* Go through the load batch and add each ID as egressed */
    for (i = 0; i < Batch->Size; i++)
    {
        sqlite3_reset(MarkEgressedStmt);

        sqlite3_bind_int64(MarkEgressedStmt, 1, Batch->BundleRowIDs[i]);

        SQLStatus = sqlite3_step(MarkEgressedStmt);
        if (SQLStatus != SQLITE_DONE)
        {
            fprintf(stderr, "Mark Egressed Failed: %s\n", sqlite3_errstr(SQLStatus));
            break;
        }
    }

    /* If there have been no errors so far, batch-write the data to persistent storage */
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
        SQLStatus = sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to rollback transaction, RC=%d\n", SQLStatus);
        }
    }

    /* Expecting SQLITE_OK */
    return SQLStatus;
}

BPLib_Status_t BPLib_SQL_LoadBundle(BPLib_Instance_t* Inst, int64_t BundleRowID,
                                    BPLib_Bundle_t** Bundle)
{
    BPLib_Status_t Status;
    SQL_Status_t   SQLStatus;
    sqlite3*       db;

    Status = BPLIB_SUCCESS;
    db     = Inst->BundleStorage.db;

    if ((Inst == NULL) || (Bundle == NULL))
    {
        return BPLIB_NULL_PTR_ERROR;
    }

    if (BundleRowID < 0)
    {
        return BPLIB_STOR_PARAM_ERR;
    }

    SQLStatus = sqlite3_prepare_v2(db, FindBlobSQL, -1, &FindBlobStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Programming Error: FindBlobSQL prepare failed, error=%s\n", sqlite3_errmsg(db));
        Status = BPLIB_STOR_SQL_LOAD_ERR;
    }

    if (Status == BPLIB_SUCCESS)
    {
        SQLStatus = BPLib_SQL_LoadBundleImpl(Inst, BundleRowID, Bundle);
    }

    sqlite3_finalize(FindBlobStmt);

    if (SQLStatus == SQLITE_NOMEM)
    {
        Status = BPLIB_STOR_NO_MEM_ERR;
    }
    else if (*Bundle == NULL)
    {
        Status = BPLIB_STOR_NO_BUNDLE_FOUND_ERR;
    }
    else if (SQLStatus != SQLITE_OK)
    {
        Status = BPLIB_STOR_SQL_LOAD_ERR;
    }

    return Status;
}

SQL_Status_t BPLib_SQL_LoadBundleImpl(BPLib_Instance_t* Inst, int64_t BundleRowID,
                                        BPLib_Bundle_t** Bundle)
{
    SQL_Status_t       SQLStatus;
    sqlite3_blob*      blob;
    int64_t            BlobRowId;
    size_t             ChunkSize;
    BPLib_MEM_Block_t* BundleHead;
    BPLib_MEM_Block_t* CurrBlock;
    BPLib_MEM_Block_t* NextBlock;
    BPLib_Bundle_t*    RetBundle;
    sqlite3*           db;
    BPLib_MEM_Pool_t*  Pool;

    blob       = NULL;
    BundleHead = NULL;
    CurrBlock  = NULL;
    NextBlock  = NULL;
    db         = Inst->BundleStorage.db;
    Pool       = &Inst->pool;

    sqlite3_reset(FindBlobStmt);

    SQLStatus = sqlite3_bind_int(FindBlobStmt, 1, BundleRowID);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "bind failed: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    /* Load each block of the blob into a mem pool block */
    while ((SQLStatus = sqlite3_step(FindBlobStmt)) == SQLITE_ROW)
    {
        /* Determine the row ID of the blob in the bundle_blobs table */
        BlobRowId = sqlite3_column_int64(FindBlobStmt, 0);

        /* We're loading bblocks_t */
        if (BundleHead == NULL)
        {
            /* Open the metadata using the blob streaming API */
            SQLStatus = sqlite3_blob_open(db, "main", "bundle_blobs", "blob_data", BlobRowId, 0, &blob);
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "sqlite3_blob_open failed for rowid %ld: %s\n", BlobRowId, sqlite3_errmsg(db));
                break;
            }

            /* Ensure the metadata size exactly matches bblocks_t size. Otherwise, don't bother loading it */
            ChunkSize = sqlite3_blob_bytes(blob);
            if (ChunkSize != sizeof(BPLib_BBlocks_t))
            {
                fprintf(stderr, "Expected to read metadata chunk and got wrong size %lu != %lu\n",
                        ChunkSize, sizeof(BPLib_BBlocks_t));

                SQLStatus = SQLITE_CORRUPT;

                sqlite3_blob_close(blob);
                break;
            }

            /* Allocate a MEM pool block for the meta data */
            BundleHead = BPLib_MEM_BlockAlloc(Pool);
            if (BundleHead == NULL)
            {
                SQLStatus = SQLITE_NOMEM;
                sqlite3_blob_close(blob);
                break;
            }

            /* Load the metadata directly into the mempool block */
            SQLStatus = sqlite3_blob_read(blob, (void*)&BundleHead->user_data.bundle.blocks, ChunkSize, 0);
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "sqlite3_blob_read failed: %s\n", sqlite3_errmsg(db));
                sqlite3_blob_close(blob);
                break;
            }

            /* Load succeeded */
            CurrBlock = BundleHead;
            CurrBlock->used_len = ChunkSize;
        }
        else
        { /* We're loading part of the blob */
            /* Open the metadata using the blob streaming API */
            SQLStatus = sqlite3_blob_open(db, "main", "bundle_blobs", "blob_data", BlobRowId, 0, &blob);
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "sqlite3_blob_open failed for rowid %ld: %s\n", BlobRowId, sqlite3_errmsg(db));
                break;
            }

            /* Allocate a MEM pool block for the blob data */
            NextBlock = BPLib_MEM_BlockAlloc(Pool);
            if (NextBlock == NULL)
            {
                SQLStatus = SQLITE_NOMEM;
                sqlite3_blob_close(blob);
                break;
            }

            CurrBlock->next = NextBlock;

            /* Make sure the chunk isn't larger than the buffer. This could happen if a previous
            ** database or BPLib version had a different chunk size. We can't support this case safely. User
            ** will have to create a new database.
            */
            ChunkSize = sqlite3_blob_bytes(blob);
            if (ChunkSize > sizeof(NextBlock->user_data.raw_bytes))
            {
                fprintf(stderr, "Stored BLOB is too large for buffer. DB is corrupted %lu > %lu\n",
                        ChunkSize, sizeof(NextBlock->user_data.raw_bytes));

                SQLStatus = SQLITE_CORRUPT;

                sqlite3_blob_close(blob);
                break;
            }


            /* Load the blob directly into the mempool block */
            SQLStatus = sqlite3_blob_read(blob, (void*)&NextBlock->user_data.raw_bytes, ChunkSize, 0);
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "sqlite3_blob_read failed: %s\n", sqlite3_errmsg(db));
                sqlite3_blob_close(blob);
                break;
            }

            /* Load Succeeded */
            CurrBlock = NextBlock;
            CurrBlock->used_len = ChunkSize;
        }

        sqlite3_blob_close(blob);
    }

    if (BundleHead == NULL)
    {
        *Bundle = NULL;

        /*
        ** Leave status code as is, could be either a lack of memory issue (SQLITE_NOMEM)
        ** or an indication that the bundle no longer exists in storage
        */
    }
    else if (SQLStatus == SQLITE_DONE)
    { /* Expecting SQLITE_DONE */
        RetBundle       = (BPLib_Bundle_t*)(BundleHead);
        RetBundle->blob = BundleHead->next;
        *Bundle         = RetBundle;

        /* For consistency with other helpers, set status to SQLITE_OK */
        SQLStatus = SQLITE_OK;
    }
    else
    {
        /* Something in the loop above went wrong: Free memory */
        BPLib_MEM_BlockListFree(Pool, BundleHead);
    }

    /* Expecting SQLITE_OK */
    return SQLStatus;
}
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
#include "bplib_as.h"
#include "bplib_nc.h"
#include "bplib_stor_sql_store.h"

#include <stdio.h>

/* ======= */
/* Globals */
/* ======= */

/* SQL query statements */

sqlite3_stmt* InsertBlobStmt;
sqlite3_stmt* InsertMetadataStmt;

/* SQL query strings */

/* Insert Bundle Blob */
const char* InsertBlobSQL =
"INSERT INTO bundle_blobs (bundle_row, blob_data) VALUES (?, ?)";

/* Insert Bundle Metadata (duplicate bundle_id entries are ignored) */
const char* InsertMetadataSQL =
"INSERT INTO bundle_data (bundle_id, action_timestamp, retransmit_timestamp, dest_node, dest_service, is_custodial, bundle_bytes) VALUES (?, ?, ?, ?, ?, ?);";

/* ================ */
/* Helper Functions */
/* ================ */

#if 0
/* 
** Helper function to print contents of sqlite bundle_data table for debugging. Should be
** kept commented out in main. Use by calling:
**      sqlite3_exec(db, "SELECT * FROM bundle_data;", BPLib_SQL_PrintTbl, 0, 0);
*/
static int BPLib_SQL_PrintTbl(void* data, int argc, char** argv, char** azColName)
{
    int i;

    printf("*******************\n");
    fprintf(stderr, "%s: ", (const char*)data);

    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("*******************\n");
    return 0;
}
#endif 

/* ==================== */
/* Function Definitions */
/* ==================== */

SQL_Status_t BPLib_SQL_StoreMetadata(BPLib_Bundle_t* Bundle, BPLib_BundleCache_t* BundleCache)
{
    SQL_Status_t         SQLStatus;
    BPLib_PrimaryBlock_t PrimaryBlock;
    uint64_t             AgeBlockTime;
    uint64_t             ExpirationTime;
    uint64_t             MonoTimeAge;
    uint64_t             MonoTimeRemaining;
    uint16_t             ExtensionBlockIdx;

    PrimaryBlock   = Bundle->blocks.PrimaryBlock;
    ExpirationTime = 0;

    sqlite3_reset(InsertMetadataStmt);

    /* Bind bundle_id to InsertMetadataStmt */
    SQLStatus = sqlite3_bind_int64(InsertMetadataStmt, 1, 
                                    (int64_t)Bundle->blocks.PrimaryBlock.BundleId);

    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind bundle_id in store_meta\n");
    }
    else
    {
        /* Add the value of the timestamp used as an indicator for some action to the InsertMetadataStmt variable (action_timestamp) */
        
        if (PrimaryBlock.Timestamp.CreateTime != 0)
        { /* Bundle has a valid creation time */
            if (BPLib_TIME_GetCurrentDtnTime() != 0)
            { /* DTN time is valid */
                MonoTimeAge       = BPLib_TIME_GetCurrentDtnTime() - PrimaryBlock.Timestamp.CreateTime;
                MonoTimeRemaining = PrimaryBlock.Lifetime          - MonoTimeAge;
                ExpirationTime    = BPLib_TIME_GetMonotonicTime()  + MonoTimeRemaining;
            }
            else
            { /* DTN time is invalid */
                ExpirationTime = PrimaryBlock.MonoTime.Time + PrimaryBlock.Lifetime;
            }
        }
        else
        { /* Bundle creation time is invalid */
            for (ExtensionBlockIdx = 0; ExtensionBlockIdx < BPLIB_MAX_NUM_EXTENSION_BLOCKS; ExtensionBlockIdx++)
            {
                if (Bundle->blocks.ExtBlocks[ExtensionBlockIdx].Header.BlockType == BPLib_BlockType_Age)
                {
                    MonoTimeAge    = PrimaryBlock.MonoTime.Time + PrimaryBlock.Lifetime;
                    AgeBlockTime   = Bundle->blocks.ExtBlocks[ExtensionBlockIdx].BlockData.AgeBlockData.Age;
                    ExpirationTime = MonoTimeAge - AgeBlockTime;

                    break;
                }
            }
        }
        
        SQLStatus = sqlite3_bind_int64(InsertMetadataStmt, 2, (sqlite3_int64) ExpirationTime);
    }

    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind action_timestamp in store_meta\n");
    }
    else
    {
        /* Add the retransmission timestamp into the InsertMetadataStmt variable */
        SQLStatus = sqlite3_bind_int64(InsertMetadataStmt, 3, 
                                (int64_t)BPLib_TIME_GetMonotonicTime() + Bundle->Meta.RetransmitTime);
    }
    
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind retransmit_timestamp in store_meta\n");
    }
    else
    {
        /* Add the destination node into the InsertMetadataStmt variable */
        SQLStatus = sqlite3_bind_int64(InsertMetadataStmt, 4, 
                                (int64_t)Bundle->blocks.PrimaryBlock.DestEID.Node);
    }

    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind dest_node in store_meta\n");
    }
    else
    {
        /* Add the destination service into the InsertMetadataStmt variable */
        SQLStatus = sqlite3_bind_int64(InsertMetadataStmt, 5, 
                                (int64_t)Bundle->blocks.PrimaryBlock.DestEID.Service);
    }

    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind dest_service in store_meta\n");
    }
    else
    {
        /* Add whether the bundle is custodial or not into the InsertMetadataStmt variable */
        SQLStatus = sqlite3_bind_int64(InsertMetadataStmt, 6, (int64_t)Bundle->Meta.IsCustodial);
    }

    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind is_custodial in store_meta\n");
    }
    else
    {
        /* Add the bundle size in bytes into the InsertMetadataStmt variable */
        SQLStatus = sqlite3_bind_int64(InsertMetadataStmt, 7, (int64_t)Bundle->Meta.TotalBytes);
    }    

    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind bundle_size in store_meta\n");
    }
    else
    {
        /* Evaluate provided sqlite statement */
        SQLStatus = sqlite3_step(InsertMetadataStmt);
    }

    /* Expecting SQLITE_DONE */
    return SQLStatus;
}

SQL_Status_t BPLib_SQL_StoreChunk(int64_t BundleRowID, const void* Chunk, size_t ChunkSize)
{
    SQL_Status_t SQLStatus;

    sqlite3_reset(InsertBlobStmt);
    sqlite3_bind_int64(InsertBlobStmt, 1, BundleRowID);
    sqlite3_bind_blob(InsertBlobStmt, 2, Chunk, ChunkSize, SQLITE_STATIC);

    SQLStatus = sqlite3_step(InsertBlobStmt);
    if (SQLStatus != SQLITE_DONE)
    {
        return SQLStatus;
    }

    return SQLStatus;
}

SQL_Status_t BPLib_SQL_StoreBundle(sqlite3* db, BPLib_Bundle_t* Bundle, BPLib_BundleCache_t* BundleCache)
{
    SQL_Status_t       SQLStatus;
    uint64_t           BundleRowID;
    BPLib_MEM_Block_t* CurrMemBlock;

    /* Store the indexable metadata */
    SQLStatus = BPLib_SQL_StoreMetadata(Bundle, BundleCache);
    if (SQLStatus != SQLITE_DONE)
    {
        return SQLStatus;
    }

    BundleRowID = sqlite3_last_insert_rowid(db);

    /* Store the decoded metadata block */
    SQLStatus = BPLib_SQL_StoreChunk(BundleRowID, (const void*)&Bundle->blocks, sizeof(BPLib_BBlocks_t));
    if (SQLStatus != SQLITE_DONE)
    {
        return SQLStatus;
    }

    /* Store the blob chunks */
    CurrMemBlock = Bundle->blob;
    while (CurrMemBlock != NULL)
    {
        SQLStatus = BPLib_SQL_StoreChunk(BundleRowID, (const void*)CurrMemBlock->user_data.raw_bytes,
                                            CurrMemBlock->used_len);

        if (SQLStatus != SQLITE_DONE)
        {
            return SQLStatus;
        }

        CurrMemBlock = CurrMemBlock->next;
    }

    /* Expecting SQLITE_DONE */
    return SQLStatus;
}

SQL_Status_t BPLib_SQL_StoreImpl(BPLib_Instance_t* Inst, size_t *TotalBytesStored,
                                size_t *DuplicateBundlesIgnored, size_t *CustodialBundles)
{
    SQL_Status_t SQLStatus;
    size_t       i;
    sqlite3*     db;
    size_t       NewBundleBytes;

    db = Inst->BundleStorage.db;

    /* Create a batch query */
    SQLStatus = sqlite3_exec(db, "BEGIN;", 0, 0, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to start transaction: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    /* Perform an insert for every bundle */
    for (i = 0; i < Inst->BundleStorage.InsertBatchSize; i++)
    {
        /* Check that inserting the bundle won't cause the storage limit to be exceeded */
        NewBundleBytes = Inst->BundleStorage.InsertBatch[i]->Meta.TotalBytes;
        if (Inst->BundleStorage.BytesStorageInUse + *TotalBytesStored + NewBundleBytes > BPLIB_MAX_STORED_BUNDLE_BYTES)
        {
            SQLStatus = SQLITE_FULL;
            break;
        }
        
        SQLStatus = BPLib_SQL_StoreBundle(db, Inst->BundleStorage.InsertBatch[i], &(Inst->BundleStorage));
        if (SQLStatus == SQLITE_DONE)
        {
            *TotalBytesStored += NewBundleBytes;

            if (Inst->BundleStorage.InsertBatch[i]->Meta.IsCustodial)
            {
                (*CustodialBundles)++;
            }
        }
        else
        {
            if (sqlite3_extended_errcode(db) == SQLITE_CONSTRAINT_UNIQUE)
            {
                /* Duplicate bundle detected, ignore error */
                SQLStatus = SQLITE_DONE;
                (*DuplicateBundlesIgnored)++;
            }
            else
            {
                /* If there was an error, don't keep trying to construsct the SQL INSERT */
                break;
            }
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
    if (SQLStatus == SQLITE_FULL)
    {
        /* Don't want to override this error code, assume rollback works */
        (void) sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
    }
    else if (SQLStatus != SQLITE_OK)
    {
        /* Some other error occurred */
        fprintf(stderr, "Batch commit failed, RC=%d\n", sqlite3_extended_errcode(db));

        SQLStatus = sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to rollback transaction, RC=%d\n", 
                    sqlite3_extended_errcode(db));
        }
    }

    /* Expecting SQLITE_OK */
    return SQLStatus;
}

BPLib_Status_t BPLib_SQL_Store(BPLib_Instance_t* Inst, size_t *TotalBytesStored,
                                size_t *DuplicateBundlesIgnored, size_t *CustodialBundles)
{
    BPLib_Status_t Status;
    SQL_Status_t   SQLStatus;
    sqlite3*       db;

    Status = BPLIB_SUCCESS;
    db     = Inst->BundleStorage.db;

    /* Prepare Insert Statements needed for this batch query */
    SQLStatus = sqlite3_prepare_v2(db, InsertMetadataSQL, -1, &InsertMetadataStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        Status = BPLIB_STOR_SQL_STORAGE_ERR;
    }

    SQLStatus = sqlite3_prepare_v2(db, InsertBlobSQL, -1, &InsertBlobStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        Status = BPLIB_STOR_SQL_STORAGE_ERR;
    }

    if (Status == BPLIB_SUCCESS)
    {
        /* Run the batch storage logic */
        SQLStatus = BPLib_SQL_StoreImpl(Inst, TotalBytesStored, DuplicateBundlesIgnored, CustodialBundles);
        if (SQLStatus == SQLITE_FULL)
        {
            Status = BPLIB_STOR_DB_FULL_ERR;
        }
        else if (SQLStatus != SQLITE_OK)
        {
            Status = BPLIB_STOR_SQL_STORAGE_ERR;
        }
    }

    /* Finalize */
    sqlite3_finalize(InsertMetadataStmt);
    sqlite3_finalize(InsertBlobStmt);

    return Status;
}

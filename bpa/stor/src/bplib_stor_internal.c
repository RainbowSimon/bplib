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

#include "bplib_stor_internal.h"
#include "bplib_stor_sql_store.h"
#include "bplib_eid.h"
#include "bplib_as.h"

/* ======= */
/* Globals */
/* ======= */

/* SQL query statements */
sqlite3_stmt* FindForEgressIDStmt;
sqlite3_stmt* MarkEgressedStmt;
sqlite3_stmt* FindBlobStmt;
sqlite3_stmt* InsertBlobStmt;
sqlite3_stmt* InsertMetadataStmt;
sqlite3_stmt* ForeignKeyCheckStmt;
sqlite3_stmt* GetNumBundlesStmt;
sqlite3_stmt* TotalBytesStmt;
sqlite3_stmt* PageCntStmt;
sqlite3_stmt* DiscardExpiredStmt;
sqlite3_stmt* ExpiredBytesStmt;
sqlite3_stmt* DiscardEgressedStmt;
sqlite3_stmt* EgressedBytesStmt;

/* SQL query strings */
const char WhereClause[BPLIB_SQL_MAX_STRLEN / 2]    = {0};
const char FindForEgressIdSQL[BPLIB_SQL_MAX_STRLEN] = {0};

const char* FindForEgressID_RangeClause =
"((dest_node BETWEEN ? AND ?) AND (dest_service BETWEEN ? AND ?))";

const char* MarkEgressedSQL =
"UPDATE bundle_data SET egress_attempted = 1 WHERE id = ?;";

const char* FindBlobSQL =
"SELECT id\n"
"FROM bundle_blobs\n"
"WHERE bundle_id = ?;";

const char* InsertBlobSQL =
"INSERT INTO bundle_blobs (bundle_id, blob_data) VALUES (?, ?)";

const char* InsertMetadataSQL =
"INSERT INTO bundle_data (action_timestamp, dest_node, dest_service, bundle_bytes) VALUES (?, ?, ?, ?);";

const char* GetNumBundlesSQL =
"SELECT COUNT(*) FROM bundle_data;";

const char* TotalBytesSQL =
"SELECT SUM(bundle_bytes) "
"AS TotalBytes "
"FROM bundle_data;";

const char* DiscardExpiredSQL =
"WITH to_delete AS ("
"    SELECT id FROM bundle_data "
"    WHERE (action_timestamp < ?) AND (egress_attempted = 0) "
"    LIMIT ?"
") "
"DELETE FROM bundle_data "
"WHERE id IN (SELECT id FROM to_delete);";

const char* ExpiredBytesSQL =
"WITH expired_bytes AS (\n"
"   SELECT id, bundle_bytes FROM bundle_data\n"
"   WHERE (action_timestamp < ?) AND (egress_attempted = 0)\n"
"   LIMIT ?)\n"
"SELECT SUM(bundle_bytes)\n"
"AS bytes_deleted\n"
"FROM bundle_data\n"
"WHERE id IN (SELECT id FROM expired_bytes);\n";

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

/*
** Table and Index Creation for bundle_data and bundle_blobs
**
** This schema is designed to support efficient queries and operations on bundle metadata and associated blob data.
** The following indexes are created:
**
** 1. idx_bundle_blobs_bundle_id:
**    - Index on the 'bundle_id' column in the 'bundle_blobs' table. This index supports quick lookup of blob data
**      by its associated bundleID in the 'bundle_data' table.
**
** 2. idx_action_timestamp:
**    - Index on 'action_timestamp' in the 'bundle_data' table. This helps with queries that need to sort or filter
**      based on the timestamp of the bundle: This is used for expiring bundles
**
** 3. idx_find_bundle (Composite Index):
**    - Composite index on the columns 'dest_node', 'dest_service', 'egress_attempted', 'action_timestamp', and 'id'.
**    - This index optimizes queries that filter by node and service ranges, filter by egress_attempted (0),
**      and sort by action_timestamp. It can also enable an index-only scan to quickly retrieve 'id'.
**    - This composite index is designed for loading egress bundles by batch for a particular EgressID (A channel or contact)
**
** 4. idx_egress_attempted:
**    - Index on the 'egress_attempted' column in the 'bundle_data' table. This index is designed to speed up
**      DELETE queries and other queries filtering by 'egress_attempted'.
*/

const char* CreateTableSQL =
"CREATE TABLE IF NOT EXISTS bundle_data (\n"
"    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
"    action_timestamp INTEGER,\n"
"    egress_attempted INTEGER DEFAULT 0,\n"
"    dest_node INTEGER,\n"
"    dest_service INTEGER,\n"
"    bundle_bytes INTEGER\n"
");\n"
"\n"
"CREATE TABLE IF NOT EXISTS bundle_blobs (\n"
"    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
"    bundle_id INTEGER,\n"
"    blob_data BLOB,\n"
"    FOREIGN KEY (bundle_id) REFERENCES bundle_data(id) ON DELETE CASCADE\n"
");\n"
"\n"
"CREATE INDEX IF NOT EXISTS idx_bundle_blobs ON bundle_blobs (bundle_id);\n"
"CREATE INDEX IF NOT EXISTS idx_action_timestamp ON bundle_data (action_timestamp);\n"
"\n"
"CREATE INDEX IF NOT EXISTS idx_egress_id\n"
"ON bundle_data (\n"
"    dest_node,\n"
"    dest_service,\n"
"    egress_attempted,\n"
"    action_timestamp,\n"
"    id\n"
");\n"
"\n"
"CREATE INDEX IF NOT EXISTS idx_egress_attempted\n"
"ON bundle_data (egress_attempted);\n";

/* ==================== */
/* Function Definitions */
/* ==================== */

BPLib_Status_t BPLib_STOR_FlushPendingUnlocked(BPLib_Instance_t* Inst)
{
    BPLib_Status_t       Status;
    BPLib_BundleCache_t* CacheInst;
    uint8_t              i;
    size_t               TotalBytesStored;

    TotalBytesStored = 0;
    CacheInst        = &Inst->BundleStorage;

    Status = BPLib_SQL_Store(Inst, &TotalBytesStored);

    if (Status == BPLIB_SUCCESS)
    {
        CacheInst->BytesStorageInUse += TotalBytesStored;
        CacheInst->BundleCountStored += CacheInst->InsertBatchSize;
    }
    else if (Status == BPLIB_STOR_DB_FULL_ERR)
    {
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED, CacheInst->InsertBatchSize);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DISCARDED, CacheInst->InsertBatchSize);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED_NO_STORAGE, CacheInst->InsertBatchSize);

        BPLib_EM_SendEvent(BPLIB_STOR_DB_FULL_INF_EID,
                            BPLib_EM_EventType_INFORMATION,
                            "SQLite database is full, dropping %d bundles",
                            CacheInst->InsertBatchSize);
    }
    else
    {
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DELETED, CacheInst->InsertBatchSize);
        BPLib_AS_Increment(BPLIB_EID_INSTANCE, BUNDLE_COUNT_DISCARDED, CacheInst->InsertBatchSize);

        BPLib_EM_SendEvent(BPLIB_STOR_SQL_STORE_ERR_EID,
                            BPLib_EM_EventType_ERROR,
                            "BPLib_SQL_Store failed to store bundle. RC=%d",
                            Status);

    }

    /* Free the bundles, as they're now persistent
    ** Note: even if the storage fails, we free everything to avoid a leak.
    */
    for (i = 0; i < CacheInst->InsertBatchSize; i++)
    {
        BPLib_MEM_BundleFree(&Inst->pool, CacheInst->InsertBatch[i]);
    }

    CacheInst->InsertBatchSize = 0;

    return Status;
}
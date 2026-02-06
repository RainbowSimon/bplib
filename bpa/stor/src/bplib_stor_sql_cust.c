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

#include "bplib_stor_sql_cust.h"
#include "bplib_inst.h"
#include <stdio.h>

sqlite3_stmt* SetRetransmitAllStmt;
sqlite3_stmt* TriggerRetransmitStmt;
sqlite3_stmt* MarkForDeletionStmt;
sqlite3_stmt* StopRetransmitStmt;

char SetNewRetransmitTriggerSQL[BPLIB_SQL_MAX_STRLEN] = {0};

const char* SetNewRetransmitTriggerBaseSQL =
"WITH to_update AS (\n"
    "SELECT id FROM bundle_data INDEXED BY idx_egress_id WHERE (%s) \n"
    "AND is_custodial = 1 AND egress_attempted = 0\n"
")\n"
"UPDATE bundle_data SET retransmit_trigger = ?, retransmit_timestamp = ? \n"
"WHERE id IN (SELECT id FROM to_update);";

const char* TriggerRetransmitSQL =
"UPDATE bundle_data SET retransmit_timestamp = ? WHERE (bundle_id = ? AND egress_attempted = 0);";

const char* StopRetransmitSQL =
"UPDATE bundle_data SET retransmit_trigger = ? WHERE (bundle_id = ? AND egress_attempted = 0);";

const char* MarkForDeletionSQL =
"UPDATE bundle_data SET egress_attempted = 1 WHERE bundle_id = ?;";



BPLib_Status_t BPLib_SQL_SetNewRetransmitTrigger(BPLib_Instance_t *Inst, uint32_t ContactId,
                BPLib_EID_Pattern_t* DestEIDs, size_t NumEIDs, size_t RetransmitTrigger, size_t *NumUpdated)
{
    BPLib_Status_t Status;
    SQL_Status_t   SQLStatus;
    sqlite3*       db;
    char           WhereClause[BPLIB_SQL_MAX_STRLEN] = {0};

    db          = Inst->BundleStorage.db;
    *NumUpdated = 0;

    /* Destination EIDs are set to dtn:none, don't bother searching database */
    if (DestEIDs[0].MaxNode == 0 && DestEIDs[0].MaxService == 0)
    {
        return BPLIB_SUCCESS;
    }

    Status = BPLib_SQL_GetDestEidWhereClause(DestEIDs, NumEIDs, WhereClause);
    if (Status != BPLIB_SUCCESS)
    {
        return Status;
    }

    /* 
    ** Build the final query. Note that "INDEXED BY idx_egress_id" is necessary to force
    ** sqlite to use the right index, otherwise it will try to use the much slower
    ** egress_attempted index by default, which will result in worse performance.
    */
    snprintf(SetNewRetransmitTriggerSQL, BPLIB_SQL_MAX_STRLEN, SetNewRetransmitTriggerBaseSQL,
            WhereClause);

    SetNewRetransmitTriggerSQL[strlen(SetNewRetransmitTriggerSQL)] = '\0';

    /* Prepare Search Statements needed for this batch query */
    SQLStatus = sqlite3_prepare_v2(db, SetNewRetransmitTriggerSQL, -1, &SetRetransmitAllStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Programming Error: SetNewRetransmitTriggerSQL prepare failed, error=%s\n", sqlite3_errmsg(db));
        Status = BPLIB_STOR_SQL_NEW_RETRANSMIT_ERR;
    }

    if (Status == BPLIB_SUCCESS)
    {
        /* Run Batch Load Logic */
        SQLStatus = BPLib_SQL_SetNewRetransmitTriggerImpl(db, DestEIDs, NumEIDs, 
                                                            RetransmitTrigger, NumUpdated);

        if (SQLStatus != SQLITE_OK)
        {
            Status = BPLIB_STOR_SQL_NEW_RETRANSMIT_ERR;
        }
    }

    /* Cleanup/Finalize */
    sqlite3_finalize(SetRetransmitAllStmt);
    
    return Status;
}

SQL_Status_t BPLib_SQL_SetNewRetransmitTriggerImpl(sqlite3* db, BPLib_EID_Pattern_t* DestEIDs, 
                            size_t NumEIDs, size_t RetransmitTrigger, size_t *NumUpdated)
{
    SQL_Status_t  SQLStatus;
    size_t        i;
    uint64_t      BindIndex;

    /* Bind parameters for query */
    sqlite3_reset(SetRetransmitAllStmt);

    BindIndex = 1;
    /* Bind destination EID query */
    for (i = 0; i < NumEIDs; i++)
    {
        if (DestEIDs[i].MaxNode == 0 && DestEIDs[i].MaxNode == 0)
        {
            break;
        }

        SQLStatus = sqlite3_bind_int64(SetRetransmitAllStmt, BindIndex++, DestEIDs[i].MinNode);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to bind dest_node min: %s\n", sqlite3_errmsg(db));
            return SQLStatus;
        }

        if (DestEIDs[i].MinNode != DestEIDs[i].MaxNode)
        {
            SQLStatus = sqlite3_bind_int64(SetRetransmitAllStmt, BindIndex++, DestEIDs[i].MaxNode);
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "Failed to bind dest_node max: %s\n", sqlite3_errmsg(db));
                return SQLStatus;
            }
        }

        SQLStatus = sqlite3_bind_int64(SetRetransmitAllStmt, BindIndex++, DestEIDs[i].MinService);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to bind dest_node min: %s\n", sqlite3_errmsg(db));
            return SQLStatus;
        }

        if (DestEIDs[i].MinService != DestEIDs[i].MaxService)
        {
            SQLStatus = sqlite3_bind_int64(SetRetransmitAllStmt, BindIndex++, DestEIDs[i].MaxService);
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "Failed to bind dest_node max: %s\n", sqlite3_errmsg(db));
                return SQLStatus;
            }
        }

    }

    /* Bind retransmit_trigger to new value */
    SQLStatus = sqlite3_bind_int64(SetRetransmitAllStmt, BindIndex++, RetransmitTrigger);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind retransmit_trigger: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    /* Bind retransmit_time to automatically trigger retransmission */
    SQLStatus = sqlite3_bind_int64(SetRetransmitAllStmt, BindIndex++, 
                                            BPLib_TIME_GetMonotonicTime());
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind retransmit_time: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    SQLStatus = sqlite3_step(SetRetransmitAllStmt);

    if (SQLStatus == SQLITE_DONE)
    {
        /* For consistency with other helpers, convert DONE to OK */
        SQLStatus = SQLITE_OK;
    }

    *NumUpdated = sqlite3_changes(db);

    /* Expecting SQLITE_OK */
    return SQLStatus;    
}

BPLib_Status_t BPLib_SQL_UpdateCustodialBundles(BPLib_Instance_t *Inst, BPLib_CT_CcsUpdateBatch_t *Batch)
{
    SQL_Status_t   SQLStatus;
    sqlite3*       db;

    db     = Inst->BundleStorage.db;

    SQLStatus = sqlite3_prepare_v2(db, TriggerRetransmitSQL, -1, &TriggerRetransmitStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Programming Error: TriggerRetransmitSQL prepare failed, error=%s\n", sqlite3_errmsg(db));
        return BPLIB_SQL_CUSTODY_UPDATE_ERR;
    }

    SQLStatus = sqlite3_prepare_v2(db, StopRetransmitSQL, -1, &StopRetransmitStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Programming Error: StopRetransmitSQL prepare failed, error=%s\n", sqlite3_errmsg(db));
        return BPLIB_SQL_CUSTODY_UPDATE_ERR;
    }

    SQLStatus = sqlite3_prepare_v2(db, MarkForDeletionSQL, -1, &MarkForDeletionStmt, 0);    
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Programming Error: MarkForDeletionSQL prepare failed, error=%s\n", sqlite3_errmsg(db));
        return BPLIB_SQL_CUSTODY_UPDATE_ERR;
    }

    SQLStatus = BPLib_SQL_UpdateCustodialBundlesImpl(db, Batch);

    sqlite3_finalize(MarkForDeletionStmt);
    sqlite3_finalize(StopRetransmitStmt);
    sqlite3_finalize(TriggerRetransmitStmt);

    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Programming Error: MarkEgressedSQL finalize failed, error=%s\n", sqlite3_errmsg(db));
        return BPLIB_SQL_CUSTODY_UPDATE_ERR;
    }

    return BPLIB_SUCCESS;
}

SQL_Status_t BPLib_SQL_UpdateCustodialBundlesImpl(sqlite3* db, BPLib_CT_CcsUpdateBatch_t *Batch)
{
    SQL_Status_t SQLStatus;
    size_t       i;

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
        /* Start retransmission by setting the retransmission timestamp to now */
        if (Batch->Ops[i] == BPLIB_CT_START_RETRANSMIT)
        {
            sqlite3_reset(TriggerRetransmitStmt);

            /* Set the retransmit_timestamp to the current time to trigger an egress on the next egress option */
            SQLStatus = sqlite3_bind_int64(TriggerRetransmitStmt, 1, BPLib_TIME_GetMonotonicTime());
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "Failed to bind retransmit_timestamp: %s\n", sqlite3_errmsg(db));
                return SQLStatus;
            }

            /* Set bundle_id to current bundle ID in the batch */
            SQLStatus = sqlite3_bind_int64(TriggerRetransmitStmt, 2, Batch->BundleIDs[i]);
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "Failed to bind bundle_id: %s\n", sqlite3_errmsg(db));
                return SQLStatus;                
            }
            
            SQLStatus = sqlite3_step(TriggerRetransmitStmt);
            if (SQLStatus != SQLITE_DONE)
            {
                fprintf(stderr, "Start Retransmit Failed: %s\n", sqlite3_errstr(SQLStatus));
                break;
            }
        }
        /* Turn off retransmission by setting the retransmission time to garbage */
        else if (Batch->Ops[i] == BPLIB_CT_STOP_RETRANSMIT)
        {
            sqlite3_reset(StopRetransmitStmt);

            /* Set the retransmit_trigger to a garbage value to turn it off */
            SQLStatus = sqlite3_bind_int64(StopRetransmitStmt, 1, BPLIB_NO_RETRANSMIT_TRIGGER);            
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "Failed to bind retransmit_trigger: %s\n", sqlite3_errmsg(db));
                return SQLStatus;
            }

            /* Set bundle_id to current bundle ID in the batch */
            SQLStatus = sqlite3_bind_int64(StopRetransmitStmt, 2, Batch->BundleIDs[i]);
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "Failed to bind bundle_id: %s\n", sqlite3_errmsg(db));
                return SQLStatus;                
            }
            
            SQLStatus = sqlite3_step(StopRetransmitStmt);
            if (SQLStatus != SQLITE_DONE)
            {
                fprintf(stderr, "Stop Retransmit Failed: %s\n", sqlite3_errstr(SQLStatus));
                break;
            }
        }
        /* Mark bundle for deletion */
        else /* (Batch->Ops[i] == BPLIB_CT_MARK_DELETE) */
        {
            sqlite3_reset(MarkForDeletionStmt);

            /* Set bundle_id to current bundle ID in the batch */
            SQLStatus = sqlite3_bind_int64(MarkForDeletionStmt, 1, Batch->BundleIDs[i]);
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "Failed to bind bundle_id: %s\n", sqlite3_errmsg(db));
                return SQLStatus;                
            }
            
            SQLStatus = sqlite3_step(MarkForDeletionStmt);
            if (SQLStatus != SQLITE_DONE)
            {
                fprintf(stderr, "Mark for Deletion Failed: %s\n", sqlite3_errstr(SQLStatus));
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

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
#include <stdio.h>

sqlite3_stmt* SetRetransmitStmt;

char SetNewRetransmitTriggerSQL[BPLIB_SQL_MAX_STRLEN] = {0};

const char* SetNewRetransmitTriggerBaseSQL =
"WITH to_update AS (\n"
    "SELECT id FROM bundle_data INDEXED BY idx_egress_id WHERE (%s) \n"
    "AND is_custodial = 1 AND egressed_attempted = 0;\n"
")\n"
"UPDATE bundle_data SET retransmit_trigger = ? AND retransmit_timestamp = ? \n"
"WHERE id IN (SELECT id FROM to_update);";


BPLib_Status_t BPLib_STOR_SetNewRetransmitTrigger(BPLib_Instance_t *Inst, uint32_t ContactId,
                                BPLib_EID_Pattern_t* DestEIDs, size_t NumEIDs, size_t RetransmitTrigger)
{
    BPLib_Status_t Status;
    SQL_Status_t   SQLStatus;
    sqlite3*       db;
    char           WhereClause[BPLIB_SQL_MAX_STRLEN] = {0};

    db          = Inst->BundleStorage.db;

    Status = BPLib_SQL_GetDestEidWhereClause(DestEIDs, NumEIDs, WhereClause);
    if (Status != BPLIB_SUCCESS)
    {
        // TODO report?
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
    SQLStatus = sqlite3_prepare_v2(db, SetNewRetransmitTriggerSQL, -1, &SetRetransmitStmt, 0);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Programming Error: SetNewRetransmitTriggerSQL prepare failed, error=%s\n", sqlite3_errmsg(db));
        Status = BPLIB_STOR_SET_NEW_RETRANSMIT_ERR;
    }

    if (Status == BPLIB_SUCCESS)
    {
        /* Run Batch Load Logic */
        SQLStatus = BPLib_SQL_SetNewRetransmitTriggerImpl(db, DestEIDs, NumEIDs, 
                                                                    RetransmitTrigger);

        if (SQLStatus != SQLITE_OK)
        {
            Status = BPLIB_STOR_SET_NEW_RETRANSMIT_ERR;
        }
    }

    /* Cleanup/Finalize */
    sqlite3_finalize(SetRetransmitStmt);
    
    return Status;
}

SQL_Status_t BPLib_SQL_SetNewRetransmitTriggerImpl(sqlite3* db, BPLib_EID_Pattern_t* DestEIDs, 
                                                size_t NumEIDs, size_t RetransmitTrigger)
{
    SQL_Status_t  SQLStatus;
    size_t        i;
    uint64_t      BindIndex;

    /* Bind parameters for query */
    sqlite3_reset(SetRetransmitStmt);

    BindIndex = 1;
    /* Bind destination EID query */
    for (i = 0; i < NumEIDs; i++)
    {
        SQLStatus = sqlite3_bind_int64(SetRetransmitStmt, BindIndex++, DestEIDs[i].MinNode);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to bind dest_node min: %s\n", sqlite3_errmsg(db));
            return SQLStatus;
        }

        if (DestEIDs[i].MinNode != DestEIDs[i].MaxNode)
        {
            SQLStatus = sqlite3_bind_int64(SetRetransmitStmt, BindIndex++, DestEIDs[i].MaxNode);
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "Failed to bind dest_node max: %s\n", sqlite3_errmsg(db));
                return SQLStatus;
            }
        }

        SQLStatus = sqlite3_bind_int64(SetRetransmitStmt, BindIndex++, DestEIDs[i].MinService);
        if (SQLStatus != SQLITE_OK)
        {
            fprintf(stderr, "Failed to bind dest_node min: %s\n", sqlite3_errmsg(db));
            return SQLStatus;
        }

        if (DestEIDs[i].MinService != DestEIDs[i].MaxService)
        {
            SQLStatus = sqlite3_bind_int64(SetRetransmitStmt, BindIndex++, DestEIDs[i].MaxService);
            if (SQLStatus != SQLITE_OK)
            {
                fprintf(stderr, "Failed to bind dest_node max: %s\n", sqlite3_errmsg(db));
                return SQLStatus;
            }
        }

    }

    /* Bind retransmit_trigger to garbage value */
    SQLStatus = sqlite3_bind_int64(SetRetransmitStmt, BindIndex++, RetransmitTrigger);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind retransmit_trigger: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    /* Bind retransmit_time to current time plus retransmit trigger */
    SQLStatus = sqlite3_bind_int64(SetRetransmitStmt, BindIndex++, 
                                    BPLib_TIME_GetMonotonicTime() + RetransmitTrigger);
    if (SQLStatus != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind retransmit_time: %s\n", sqlite3_errmsg(db));
        return SQLStatus;
    }

    SQLStatus = sqlite3_step(SetRetransmitStmt);

    if (SQLStatus == SQLITE_DONE)
    {
        /* For consistency with other helpers, convert DONE to OK */
        SQLStatus = SQLITE_OK;
    }

    /* Expecting SQLITE_OK */
    return SQLStatus;    
}

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

#include "bplib_cbor_internal.h"

/* ==================== */
/* Function Definitions */
/* ==================== */

BPLib_Status_t BPLib_CBOR_EncodeArray(QCBOREncodeContext* Context, UsefulOutBuf* EncodeBuffer)
{
    BPLib_Status_t Status;

    if (Context != NULL)
    {
        /* 1 byte for open definite array initial byte */
        if (UsefulOutBuf_WillItFit(EncodeBuffer, 1) == 1)
        {
            Status = BPLIB_SUCCESS;

            UsefulOutBuf_AppendByte(EncodeBuffer, 0x80);
            if (UsefulOutBuf_GetError(EncodeBuffer) == 0)
            {
                QCBOREncode_OpenArray(Context);
            }
            else
            {
                /*
                ** Per UsefulBuf.h:
                ** "Possible error conditions are:
                **  - bytes to be inserted will not fit
                **  - insertion point is out of buffer or past valid data
                **  - current position is off end of buffer (probably corrupted or uninitialized)
                **  - detect corruption / uninitialized by bad magic number"
                */

                Status = BPLIB_ERROR;
            }
        }
        else
        {
            Status = BPLIB_ERROR;
        }
    }
    else
    {
        Status = BPLIB_NULL_PTR_ERROR;
    }

    return Status;
}

BPLib_Status_t BPLib_CBOR_EncodeMap(QCBOREncodeContext* Context, UsefulOutBuf* EncodeBuffer)
{
    BPLib_Status_t Status;

    if (Context != NULL)
    {
        if (UsefulOutBuf_WillItFit(EncodeBuffer, 1) == 1) /* 1 byte for open definite map initial byte */
        {
            Status = BPLIB_SUCCESS;

            UsefulOutBuf_AppendByte(EncodeBuffer, 0xA0);
            if (UsefulOutBuf_GetError(EncodeBuffer) == 0)
            {
                QCBOREncode_OpenMap(Context);
            }
            else
            {
                /*
                ** Per UsefulBuf.h:
                ** "Possible error conditions are:
                **  - bytes to be inserted will not fit
                **  - insertion point is out of buffer or past valid data
                **  - current position is off end of buffer (probably corrupted or uninitialized)
                **  - detect corruption / uninitialized by bad magic number"
                */

                Status = BPLIB_ERROR;
            }
        }
        else
        {
            Status = BPLIB_ERROR;
        }
    }
    else
    {
        Status = BPLIB_NULL_PTR_ERROR;
    }

    return Status;
}

BPLib_Status_t BPLib_CBOR_EncodeUInt(QCBOREncodeContext* Context, UsefulOutBuf* EncodeBuffer, uint64_t ValueToEncode)
{
    BPLib_Status_t Status;

    Status = BPLIB_SUCCESS;

    if (Context != NULL)
    {
        if (ValueToEncode <= 0x17)
        { /* Uses unsigned integer 0x00..0x17 (0..23) initial byte */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 1))
            {
                UsefulOutBuf_AppendByte(EncodeBuffer, ValueToEncode);
            }
            else
            {
                Status = BPLIB_ERROR;
            }
        }
        else if (ValueToEncode > 0x17 && ValueToEncode <= 0xFF)
        { /* Uses unsigned integer (one-byte uint8_t follows) initial byte */
            /* 1 for initial byte and 1 to represent value */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 2))
            {
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x18); /* uint8_t initial byte */
                UsefulOutBuf_AppendByte(EncodeBuffer, ValueToEncode);
            }
            else
            {
                Status = BPLIB_ERROR;
            }
        }
        else if (ValueToEncode > 0xFF && ValueToEncode <= 0xFFFF)
        { /* Uses unsigned integer (two-byte uint16_t follows) initial byte */
            /* 1 for initial byte and 2 to represent value */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 3))
            {
                UsefulOutBuf_AppendUint16(EncodeBuffer, ValueToEncode);
            }
            else
            {
                Status = BPLIB_ERROR;
            }
        }
        else if (ValueToEncode > 0xFFFF && ValueToEncode <= 0xFFFFFFFF)
        {   /* Uses unsigned integer (four-byte uint32_t follows) initial byte */
            /* 1 for initial byte and 4 to represent value */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 5))
            {
                UsefulOutBuf_AppendUint32(EncodeBuffer, ValueToEncode);
            }
            else
            {
                Status = BPLIB_ERROR;
            }
        }
        else if (ValueToEncode > 0xFFFFFFFF && ValueToEncode <= 0xFFFFFFFFFFFFFFFF)
        { /* Uses unsigned integer (eight-byte uint64_t follows) initial byte */
            /* 1 for initial byte and 8 to represent value */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 9))
            {
                UsefulOutBuf_AppendUint64(EncodeBuffer, ValueToEncode);
            }
            else
            {
                Status = BPLIB_ERROR;
            }
        }

        if (Status == BPLIB_SUCCESS)
        {
            if (UsefulOutBuf_GetError(EncodeBuffer) == 0)
            {
                /* This add function will properly encode for all values regardless of size */
                QCBOREncode_AddUInt64(Context, ValueToEncode);
            }
            else
            {
                /*
                ** Per UsefulBuf.h:
                ** "Possible error conditions are:
                **  - bytes to be inserted will not fit
                **  - insertion point is out of buffer or past valid data
                **  - current position is off end of buffer (probably corrupted or uninitialized)
                **  - detect corruption / uninitialized by bad magic number"
                */

                Status = BPLIB_ERROR;
            }
        }
        else
        {
            Status = BPLIB_ERROR;
        }
    }
    else
    {
        Status = BPLIB_NULL_PTR_ERROR;
    }

    return Status;
}

BPLib_Status_t BPLib_CBOR_EncodeGetBufferSize(UsefulOutBuf* EncodeBuffer, size_t* EncodedSize)
{
    BPLib_Status_t Status;
    UsefulBufC     SizeData;
    uint8_t        UsefulOutBufError;

    Status = BPLIB_SUCCESS;

    SizeData          = UsefulOutBuf_OutUBuf(EncodeBuffer);
    UsefulOutBufError = UsefulOutBuf_GetError(EncodeBuffer);

    if (UsefulOutBufError == 0)
    {
        /* Everything is ok */
        *EncodedSize = SizeData.len;
    }
    else
    {
        /*
        ** Per UsefulBuf.h:
        ** "Possible error conditions are:
        **  - bytes to be inserted will not fit
        **  - insertion point is out of buffer or past valid data
        **  - current position is off end of buffer (probably corrupted or uninitialized)
        **  - detect corruption / uninitialized by bad magic number"
        */

        *EncodedSize = 0;
        Status       = BPLIB_ERROR;
    }

    return Status;
}

BPLib_Status_t BPLib_CBOR_EncodeAdu(QCBOREncodeContext* Context, UsefulOutBuf* EncodeBuffer, uint8_t* Adu, size_t AduLen)
{
    BPLib_Status_t Status;
    UsefulBufC     AduBuf;

    Status = BPLIB_SUCCESS;

    if (Context != NULL)
    {
        /* +1 for byte string initial byte */
        if (UsefulOutBuf_WillItFit(EncodeBuffer, AduLen + 1) == 1)
        {
            AduBuf.ptr = Adu;
            AduBuf.len = AduLen;

            UsefulOutBuf_AppendByte(EncodeBuffer, 0x40);
            UsefulOutBuf_AppendUsefulBuf(EncodeBuffer, AduBuf);

            if (UsefulOutBuf_GetError(EncodeBuffer) == 0)
            {
                QCBOREncode_AddBytes(Context, AduBuf);
            }
            else
            {
                /*
                ** Per UsefulBuf.h:
                ** "Possible error conditions are:
                **  - bytes to be inserted will not fit
                **  - insertion point is out of buffer or past valid data
                **  - current position is off end of buffer (probably corrupted or uninitialized)
                **  - detect corruption / uninitialized by bad magic number"
                */

                Status = BPLIB_ERROR;
            }
        }
        else
        {
            Status = BPLIB_ERROR;
        }
    }
    else
    {
        Status = BPLIB_NULL_PTR_ERROR;
    }

    return Status;
}

BPLib_Status_t BPLib_CBOR_EncodeEID(QCBOREncodeContext* Context, BPLib_EID_t* SourceData)
{
    BPLib_Status_t ReturnStatus = BPLIB_SUCCESS;

    if ((Context == NULL) || (SourceData == NULL))
    {
        ReturnStatus = BPLIB_NULL_PTR_ERROR;
    }
    else
    {
        /*
        ** Open Outer Array (EID)
        */
        QCBOREncode_OpenArray(Context);

        /*
        ** Add outer content (URI Type)
        */
        QCBOREncode_AddUInt64(Context, SourceData->Scheme);

        if (SourceData->Scheme == BPLIB_EID_SCHEME_IPN)
        {
            /*
            ** Open Inner Array (SSP)
            */
            QCBOREncode_OpenArray(Context);

            /*
            ** Add inner content (SSP)
            */
            QCBOREncode_AddUInt64(Context, SourceData->Node);
            QCBOREncode_AddUInt64(Context, SourceData->Service);

            /*
            ** Close Inner Array (SSP)
            */
            QCBOREncode_CloseArray(Context);
        }
        else if (SourceData->Scheme == BPLIB_EID_SCHEME_DTN)
        {
            /* We only support the dtn:none encoding so just encode a 0 */
            QCBOREncode_AddUInt64(Context, 0);
        }
        else
        {
            /* This should never happen on encode but just in case? */
            ReturnStatus = BPLIB_ERROR;
        }

        /*
        ** Close Outer Array (EID)
        */
        QCBOREncode_CloseArray(Context);
    }

    return ReturnStatus;
}

BPLib_Status_t BPLib_CBOR_EncodeCreationTimeStamp(QCBOREncodeContext* Context, BPLib_CreationTimeStamp_t* TimeStamp)
{
    BPLib_Status_t ReturnStatus;

    if ((Context == NULL) || (TimeStamp == NULL))
    {
        ReturnStatus = BPLIB_NULL_PTR_ERROR;
    }
    else
    {
        /*
        ** Open Array
        */
        QCBOREncode_OpenArray(Context);

        /*
        ** Add content
        */
        QCBOREncode_AddUInt64(Context, TimeStamp->CreateTime);
        QCBOREncode_AddUInt64(Context, TimeStamp->SequenceNumber);

        /*
        ** Close Array
        */
        QCBOREncode_CloseArray(Context);
        ReturnStatus = BPLIB_SUCCESS;
    }

    return ReturnStatus;
}

BPLib_Status_t BPLib_CBOR_EncodeCrcValue(QCBOREncodeContext* Context, uint64_t CrcValue, uint64_t CrcType)
{
    BPLib_Status_t ReturnStatus;
    UsefulBufC     CrcInfo;

    CrcInfo.ptr = &CrcValue;

    if (Context != NULL)
    {
        if (CrcType == BPLib_CRC_Type_None)
        {
            /* If CRC is none, there's nothing to do */
            ReturnStatus = BPLIB_SUCCESS;
        }
        else if (CrcType == BPLib_CRC_Type_CRC16)
        {
            /* Encode 16-bit CRC */
            CrcInfo.len = 2;
            QCBOREncode_AddBytes(Context, CrcInfo);
            ReturnStatus = BPLIB_SUCCESS;
        }
        else if (CrcType == BPLib_CRC_Type_CRC32C)
        {
            /* Encode 32-bit CRC */
            CrcInfo.len = 4;
            QCBOREncode_AddBytes(Context, CrcInfo);
            ReturnStatus = BPLIB_SUCCESS;
        }
        else
        {
            /* Unrecognized CRC type */
            ReturnStatus = BPLIB_ERROR;
        }
    }
    else
    {
        ReturnStatus = BPLIB_NULL_PTR_ERROR;
    }

    return ReturnStatus;
}

BPLib_Status_t BPLib_CBOR_EncodeAdminRecord(QCBOREncodeContext* Context, UsefulOutBuf* EncodeBuffer, BPLib_Bundle_t* Bundle)
{
    BPLib_Status_t          Status;
    BPLib_ARP_AdminRecord_t AdminRecord;

    if (Context != NULL)
    {
        /*
        ** The bundle's payload has been determined to be an admin record, so copy the
        ** payload into an admin record struct
        */
        memcpy((void*) &AdminRecord, (void*) Bundle->blob->user_data.BigData, sizeof(BPLib_ARP_AdminRecord_t));

        /* Start an array that represents the admin record */
        Status = BPLib_CBOR_EncodeArray(Context, EncodeBuffer);
        if (Status == BPLIB_SUCCESS)
        {
            /* Encode the admin record type */
            Status = BPLib_CBOR_EncodeUInt(Context, EncodeBuffer, AdminRecord.AdminRecordType);
            if (Status == BPLIB_SUCCESS)
            {
                /* Determine which encoding is needed by checking the admin record type */
                switch(AdminRecord.AdminRecordType)
                {
                    case BPLib_CT_BsrRecordTypeCode:
                        // Status = BPLib_CBOR_EncodeBsr(&Context, &(AdminRecord.AdminRecordBody.BSR), &EncodeBuffer);
                        break;

                    case BPLib_CT_CrsRecordTypeCode:
                        // Status = BPLib_CBOR_EncodeCrs(&Context, &(AdminRecord.AdminRecordBody.CRS), &EncodeBuffer);
                        break;

                    case BPLib_CT_CcsRecordTypeCode:
                        Status = BPLib_CBOR_EncodeCcs(Context, &(AdminRecord.AdminRecordBody.CCS), EncodeBuffer);
                        break;

                    default:
                        /* TODO: Handle unsupported admin record type */
                        Status = BPLIB_ARP_UNK_REC_TYPE_ERR;
                        break;
                }
            }

            QCBOREncode_CloseArray(Context);
        }
    }
    else
    {
        Status = BPLIB_NULL_PTR_ERROR;
    }

    return Status;
}

/*
BPLib_Status_t BPLib_CBOR_EncodeBsr(QCBOREncodeContext* Context,
                                    BPLib_ARP_BundleStatusReport_t* BSR,
                                    size_t* EncodedSize)
{
    return BPLIB_SUCCESS;
}

BPLib_Status_t BPLib_CBOR_EncodeCrs(QCBOREncodeContext* Context,
                                    BPLib_ARP_CompressedReportingSignal_t* CRS,
                                    size_t* EncodedSize)
{
    return BPLIB_SUCCESS;
}
*/

BPLib_Status_t BPLib_CBOR_EncodeCcs(QCBOREncodeContext* Context,
                                    BPLib_CT_DeserializedCcs_t* CCS,
                                    UsefulOutBuf* EncodeBuffer)
{
    BPLib_Status_t                 Status;
    uint8_t                        CollectionNum;
    uint8_t                        SeqRangeEntry;
    BPLib_CT_BundleSeqCollection_t BundleSeqCollection;

    if (Context != NULL)
    {
        Status = BPLib_CBOR_EncodeMap(Context, EncodeBuffer);
        if (Status == BPLIB_SUCCESS)
        {
            for (CollectionNum = 0; CollectionNum < CCS->NumBundleSeqCollections; CollectionNum++)
            {
                BundleSeqCollection = CCS->BundleSeqCollections[CollectionNum];

                Status = BPLib_CBOR_EncodeUInt(Context, EncodeBuffer, BundleSeqCollection.DispositionCode);
                if (Status == BPLIB_SUCCESS)
                {
                    Status = BPLib_CBOR_EncodeArray(Context, EncodeBuffer);
                    if (Status == BPLIB_SUCCESS)
                    {
                        for (SeqRangeEntry = 0; SeqRangeEntry < BundleSeqCollection.SeqRangeLen; SeqRangeEntry++)
                        {
                            /* Encode bundle sequence range value into map under disposition code label */
                            Status = BPLib_CBOR_EncodeUInt(Context, EncodeBuffer, BundleSeqCollection.SeqRange[SeqRangeEntry]);
                            if (Status != BPLIB_SUCCESS)
                            {
                                /* Exit the loop if something went wrong */
                                /* TODO: Status = ??? */
                                break;
                            }
                        }

                        QCBOREncode_CloseArray(Context);
                    }
                }
            }

            QCBOREncode_CloseMap(Context);
        }
        else
        {
            Status = BPLIB_ERROR;
        }
    }
    else
    {
        Status = BPLIB_NULL_PTR_ERROR;
    }

    return Status;
}
/*
 * NASA Docket No. GSC-19,559-1, and identified as "Delay/Disruption Tolerant Networking 
 * (DTN) Bundle Protocol (BP) v7 Core Flight System (cFS) Application Build 7.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this 
 * file except in compliance with the License. You may obtain a copy of the License at 
 *
 * http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software distributed under 
 * the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF 
 * ANY KIND, either express or implied. See the License for the specific language 
 * governing permissions and limitations under the License. The copyright notice to be 
 * included in the software is as follows: 
 *
 * Copyright 2025 United States Government as represented by the Administrator of the 
 * National Aeronautics and Space Administration. All Rights Reserved.
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

    Status = BPLIB_SUCCESS;
    if (Context != NULL)
    {
        /* 1 byte for open definite array initial byte */
        if (UsefulOutBuf_WillItFit(EncodeBuffer, 1) == 1)
        {
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

                Status = BPLIB_CBOR_ENC_ARRAY_ERR;
            }
        }
        else
        {
            Status = BPLIB_CBOR_ENC_ARRAY_ERR;
        }
    }
    else
    {
        Status = BPLIB_CBOR_ENC_ARRAY_ERR;
    }

    return Status;
}

BPLib_Status_t BPLib_CBOR_EncodeMap(QCBOREncodeContext* Context, UsefulOutBuf* EncodeBuffer)
{
    BPLib_Status_t Status;

    Status = BPLIB_SUCCESS;
    if (Context != NULL)
    {
        if (UsefulOutBuf_WillItFit(EncodeBuffer, 1) == 1) /* 1 byte for open definite map initial byte */
        {
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

                Status = BPLIB_CBOR_ENC_MAP_ERR;
            }
        }
        else
        {
            Status = BPLIB_CBOR_ENC_MAP_ERR;
        }
    }
    else
    {
        Status = BPLIB_CBOR_ENC_MAP_ERR;
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
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 1) == 1)
            {
                UsefulOutBuf_AppendByte(EncodeBuffer, ValueToEncode);
            }
            else
            {
                Status = BPLIB_CBOR_ENC_UINT_ERR;
            }
        }
        else if (ValueToEncode > 0x17 && ValueToEncode <= 0xFF)
        { /* Uses unsigned integer (one-byte uint8_t follows) initial byte */
            /* 1 for initial byte and 1 to represent value */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 2) == 1)
            {
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x18); /* uint8_t initial byte */
                UsefulOutBuf_AppendByte(EncodeBuffer, ValueToEncode);
            }
            else
            {
                Status = BPLIB_CBOR_ENC_UINT_ERR;
            }
        }
        else if (ValueToEncode > 0xFF && ValueToEncode <= 0xFFFF)
        { /* Uses unsigned integer (two-byte uint16_t follows) initial byte */
            /* 1 for initial byte and 2 to represent value */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 3) == 1)
            {
                UsefulOutBuf_AppendUint16(EncodeBuffer, ValueToEncode);
            }
            else
            {
                Status = BPLIB_CBOR_ENC_UINT_ERR;
            }
        }
        else if (ValueToEncode > 0xFFFF && ValueToEncode <= 0xFFFFFFFF)
        {   /* Uses unsigned integer (four-byte uint32_t follows) initial byte */
            /* 1 for initial byte and 4 to represent value */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 5) == 1)
            {
                UsefulOutBuf_AppendUint32(EncodeBuffer, ValueToEncode);
            }
            else
            {
                Status = BPLIB_CBOR_ENC_UINT_ERR;
            }
        }
        else if (ValueToEncode > 0xFFFFFFFF && ValueToEncode <= 0xFFFFFFFFFFFFFFFF)
        { /* Uses unsigned integer (eight-byte uint64_t follows) initial byte */
            /* 1 for initial byte and 8 to represent value */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 9) == 1)
            {
                UsefulOutBuf_AppendUint64(EncodeBuffer, ValueToEncode);
            }
            else
            {
                Status = BPLIB_CBOR_ENC_UINT_ERR;
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

                Status = BPLIB_CBOR_ENC_UINT_ERR;
            }
        }
        else
        {
            Status = BPLIB_CBOR_ENC_UINT_ERR;
        }
    }
    else
    {
        Status = BPLIB_CBOR_ENC_UINT_ERR;
    }

    return Status;
}

BPLib_Status_t BPLib_CBOR_EncodeGetBufferSize(UsefulOutBuf* EncodeBuffer, size_t* EncodedSize)
{
    BPLib_Status_t Status;
    uint8_t        UsefulOutBufError;

    Status            = BPLIB_SUCCESS;
    UsefulOutBufError = UsefulOutBuf_GetError(EncodeBuffer);

    if (UsefulOutBufError == 0)
    {
        /* Everything is ok */
        *EncodedSize += EncodeBuffer->data_len;
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
        Status       = BPLIB_CBOR_ENC_GET_BUFF_SIZE_ERR;
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
         if (AduLen <= 0x17)
        { /* Uses byte string (0x00..0x17 bytes follow) initial byte */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 1) == 1)
            {
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x40);
            }
            else
            {
                Status = BPLIB_CBOR_ENC_ADU_ERR;
            }
        }
        else if (AduLen > 0x17 && AduLen <= 0xFF)
        { /* Uses byte string (one-byte uint8_t for n, and then n bytes follow) initial byte */
            /* 1 for initial byte and 1 to represent length of byte string */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 2) == 1)
            {
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x58);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
            }
            else
            {
                Status = BPLIB_CBOR_ENC_ADU_ERR;
            }
        }
        else if (AduLen > 0xFF && AduLen <= 0xFFFF)
        { /* Uses byte string (two-byte uint16_t for n, and then n bytes follow) initial byte */
            /* 1 for initial byte and 2 to represent length of byte string */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 3) == 1)
            {
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x59);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
            }
            else
            {
                Status = BPLIB_CBOR_ENC_ADU_ERR;
            }
        }
        else if (AduLen > 0xFFFF && AduLen <= 0xFFFFFFFF)
        { /* Uses byte string (four-byte uint32_t for n, and then n bytes follow) initial byte */
            /* 1 for initial byte and 4 to represent length of byte string */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 5) == 1)
            {
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x5A);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
            }
            else
            {
                Status = BPLIB_CBOR_ENC_ADU_ERR;
            }
        }
        else if (AduLen > 0xFFFFFFFF && AduLen <= 0xFFFFFFFFFFFFFFFF)
        { /* Uses byte string (eight-byte uint64_t for n, and then n bytes follow) initial byte */
            /* 1 for initial byte and 8 to represent length of byte string */
            if (UsefulOutBuf_WillItFit(EncodeBuffer, 9) == 1)
            {
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x5B);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
                UsefulOutBuf_AppendByte(EncodeBuffer, 0x00);
            }
            else
            {
                Status = BPLIB_CBOR_ENC_ADU_ERR;
            }
        }

        if (UsefulOutBuf_WillItFit(EncodeBuffer, AduLen) == 1)
        {
            AduBuf.ptr = Adu;
            AduBuf.len = AduLen;

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

                Status = BPLIB_CBOR_ENC_ADU_ERR;
            }
        }
        else
        {
            Status = BPLIB_CBOR_ENC_ADU_ERR;
        }
    }
    else
    {
        Status = BPLIB_CBOR_ENC_ADU_ERR;
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
            ReturnStatus = BPLIB_CBOR_ENC_CRC_ERR;
        }
    }
    else
    {
        ReturnStatus = BPLIB_CBOR_ENC_CRC_ERR;
    }

    return ReturnStatus;
}

BPLib_Status_t BPLib_CBOR_EncodeAdminRecord(QCBOREncodeContext *Context, BPLib_Bundle_t* Bundle)
{
    BPLib_Status_t           Status = BPLIB_SUCCESS;
    BPLib_ARP_AdminRecord_t *AdminRecord;

    /*
    ** The bundle's payload has been determined to be a deserialized admin record
    */
    AdminRecord = &(Bundle->blob->user_data.AdminRecord);

    QCBOREncode_OpenArray(Context);

    QCBOREncode_AddUInt64(Context, (uint64_t) AdminRecord->AdminRecordType);

    /* Determine which encoding is needed by checking the admin record type */
    switch(AdminRecord->AdminRecordType)
    {
        case BPLib_CT_BsrRecordTypeCode:
            // BPLib_CBOR_EncodeBsr(&Context, &(AdminRecord.AdminRecordBody.BSR));
            break;

        case BPLib_CT_CrsRecordTypeCode:
            // BPLib_CBOR_EncodeCrs(&Context, &(AdminRecord.AdminRecordBody.CRS));
            break;

        case BPLib_CT_CcsRecordTypeCode:
            Status = BPLib_CBOR_EncodeCcs(Context, &(AdminRecord->AdminRecordBody.CCS));
            break;

        default:
            Status = BPLIB_CBOR_ENC_ADMIN_RECORD_ERR;
            break;
    }

    QCBOREncode_CloseArray(Context);

    return Status;
}

/*
void BPLib_CBOR_EncodeBsr(QCBOREncodeContext* Context, BPLib_ARP_BundleStatusReport_t* BSR)
{
    return;
}

void BPLib_CBOR_EncodeCrs(QCBOREncodeContext* Context, BPLib_ARP_CompressedReportingSignal_t* CRS)
{
    return;
}
*/

BPLib_Status_t BPLib_CBOR_EncodeCcs(QCBOREncodeContext* Context, BPLib_CT_DeserializedCcs_t* CCS)
{
    size_t BscIdx;
    size_t SeqRangeIdx;
    
    QCBOREncode_OpenMap(Context);

    if (CCS->NumBundleSeqCollections > BPLIB_CT_MAX_SEQ_COLLECTIONS)
    {
        return BPLIB_CBOR_ENC_CORRUPT_CCS_ERR;
    }

    for (BscIdx = 0; BscIdx < CCS->NumBundleSeqCollections; BscIdx++)
    {
        if (CCS->BundleSeqCollections[BscIdx].SeqRangeLen > BPLIB_CT_MAX_SEQ_RANGE_LEN)
        {
            return BPLIB_CBOR_ENC_CORRUPT_CCS_ERR;
        }

        QCBOREncode_AddInt64(Context, (int64_t) CCS->BundleSeqCollections[BscIdx].DispositionCode);

        QCBOREncode_OpenArray(Context);

        QCBOREncode_AddUInt64(Context, (uint64_t) CCS->BundleSeqCollections[BscIdx].SeqId);
        QCBOREncode_AddUInt64(Context, (uint64_t) CCS->BundleSeqCollections[BscIdx].FirstSeqNum);

        QCBOREncode_OpenArray(Context);

        for (SeqRangeIdx = 0; SeqRangeIdx < CCS->BundleSeqCollections[BscIdx].SeqRangeLen; SeqRangeIdx++)
        {
            QCBOREncode_AddUInt64(Context, (uint64_t) CCS->BundleSeqCollections[BscIdx].SeqRange[SeqRangeIdx]);
        }

        QCBOREncode_CloseArray(Context);

        QCBOREncode_CloseArray(Context);
    }

    QCBOREncode_CloseMap(Context);

    return BPLIB_SUCCESS;
}

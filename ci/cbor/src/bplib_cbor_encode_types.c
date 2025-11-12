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
        *EncodedSize += SizeData.len;
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

    return Status;
}

void BPLib_CBOR_EncodeUInt64(QCBOREncodeContext* Context, UsefulOutBuf* EncodeBuffer, uint64_t ValueToEncode)
{
    QCBOREncode_AddUInt64(Context, ValueToEncode);
    UsefulOutBuf_AppendUint64(EncodeBuffer, ValueToEncode);
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
    UsefulBufC CrcInfo;
    CrcInfo.ptr = &CrcValue;

    if (Context == NULL)
    {
        ReturnStatus = BPLIB_NULL_PTR_ERROR;
    }
    else
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

    return ReturnStatus;
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
    uint8_t                        UsefulBufError;
    uint8_t                        CollectionNum;
    uint8_t                        SeqRangeEntry;
    UsefulBufC                     SizeData;
    BPLib_CT_BundleSeqCollection_t BundleSeqCollection;
    
    Status = BPLIB_SUCCESS;

    if (Context != NULL)
    {
        QCBOREncode_OpenMap(Context);

        for (CollectionNum = 0; CollectionNum < CCS->NumBundleSeqCollections; CollectionNum++)
        {
            BundleSeqCollection = CCS->BundleSeqCollections[CollectionNum];
            
            /* Use disposition code to create map label */
            QCBOREncode_OpenArrayInMapN(Context, (uint64_t) BundleSeqCollection.DispositionCode);
            UsefulOutBuf_AppendUint64(EncodeBuffer, (uint64_t) BundleSeqCollection.DispositionCode);

            for (SeqRangeEntry = 0; SeqRangeEntry < BundleSeqCollection.SeqRangeLen; SeqRangeEntry++)
            {
                /* Encode bundle sequence range value into map under disposition code label */
                BPLib_CBOR_EncodeUInt64(Context, EncodeBuffer, BundleSeqCollection.SeqRange[SeqRangeEntry]);
            }

            QCBOREncode_CloseArray(Context);
        }

        QCBOREncode_CloseMap(Context);
    }
    else
    {
        Status = BPLIB_NULL_PTR_ERROR;
    }

    return Status;
}
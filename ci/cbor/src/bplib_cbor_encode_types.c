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

BPLib_Status_t BPLIB_CBOR_EncodeCcs(QCBOREncodeContext* Context,
                                    BPLib_CT_DeserializedCcs_t* CCS,
                                    size_t* EncodedSize)
{
    BPLib_Status_t                 Status = BPLIB_SUCCESS;
    QCBORError                     QCBOR_Status;
    uint8_t                        CollectionNum;
    uint8_t                        SeqRangeEntry;
    UsefulBufC                     SizeData;
    UsefulOutBuf                   SizeFinder;
    BPLib_CT_BundleSeqCollection_t BundleSeqCollection;
    
    if (Context != NULL)
    {
        UsefulOutBuf_Init(&SizeFinder, SizeCalculateUsefulBuf);
        QCBOREncode_OpenMap(Context);

        for (CollectionNum = 0; CollectionNum < CCS->NumBundleSeqCollections; CollectionNum++)
        {
            BundleSeqCollection = CCS->BundleSeqCollections[CollectionNum];
            
            QCBOREncode_OpenArrayInMapN(Context, (uint64_t) BundleSeqCollection.DispositionCode);
            UsefulOutBuf_AppendUint64(&SizeFinder, (uint64_t) BundleSeqCollection.DispositionCode);

            for (SeqRangeEntry = 0; SeqRangeEntry < BundleSeqCollection.SeqRangeLen; SeqRangeEntry++)
            {
                QCBOREncode_AddUInt64(Context, BundleSeqCollection.SeqRange[SeqRangeEntry]);
                UsefulOutBuf_AppendUint64(&SizeFinder, BundleSeqCollection.SeqRange[SeqRangeEntry]);
            }

            QCBOREncode_CloseArray(Context);
        }

        QCBOREncode_CloseMap(Context);

        SizeData      = UsefulOutBuf_OutUBuf(&SizeFinder);
        *EncodedSize += SizeData.len;
    }
    else
    {
        Status = BPLIB_NULL_PTR_ERROR;
    }

    return Status;
}
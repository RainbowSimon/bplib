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

#include "bplib_cbor_internal.h"

BPLib_Status_t BPLib_CBOR_EncodePayload(BPLib_Bundle_t* StoredBundle,
                                        void* OutputBuffer,
                                        size_t OutputBufferSize,
                                        size_t* NumBytesCopied)
{
    BPLib_Status_t     Status;
    QCBOREncodeContext Context;
    UsefulBuf          InitStorage;
    UsefulBuf          MirrorStorage;
    uint8_t            MirrorBuffer[OutputBufferSize];
    QCBORError         QcborStatus;
    size_t             BytesLeftInOutputBuffer;
    UsefulOutBuf       EncodeBuffer;
    uint8_t            AduByteString[BPLIB_MEM_BIG_BLK_DATA_SIZE];
    uint8_t            CrcValueSize;
    uint8_t            CrcLoop;
    size_t             temp = 0;

    if ((StoredBundle   == NULL) ||
        (OutputBuffer   == NULL) ||
        (NumBytesCopied == NULL))
    {
        Status = BPLIB_NULL_PTR_ERROR;
    }
    else
    {
        memcpy(MirrorBuffer, OutputBuffer, OutputBufferSize);

        /*
        ** Initialize the encoder
        */
        InitStorage.ptr = OutputBuffer;
        InitStorage.len = OutputBufferSize;
        QCBOREncode_Init(&Context, InitStorage);

        /* Mirror the encoder with a UsefulOutBuf to handle errors */
        MirrorStorage.ptr = MirrorBuffer;
        MirrorStorage.len = OutputBufferSize;
        UsefulOutBuf_Init(&EncodeBuffer, MirrorStorage);

        Status = BPLib_CBOR_EncodeArray(&Context, &EncodeBuffer);
        if (Status == BPLIB_SUCCESS)
        { /* Addition of payload array needs to succeed for the payload to be properly encoded */
            /* Add our block header data */
            Status = BPLib_CBOR_EncodeUInt(&Context, &EncodeBuffer, StoredBundle->blocks.PayloadHeader.BlockType);

            if (Status == BPLIB_SUCCESS)
            {
                Status = BPLib_CBOR_EncodeUInt(&Context, &EncodeBuffer, StoredBundle->blocks.PayloadHeader.BlockNum);
            }

            if (Status == BPLIB_SUCCESS)
            {
                Status = BPLib_CBOR_EncodeUInt(&Context, &EncodeBuffer, StoredBundle->blocks.PayloadHeader.BlockProcFlags);
            }

            if (Status == BPLIB_SUCCESS)
            {
                Status = BPLib_CBOR_EncodeUInt(&Context, &EncodeBuffer, StoredBundle->blocks.PayloadHeader.CrcType);
            }
            else
            {
                /* Any of the previous header encode operations failed */
                Status = BPLIB_CBOR_ENC_PAYL_HEADER_ERR;
            }

            if (Status == BPLIB_SUCCESS)
            {
                BytesLeftInOutputBuffer = UsefulOutBuf_RoomLeft(&EncodeBuffer);

                if (StoredBundle->blocks.PrimaryBlock.BundleProcFlags & BPLIB_BUNDLE_PROC_ADMIN_RECORD_FLAG)
                {
                    /* Attempt to encapsultate block-specific data in a CBOR byte string */
                    size_t EncodedSize = 0;
                    Status = BPLib_CBOR_EncodeGetBufferSize(&EncodeBuffer, &EncodedSize);
                    printf("EncodedSize: %ld\n", EncodedSize);

                    QCBOREncode_OpenBytes(&Context, &NULLUsefulBuf);
                    Status = BPLib_CBOR_EncodeAdminRecord(&Context, &EncodeBuffer, StoredBundle);

                    size_t afterSize = 0;
                    Status = BPLib_CBOR_EncodeGetBufferSize(&EncodeBuffer, &afterSize);
                    printf("EncodedSize after: %ld\n", afterSize);

                    size_t ByteStringSize = afterSize - EncodedSize;
                    printf("ByteStringSize: %ld\n", ByteStringSize);
                    if (ByteStringSize <= 0x17)
                    {
                        /* 1 byte total*/
                        UsefulOutBuf_Advance(&EncodeBuffer, 1);
                    }
                    else if (ByteStringSize > 0x17 && ByteStringSize <= sizeof(uint8_t))
                    {
                        /* 2 bytes total */
                        UsefulOutBuf_Advance(&EncodeBuffer, 2);
                    }
                    else if (ByteStringSize > sizeof(uint8_t) && ByteStringSize <= sizeof(uint16_t))
                    {
                        /* 3 bytes total */
                        UsefulOutBuf_Advance(&EncodeBuffer, 3);
                    }
                    else if (ByteStringSize > sizeof(uint16_t) && ByteStringSize <= sizeof(uint32_t))
                    {
                        /* 5 bytes total */
                        UsefulOutBuf_Advance(&EncodeBuffer, 5);
                    }
                    else if (ByteStringSize > sizeof(uint32_t) && ByteStringSize <= sizeof(uint64_t))
                    {
                        /* 9 bytes total */
                        UsefulOutBuf_Advance(&EncodeBuffer, 9);
                    }

                    QCBOREncode_CloseBytes(&Context, ByteStringSize);
                }
                else
                {
                    /* Add the ADU data */
                    Status = BPLib_MEM_CopyOutFromOffset(StoredBundle,
                                                            StoredBundle->blocks.PayloadHeader.DataOffsetStart,
                                                            StoredBundle->blocks.PayloadHeader.DataSize,
                                                            AduByteString,
                                                            BytesLeftInOutputBuffer);

                    if (Status == BPLIB_SUCCESS)
                    {
                        Status = BPLib_CBOR_EncodeAdu(&Context, &EncodeBuffer, AduByteString,
                                                        StoredBundle->blocks.PayloadHeader.DataSize);
                    }
                }

                if (Status != BPLIB_SUCCESS)
                {
                    /* An error occured while encoding the payload */
                    Status = BPLIB_CBOR_ENC_PAYL_ERR;
                }
            }

            /* Add the CRC */
            if (Status == BPLIB_SUCCESS)
            {
                CrcValueSize = 0;

                /* Cloogy integration with BPLib_CBOR_EncodeCrcValue since it's used elsewhere and thus can't use UsefulOutBuf */
                switch (StoredBundle->blocks.PayloadHeader.CrcType)
                {
                    case BPLib_CRC_Type_None:
                        /* If CRC is none, there's nothing to do */
                        break;

                    case BPLib_CRC_Type_CRC16:
                        /* Encode 16-bit CRC */
                        CrcValueSize = 2;
                        break;

                    case BPLib_CRC_Type_CRC32C:
                        /* Encode 32-bit CRC */
                        CrcValueSize = 4;
                        break;

                    default:
                        /* Unrecognized CRC type */
                        Status = BPLIB_ERROR;
                }

                if (Status == BPLIB_SUCCESS)
                {
                    /* +1 for byte string initial byte */
                    if (UsefulOutBuf_WillItFit(&EncodeBuffer, CrcValueSize + 1) == 1)
                    {
                        UsefulOutBuf_AppendByte(&EncodeBuffer, 0x44);
                        for (CrcLoop = 0; CrcLoop < CrcValueSize; CrcLoop++)
                        {
                            /* Add the CRC dummy value to the UsefulOutBuf */
                            UsefulOutBuf_AppendByte(&EncodeBuffer, 0x00);
                        }

                        if (UsefulOutBuf_GetError(&EncodeBuffer) == 0)
                        {
                            /* Set CRC value to 0, real value will be jammed in after encoding is done */
                            Status = BPLib_CBOR_EncodeCrcValue(&Context, 0, StoredBundle->blocks.PayloadHeader.CrcType);
                        }
                    }
                    else
                    {
                        Status = BPLIB_ERROR;
                    }
                }

                if (Status != BPLIB_SUCCESS)
                {
                    Status = BPLIB_CBOR_ENC_PAYL_CRC_ERR;
                }
            }

            /* Close payload array */
            QCBOREncode_CloseArray(&Context);

            /*
            ** Finish encoding, and check for errors
            */
            QcborStatus = QCBOREncode_Finish(&Context, &NULLUsefulBufC);
            if (QcborStatus == QCBOR_SUCCESS)
            {
                temp = 0;
                Status = BPLib_CBOR_EncodeGetBufferSize(&EncodeBuffer, &temp);
            }

            if (Status == BPLIB_SUCCESS)
            {
                /* Calculate new CRC for encoded block */
                BPLib_CBOR_GenerateBlockCrc(OutputBuffer,
                                            StoredBundle->blocks.PayloadHeader.CrcType,
                                            0, temp);
            }

            if (Status != BPLIB_SUCCESS)
            {
                Status = BPLIB_CBOR_ENC_QCBOR_FINISH_TAIL_ERR;
            }
        }

        if (Status != BPLIB_SUCCESS)
        {
            *NumBytesCopied = 0;
        }
        else
        {
            *NumBytesCopied += temp;
        }
    }

    return Status;
}


BPLib_Status_t BPLib_CBOR_CopyOrEncodePayload(BPLib_Bundle_t* StoredBundle,
                                              void* OutputBuffer,
                                              size_t OutputBufferSize,
                                              size_t* NumBytesCopied)
{
    BPLib_Status_t ReturnStatus;
    uint64_t TotalPayloadSize;
    BPLib_Status_t PayloadDataCopyStatus;

    if (StoredBundle->blocks.PayloadHeader.RequiresEncode)
    {
        ReturnStatus = BPLib_CBOR_EncodePayload(StoredBundle,
                                                OutputBuffer,
                                                OutputBufferSize,
                                                NumBytesCopied);
    }
    /* Verify that the block offset values are reasonable */
    else if (StoredBundle->blocks.PayloadHeader.BlockOffsetStart >=
             (StoredBundle->blocks.PayloadHeader.BlockOffsetEnd + 1))
    {
        *NumBytesCopied = 0;
        ReturnStatus = BPLIB_CBOR_ENC_PAYL_SIZES_CRRPTD_ERR;
    }
    else
    {
        /*
        ** Calculate the total payload size
        */
        TotalPayloadSize = StoredBundle->blocks.PayloadHeader.BlockOffsetEnd
                         - StoredBundle->blocks.PayloadHeader.BlockOffsetStart
                         + 1;

        /*
        ** Copy in the whole payload (header, data, and crc value)
        */
        if (TotalPayloadSize > OutputBufferSize)
        {
            *NumBytesCopied = 0;
            ReturnStatus = BPLIB_CBOR_ENC_PAYL_COPY_SIZE_GT_OUTPUT_ERR;
        }
        else
        {
            PayloadDataCopyStatus = BPLib_MEM_CopyOutFromOffset(StoredBundle,
                StoredBundle->blocks.PayloadHeader.BlockOffsetStart,
                TotalPayloadSize,
                OutputBuffer,
                OutputBufferSize);

            if (PayloadDataCopyStatus == BPLIB_SUCCESS)
            {
                *NumBytesCopied += TotalPayloadSize;
                ReturnStatus = BPLIB_SUCCESS;
            }
            else
            {
                *NumBytesCopied = 0;
                ReturnStatus = PayloadDataCopyStatus;
            }
        }
    }
    return ReturnStatus;
}

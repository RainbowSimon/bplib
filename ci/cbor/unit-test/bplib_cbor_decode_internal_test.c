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

/*
 * Include
 */
#include "bplib_cbor_test_utils.h"

/*
Primary Block:
         CRC Type: 1
         Flags: 4
         Dest EID (scheme.node.service): 2.200.1
         Source EID (scheme.node.service): 2.100.1
         Report-To EID (scheme.node.service): 2.100.1
         Timestamp (created, seq): 755533838904, 0
         Lifetime: 3600000
         CRC Value: 0xB19
*/
uint8_t CandPrimary[] = {
    0x9f, 0x89, 0x07, 0x04, 0x01, 0x82, 0x02, 0x82,
    0x18, 0xc8, 0x01, 0x82, 0x02, 0x82, 0x18, 0x64,
    0x01, 0x82, 0x02, 0x82, 0x18, 0x64, 0x01, 0x82,
    0x1b, 0x00, 0x00, 0x00, 0xaf, 0xe9, 0x53, 0x7a,
    0x38, 0x00, 0x1a, 0x00, 0x36, 0xee, 0x80, 0x42,
    0x0b, 0x19, 0xff
};

/*
Canonical Block [0]:
         Block Type: 1
         Block Number: 1
         Flags: 0
         CRC Type: 1
         CRC Value: 0xc68f
*/
uint8_t CandPayload[] = {
    0x86, 0x01, 0x01, 0x00, 0x01, 0x54,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0x42, 0xc6, 0x8f,
};

/*
Primary Block:
         CRC Type: 1
         Flags: 4
         Dest EID (scheme.node.service): 2.200.1
         Source EID (scheme.node.service): 2.100.1
         Report-To EID (scheme.node.service): 2.100.1
         Timestamp (created, seq): 755533838904, 0
         Lifetime: 3600000
         CRC Value: 0xB19
Canonical Block [0]:
         Block Type: 1
         Block Number: 1
         Flags: 0
         CRC Type: 1
         CRC Value: 0xc68f
         Offset Into Encoded Bundle: 42
*/
uint8_t CandBundle[] = {
    0x9f, 0x89, 0x07, 0x04, 0x01, 0x82, 0x02, 0x82,
    0x18, 0xc8, 0x01, 0x82, 0x02, 0x82, 0x18, 0x64,
    0x01, 0x82, 0x02, 0x82, 0x18, 0x64, 0x01, 0x82,
    0x1b, 0x00, 0x00, 0x00, 0xaf, 0xe9, 0x53, 0x7a,
    0x38, 0x00, 0x1a, 0x00, 0x36, 0xee, 0x80, 0x42,
    0x0b, 0x19, 0x86, 0x01, 0x01, 0x00, 0x01, 0x54,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0x42, 0xc6, 0x8f, 0xff,
};

/* Test an invalid CRC-16 in the primary block */
void Test_BPLib_CBOR_DecodePrimary_InvalidCrc(void)
{
    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;
    QCBORItem OuterArr;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void *)(CandBundle);
    UBufC.len = sizeof(CandBundle);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);
    QCBORDecode_EnterArray(&ctx, &OuterArr);

    /* Set CRC calculation to return different CRC from what's in the primary block */
    UT_SetDeferredRetcode(UT_KEY(BPLib_CRC_Calculate), 1, 0xbeef);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodePrimary(&ctx, &Bundle, CandBundle), BPLIB_INVALID_CRC_ERROR);
}

/* Test a bundle with no canonical blocks */
void Test_BPLib_CBOR_DecodePrimary_NoCanonBlks(void)
{
    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;
    QCBORItem OuterArr;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void *)(CandPrimary);
    UBufC.len = sizeof(CandPrimary);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);
    QCBORDecode_EnterArray(&ctx, &OuterArr);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodePrimary(&ctx, &Bundle, CandPrimary), BPLIB_CBOR_DEC_NO_PAYLOAD_ERR);
}

void Test_BPLib_CBOR_DecodePrimary_InvalidFlags(void)
{
    /* Primary block with flags set to 0x0f (byte 4) */
    uint8_t UnsupportedFlagPrimaryBlk[] = {
        0x9f, 0x89, 0x07, 0x0f, 0x01, 0x82, 0x02, 0x82,
        0x18, 0xc8, 0x01, 0x82, 0x02, 0x82, 0x18, 0x64,
        0x01, 0x82, 0x02, 0x82, 0x18, 0x64, 0x01, 0x82,
        0x1b, 0x00, 0x00, 0x00, 0xaf, 0xe9, 0x53, 0x7a,
        0x38, 0x00, 0x1a, 0x00, 0x36, 0xee, 0x80, 0x42,
        0x0b, 0x19, 0x86, 0x01, 0x01, 0x00, 0x01, 0x54,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0x42, 0xc6, 0x8f, 0xff,
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;
    QCBORItem OuterArr;

    /* Have CRC calculator return the CRC from the block (too lazy to do calculation on my own) */
    UT_SetDefaultReturnValue(UT_KEY(BPLib_CRC_Calculate), 0xB19);

    /* Initialize QCBOR context */
    UBufC.ptr = (const void *)(UnsupportedFlagPrimaryBlk);
    UBufC.len = sizeof(UnsupportedFlagPrimaryBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);
    QCBORDecode_EnterArray(&ctx, &OuterArr);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodePrimary(&ctx, &Bundle, UnsupportedFlagPrimaryBlk), BPLIB_SUCCESS);
}

void Test_BPLib_CBOR_DecodePrimary_CrcNone(void)
{
    /* Primary block with CRC set to none (byte 5) */
    uint8_t BadFlagPrimaryBlk[] = {
        0x9f, 0x89, 0x07, 0x04, 0x00, 0x82, 0x02, 0x82,
        0x18, 0xc8, 0x01, 0x82, 0x02, 0x82, 0x18, 0x64,
        0x01, 0x82, 0x02, 0x82, 0x18, 0x64, 0x01, 0x82,
        0x1b, 0x00, 0x00, 0x00, 0xaf, 0xe9, 0x53, 0x7a,
        0x38, 0x00, 0x1a, 0x00, 0x36, 0xee, 0x80, 0x42,
        0x0b, 0x19,
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;
    QCBORItem OuterArr;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void *)(BadFlagPrimaryBlk);
    UBufC.len = sizeof(BadFlagPrimaryBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);
    QCBORDecode_EnterArray(&ctx, &OuterArr);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodePrimary(&ctx, &Bundle, BadFlagPrimaryBlk), BPLIB_CBOR_DEC_TYPES_CRC_UNSUPPORTED_TYPE_ERR);
}

/* Test an invalid CRC-16 in the payload block */
void Test_BPLib_CBOR_DecodeCanonical_InvalidCrc(void)
{
    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(CandPayload);
    UBufC.len = sizeof(CandPayload);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    /* Set CRC calculation to return different CRC from what's in the primary block */
    UT_SetDeferredRetcode(UT_KEY(BPLib_CRC_Calculate), 1, 0xbeef);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, CandPayload), BPLIB_INVALID_CRC_ERROR);
}

/* Test an invalid block number in a payload block */
void Test_BPLib_CBOR_DecodeCanonical_BadBlockNum(void)
{
    /* Payload block with number set to 0x02 (byte 3) */
    uint8_t BadPayload[] = {
        0x86, 0x01, 0x02, 0x00, 0x01, 0x54,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0x42, 0xc6, 0x8f,
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(BadPayload);
    UBufC.len = sizeof(BadPayload);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    memset(&Bundle, 0, sizeof(Bundle));

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, BadPayload), BPLIB_CBOR_DEC_CANON_BLOCK_NUM_DEC_ERR);
}

/* Test an invalid CRC type in a payload block */
void Test_BPLib_CBOR_DecodeCanonical_BadCrcType(void)
{
    /* Payload block with CRC type set to 0x04 (byte 5) */
    uint8_t BadPayload[] = {
        0x86, 0x01, 0x01, 0x00, 0x04, 0x54,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0x42, 0xc6, 0x8f,
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(BadPayload);
    UBufC.len = sizeof(BadPayload);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    memset(&Bundle, 0, sizeof(Bundle));

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, BadPayload), BPLIB_CBOR_DEC_CANON_CRC_TYPE_DEC_ERR);
}

/* Test a valid age block */
void Test_BPLib_CBOR_DecodeCanonical_AgeBlk(void)
{
    /*
      Canonical Block [0]:
         Block Type: 7
         Block Number: 2
         Flags: 0
         CRC Type: 1
         Block Offset Start: 45
         Data Offset Start: 51
         Data Size: 5
         Block Offset End: 58
         Block Size: 14
         Age Block Data:
                 Age Block MetaData Length: 8
                 Age (in milliseconds): 108000
         CRC Value: 0x3129
    */
    uint8_t GoodAgeBlk[] = {
        0x86, 0x07, 0x02, 0x00, 0x01, 0x45,
        0x1a, 0x00, 0x01, 0xa5, 0xe0, 0x42,
        0x31, 0x29
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(GoodAgeBlk);
    UBufC.len = sizeof(GoodAgeBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    /* Set CRC calculation to return same CRC as block CRC */
    UT_SetDeferredRetcode(UT_KEY(BPLib_CRC_Calculate), 1, 0x3129);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, GoodAgeBlk), BPLIB_SUCCESS);
}

/* Test an age block with a bad block number */
void Test_BPLib_CBOR_DecodeCanonical_InvBlkNum(void)
{
    /*
      Canonical Block [0]:
         Block Type: 7
         Block Number: 0
         Flags: 0
         CRC Type: 1
         Block Offset Start: 45
         Data Offset Start: 51
         Data Size: 5
         Block Offset End: 58
         Block Size: 14
         Age Block Data:
                 Age Block MetaData Length: 8
                 Age (in milliseconds): 108000
         CRC Value: 0x3129
    */
    uint8_t AgeBlk[] = {
        0x86, 0x07, 0x00, 0x00, 0x01, 0x45,
        0x1a, 0x00, 0x01, 0xa5, 0xe0, 0x42,
        0x31, 0x29
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(AgeBlk);
    UBufC.len = sizeof(AgeBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, AgeBlk), BPLIB_CBOR_DEC_CANON_BLOCK_NUM_DEC_ERR);
}

/* Test a valid prev node block */
void Test_BPLib_CBOR_DecodeCanonical_PrevNodeBlk(void)
{
    /*
      Canonical Block [0]:
         Block Type: 6
         Block Number: 6
         Flags: 0
         CRC Type: 1
         Block Offset Start: 42
         Data Offset Start: 48
         Data Offset Size: 7
         Block Offset End: 57
         Block Size: 16
         Prev Node Block Data:
                 Prev Node Block MetaData Length: 40
                 EID Forwarded (scheme.node.service): 2.300.2
         CRC Value: 0x25D4
    */
    uint8_t GoodPrevNodeBlk[] = {
        0x86, 0x06, 0x06, 0x00, 0x01, 0x47, 0x82, 0x02,
        0x82, 0x19, 0x01, 0x2c, 0x02, 0x42, 0x25, 0xd4,
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(GoodPrevNodeBlk);
    UBufC.len = sizeof(GoodPrevNodeBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    /* Set CRC calculation to return same CRC as block CRC */
    UT_SetDeferredRetcode(UT_KEY(BPLib_CRC_Calculate), 1, 0x25d4);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, GoodPrevNodeBlk), BPLIB_SUCCESS);
}

/* Test a valid hop count block */
void Test_BPLib_CBOR_DecodeCanonical_HopCountBlk(void)
{
    /*
      Canonical Block [0]:
         Block Type: 10
         Block Number: 3
         Flags: 0
         CRC Type: 1
         Block Offset Start: 45
         Data Offset Start: 51
         Data Size: 3
         Block Offset End: 56
         Block Size: 12
         Hop Count Block Data:
                 Hop Count MetaData Length: 16
                 Hop Count Definite Array Length: 2
                 Hop Limit: 15
                 Hop Count: 3
         CRC Value: 0xB5EE
    */
    uint8_t GoodHopCountBlk[] = {
        0x86, 0x0a, 0x03, 0x00, 0x01, 0x43,
        0x82, 0x0f, 0x03, 0x42, 0xb5, 0xee
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(GoodHopCountBlk);
    UBufC.len = sizeof(GoodHopCountBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    /* Set CRC calculation to return same CRC as block CRC */
    UT_SetDeferredRetcode(UT_KEY(BPLib_CRC_Calculate), 1, 0xb5ee);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, GoodHopCountBlk), BPLIB_SUCCESS);
}

/* Test a hop count block with a limit that is too small */
void Test_BPLib_CBOR_DecodeCanonical_LimitTooSmall(void)
{
    /*
      Canonical Block [0]:
         Block Type: 10
         Block Number: 3
         Flags: 0
         CRC Type: 1
         Block Offset Start: 45
         Data Offset Start: 51
         Data Size: 3
         Block Offset End: 56
         Block Size: 12
         Hop Count Block Data:
                 Hop Count MetaData Length: 16
                 Hop Count Definite Array Length: 2
                 Hop Limit: 0 (too small)
                 Hop Count: 3
         CRC Value: 0xB5EE
    */
    uint8_t GoodHopCountBlk[] = {
        0x86, 0x0a, 0x03, 0x00, 0x01, 0x43,
        0x82, 0x00, 0x03, 0x42, 0xb5, 0xee
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(GoodHopCountBlk);
    UBufC.len = sizeof(GoodHopCountBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, GoodHopCountBlk), BPLIB_CBOR_DEC_HOP_BLOCK_INVALID_DEC_ERR);
}

/* Test a hop count block with a limit that is too small */
void Test_BPLib_CBOR_DecodeCanonical_PastLimit(void)
{
    /*
      Canonical Block [0]:
         Block Type: 10
         Block Number: 3
         Flags: 0
         CRC Type: 1
         Block Offset Start: 45
         Data Offset Start: 51
         Data Size: 3
         Block Offset End: 56
         Block Size: 12
         Hop Count Block Data:
                 Hop Count MetaData Length: 16
                 Hop Count Definite Array Length: 2
                 Hop Limit: 3
                 Hop Count: 4 (past limit)
         CRC Value: 0xB5EE
    */
    uint8_t GoodHopCountBlk[] = {
        0x86, 0x0a, 0x03, 0x00, 0x01, 0x43,
        0x82, 0x03, 0x04, 0x42, 0xb5, 0xee
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(GoodHopCountBlk);
    UBufC.len = sizeof(GoodHopCountBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, GoodHopCountBlk), BPLIB_CBOR_DEC_HOP_BLOCK_EXCEEDED_ERR);
}

/* Test a valid custody transfer block */
void Test_BPLib_CBOR_DecodeCanonical_CustodyBlk(void)
{
    /*
      Canonical Block [0]:
        Block Type: 15
        Block Number: 4
        Flags: 0
        CRC Type: 1
        Block Offset Start:
        Data Offset Start:
        Data Size:
        Block Offset End:
        Block Size:
        Custody Transfer Block Data:
            Custody Transfer Block MetaData Length:
            Bundle Sequence Number: 01
            Bundle Sequence ID: 02
            Block Source Administrative Endpoint ID: 2.200.2
        CRC Value: 0x4C2A
    */

    uint8_t GoodCustodyBlk[] = {
        0x86, 0x0F, 0x02, 0x00, 0x01,
        0x49, 0x83, 0x01, 0x02, 0x82,
        0x02, 0x82, 0x18, 0xC8, 0x02,
        0x42, 0x4C, 0x2A};

    BPLib_Bundle_t     Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC         UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(GoodCustodyBlk);
    UBufC.len = sizeof(GoodCustodyBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    /* Set CRC calculation to return same CRC as block CRC */
    UT_SetDeferredRetcode(UT_KEY(BPLib_CRC_Calculate), 1, 0x4C2A);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, GoodCustodyBlk), BPLIB_SUCCESS);
}

/* Test an unknown block type that will be deleted */
void Test_BPLib_CBOR_DecodeCanonical_UnknownDel(void)
{
    /*
      Canonical Block [0]:
         Block Type: 16
         Block Number: 3
         Flags: 0x04 (delete bundle if unsupported)
         CRC Type: 1
    */
    uint8_t UnknownBlk[] = {
        0x86, 0x0e, 0x03, 0x04, 0x01, 0x43,
        0x82, 0x0f, 0x03, 0x42, 0xb5, 0xee
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(UnknownBlk);
    UBufC.len = sizeof(UnknownBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, UnknownBlk), BPLIB_CBOR_DEC_UNKNOWN_BLOCK_DEC_ERR);
}

/* Test an unknown block type that will be discarded */
void Test_BPLib_CBOR_DecodeCanonical_UnknownDisc(void)
{
    /*
      Canonical Block [0]:
         Block Type: 14
         Block Number: 3
         Flags: 0x10 (discard block if unsupported)
         CRC Type: 1
    */
    uint8_t UnknownBlk[] = {
        0x86, 0x0e, 0x03, 0x10, 0x01, 0x43,
        0x82, 0x0f, 0x03, 0x42, 0xb5, 0xee
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(UnknownBlk);
    UBufC.len = sizeof(UnknownBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    /* Set CRC calculation to return same CRC as block CRC */
    UT_SetDeferredRetcode(UT_KEY(BPLib_CRC_Calculate), 1, 0xb5ee);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, UnknownBlk), BPLIB_SUCCESS);
    UtAssert_BOOL_TRUE(Bundle.blocks.ExtBlocks[0].Header.RequiresDiscard);
}

/* Test an unknown block type that will be kept */
void Test_BPLib_CBOR_DecodeCanonical_UnknownKeep(void)
{
    /*
      Canonical Block [0]:
         Block Type: 14
         Block Number: 3
         Flags: 0x00 (don't discard or delete)
         CRC Type: 1
    */
    uint8_t UnknownBlk[] = {
        0x86, 0x0e, 0x03, 0x00, 0x01, 0x43,
        0x82, 0x0f, 0x03, 0x42, 0xb5, 0xee
    };

    BPLib_Bundle_t Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(UnknownBlk);
    UBufC.len = sizeof(UnknownBlk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    /* Set CRC calculation to return same CRC as block CRC */
    UT_SetDeferredRetcode(UT_KEY(BPLib_CRC_Calculate), 1, 0xb5ee);

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, UnknownBlk), BPLIB_SUCCESS);
    UtAssert_BOOL_FALSE(Bundle.blocks.ExtBlocks[0].Header.RequiresDiscard);
}

void Test_BPLib_CBOR_DecodeCanonical_CCS(void)
{
    uint8_t CCS_Blk[] = {
        0x86, /* Array, 6 bytes follow */
            0x01, /* Block type */
            0x01, /* Block number */
            0x00, /* Control Flags */
            0x01, /* CRC Type */
            0x53, /* Byte string, 19 bytes follow */
                0x82, /* Array, 2 bytes follow */
                    0x04, /* Compressed Custody Signal Record Type */

                    /* === CCS Data === */

                    0xA2, /* Map, 2 key value pairs */
                        0x01, /* Custody Accepted */
                        0x83, /* Array, 3 bytes follow */
                            0x01, /* Bundle Sequence ID */
                            0x0C, /* First Sequence Number */
                            0x83, /* Array, 3 bytes follow */
                                0x02, /* Included */
                                0x04, /* Excluded */
                                0x05, /* Included */

                        0x20, /* Custody Rejected */
                        0x83, /* Array, 3 bytes follow */
                            0x03, /* Bundle Sequence ID */
                            0x0C, /* First Sequence Number */
                            0x83, /* Array, 3 bytes follow */
                                0x02, /* Included */
                                0x04, /* Excluded */
                                0x05, /* Included */

            0x42, /* Byte string, 2 bytes follow */
                0x58, /* CRC MSB */
                0x0A  /* CRC LSB */
    };

    BPLib_Bundle_t     Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC         UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(CCS_Blk);
    UBufC.len = sizeof(CCS_Blk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    Bundle.blocks.PrimaryBlock.BundleProcFlags = BPLIB_BUNDLE_PROC_ADMIN_RECORD_FLAG;

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, CCS_Blk), BPLIB_SUCCESS);

    UtAssert_STUB_COUNT(BPLib_CRC_Calculate, 0);
    UtAssert_STUB_COUNT(BPLib_ARP_ProcessCcs, 1);
}

void Test_BPLib_CBOR_DecodeCanonical_CCS_RecordTypeErr(void)
{
    uint8_t CCS_Blk[] = {
        0x86, /* Array, 6 bytes follow */
            0x01, /* Block type */
            0x01, /* Block number */
            0x00, /* Control Flags */
            0x01, /* CRC Type */
            0x53, /* Byte string, 19 bytes follow */
                0x82, /* Array, 2 bytes follow */
                    0x24, /* INVALID: Compressed Custody Signal Record Type */

                    /* === CCS Data === */

                    0xA2, /* Map, 2 key value pairs */
                        0x01, /* Custody Accepted */
                        0x83, /* Array, 3 bytes follow */
                            0x01, /* Bundle Sequence ID */
                            0x0C, /* First Sequence Number */
                            0x83, /* Array, 3 bytes follow */
                                0x02, /* Included */
                                0x04, /* Excluded */
                                0x05, /* Included */

                        0x20, /* Custody Rejected */
                        0x83, /* Array, 3 bytes follow */
                            0x03, /* Bundle Sequence ID */
                            0x0C, /* First Sequence Number */
                            0x83, /* Array, 3 bytes follow */
                                0x02, /* Included */
                                0x04, /* Excluded */
                                0x05, /* Included */

            0x42, /* Byte string, 2 bytes follow */
                0x58, /* CRC MSB */
                0x0A  /* CRC LSB */
    };

    BPLib_Bundle_t     Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC         UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(CCS_Blk);
    UBufC.len = sizeof(CCS_Blk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    Bundle.blocks.PrimaryBlock.BundleProcFlags = BPLIB_BUNDLE_PROC_ADMIN_RECORD_FLAG;

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, CCS_Blk), BPLIB_CBOR_DEC_CANON_ADMIN_REC_REC_TYPE_ERR);

    UtAssert_STUB_COUNT(BPLib_CRC_Calculate, 0);
    UtAssert_STUB_COUNT(BPLib_ARP_ProcessCcs, 0);
}

void Test_BPLib_CBOR_DecodeCanonical_CCS_ContentErr(void)
{
    uint8_t CCS_Blk[] = {
        0x86, /* Array, 6 bytes follow */
            0x01, /* Block type */
            0x01, /* Block number */
            0x00, /* Control Flags */
            0x01, /* CRC Type */
            0x53, /* Byte string, 19 bytes follow */
                0x82, /* Array, 2 bytes follow */
                    0x04, /* Compressed Custody Signal Record Type */

                    /* === CCS Data === */

                    0xA2, /* Map, 2 key value pairs */
                        0x01, /* Custody Accepted */
                        0x83, /* Array, 3 bytes follow */
                            0x01, /* Bundle Sequence ID */
                            0x2C, /* INVALID First Sequence Number */
                            0x83, /* Array, 3 bytes follow */
                                0x02, /* Included */
                                0x04, /* Excluded */
                                0x05, /* Included */

                        0x20, /* Custody Rejected */
                        0x83, /* Array, 3 bytes follow */
                            0x03, /* Bundle Sequence ID */
                            0x0C, /* First Sequence Number */
                            0x83, /* Array, 3 bytes follow */
                                0x02, /* Included */
                                0x04, /* Excluded */
                                0x05, /* Included */

            0x42, /* Byte string, 2 bytes follow */
                0x58, /* CRC MSB */
                0x0A  /* CRC LSB */
    };

    BPLib_Bundle_t     Bundle;
    QCBORDecodeContext ctx;
    UsefulBufC         UBufC;

    /* Initialize QCBOR context */
    UBufC.ptr = (const void*)(CCS_Blk);
    UBufC.len = sizeof(CCS_Blk);
    QCBORDecode_Init(&ctx, UBufC, QCBOR_DECODE_MODE_NORMAL);

    Bundle.blocks.PrimaryBlock.BundleProcFlags = BPLIB_BUNDLE_PROC_ADMIN_RECORD_FLAG;

    UtAssert_INT32_EQ(BPLib_CBOR_DecodeCanonical(&ctx, &Bundle, 0, CCS_Blk), BPLIB_CBOR_DEC_CANON_ADMIN_REC_CONT_ERR);

    UtAssert_STUB_COUNT(BPLib_CRC_Calculate, 0);
    UtAssert_STUB_COUNT(BPLib_ARP_ProcessCcs, 0);
}

void TestBplibCborDecodeInternal_Register(void)
{
    ADD_TEST(Test_BPLib_CBOR_DecodePrimary_InvalidCrc);
    ADD_TEST(Test_BPLib_CBOR_DecodePrimary_NoCanonBlks);
    ADD_TEST(Test_BPLib_CBOR_DecodePrimary_InvalidFlags);
    ADD_TEST(Test_BPLib_CBOR_DecodePrimary_CrcNone);

    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_InvalidCrc);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_BadBlockNum);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_BadCrcType);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_AgeBlk);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_InvBlkNum);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_PrevNodeBlk);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_HopCountBlk);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_LimitTooSmall);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_PastLimit);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_CustodyBlk);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_UnknownDel);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_UnknownDisc);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_UnknownKeep);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_CCS);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_CCS_RecordTypeErr);
    ADD_TEST(Test_BPLib_CBOR_DecodeCanonical_CCS_ContentErr);
}

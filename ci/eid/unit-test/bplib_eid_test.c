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

#include "bplib_eid_test_utils.h"

/* ==================== */
/* Function Definitions */
/* ==================== */

void Test_BPLib_EID_IsValid_Null(void)
{
    UtAssert_BOOL_FALSE(BPLib_EID_IsValid(NULL));
}

void Test_BPLib_EID_IsValid_DTN_Nominal(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_DTN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Actual.Node         = 0;
    EID_Actual.Service      = 0;

    UtAssert_BOOL_TRUE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_IsValid_IPN2D_None_Nominal(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Actual.Allocator    = 0;
    EID_Actual.Node         = 0;
    EID_Actual.Service      = 0;

    UtAssert_BOOL_TRUE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_IsValid_IPN2D_Nominal(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Actual.Allocator    = 0;
    EID_Actual.Node         = 1;
    EID_Actual.Service      = 1;

    UtAssert_BOOL_TRUE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_IsValid_IPN3D_None_Nominal(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 0;
    EID_Actual.Node         = 0;
    EID_Actual.Service      = 0;

    UtAssert_BOOL_TRUE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_IsValid_IPN3D_Nominal(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;

    /* === Zero Allocator === */
    EID_Actual.Allocator = 0;
    EID_Actual.Node      = 1;
    EID_Actual.Service   = 2;

    UtAssert_BOOL_TRUE(BPLib_EID_IsValid(&EID_Actual));

    /* === Non-zero allocator === */
    EID_Actual.Allocator = 1;
    EID_Actual.Node      = 1;
    EID_Actual.Service   = 1;

    UtAssert_BOOL_TRUE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_IsValid_InvalidScheme_Error(void)
{
    EID_Actual.Scheme = BPLIB_EID_IPN_SSP_FORMAT_RESERVED;
    UtAssert_BOOL_FALSE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_IsValid_InvalidDTN_Error(void)
{
    EID_Actual.Scheme = BPLIB_EID_SCHEME_DTN;
    EID_Actual.Node   = 1;

    UtAssert_BOOL_FALSE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_IsValid_IPN3dNode_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 1;
    EID_Actual.Node         = 0;

    UtAssert_BOOL_FALSE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_IsValid_IPN3dService_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 0;
    EID_Actual.Node         = 0;
    EID_Actual.Service      = 1;

    UtAssert_BOOL_FALSE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_IsValid_IPN2dAllocator_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Actual.Allocator    = 1;

    UtAssert_BOOL_FALSE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_IsValid_IPN2dService_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Actual.Allocator    = 0;
    EID_Actual.Node         = 0;
    EID_Actual.Service      = 1;

    UtAssert_BOOL_FALSE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_IsValid_Format_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = 5;

    UtAssert_BOOL_FALSE(BPLib_EID_IsValid(&EID_Actual));
}

void Test_BPLib_EID_PatternIsMatch_Null(void)
{
    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(NULL, &EID_Pattern));
    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, NULL));
}

void Test_BPLib_EID_Pattern_IsMatch_Nominal(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 10;
    EID_Actual.Node         = 11;
    EID_Actual.Service      = 12;

    EID_Pattern.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Pattern.MaxAllocator = 20;
    EID_Pattern.MinAllocator = 0;
    EID_Pattern.MaxNode      = 30;
    EID_Pattern.MinNode      = 5;
    EID_Pattern.MaxService   = 40;
    EID_Pattern.MinService   = 10;

    UtAssert_BOOL_TRUE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));
}

void Test_BPLib_EID_Pattern_IsMatch_AllocatorWildcard_Nominal(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 5;
    EID_Actual.Node         = 6;
    EID_Actual.Service      = 7;

    EID_Pattern.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Pattern.MaxAllocator = 0xFFFFFFFFFFFFFFFF;
    EID_Pattern.MinAllocator = 0;
    EID_Pattern.MaxNode      = 100;
    EID_Pattern.MinNode      = 0;
    EID_Pattern.MaxService   = 100;
    EID_Pattern.MinService   = 0;

    UtAssert_BOOL_TRUE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));
}

void Test_BPLib_EID_Pattern_IsMatch_NodeWildcard_Nominal(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Actual.Node         = 9;
    EID_Actual.Service      = 10;

    EID_Pattern.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Pattern.MaxNode      = 0xFFFFFFFFFFFFFFFF;
    EID_Pattern.MinNode      = 0;
    EID_Pattern.MaxService   = 100;
    EID_Pattern.MinService   = 0;

    UtAssert_BOOL_TRUE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));
}

void Test_BPLib_EID_Pattern_IsMatch_ServiceWildcard_Nominal(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Actual.Node         = 12;
    EID_Actual.Service      = 13;

    EID_Pattern.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Pattern.MaxNode      = 100;
    EID_Pattern.MinNode      = 0;
    EID_Pattern.MaxService   = 0xFFFFFFFFFFFFFFFF;
    EID_Pattern.MinService   = 0;

    UtAssert_BOOL_TRUE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));
}

void Test_BPLib_EID_Pattern_IsMatch_SchemeMismatch_Error(void)
{
    EID_Actual.Scheme    = BPLIB_EID_SCHEME_IPN;
    EID_Pattern.Scheme = BPLIB_EID_SCHEME_DTN;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));
}

void Test_BPLib_EID_Pattern_IsMatch_FormatMismatch_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;

    EID_Pattern.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));
}

void Test_BPLib_EID_Pattern_IsMatch_AllocatorMismatch_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_DTN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;

    EID_Pattern.Scheme       = BPLIB_EID_SCHEME_DTN;
    EID_Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;

    /* === Allocator > Max Allocator === */
    EID_Actual.Allocator = 17;

    EID_Pattern.MaxAllocator = 2;
    EID_Pattern.MinAllocator = 1;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));

    /* === Allocator < Min Allocator === */
    EID_Actual.Allocator = 1;

    EID_Pattern.MaxAllocator = 10;
    EID_Pattern.MinAllocator = 9;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));
}

void Test_BPLib_EID_Pattern_IsMatch_NodeMismatch_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_DTN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;

    EID_Pattern.Scheme       = BPLIB_EID_SCHEME_DTN;
    EID_Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;

    /* === Node > Max Node === */
    EID_Actual.Node = 22;

    EID_Pattern.MaxNode = 20;
    EID_Pattern.MinNode = 0;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));

    /* === Node < Min Node === */
    EID_Actual.Node = 0;

    EID_Pattern.MaxNode = 20;
    EID_Pattern.MinNode = 5;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));
}

void Test_BPLib_EID_Pattern_IsMatch_ServiceMismatch_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_DTN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Actual.Node         = 24;

    EID_Pattern.Scheme       = BPLIB_EID_SCHEME_DTN;
    EID_Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Pattern.MaxNode      = 30;
    EID_Pattern.MinNode      = 0;

    /* === Service > Max Service === */
    EID_Actual.Service = 20;

    EID_Pattern.MaxService = 10;
    EID_Pattern.MinService = 0;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));

    /* === Service < Min Service === */
    EID_Actual.Service = 0;

    EID_Pattern.MaxService = 10;
    EID_Pattern.MinService = 5;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));
}

void Test_BPLib_EID_Pattern_IsMatch_InvalidRanges_Error(void)
{
    /* === Invalid Allocator pattern range === */
    EID_Pattern.MaxAllocator = 0;
    EID_Pattern.MinAllocator = 100;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));

    /* === Invalid Node pattern range === */
    EID_Pattern.MaxAllocator = 100; /* Isolate node range error */
    EID_Pattern.MinAllocator = 0;   /* Isolate node range error */
    EID_Pattern.MaxNode      = 0;
    EID_Pattern.MinNode      = 100;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));

    /* === Invalid Service pattern range === */
    EID_Pattern.MaxNode    = 100; /* Isolate service range error */
    EID_Pattern.MinNode    = 0;   /* Isolate service range error */
    EID_Pattern.MaxService = 0;
    EID_Pattern.MinService = 100;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsMatch(&EID_Actual, &EID_Pattern));
}

void Test_BPLib_EID_IsMatch_Null(void)
{
    UtAssert_BOOL_FALSE(BPLib_EID_IsMatch(NULL, &EID_Reference));
    UtAssert_BOOL_FALSE(BPLib_EID_IsMatch(&EID_Actual, NULL));
}

void Test_BPLib_EID_IsMatch_Nominal(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_DTN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 1;
    EID_Actual.Node         = 2;
    EID_Actual.Service      = 3;

    EID_Reference.Scheme       = BPLIB_EID_SCHEME_DTN;
    EID_Reference.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Reference.Allocator    = 1;
    EID_Reference.Node         = 2;
    EID_Reference.Service      = 3;

    UtAssert_BOOL_TRUE(BPLib_EID_IsMatch(&EID_Actual, &EID_Reference));
}

void Test_BPLib_EID_IsMatch_MismatchScheme_Error(void)
{
    EID_Actual.Scheme    = BPLIB_EID_SCHEME_DTN;
    EID_Reference.Scheme = BPLIB_EID_SCHEME_IPN;

    UtAssert_BOOL_FALSE(BPLib_EID_IsMatch(&EID_Actual, &EID_Reference));
}

void Test_BPLib_EID_IsMatch_MismatchIpnSspFormat_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_DTN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;

    EID_Reference.Scheme    = EID_Actual.Scheme;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;

    UtAssert_BOOL_FALSE(BPLib_EID_IsMatch(&EID_Actual, &EID_Reference));
}

void Test_BPLib_EID_IsMatch_MismatchAllocator_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 1;

    EID_Reference.Scheme       = EID_Actual.Scheme;
    EID_Reference.IpnSspFormat = EID_Actual.IpnSspFormat;
    EID_Reference.Allocator    = 2;

    UtAssert_BOOL_FALSE(BPLib_EID_IsMatch(&EID_Actual, &EID_Reference));
}

void Test_BPLib_EID_IsMatch_MismatchNode_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 1;
    EID_Actual.Node         = 2;

    EID_Reference.Scheme       = EID_Actual.Scheme;
    EID_Reference.IpnSspFormat = EID_Actual.IpnSspFormat;
    EID_Reference.Allocator    = EID_Actual.Allocator;
    EID_Reference.Node         = 3;

    UtAssert_BOOL_FALSE(BPLib_EID_IsMatch(&EID_Actual, &EID_Reference));
}

void Test_BPLib_EID_IsMatch_MismatchService_Error(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 1;
    EID_Actual.Node         = 2;
    EID_Actual.Service      = 3;

    EID_Reference.Scheme       = EID_Actual.Scheme;
    EID_Reference.IpnSspFormat = EID_Actual.IpnSspFormat;
    EID_Reference.Allocator    = EID_Actual.Allocator;
    EID_Reference.Node         = EID_Actual.Node;
    EID_Reference.Service      = 4;

    UtAssert_BOOL_FALSE(BPLib_EID_IsMatch(&EID_Actual, &EID_Reference));
}

void Test_BPLib_EID_NodeIsMatch_Null(void)
{
    BPLib_EID_t EID;

    UtAssert_BOOL_FALSE(BPLib_EID_NodeIsMatch(NULL, &EID));
    UtAssert_BOOL_FALSE(BPLib_EID_NodeIsMatch(&EID, NULL));
}

void Test_BPLib_EID_NodeIsMatch_Nominal(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 1;
    EID_Actual.Node         = 2;

    EID_Reference.Scheme       = EID_Actual.Scheme;
    EID_Reference.IpnSspFormat = EID_Actual.IpnSspFormat;
    EID_Reference.Allocator    = EID_Actual.Allocator;
    EID_Reference.Node         = EID_Actual.Node;

    UtAssert_BOOL_TRUE(BPLib_EID_NodeIsMatch(&EID_Actual, &EID_Reference));
}

void Test_BPLib_EID_NodeIsMatch_DiffNode(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 1;
    EID_Actual.Node         = 2;

    EID_Reference.Scheme       = EID_Actual.Scheme;
    EID_Reference.IpnSspFormat = EID_Actual.IpnSspFormat;
    EID_Reference.Allocator    = EID_Actual.Allocator;
    EID_Reference.Node         = EID_Actual.Node + 1;

    UtAssert_BOOL_FALSE(BPLib_EID_NodeIsMatch(&EID_Actual, &EID_Reference));
}

void Test_BPLib_EID_NodeIsMatch_DiffAlloc(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 1;
    EID_Actual.Node         = 2;

    EID_Reference.Scheme       = EID_Actual.Scheme;
    EID_Reference.IpnSspFormat = EID_Actual.IpnSspFormat;
    EID_Reference.Allocator    = EID_Actual.Allocator + 1;
    EID_Reference.Node         = EID_Actual.Node;

    UtAssert_BOOL_FALSE(BPLib_EID_NodeIsMatch(&EID_Actual, &EID_Reference));
}

void Test_BPLib_EID_NodeIsMatch_DiffFormat(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 1;
    EID_Actual.Node         = 2;

    EID_Reference.Scheme       = EID_Actual.Scheme;
    EID_Reference.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID_Reference.Allocator    = EID_Actual.Allocator;
    EID_Reference.Node         = EID_Actual.Node;

    UtAssert_BOOL_FALSE(BPLib_EID_NodeIsMatch(&EID_Actual, &EID_Reference));
}

void Test_BPLib_EID_NodeIsMatch_DiffScheme(void)
{
    EID_Actual.Scheme       = BPLIB_EID_SCHEME_IPN;
    EID_Actual.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID_Actual.Allocator    = 1;
    EID_Actual.Node         = 2;

    EID_Reference.Scheme       = BPLIB_EID_SCHEME_DTN;
    EID_Reference.IpnSspFormat = EID_Actual.IpnSspFormat;
    EID_Reference.Allocator    = EID_Actual.Allocator;
    EID_Reference.Node         = EID_Actual.Node;

    UtAssert_BOOL_FALSE(BPLib_EID_NodeIsMatch(&EID_Actual, &EID_Reference));
}

void Test_BPLib_EID_CopyEids_Null(void)
{
    BPLib_EID_t *Actual = NULL;
    BPLib_EID_t Ref;

    BPLib_EID_CopyEids(Actual, Ref);

    UtAssert_NULL(Actual);
}

void Test_BPLib_EID_CopyEids_Nominal(void)
{
    BPLib_EID_t Actual;
    BPLib_EID_t Ref;

    memset(&Ref, 0, sizeof(Ref));

    Actual.Allocator = 1;
    Actual.IpnSspFormat = 2;
    Actual.Node = 3;
    Actual.Scheme = 4;
    Actual.Service = 5;

    BPLib_EID_CopyEids(&Actual, Ref);

    UtAssert_EQ(uint64_t, Actual.Allocator, Ref.Allocator);
    UtAssert_EQ(uint64_t, Actual.IpnSspFormat, Ref.IpnSspFormat);
    UtAssert_EQ(uint64_t, Actual.Node, Ref.Node);
    UtAssert_EQ(uint64_t, Actual.Scheme, Ref.Scheme);
    UtAssert_EQ(uint64_t, Actual.Service, Ref.Service);
}

void Test_BPLib_EID_CopyEidPatterns_Null(void)
{
    BPLib_EID_Pattern_t *Actual = NULL;
    BPLib_EID_Pattern_t Ref;

    BPLib_EID_CopyEidPatterns(Actual, Ref);

    UtAssert_NULL(Actual);
}

void Test_BPLib_EID_CopyEidPatterns_Nominal(void)
{
    BPLib_EID_Pattern_t Actual;
    BPLib_EID_Pattern_t Ref;

    memset(&Ref, 0, sizeof(Ref));

    Actual.Scheme = 1;
    Actual.IpnSspFormat = 2;
    Actual.MaxAllocator = 3;
    Actual.MinAllocator = 4;
    Actual.MaxNode = 5;
    Actual.MinNode = 6;
    Actual.MinService = 7;
    Actual.MaxService = 8;

    BPLib_EID_CopyEidPatterns(&Actual, Ref);

    UtAssert_EQ(uint64_t, Actual.Scheme, Ref.Scheme);
    UtAssert_EQ(uint64_t, Actual.IpnSspFormat, Ref.IpnSspFormat);
    UtAssert_EQ(uint64_t, Actual.MaxAllocator, Ref.MinAllocator);
    UtAssert_EQ(uint64_t, Actual.MinAllocator, Ref.MinAllocator);
    UtAssert_EQ(uint64_t, Actual.MaxNode, Ref.MaxNode);
    UtAssert_EQ(uint64_t, Actual.MinNode, Ref.MinNode);
    UtAssert_EQ(uint64_t, Actual.MaxService, Ref.MaxService);
    UtAssert_EQ(uint64_t, Actual.MinService, Ref.MinService);
}

void Test_BPLib_EID_PatternIsValid_Null(void)
{
    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsValid(NULL));
}

void Test_BPLib_EID_PatternIsValid_Nominal(void)
{
    BPLib_EID_Pattern_t Pattern;

    Pattern.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    Pattern.MaxAllocator = 0;
    Pattern.MinAllocator = 0;
    Pattern.MaxNode = 10;
    Pattern.MinNode = 9;
    Pattern.MaxService = 10;
    Pattern.MinService = 9;

    UtAssert_BOOL_TRUE(BPLib_EID_PatternIsValid(&Pattern));
}

void Test_BPLib_EID_PatternIsValid_IpnThree(void)
{
    BPLib_EID_Pattern_t Pattern;

    Pattern.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern.MaxAllocator = 10;
    Pattern.MinAllocator = 9;
    Pattern.MaxNode = 10;
    Pattern.MinNode = 9;
    Pattern.MaxService = 10;
    Pattern.MinService = 9;

    UtAssert_BOOL_TRUE(BPLib_EID_PatternIsValid(&Pattern));
}

void Test_BPLib_EID_PatternIsValid_InvAlloc(void)
{
    BPLib_EID_Pattern_t Pattern;

    Pattern.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern.MaxAllocator = 8;
    Pattern.MinAllocator = 9;
    Pattern.MaxNode = 10;
    Pattern.MinNode = 9;
    Pattern.MaxService = 10;
    Pattern.MinService = 9;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsValid(&Pattern));
}

void Test_BPLib_EID_PatternIsValid_InvNode(void)
{
    BPLib_EID_Pattern_t Pattern;

    Pattern.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern.MaxAllocator = 10;
    Pattern.MinAllocator = 9;
    Pattern.MaxNode = 8;
    Pattern.MinNode = 9;
    Pattern.MaxService = 10;
    Pattern.MinService = 9;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsValid(&Pattern));
}

void Test_BPLib_EID_PatternIsValid_InvServ(void)
{
    BPLib_EID_Pattern_t Pattern;

    Pattern.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern.MaxAllocator = 10;
    Pattern.MinAllocator = 9;
    Pattern.MaxNode = 10;
    Pattern.MinNode = 9;
    Pattern.MaxService = 8;
    Pattern.MinService = 9;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsValid(&Pattern));
}

void Test_BPLib_EID_PatternIsValid_InvScheme(void)
{
    BPLib_EID_Pattern_t Pattern;

    Pattern.Scheme = BPLIB_EID_SCHEME_DTN;
    Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern.MaxAllocator = 10;
    Pattern.MinAllocator = 9;
    Pattern.MaxNode = 10;
    Pattern.MinNode = 9;
    Pattern.MaxService = 10;
    Pattern.MinService = 9;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsValid(&Pattern));
}

void Test_BPLib_EID_PatternIsValid_InvFormat(void)
{
    BPLib_EID_Pattern_t Pattern;

    Pattern.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern.IpnSspFormat = 0;
    Pattern.MaxAllocator = 10;
    Pattern.MinAllocator = 9;
    Pattern.MaxNode = 10;
    Pattern.MinNode = 9;
    Pattern.MaxService = 10;
    Pattern.MinService = 9;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternIsValid(&Pattern));
}

void Test_BPLib_EID_PatternsAreMatch_Null(void)
{
    BPLib_EID_Pattern_t EID;

    UtAssert_BOOL_FALSE(BPLib_EID_PatternsAreMatch(NULL, &EID));
    UtAssert_BOOL_FALSE(BPLib_EID_PatternsAreMatch(&EID, NULL));
}

void Test_BPLib_EID_PatternsAreMatch_Nominal(void)
{
    BPLib_EID_Pattern_t Pattern1;
    BPLib_EID_Pattern_t Pattern2;

    Pattern1.Scheme = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern1.IpnSspFormat = 0;
    Pattern1.MaxAllocator = 10;
    Pattern1.MinAllocator = 9;
    Pattern1.MaxNode = 10;
    Pattern1.MinNode = 9;
    Pattern1.MaxService = 10;
    Pattern1.MinService = 9;

    Pattern2.Scheme = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern2.IpnSspFormat = 0;
    Pattern2.MaxAllocator = 10;
    Pattern2.MinAllocator = 9;
    Pattern2.MaxNode = 10;
    Pattern2.MinNode = 9;
    Pattern2.MaxService = 10;
    Pattern2.MinService = 9;    

    UtAssert_BOOL_TRUE(BPLib_EID_PatternsAreMatch(&Pattern1, &Pattern2));
}

void Test_BPLib_EID_PatternsAreMatch_DiffScheme(void)
{
    BPLib_EID_Pattern_t Pattern1;
    BPLib_EID_Pattern_t Pattern2;

    Pattern1.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern1.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern1.MaxAllocator = 10;
    Pattern1.MinAllocator = 9;
    Pattern1.MaxNode = 10;
    Pattern1.MinNode = 9;
    Pattern1.MaxService = 10;
    Pattern1.MinService = 9;

    Pattern2.Scheme = BPLIB_EID_SCHEME_DTN;
    Pattern2.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern2.MaxAllocator = 10;
    Pattern2.MinAllocator = 9;
    Pattern2.MaxNode = 10;
    Pattern2.MinNode = 9;
    Pattern2.MaxService = 10;
    Pattern2.MinService = 9;    

    UtAssert_BOOL_FALSE(BPLib_EID_PatternsAreMatch(&Pattern1, &Pattern2));
}

void Test_BPLib_EID_PatternsAreMatch_DiffFormat(void)
{
    BPLib_EID_Pattern_t Pattern1;
    BPLib_EID_Pattern_t Pattern2;

    Pattern1.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern1.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern1.MaxAllocator = 10;
    Pattern1.MinAllocator = 9;
    Pattern1.MaxNode = 10;
    Pattern1.MinNode = 9;
    Pattern1.MaxService = 10;
    Pattern1.MinService = 9;

    Pattern2.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern2.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    Pattern2.MaxAllocator = 10;
    Pattern2.MinAllocator = 9;
    Pattern2.MaxNode = 10;
    Pattern2.MinNode = 9;
    Pattern2.MaxService = 10;
    Pattern2.MinService = 9;    

    UtAssert_BOOL_FALSE(BPLib_EID_PatternsAreMatch(&Pattern1, &Pattern2));
}

void Test_BPLib_EID_PatternsAreMatch_DiffMaxAlloc(void)
{
    BPLib_EID_Pattern_t Pattern1;
    BPLib_EID_Pattern_t Pattern2;

    Pattern1.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern1.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern1.MaxAllocator = 10;
    Pattern1.MinAllocator = 9;
    Pattern1.MaxNode = 10;
    Pattern1.MinNode = 9;
    Pattern1.MaxService = 10;
    Pattern1.MinService = 9;

    Pattern2.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern2.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern2.MaxAllocator = 11;
    Pattern2.MinAllocator = 9;
    Pattern2.MaxNode = 10;
    Pattern2.MinNode = 9;
    Pattern2.MaxService = 10;
    Pattern2.MinService = 9;    

    UtAssert_BOOL_FALSE(BPLib_EID_PatternsAreMatch(&Pattern1, &Pattern2));
}

void Test_BPLib_EID_PatternsAreMatch_DiffMinAlloc(void)
{
    BPLib_EID_Pattern_t Pattern1;
    BPLib_EID_Pattern_t Pattern2;

    Pattern1.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern1.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern1.MaxAllocator = 10;
    Pattern1.MinAllocator = 9;
    Pattern1.MaxNode = 10;
    Pattern1.MinNode = 9;
    Pattern1.MaxService = 10;
    Pattern1.MinService = 9;

    Pattern2.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern2.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern2.MaxAllocator = 10;
    Pattern2.MinAllocator = 8;
    Pattern2.MaxNode = 10;
    Pattern2.MinNode = 9;
    Pattern2.MaxService = 10;
    Pattern2.MinService = 9;    

    UtAssert_BOOL_FALSE(BPLib_EID_PatternsAreMatch(&Pattern1, &Pattern2));
}

void Test_BPLib_EID_PatternsAreMatch_DiffMaxNode(void)
{
    BPLib_EID_Pattern_t Pattern1;
    BPLib_EID_Pattern_t Pattern2;

    Pattern1.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern1.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern1.MaxAllocator = 10;
    Pattern1.MinAllocator = 9;
    Pattern1.MaxNode = 10;
    Pattern1.MinNode = 9;
    Pattern1.MaxService = 10;
    Pattern1.MinService = 9;

    Pattern2.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern2.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern2.MaxAllocator = 10;
    Pattern2.MinAllocator = 9;
    Pattern2.MaxNode = 11;
    Pattern2.MinNode = 9;
    Pattern2.MaxService = 10;
    Pattern2.MinService = 9;    

    UtAssert_BOOL_FALSE(BPLib_EID_PatternsAreMatch(&Pattern1, &Pattern2));
}

void Test_BPLib_EID_PatternsAreMatch_DiffMinNode(void)
{
    BPLib_EID_Pattern_t Pattern1;
    BPLib_EID_Pattern_t Pattern2;

    Pattern1.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern1.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern1.MaxAllocator = 10;
    Pattern1.MinAllocator = 9;
    Pattern1.MaxNode = 10;
    Pattern1.MinNode = 9;
    Pattern1.MaxService = 10;
    Pattern1.MinService = 9;

    Pattern2.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern2.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern2.MaxAllocator = 10;
    Pattern2.MinAllocator = 9;
    Pattern2.MaxNode = 10;
    Pattern2.MinNode = 8;
    Pattern2.MaxService = 10;
    Pattern2.MinService = 9;    

    UtAssert_BOOL_FALSE(BPLib_EID_PatternsAreMatch(&Pattern1, &Pattern2));
}

void Test_BPLib_EID_PatternsAreMatch_DiffMaxServ(void)
{
    BPLib_EID_Pattern_t Pattern1;
    BPLib_EID_Pattern_t Pattern2;

    Pattern1.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern1.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern1.MaxAllocator = 10;
    Pattern1.MinAllocator = 9;
    Pattern1.MaxNode = 10;
    Pattern1.MinNode = 9;
    Pattern1.MaxService = 10;
    Pattern1.MinService = 9;

    Pattern2.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern2.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern2.MaxAllocator = 10;
    Pattern2.MinAllocator = 9;
    Pattern2.MaxNode = 10;
    Pattern2.MinNode = 9;
    Pattern2.MaxService = 11;
    Pattern2.MinService = 9;    

    UtAssert_BOOL_FALSE(BPLib_EID_PatternsAreMatch(&Pattern1, &Pattern2));
}

void Test_BPLib_EID_PatternsAreMatch_DiffMinServ(void)
{
    BPLib_EID_Pattern_t Pattern1;
    BPLib_EID_Pattern_t Pattern2;

    Pattern1.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern1.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern1.MaxAllocator = 10;
    Pattern1.MinAllocator = 9;
    Pattern1.MaxNode = 10;
    Pattern1.MinNode = 9;
    Pattern1.MaxService = 10;
    Pattern1.MinService = 9;

    Pattern2.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern2.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern2.MaxAllocator = 10;
    Pattern2.MinAllocator = 9;
    Pattern2.MaxNode = 10;
    Pattern2.MinNode = 9;
    Pattern2.MaxService = 10;
    Pattern2.MinService = 8;    

    UtAssert_BOOL_FALSE(BPLib_EID_PatternsAreMatch(&Pattern1, &Pattern2));
}

void Test_BPLib_EID_GetPatternString_Null(void)
{
    BPLib_EID_Pattern_t Pattern;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    UtAssert_VOIDCALL(BPLib_EID_GetPatternString(NULL, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_VOIDCALL(BPLib_EID_GetPatternString(&Pattern, NULL, BPLIB_MAX_STR_LENGTH));
}

void Test_BPLib_EID_GetPatternString_Dtn(void)
{
    BPLib_EID_Pattern_t Pattern;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    Pattern.Scheme = BPLIB_EID_SCHEME_DTN;

    UtAssert_VOIDCALL(BPLib_EID_GetPatternString(&Pattern, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_STRINGBUF_EQ(StrBuf, BPLIB_MAX_STR_LENGTH, "dtn:none", BPLIB_MAX_STR_LENGTH);
}

void Test_BPLib_EID_GetPatternString_Ipn2(void)
{
    BPLib_EID_Pattern_t Pattern;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    Pattern.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    Pattern.MinNode = 3;
    Pattern.MaxNode = 4;
    Pattern.MinService = 5;
    Pattern.MaxService = 6;

    UtAssert_VOIDCALL(BPLib_EID_GetPatternString(&Pattern, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_STRINGBUF_EQ(StrBuf, BPLIB_MAX_STR_LENGTH, "ipn:[3-4].[5-6]", BPLIB_MAX_STR_LENGTH);
}

void Test_BPLib_EID_GetPatternString_Ipn3(void)
{
    BPLib_EID_Pattern_t Pattern;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    Pattern.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    Pattern.MinAllocator = 1;
    Pattern.MaxAllocator = 2;
    Pattern.MinNode = 3;
    Pattern.MaxNode = 4;
    Pattern.MinService = 5;
    Pattern.MaxService = 6;

    UtAssert_VOIDCALL(BPLib_EID_GetPatternString(&Pattern, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_STRINGBUF_EQ(StrBuf, BPLIB_MAX_STR_LENGTH, "ipn:[1-2].[3-4].[5-6]", BPLIB_MAX_STR_LENGTH);
}

void Test_BPLib_EID_GetPatternString_InvFormat(void)
{
    BPLib_EID_Pattern_t Pattern;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    Pattern.Scheme = BPLIB_EID_SCHEME_IPN;
    Pattern.IpnSspFormat = 0;

    UtAssert_VOIDCALL(BPLib_EID_GetPatternString(&Pattern, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_STRINGBUF_EQ(StrBuf, BPLIB_MAX_STR_LENGTH, "INVALID EID", BPLIB_MAX_STR_LENGTH);
}

void Test_BPLib_EID_GetPatternString_InvScheme(void)
{
    BPLib_EID_Pattern_t Pattern;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    Pattern.Scheme = BPLIB_EID_SCHEME_RESERVED;

    UtAssert_VOIDCALL(BPLib_EID_GetPatternString(&Pattern, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_STRINGBUF_EQ(StrBuf, BPLIB_MAX_STR_LENGTH, "INVALID EID", BPLIB_MAX_STR_LENGTH);
}

void Test_BPLib_EID_GetString_Null(void)
{
    BPLib_EID_t EID;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    UtAssert_VOIDCALL(BPLib_EID_GetString(NULL, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_VOIDCALL(BPLib_EID_GetString(&EID, NULL, BPLIB_MAX_STR_LENGTH));
}

void Test_BPLib_EID_GetString_Dtn(void)
{
    BPLib_EID_t EID;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    EID.Scheme = BPLIB_EID_SCHEME_DTN;

    UtAssert_VOIDCALL(BPLib_EID_GetString(&EID, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_STRINGBUF_EQ(StrBuf, BPLIB_MAX_STR_LENGTH, "dtn:none", BPLIB_MAX_STR_LENGTH);
}

void Test_BPLib_EID_GetString_Ipn2(void)
{
    BPLib_EID_t EID;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    EID.Scheme = BPLIB_EID_SCHEME_IPN;
    EID.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT;
    EID.Node = 2;
    EID.Service = 3;

    UtAssert_VOIDCALL(BPLib_EID_GetString(&EID, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_STRINGBUF_EQ(StrBuf, BPLIB_MAX_STR_LENGTH, "ipn:2.3", BPLIB_MAX_STR_LENGTH);
}

void Test_BPLib_EID_GetString_Ipn3(void)
{
    BPLib_EID_t EID;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    EID.Scheme = BPLIB_EID_SCHEME_IPN;
    EID.IpnSspFormat = BPLIB_EID_IPN_SSP_FORMAT_THREE_DIGIT;
    EID.Allocator = 1;
    EID.Node = 2;
    EID.Service = 3;

    UtAssert_VOIDCALL(BPLib_EID_GetString(&EID, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_STRINGBUF_EQ(StrBuf, BPLIB_MAX_STR_LENGTH, "ipn:1.2.3", BPLIB_MAX_STR_LENGTH);
}

void Test_BPLib_EID_GetString_InvFormat(void)
{
    BPLib_EID_t EID;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    EID.Scheme = BPLIB_EID_SCHEME_IPN;
    EID.IpnSspFormat = 0;

    UtAssert_VOIDCALL(BPLib_EID_GetString(&EID, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_STRINGBUF_EQ(StrBuf, BPLIB_MAX_STR_LENGTH, "INVALID EID", BPLIB_MAX_STR_LENGTH);
}

void Test_BPLib_EID_GetString_InvScheme(void)
{
    BPLib_EID_t EID;
    char StrBuf[BPLIB_MAX_STR_LENGTH];

    EID.Scheme = BPLIB_EID_SCHEME_RESERVED;

    UtAssert_VOIDCALL(BPLib_EID_GetString(&EID, StrBuf, BPLIB_MAX_STR_LENGTH));
    UtAssert_STRINGBUF_EQ(StrBuf, BPLIB_MAX_STR_LENGTH, "INVALID EID", BPLIB_MAX_STR_LENGTH);
}

void TestBplibEid_Register(void)
{
    ADD_TEST(Test_BPLib_EID_IsValid_Null);
    ADD_TEST(Test_BPLib_EID_IsValid_DTN_Nominal);
    ADD_TEST(Test_BPLib_EID_IsValid_IPN2D_None_Nominal);
    ADD_TEST(Test_BPLib_EID_IsValid_IPN2D_Nominal);
    ADD_TEST(Test_BPLib_EID_IsValid_IPN3D_None_Nominal);
    ADD_TEST(Test_BPLib_EID_IsValid_IPN3D_Nominal);
    ADD_TEST(Test_BPLib_EID_IsValid_InvalidScheme_Error);
    ADD_TEST(Test_BPLib_EID_IsValid_InvalidDTN_Error);
    ADD_TEST(Test_BPLib_EID_IsValid_IPN3dNode_Error);
    ADD_TEST(Test_BPLib_EID_IsValid_IPN3dService_Error);
    ADD_TEST(Test_BPLib_EID_IsValid_IPN2dAllocator_Error);
    ADD_TEST(Test_BPLib_EID_IsValid_IPN2dService_Error);
    ADD_TEST(Test_BPLib_EID_IsValid_Format_Error);

    ADD_TEST(Test_BPLib_EID_PatternIsMatch_Null);
    ADD_TEST(Test_BPLib_EID_Pattern_IsMatch_Nominal);
    ADD_TEST(Test_BPLib_EID_Pattern_IsMatch_AllocatorWildcard_Nominal);
    ADD_TEST(Test_BPLib_EID_Pattern_IsMatch_NodeWildcard_Nominal);
    ADD_TEST(Test_BPLib_EID_Pattern_IsMatch_ServiceWildcard_Nominal);
    ADD_TEST(Test_BPLib_EID_Pattern_IsMatch_SchemeMismatch_Error);
    ADD_TEST(Test_BPLib_EID_Pattern_IsMatch_FormatMismatch_Error);
    ADD_TEST(Test_BPLib_EID_Pattern_IsMatch_AllocatorMismatch_Error);
    ADD_TEST(Test_BPLib_EID_Pattern_IsMatch_NodeMismatch_Error);
    ADD_TEST(Test_BPLib_EID_Pattern_IsMatch_ServiceMismatch_Error);
    ADD_TEST(Test_BPLib_EID_Pattern_IsMatch_InvalidRanges_Error);

    ADD_TEST(Test_BPLib_EID_IsMatch_Null);
    ADD_TEST(Test_BPLib_EID_IsMatch_Nominal);
    ADD_TEST(Test_BPLib_EID_IsMatch_MismatchScheme_Error);
    ADD_TEST(Test_BPLib_EID_IsMatch_MismatchIpnSspFormat_Error);
    ADD_TEST(Test_BPLib_EID_IsMatch_MismatchAllocator_Error);
    ADD_TEST(Test_BPLib_EID_IsMatch_MismatchNode_Error);
    ADD_TEST(Test_BPLib_EID_IsMatch_MismatchService_Error);

    ADD_TEST(Test_BPLib_EID_NodeIsMatch_Null);
    ADD_TEST(Test_BPLib_EID_NodeIsMatch_Nominal);
    ADD_TEST(Test_BPLib_EID_NodeIsMatch_DiffAlloc);
    ADD_TEST(Test_BPLib_EID_NodeIsMatch_DiffFormat);
    ADD_TEST(Test_BPLib_EID_NodeIsMatch_DiffNode);
    ADD_TEST(Test_BPLib_EID_NodeIsMatch_DiffScheme);

    ADD_TEST(Test_BPLib_EID_CopyEids_Null);
    ADD_TEST(Test_BPLib_EID_CopyEids_Nominal);

    ADD_TEST(Test_BPLib_EID_CopyEidPatterns_Null);
    ADD_TEST(Test_BPLib_EID_CopyEidPatterns_Nominal);

    ADD_TEST(Test_BPLib_EID_PatternIsValid_Null);
    ADD_TEST(Test_BPLib_EID_PatternIsValid_Nominal);
    ADD_TEST(Test_BPLib_EID_PatternIsValid_IpnThree);
    ADD_TEST(Test_BPLib_EID_PatternIsValid_InvAlloc);
    ADD_TEST(Test_BPLib_EID_PatternIsValid_InvNode);
    ADD_TEST(Test_BPLib_EID_PatternIsValid_InvServ);
    ADD_TEST(Test_BPLib_EID_PatternIsValid_InvScheme);
    ADD_TEST(Test_BPLib_EID_PatternIsValid_InvFormat);

    ADD_TEST(Test_BPLib_EID_PatternsAreMatch_Null);
    ADD_TEST(Test_BPLib_EID_PatternsAreMatch_Nominal);
    ADD_TEST(Test_BPLib_EID_PatternsAreMatch_DiffScheme);
    ADD_TEST(Test_BPLib_EID_PatternsAreMatch_DiffFormat);
    ADD_TEST(Test_BPLib_EID_PatternsAreMatch_DiffMaxAlloc);
    ADD_TEST(Test_BPLib_EID_PatternsAreMatch_DiffMinAlloc);
    ADD_TEST(Test_BPLib_EID_PatternsAreMatch_DiffMaxNode);
    ADD_TEST(Test_BPLib_EID_PatternsAreMatch_DiffMinNode);
    ADD_TEST(Test_BPLib_EID_PatternsAreMatch_DiffMaxServ);
    ADD_TEST(Test_BPLib_EID_PatternsAreMatch_DiffMinServ);

    ADD_TEST(Test_BPLib_EID_GetPatternString_Null);
    ADD_TEST(Test_BPLib_EID_GetPatternString_Dtn);
    ADD_TEST(Test_BPLib_EID_GetPatternString_Ipn2);
    ADD_TEST(Test_BPLib_EID_GetPatternString_Ipn3);
    ADD_TEST(Test_BPLib_EID_GetPatternString_InvFormat);
    ADD_TEST(Test_BPLib_EID_GetPatternString_InvScheme);

    ADD_TEST(Test_BPLib_EID_GetString_Null);
    ADD_TEST(Test_BPLib_EID_GetString_Dtn);
    ADD_TEST(Test_BPLib_EID_GetString_Ipn2);
    ADD_TEST(Test_BPLib_EID_GetString_Ipn3);
    ADD_TEST(Test_BPLib_EID_GetString_InvFormat);
    ADD_TEST(Test_BPLib_EID_GetString_InvScheme);    
}
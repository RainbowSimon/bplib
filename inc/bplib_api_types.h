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

#ifndef BPLIB_API_TYPES_H
#define BPLIB_API_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** Include Files
*/

#include "bplib_cfg.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

/*
** Type Definitions
*/

/**
 * @brief Canonical block type
 *
 * @note The numeric values match the block type values in BPv7 section 9.1. The v6 types
 *       are not included.
 */
enum BPLib_BlockType
{
    BPLib_BlockType_Reserved = 0,
    BPLib_BlockType_Payload = 1,
    BPLib_BlockType_PrevNode = 6,
    BPLib_BlockType_Age = 7,
    BPLib_BlockType_HopCount = 10,
    BPLib_BlockType_CTEB = 13,
    BPLib_BlockType_CREB = 16,
    BPLib_BlockType_UNKNOWN = 255
};

typedef uint8_t BPLib_BlockType_t;

/**
 * \brief BPLib status type for type safety
 */
typedef int32_t BPLib_Status_t;

// Integer typedefs
typedef uint64_t BPLib_Val_t;

typedef struct BPLib_Instance BPLib_Instance_t;

typedef struct BPLib_BundleCache BPLib_BundleCache_t;

typedef struct BPLib_MEM_Block BPLib_MEM_Block_t;

typedef struct BPLib_ARP_AdminRecord BPLib_ARP_AdminRecord_t;

typedef enum
{
    BPLIB_BUNDLE_PROC_FRAG_FLAG         = 0x0000001, /** \brief Bundle is a fragment flag */
    BPLIB_BUNDLE_PROC_ADMIN_RECORD_FLAG = 0x0000002, /** \brief ADU is an administrative record flag */
    BPLIB_BUNDLE_PROC_NO_FRAG_FLAG      = 0x0000004, /** \brief Bundle must not be fragmented flag */
    /* RESERVED                         = 0x0000008, */
    /* RESERVED                         = 0x0000010, */
    BPLIB_BUNDLE_PROC_ACK_FLAG          = 0x0000020, /** \brief Acknowledgement by application is requested flag */
    BPLIB_BUNDLE_PROC_STATUS_TIME_FLAG  = 0x0000040, /** \brief Status time requested in reports flag */
    /* RESERVED                         = 0x0000080, */
    /* RESERVED                         = 0x0000100, */
    /* RESERVED                         = 0x0000200, */
    /* RESERVED                         = 0x0000400, */
    /* RESERVED                         = 0x0000800, */
    /* RESERVED                         = 0x0001000, */
    /* RESERVED                         = 0x0002000, */
    BPLIB_BUNDLE_PROC_RECV_REPORT_FLAG  = 0x0004000, /** \brief Request reporting of bundle reception flag */
    /* RESERVED                         = 0x0008000, */
    BPLIB_BUNDLE_PROC_FORWARD_FLAG      = 0x0010000, /** \brief Request reporting of bundle forwarding flag */
    BPLIB_BUNDLE_PROC_DELIVERY_FLAG     = 0x0020000, /** \brief Request reporting of bundle delivery flag */
    BPLIB_BUNDLE_PROC_DELETE_FLAG       = 0x0040000, /** \brief Request reporting of bundle deletion flag */
    /* RESERVED                         = 0x0080000, */
    /* RESERVED                         = 0x0100000, */
    /* UNASSIGNED                       = 0x0200000 to 0x8000000000000000 */
} BPLib_BundleProcFlagBits_t;

/*
** Macros
*/

#define BPLIB_BUNDLE_PROTOCOL_VERSION                   (7u)     /** @brief Version of Bundle Protocol being implemented */

#define BPLIB_NO_RETRANSMIT_TRIGGER                     (0u)     /** @brief Garbage retransmit time trigger */

#define BPLIB_BITS_IN_BYTE                              (8u)     /** \brief Number of bits in a byte */

/**
 * @defgroup Block Processing Control Flags
 * @{
 */
#define BPLIB_BLOCK_PROC_REPLCT_FRAG_FLAG   0x01        /** @brief Block must be replicated in every fragment */
#define BPLIB_BLOCK_PROC_STATUS_REPORT_FLAG 0x02        /** @brief Transmit status report if block can't be processed */
#define BPLIB_BLOCK_PROC_DELETE_BUNDLE_FLAG 0x04        /** @brief Delete bundle if block can't be processed */
#define BPLIB_BLOCK_PROC_DISCARD_BLOCK_FLAG 0x10        /** @brief Discard block if it can't be processed */

/* Currently only these block flags are supported */
#define BPLIB_VALID_BLOCK_PROC_FLAG_MASK    (BPLIB_BLOCK_PROC_DELETE_BUNDLE_FLAG | \
                                             BPLIB_BLOCK_PROC_DISCARD_BLOCK_FLAG)
/** @} */

/*
 * \brief Job egress ID to use before a bundle's route is known
*/
#define BPLIB_UNKNOWN_ROUTE_ID                          (BPLIB_MAX_NUM_CHANNELS + BPLIB_MAX_NUM_CONTACTS)

/**
 * @defgroup BPLib_ReturnCodes BPLib Return Codes
 * @{
 */
/* General Return Codes */
#define BPLIB_TBL_UPDATED                              ((BPLib_Status_t)  1)  /* Configuration has been updated */
#define BPLIB_SUCCESS                                  ((BPLib_Status_t)  0)  /* Successful execution */
#define BPLIB_ERROR                                    ((BPLib_Status_t) -1)  /* Failed execution */
#define BPLIB_UNIMPLEMENTED                            ((BPLib_Status_t) -2)  /* Unimplemented function */
#define BPLIB_UNKNOWN                                  ((BPLib_Status_t) -3)  /* Unknown return status */
#define BPLIB_TABLE_OUT_OF_RANGE_ERR_CODE              ((BPLib_Status_t) -4)  /* Configuration validation error code */
#define BPLIB_RBT_DUPLICATE                            ((BPLib_Status_t) -5)  /* BPLib Red-Black Tree (RBT) Duplicate Search Result */
#define BPLIB_TIMEOUT                                  ((BPLib_Status_t) -6)  /* Timeout pending on a queue */
#define BPLIB_NULL_PTR_ERROR                           ((BPLib_Status_t) -7)  /* Null pointer error */
#define BPLIB_BUF_LEN_ERROR                            ((BPLib_Status_t) -8)  /* Buffer length error */
#define BPLIB_INVALID_EID                              ((BPLib_Status_t) -9)  /* Invalid endpoint identification */
#define BPLIB_INVALID_EID_PATTERN                      ((BPLib_Status_t) -10) /* Invalid endpoint identification pattern */
#define BPLIB_INVALID_CRC_ERROR                        ((BPLib_Status_t) -11) /* Invalid CRC */
#define BPLIB_OS_ERROR                                 ((BPLib_Status_t) -12)
#define BPLIB_INVALID_CHAN_ID_ERR                      ((BPLib_Status_t) -13) /* Invalid Channel ID */
#define BPLIB_INVALID_CONT_ID_ERR                      ((BPLib_Status_t) -14) /* Invalid Contact ID */
#define BPLIB_INVALID_CONFIG_ERR                       ((BPLib_Status_t) -15) /* Invalid configuration */
#define BPLIB_APP_STATE_ERR                            ((BPLib_Status_t) -16) /* Invalid application state */
#define BPLIB_NOT_FOUND_ERR                            ((BPLib_Status_t) -17) /* Search item not found */
#define BPLIB_NO_STOR_ERR                              ((BPLib_Status_t) -18) /* No storage remaining */
/*
#define BPLIB_GENERIC_ERROR_19                         ((BPLib_Status_t) -19) // Error description
#define BPLIB_GENERIC_ERROR_20                         ((BPLib_Status_t) -20) // Error description
#define BPLIB_GENERIC_ERROR_21                         ((BPLib_Status_t) -21) // Error description
#define BPLIB_GENERIC_ERROR_22                         ((BPLib_Status_t) -22) // Error description
#define BPLIB_GENERIC_ERROR_23                         ((BPLib_Status_t) -23) // Error description
#define BPLIB_GENERIC_ERROR_24                         ((BPLib_Status_t) -24) // Error description
#define BPLIB_GENERIC_ERROR_25                         ((BPLib_Status_t) -25) // Error description
*/

/* Framework Proxy Errors */
#define BPLIB_FWP_CALLBACK_INIT_ERROR                  ((BPLib_Status_t) -26)

/* Time Management Errors */
#define BPLIB_TIME_UNDEF_DELTA_ERROR                   ((BPLib_Status_t) -27)
#define BPLIB_TIME_WRITE_ERROR                         ((BPLib_Status_t) -28)
#define BPLIB_TIME_READ_ERROR                          ((BPLib_Status_t) -29)
#define BPLIB_TIME_UNINIT_ERROR                        ((BPLib_Status_t) -30)

/* Event Management Errors */
#define BPLIB_EM_STRING_TRUNCATED                      ((BPLib_Status_t) -31)
#define BPLIB_EM_ILLEGAL_APP_ID                        ((BPLib_Status_t) -32)
#define BPLIB_EM_UNKNOWN_FILTER                        ((BPLib_Status_t) -33)
#define BPLIB_EM_BAD_ARGUMENT                          ((BPLib_Status_t) -34)
#define BPLIB_EM_INVALID_PARAMETER                     ((BPLib_Status_t) -35)
#define BPLIB_EM_APP_NOT_REGISTERED                    ((BPLib_Status_t) -36)
#define BPLIB_EM_APP_SQUELCHED                         ((BPLib_Status_t) -37)
#define BPLIB_EM_EXPANDED_TEXT_ERROR                   ((BPLib_Status_t) -38)

/* PerfLog Proxy Errors*/
#define BPLIB_PL_NULL_CALLBACK_ERROR                   ((BPLib_Status_t) -39)

/* Node Configuration (NC) errors */
#define BPLIB_NC_INIT_CONFIG_PTRS_ERROR                ((BPLib_Status_t) -40)
#define BPLIB_NC_INVALID_MIB_ITEM_INDEX                ((BPLib_Status_t) -41)
#define BPLIB_NC_INVALID_MIB_VALUE                     ((BPLib_Status_t) -42)

/* CLA Errors*/
#define BPLIB_CLA_TIMEOUT                              ((BPLib_Status_t) -43)

/* Payload Interface Errors */
#define BPLIB_INV_REG_STATE                            ((BPLib_Status_t) -44)
#define BPLIB_PI_TIMEOUT                               ((BPLib_Status_t) -45)

/* Admin Statistics (AS) Errors */
#define BPLIB_AS_INIT_MUTEX_ERR                        ((BPLib_Status_t) -46)
#define BPLIB_AS_INVALID_EID                           ((BPLib_Status_t) -47)
#define BPLIB_AS_UNKNOWN_NODE_CNTR                     ((BPLib_Status_t) -48)
#define BPLIB_AS_UNKNOWN_SRC_CNTR                      ((BPLib_Status_t) -49)
#define BPLIB_AS_UNKNOWN_MIB_ARRAY_EID                 ((BPLib_Status_t) -50)
#define BPLIB_AS_MIB_KEY_ARRAY_FULL                    ((BPLib_Status_t) -51)
#define BPLIB_AS_MIB_KEYS_OVERLAP                      ((BPLib_Status_t) -52)
#define BPLIB_AS_INVALID_MIB_INDEX                     ((BPLib_Status_t) -53)
#define BPLIB_AS_NO_KEYS_GIVEN                         ((BPLib_Status_t) -54)

/* Queue Manager (QM) Errors */
#define BPLIB_QM_PUSH_ERROR                            ((BPLib_Status_t) -55)

/* MEM Errors */
#define BPLIB_MEM_INITMEM_UNALIGN                      ((BPLib_Status_t) -56)
#define BPLIB_MEM_CPY_FRM_OFFSET_NE_ERR                ((BPLib_Status_t) -57) /* BPLib_MEM_CopyOutFromOffset: bytes copied != requested */

/* Node Config Errors */
#define BPLIB_NC_TBL_UPDATE_ERR                        ((BPLib_Status_t) -80)

/* CBOR Decode Errors */
#define BPLIB_CBOR_DEC_BUNDLE_TOO_SHORT_ERR            ((BPLib_Status_t) -120) /* CBOR decode error: bundle too short */
#define BPLIB_CBOR_DEC_BUNDLE_ENTER_ARRAY_ERR          ((BPLib_Status_t) -121) /* CBOR decode error: entry array */
#define BPLIB_CBOR_DEC_BUNDLE_MAX_BLOCKS_ERR           ((BPLib_Status_t) -122) /* CBOR decode error: max blocks */
#define BPLIB_CBOR_DEC_BUNDLE_EXIT_ARRAY_ERR           ((BPLib_Status_t) -123) /* CBOR decode error: entry array */

#define BPLIB_CBOR_DEC_PRIM_ENTER_ARRAY_ERR            ((BPLib_Status_t) -124) /* CBOR primary block decode error: entry array */
#define BPLIB_CBOR_DEC_PRIM_EXIT_ARRAY_ERR             ((BPLib_Status_t) -125) /* CBOR primary block decode error: exit array */
#define BPLIB_CBOR_DEC_PRIM_VERSION_DEC_ERR            ((BPLib_Status_t) -126) /* CBOR primary block decode error: decode version field */
#define BPLIB_CBOR_DEC_PRIM_WRONG_VERSION_ERR          ((BPLib_Status_t) -127) /* CBOR primary block decode error: wrong version field */
#define BPLIB_CBOR_DEC_PRIM_FLAG_DEC_ERR               ((BPLib_Status_t) -128) /* CBOR primary block decode error: decode flag field */
#define BPLIB_CBOR_DEC_PRIM_CRC_TYPE_DEC_ERR           ((BPLib_Status_t) -129) /* CBOR primary block decode error: decode crc type field */
#define BPLIB_CBOR_DEC_PRIM_DEST_EID_DEC_ERR           ((BPLib_Status_t) -130) /* CBOR primary block decode error: decode Dest EID field */
#define BPLIB_CBOR_DEC_PRIM_SRC_EID_DEC_ERR            ((BPLib_Status_t) -131) /* CBOR primary block decode error: decode Src EID field */
#define BPLIB_CBOR_DEC_PRIM_REPORT_EID_DEC_ERR         ((BPLib_Status_t) -132) /* CBOR primary block decode error: decode Report-To EID field */
#define BPLIB_CBOR_DEC_PRIM_CREATE_TIME_DEC_ERR        ((BPLib_Status_t) -133) /* CBOR primary block decode error: decode create time field */
#define BPLIB_CBOR_DEC_PRIM_LIFETIME_DEC_ERR           ((BPLib_Status_t) -134) /* CBOR primary block decode error: decode lifetime field */
#define BPLIB_CBOR_DEC_PRIM_CRC_VAL_DEC_ERR            ((BPLib_Status_t) -135) /* CBOR primary block decode error: decode crc value field */
#define BPLIB_CBOR_DEC_PRIM_WRONG_FLAG_ERR             ((BPLib_Status_t) -136) /* CBOR primary block decode error: wrong flag field */

#define BPLIB_CBOR_DEC_CANON_BLOCK_INDEX_ERR           ((BPLib_Status_t) -137) /* CBOR canon block decode error: invalid block index */
#define BPLIB_CBOR_DEC_CANON_ENTER_ARRAY_ERR           ((BPLib_Status_t) -138) /* CBOR canon block decode error: entry array */
#define BPLIB_CBOR_DEC_CANON_EXIT_ARRAY_ERR            ((BPLib_Status_t) -139) /* CBOR canon block decode error: exit array */
#define BPLIB_CBOR_DEC_CANON_BLOCK_TYPE_DEC_ERR        ((BPLib_Status_t) -140) /* CBOR canon block decode error: decode block type */
#define BPLIB_CBOR_DEC_CANON_BLOCK_NUM_DEC_ERR         ((BPLib_Status_t) -141) /* CBOR canon block decode error: decode block num */
#define BPLIB_CBOR_DEC_CANON_BLOCK_FLAG_DEC_ERR        ((BPLib_Status_t) -142) /* CBOR canon block decode error: decode block proc flags */
#define BPLIB_CBOR_DEC_CANON_CRC_TYPE_DEC_ERR          ((BPLib_Status_t) -143) /* CBOR canon block decode error: decode crc type */
#define BPLIB_CBOR_DEC_CANON_ENTER_BYTE_STR_ERR        ((BPLib_Status_t) -144) /* CBOR canon block decode error: enter data byte string */
#define BPLIB_CBOR_DEC_CANON_EXIT_BYTE_STR_ERR         ((BPLib_Status_t) -145) /* CBOR canon block decode error: exit data byte string */
#define BPLIB_CBOR_DEC_CANON_CRC_VAL_DEC_ERR           ((BPLib_Status_t) -146) /* CBOR canon block decode error: decode crc value field */
#define BPLIB_CBOR_DEC_CANON_ADMIN_REC_ENTER_ARR_ERR   ((BPLib_Status_t) -147) /* CBOR canon block decode error: admin record enter array */
#define BPLIB_CBOR_DEC_CANON_ADMIN_REC_REC_TYPE_ERR    ((BPLib_Status_t) -148) /* CBOR canon block decode error: admin record type decode */
#define BPLIB_CBOR_DEC_CANON_ADMIN_REC_CONT_ERR        ((BPLib_Status_t) -149) /* CBOR canon block decode error: admin record exit array */
#define BPLIB_CBOR_DEC_CANON_ADMIN_REC_EXIT_ARR_ERR    ((BPLib_Status_t) -150) /* CBOR canon block decode error: admin record exit array */

#define BPLIB_CBOR_DEC_PREV_NODE_EID_DEC_ERR           ((BPLib_Status_t) -151) /* CBOR Prev Node block decode error: decode eid field */

#define BPLIB_CBOR_DEC_AGE_BLOCK_DEC_ERR               ((BPLib_Status_t) -152) /* CBOR Age block decode error: decode eid field */

#define BPLIB_CBOR_DEC_HOP_BLOCK_ENTER_ARRAY_ERR       ((BPLib_Status_t) -153) /* CBOR Hop Count block decode error: enter array */
#define BPLIB_CBOR_DEC_HOP_BLOCK_EXIT_ARRAY_ERR        ((BPLib_Status_t) -154) /* CBOR Hop Count block decode error: exit array */
#define BPLIB_CBOR_DEC_HOP_BLOCK_HOP_LIMIT_DEC_ERR     ((BPLib_Status_t) -155) /* CBOR Hop Count block decode error: hop limit decode */
#define BPLIB_CBOR_DEC_HOP_BLOCK_HOP_COUNT_DEC_ERR     ((BPLib_Status_t) -156) /* CBOR Hop Count block decode error: hop count decode */
#define BPLIB_CBOR_DEC_HOP_BLOCK_INVALID_DEC_ERR       ((BPLib_Status_t) -157) /* CBOR Hop Count block decode error: invalid block data values */
#define BPLIB_CBOR_DEC_HOP_BLOCK_EXCEEDED_ERR          ((BPLib_Status_t) -158) /* CBOR Hop Count block decode error: hop limit exceeded */

#define BPLIB_CBOR_DEC_CUSTODY_BLOCK_ENTER_ARRAY_ERR   ((BPLib_Status_t) -159) /* CBOR Custody Transfer block decode error: enter array */
#define BPLIB_CBOR_DEC_CUSTODY_BLOCK_EXIT_ARRAY_ERR    ((BPLib_Status_t) -160) /* CBOR Custody Transfer block decode error: exit array */
#define BPLIB_CBOR_DEC_CUSTODY_BLOCK_SEQ_NUM_DEC_ERR   ((BPLib_Status_t) -161) /* CBOR Custody Transfer block decode error: bundle sequence number decode */
#define BPLIB_CBOR_DEC_CUSTODY_BLOCK_SEQ_ID_DEC_ERR    ((BPLib_Status_t) -162) /* CBOR Custody Transfer block decode error: bundle sequence ID decode */
#define BPLIB_CBOR_DEC_CUSTODY_BLOCK_SRC_EID_DEC_ERR   ((BPLib_Status_t) -163) /* CBOR Custody Transfer block decode error: block source administrative endpoint ID decode */

#define BPLIB_CBOR_DEC_UNKNOWN_BLOCK_DEC_ERR           ((BPLib_Status_t) -164) /* CBOR canon block decode error: can't process block */
#define BPLIB_CBOR_DEC_BUNDLE_TOO_LONG_DEC_ERR         ((BPLib_Status_t) -165) /* CBOR decode error: bundle is too long */
#define BPLIB_CBOR_DEC_EXTRA_DATA_DEC_ERR              ((BPLib_Status_t) -166) /* CBOR decode error: extra data after payload  */
#define BPLIB_CBOR_DEC_NO_PAYLOAD_ERR                  ((BPLib_Status_t) -167) /* CBOR decode error: no payload detected in bundle */

#define BPLIB_CBOR_DEC_TYPES_ENTER_DEF_ARRAY_QCBOR_ERR ((BPLib_Status_t) -168) /* CBOR decode types error: enter def array */
#define BPLIB_CBOR_DEC_TYPES_ENTER_DEF_ARRAY_COUNT_ERR ((BPLib_Status_t) -169) /* CBOR decode types error: def array size */
#define BPLIB_CBOR_DEC_TYPES_EXIT_DEF_ARRAY_QCBOR_ERR  ((BPLib_Status_t) -170) /* CBOR decode types error: exit array */
#define BPLIB_CBOR_DEC_TYPES_GET_UINT64_QCBOR_ERR      ((BPLib_Status_t) -171) /* CBOR decode types error: get uint64 */
#define BPLIB_CBOR_DEC_TYPES_EID_ENTER_OUTER_ARRAY_ERR ((BPLib_Status_t) -172) /* CBOR decode types error: EID enter outer array */
#define BPLIB_CBOR_DEC_TYPES_EID_SCHEME_NOT_IMPL_ERR   ((BPLib_Status_t) -173) /* CBOR decode types error: EID scheme not implemented */
#define BPLIB_CBOR_DEC_TYPES_EID_ENTER_SSP_ARRAY_ERR   ((BPLib_Status_t) -174) /* CBOR decode types error: EID enter SSP array */
#define BPLIB_CBOR_DEC_TYPES_EID_IPN_NODE_DEC_ERR      ((BPLib_Status_t) -175) /* CBOR decode types error: EID IPN node num decode */
#define BPLIB_CBOR_DEC_TYPES_EID_IPN_SERV_DEC_ERR      ((BPLib_Status_t) -176) /* CBOR decode types error: EID IPN serv num decode */
#define BPLIB_CBOR_DEC_TYPES_EID_EXIT_SSP_ARRAY_ERR    ((BPLib_Status_t) -177) /* CBOR decode types error: EID exit SSP array */
#define BPLIB_CBOR_DEC_TYPES_EID_EXIT_OUTER_ARRAY_ERR  ((BPLib_Status_t) -178) /* CBOR decode types error: EID exit outer array */
#define BPLIB_CBOR_DEC_TYPES_ENTER_MAP_ERR             ((BPLib_Status_t) -179) /* CBOR decode types error: enter map */
#define BPLIB_CBOR_DEC_TYPES_ENTER_MAP_COUNT_ERR       ((BPLib_Status_t) -180) /* CBOR decode types error: map size */
#define BPLIB_CBOR_DEC_TYPES_EXIT_MAP_ERR              ((BPLib_Status_t) -181) /* CBOR decode types error: exit map */

#define BPLIB_CBOR_DEC_TYPES_TIMESTAMP_ENTER_ARRAY_ERR ((BPLib_Status_t) -182) /* CBOR decode types error: timestamp enter array */
#define BPLIB_CBOR_DEC_TYPES_TIMESTAMP_EXIT_ARRAY_ERR  ((BPLib_Status_t) -183) /* CBOR decode types error: timestamp exit array */
#define BPLIB_CBOR_DEC_TYPES_TIMESTAMP_CREATE_DEC_ERR  ((BPLib_Status_t) -184) /* CBOR decode types error: timestamp create time decode */
#define BPLIB_CBOR_DEC_TYPES_TIMESTAMP_SEQ_NUM_DEC_ERR ((BPLib_Status_t) -185) /* CBOR decode types error: timestamp seq num decode */

#define BPLIB_CBOR_DEC_TYPES_CRC_ENTER_BYTE_STR_ERR    ((BPLib_Status_t) -186) /* CBOR decode types error: CRC Val enter byte-string */
#define BPLIB_CBOR_DEC_TYPES_CRC_16_LEN_ERR            ((BPLib_Status_t) -187) /* CBOR decode types error: CRC Val length not 16 */
#define BPLIB_CBOR_DEC_TYPES_CRC_32_LEN_ERR            ((BPLib_Status_t) -188) /* CBOR decode types error: CRC Val length not 32 */
#define BPLIB_CBOR_DEC_TYPES_CRC_UNSUPPORTED_TYPE_ERR  ((BPLib_Status_t) -189) /* CBOR decode types error: CRC Val type */
#define BPLIB_CBOR_DEC_TYPES_EID_DTN_ERR               ((BPLib_Status_t) -190) /* CBOR decode types error: DTN EID decode failed */

#define BPLIB_CBOR_DEC_TYPES_BNDL_SEQ_ENTER_MAP_ERR    ((BPLib_Status_t) -191) /* CBOR decode types error: bundle sequence collection enter map */
#define BPLIB_CBOR_DEC_TYPES_BNDL_SEQ_MAP_SIZE_ERR     ((BPLib_Status_t) -192) /* CBOR decode types error: bundle sequence collection map size */
#define BPLIB_CBOR_DEC_TYPES_BNDL_SEQ_ID_ERR           ((BPLib_Status_t) -193) /* CBOR decode types error: bundle sequence collection sequence ID decode */
#define BPLIB_CBOR_DEC_TYPES_BNDL_SEQ_NUM_ERR          ((BPLib_Status_t) -194) /* CBOR decode types error: bundle sequence collection first sequence number decode */
#define BPLIB_CBOR_DEC_TYPES_BNDL_SEQ_RNGE_ENTER_ERR   ((BPLib_Status_t) -195) /* CBOR decode types error: bundle sequence collection enter sequence range array */
#define BPLIB_CBOR_DEC_TYPES_BNDL_SEQ_RNGE_SIZE_ERR    ((BPLib_Status_t) -196) /* CBOR decode types error: bundle sequence collection sequence range size */
#define BPLIB_CBOR_DEC_TYPES_BNDL_SEQ_RNGE_ERR         ((BPLib_Status_t) -197) /* CBOR decode types error: bundle sequence collection sequence range decode */
#define BPLIB_CBOR_DEC_TYPES_BNDL_SEQ_RNGE_EXIT_ERR    ((BPLib_Status_t) -198) /* CBOR decode types error: bundle sequence collection exit sequence range array */
#define BPLIB_CBOR_DEC_TYPES_BNDL_SEQ_EXIT_MAP_ERR     ((BPLib_Status_t) -199) /* CBOR decode types error: bundle sequence collection exit map */
#define BPLIB_CBOR_DEC_TYPES_BNDL_SEQ_MAP_ENTR_ARR_ERR ((BPLib_Status_t) -200) /* CBOR decode types error: bundle sequence collection enter entries array error */
#define BPLIB_CBOR_DEC_TYPES_BNDL_SEQ_MAP_EXIT_ARR_ERR ((BPLib_Status_t) -201) /* CBOR decode types error: bundle sequence collection exit entries array error */

#define BPLIB_CBOR_DEC_TYPES_ENTER_INDEF_ARRAY_ERR     ((BPLib_Status_t) -202) /* CBOR decode types error: enter indef array error */

#define BPLIB_CBOR_DEC_TYPES_ADMIN_REC_INV_REC_TYPE    ((BPLib_Status_t) -203) /* CBOR decode types error: admin record invalid record type */
#define BPLIB_CBOR_DEC_ADMIN_RECORD_NULL_ERR           ((BPLib_Status_t) -204) /* CBOR decode error: could not allocate an admin record block */
#define BPLIB_CBOR_DEC_INV_DISP_CODE_ERR               ((BPLib_Status_t) -205) /* CBOR decode error: invalid disposition code */
/*
#define BPLIB_CBOR_DEC_GENERIC_ERR_206                 ((BPLib_Status_t) -206) // CBOR decode error
#define BPLIB_CBOR_DEC_GENERIC_ERR_207                 ((BPLib_Status_t) -207) // CBOR decode error
#define BPLIB_CBOR_DEC_GENERIC_ERR_208                 ((BPLib_Status_t) -208) // CBOR decode error
#define BPLIB_CBOR_DEC_GENERIC_ERR_209                 ((BPLib_Status_t) -209) // CBOR decode error
#define BPLIB_CBOR_DEC_GENERIC_ERR_210                 ((BPLib_Status_t) -210) // CBOR decode error
#define BPLIB_CBOR_DEC_GENERIC_ERR_211                 ((BPLib_Status_t) -211) // CBOR decode error
*/

/* CBOR Encode Errors */
#define BPLIB_CBOR_ENC_QCBOR_FINISH_TAIL_ERR           ((BPLib_Status_t) -212) /* QCBOREncode_Finish Error */
#define BPLIB_CBOR_ENC_ARRAY_ERR                       ((BPLib_Status_t) -213) /* Error while attempting to encode array */
#define BPLIB_CBOR_ENC_MAP_ERR                         ((BPLib_Status_t) -214) /* Error while attempting to encode map */
#define BPLIB_CBOR_ENC_UINT_ERR                        ((BPLib_Status_t) -215) /* Error while attempting to encode unsigned integer */
#define BPLIB_CBOR_ENC_GET_BUFF_SIZE_ERR               ((BPLib_Status_t) -216) /* Error while attempting to get the size of the encoded data thusfar */
#define BPLIB_CBOR_ENC_ADU_ERR                         ((BPLib_Status_t) -217) /* Error while attempting to encode ADU */
#define BPLIB_CBOR_ENC_CRC_ERR                         ((BPLib_Status_t) -218) /* Error while attempting to encode CRC */
#define BPLIB_CBOR_ENC_ADMIN_RECORD_ERR                ((BPLib_Status_t) -219) /* Error while attempting to encode administrative record */
#define BPLIB_CBOR_ENC_CCS_ERR                         ((BPLib_Status_t) -220) /* Error while attempting to encode CCS */
#define BPLIB_CBOR_ENC_EXT_SIZES_CRRPTD_ERR            ((BPLib_Status_t) -221) /* BPLib_CBOR_EncodeExtensionBlock: Block Sizes Corrupted Error */
#define BPLIB_CBOR_ENC_EXT_INPUT_BLOCK_INDEX_ERR       ((BPLib_Status_t) -222) /* BPLib_CBOR_EncodeExtensionBlock: Ext Block Index Error */
#define BPLIB_CBOR_ENC_EXT_QCBOR_FINISH_ERR            ((BPLib_Status_t) -223) /* BPLib_CBOR_EncodeExtensionBlock: QCBOREncode_Finish Error */
#define BPLIB_CBOR_ENC_PRIM_SIZES_CRRPTD_ERR           ((BPLib_Status_t) -224) /* BPLib_CBOR_CopyOrEncodePrimary: Block Sizes Corrupted Error */
#define BPLIB_CBOR_ENC_PRIM_COPY_SIZE_GT_OUTPUT_ERR    ((BPLib_Status_t) -225) /* BPLib_CBOR_CopyOrEncodePrimary: Copy Size Error */
#define BPLIB_CBOR_ENC_PRIM_QCBOR_FINISH_ERR           ((BPLib_Status_t) -226) /* BPLib_CBOR_EncodePrimary: QCBOREncode_Finish Error */
#define BPLIB_CBOR_ENC_PAYL_COPY_SIZE_GT_OUTPUT_ERR    ((BPLib_Status_t) -227) /* BPLib_CBOR_CopyOrEncodePayload: Copy Size Error */
#define BPLIB_CBOR_ENC_PAYL_SIZES_CRRPTD_ERR           ((BPLib_Status_t) -228) /* BPLib_CBOR_CopyOrEncodePayload: Block Sizes Corrupted Error */
#define BPLIB_CBOR_ENC_PAYL_HEADER_ERR                 ((BPLib_Status_t) -229) /* BPLib_CBOR_EncodePayload: Error while encoding payload header */
#define BPLIB_CBOR_ENC_PAYL_ERR                        ((BPLib_Status_t) -230) /* BPLib_CBOR_EncodePayload: Error while encoding bundle's actual payload */
#define BPLIB_CBOR_ENC_PAYL_CRC_ERR                    ((BPLib_Status_t) -231) /* BPLib_CBOR_EncodePayload: Error while encoding payload's CRC value */

#define BPLIB_CBOR_ENC_BUNDLE_OUTPUT_BUF_LEN_1_ERR     ((BPLib_Status_t) -232) /* BPLib_CBOR_EncodeBundle: Output buf too small (check 1) */
#define BPLIB_CBOR_ENC_BUNDLE_OUTPUT_BUF_LEN_2_ERR     ((BPLib_Status_t) -233) /* BPLib_CBOR_EncodeBundle: Output buf too small (check 2) */
#define BPLIB_CBOR_ENC_BUNDLE_OUTPUT_BUF_LEN_3_ERR     ((BPLib_Status_t) -234) /* BPLib_CBOR_EncodeBundle: Output buf too small (check 3) */
#define BPLIB_CBOR_ENC_BUNDLE_OUTPUT_BUF_LEN_4_ERR     ((BPLib_Status_t) -235) /* BPLib_CBOR_EncodeBundle: Output buf too small (check 4) */
#define BPLIB_CBOR_ENC_PAYL_QCBOR_FINISH_HEAD_ERR      ((BPLib_Status_t) -236) /* BPLib_CBOR_EncodePayload: Error encoding payload header */
#define BPLIB_CBOR_ENC_PAYL_ADD_BYTE_STR_HEAD_ERR      ((BPLib_Status_t) -237) /* BPLib_CBOR_EncodePayload: Error adding payload bytestring head */
#define BPLIB_CBOR_ENC_PAYL_QCBOR_FINISH_TAIL_ERR      ((BPLib_Status_t) -238) /* BPLib_CBOR_EncodePayload: Error ending payload block */
#define BPLIB_CBOR_ENC_CORRUPT_CCS_ERR                 ((BPLib_Status_t) -239) /* BPLib_CBOR_EncodePayload: Error encoding a corrupt CCS payload */

/*
#define BPLIB_CBOR_ENC_GENERIC_ERR_240                 ((BPLib_Status_t) -240) // CBOR encode error
#define BPLIB_CBOR_ENC_GENERIC_ERR_241                 ((BPLib_Status_t) -241) // CBOR encode error
#define BPLIB_CBOR_ENC_GENERIC_ERR_242                 ((BPLib_Status_t) -242) // CBOR encode error
#define BPLIB_CBOR_ENC_GENERIC_ERR_243                 ((BPLib_Status_t) -243) // CBOR encode error
#define BPLIB_CBOR_ENC_GENERIC_ERR_244                 ((BPLib_Status_t) -244) // CBOR encode error
#define BPLIB_CBOR_ENC_GENERIC_ERR_245                 ((BPLib_Status_t) -245) // CBOR encode error
#define BPLIB_CBOR_ENC_GENERIC_ERR_246                 ((BPLib_Status_t) -246) // CBOR encode error
#define BPLIB_CBOR_ENC_GENERIC_ERR_247                 ((BPLib_Status_t) -247) // CBOR encode error
#define BPLIB_CBOR_ENC_GENERIC_ERR_248                 ((BPLib_Status_t) -248) // CBOR encode error
#define BPLIB_CBOR_ENC_GENERIC_ERR_249                 ((BPLib_Status_t) -249) // CBOR encode error
*/

/* CLA errors */
#define BPLIB_CLA_INCORRECT_STATE                      ((BPLib_Status_t) -250) /* Contact is in the incorrect state to be modified */
#define BPLIB_CLA_IO_ERROR                             ((BPLib_Status_t) -251) /* PSP IO driver API call failed */
/*
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -252) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -253) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -254) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -255) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -256) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -257) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -258) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -259) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -260) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -261) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -262) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -263) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -264) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -265) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -266) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -267) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -268) // CLA error
#define BPLIB_CLA_GENERIC_ERR                          ((BPLib_Status_t) -269) // CLA error
*/

/* QM Errors */
#define BPLIB_QM_INIT_ERROR                            ((BPLib_Status_t) -270)
#define BPLIB_QM_BAD_WRKR_ID                           ((BPLib_Status_t) -271)
#define BPLIB_QM_WRKR_REGISTER_ERROR                   ((BPLib_Status_t) -272)
/*
#define BPLIB_QM_GENERIC_ERR_273                       ((BPLib_Status_t) -273) // QM error
#define BPLIB_QM_GENERIC_ERR_274                       ((BPLib_Status_t) -274) // QM error
#define BPLIB_QM_GENERIC_ERR_275                       ((BPLib_Status_t) -275) // QM error
#define BPLIB_QM_GENERIC_ERR_276                       ((BPLib_Status_t) -276) // QM error
#define BPLIB_QM_GENERIC_ERR_277                       ((BPLib_Status_t) -277) // QM error
#define BPLIB_QM_GENERIC_ERR_278                       ((BPLib_Status_t) -278) // QM error
#define BPLIB_QM_GENERIC_ERR_279                       ((BPLib_Status_t) -279) // QM error
#define BPLIB_QM_GENERIC_ERR_280                       ((BPLib_Status_t) -280) // QM error
#define BPLIB_QM_GENERIC_ERR_281                       ((BPLib_Status_t) -281) // QM error
#define BPLIB_QM_GENERIC_ERR_282                       ((BPLib_Status_t) -282) // QM error
#define BPLIB_QM_GENERIC_ERR_283                       ((BPLib_Status_t) -283) // QM error
#define BPLIB_QM_GENERIC_ERR_284                       ((BPLib_Status_t) -284) // QM error
#define BPLIB_QM_GENERIC_ERR_285                       ((BPLib_Status_t) -285) // QM error
#define BPLIB_QM_GENERIC_ERR_286                       ((BPLib_Status_t) -286) // QM error
#define BPLIB_QM_GENERIC_ERR_287                       ((BPLib_Status_t) -287) // QM error
#define BPLIB_QM_GENERIC_ERR_288                       ((BPLib_Status_t) -288) // QM error
#define BPLIB_QM_GENERIC_ERR_289                       ((BPLib_Status_t) -289) // QM error
*/

/* Storage Errors: SQL */
#define BPLIB_STOR_SQL_INIT_ERR                        ((BPLib_Status_t) -290)
#define BPLIB_STOR_SQL_STORAGE_ERR                     ((BPLib_Status_t) -291)
#define BPLIB_STOR_SQL_LOAD_ERR                        ((BPLib_Status_t) -292)
#define BPLIB_STOR_SQL_DISCARD_ERR                     ((BPLib_Status_t) -293)
#define BPLIB_STOR_PARAM_ERR                           ((BPLib_Status_t) -294)
#define BPLIB_STOR_BATCH_EMPTY                         ((BPLib_Status_t) -295)
#define BPLIB_STOR_BATCH_FULL                          ((BPLib_Status_t) -296)
#define BPLIB_STOR_BATCH_CONSUMED                      ((BPLib_Status_t) -297)
#define BPLIB_STOR_DB_FULL_ERR                         ((BPLib_Status_t) -298)
#define BPLIB_STOR_NO_MEM_ERR                          ((BPLib_Status_t) -299)
#define BPLIB_STOR_SQL_OVERFLOW_ERR                    ((BPLib_Status_t) -300)
#define BPLIB_STOR_SQL_LOAD_IDS_ERR                    ((BPLib_Status_t) -301)
#define BPLIB_STOR_SQL_MARK_EGRESSED_ERR               ((BPLib_Status_t) -302)
#define BPLIB_STOR_NO_BUNDLE_FOUND_ERR                 ((BPLib_Status_t) -303)
#define BPLIB_STOR_CLEANUP_ERR                         ((BPLib_Status_t) -304)
#define BPLIB_STOR_SQL_NEW_RETRANSMIT_ERR              ((BPLib_Status_t) -305)
#define BPLIB_SQL_CUSTODY_UPDATE_ERR                   ((BPLib_Status_t) -306)

/*
#define BPLIB_STOR_GENERIC_ERR_307                     ((BPLib_Status_t) -307) // STOR error
#define BPLIB_STOR_GENERIC_ERR_308                     ((BPLib_Status_t) -308) // STOR error
#define BPLIB_STOR_GENERIC_ERR_309                     ((BPLib_Status_t) -309) // STOR error
*/

/* Bundle Interface Errors */
#define BPLIB_BI_INVALID_BUNDLE_ERR                    ((BPLib_Status_t) -310)
#define BPLIB_BI_EXPIRED_BUNDLE_ERR                    ((BPLib_Status_t) -311)
/*
#define BPLIB_BI_GENERIC_ERR_312                       ((BPLib_Status_t) -312) // BI error
#define BPLIB_BI_GENERIC_ERR_313                       ((BPLib_Status_t) -313) // BI error
#define BPLIB_BI_GENERIC_ERR_314                       ((BPLib_Status_t) -314) // BI error
#define BPLIB_BI_GENERIC_ERR_315                       ((BPLib_Status_t) -315) // BI error
#define BPLIB_BI_GENERIC_ERR_316                       ((BPLib_Status_t) -316) // BI error
#define BPLIB_BI_GENERIC_ERR_317                       ((BPLib_Status_t) -317) // BI error
#define BPLIB_BI_GENERIC_ERR_318                       ((BPLib_Status_t) -318) // BI error
#define BPLIB_BI_GENERIC_ERR_319                       ((BPLib_Status_t) -319) // BI error
#define BPLIB_BI_GENERIC_ERR_320                       ((BPLib_Status_t) -320) // BI error
#define BPLIB_BI_GENERIC_ERR_321                       ((BPLib_Status_t) -321) // BI error
#define BPLIB_BI_GENERIC_ERR_322                       ((BPLib_Status_t) -322) // BI error
#define BPLIB_BI_GENERIC_ERR_323                       ((BPLib_Status_t) -323) // BI error
#define BPLIB_BI_GENERIC_ERR_324                       ((BPLib_Status_t) -324) // BI error
#define BPLIB_BI_GENERIC_ERR_325                       ((BPLib_Status_t) -325) // BI error
#define BPLIB_BI_GENERIC_ERR_326                       ((BPLib_Status_t) -326) // BI error
#define BPLIB_BI_GENERIC_ERR_327                       ((BPLib_Status_t) -327) // BI error
#define BPLIB_BI_GENERIC_ERR_328                       ((BPLib_Status_t) -328) // BI error
#define BPLIB_BI_GENERIC_ERR_329                       ((BPLib_Status_t) -329) // BI error
*/

/* Extra Node Configuration errors */
#define BPLIB_NC_FWP_INIT_ERR                          ((BPLib_Status_t) -330)
#define BPLIB_NC_EM_INIT_ERR                           ((BPLib_Status_t) -331)
#define BPLIB_NC_TABLEP_INIT_ERR                       ((BPLib_Status_t) -332)
#define BPLIB_NC_TIME_INIT_ERR                         ((BPLib_Status_t) -333)
#define BPLIB_NC_INIT_ERR                              ((BPLib_Status_t) -334)
#define BPLIB_NC_AS_INIT_ERR                           ((BPLib_Status_t) -335)
#define BPLIB_NC_QM_INIT_ERR                           ((BPLib_Status_t) -336)
#define BPLIB_NC_MEM_INIT_ERR                          ((BPLib_Status_t) -337)
/*
#define BPLIB_NC_GENERIC_ERR_338                       ((BPLib_Status_t) -338) // NC error
#define BPLIB_NC_GENERIC_ERR_339                       ((BPLib_Status_t) -339) // NC error
#define BPLIB_NC_GENERIC_ERR_340                       ((BPLib_Status_t) -340) // NC error
#define BPLIB_NC_GENERIC_ERR_341                       ((BPLib_Status_t) -341) // NC error
#define BPLIB_NC_GENERIC_ERR_342                       ((BPLib_Status_t) -342) // NC error
#define BPLIB_NC_GENERIC_ERR_343                       ((BPLib_Status_t) -343) // NC error
#define BPLIB_NC_GENERIC_ERR_344                       ((BPLib_Status_t) -344) // NC error
#define BPLIB_NC_GENERIC_ERR_345                       ((BPLib_Status_t) -345) // NC error
#define BPLIB_NC_GENERIC_ERR_346                       ((BPLib_Status_t) -346) // NC error
#define BPLIB_NC_GENERIC_ERR_347                       ((BPLib_Status_t) -347) // NC error
#define BPLIB_NC_GENERIC_ERR_348                       ((BPLib_Status_t) -348) // NC error
#define BPLIB_NC_GENERIC_ERR_349                       ((BPLib_Status_t) -349) // NC error
*/

/* Custody Transfer Errors */
#define BPLIB_CT_CUSTODY_REFUSED_ERR                   ((BPLib_Status_t) -350)
#define BPLIB_CT_FULL_DB_ERR                           ((BPLib_Status_t) -351)
#define BPLIB_CT_NO_CUST_ERR                           ((BPLib_Status_t) -352)
#define BPLIB_CT_DUPLICATE_ERR                         ((BPLib_Status_t) -353)

/*
#define BPLIB_CT_GENERIC_ERR_353                       ((BPLib_Status_t) -353) // CT error
#define BPLIB_CT_GENERIC_ERR_354                       ((BPLib_Status_t) -354) // CT error
#define BPLIB_CT_GENERIC_ERR_355                       ((BPLib_Status_t) -355) // CT error
#define BPLIB_CT_GENERIC_ERR_356                       ((BPLib_Status_t) -356) // CT error
#define BPLIB_CT_GENERIC_ERR_357                       ((BPLib_Status_t) -357) // CT error
#define BPLIB_CT_GENERIC_ERR_358                       ((BPLib_Status_t) -358) // CT error
#define BPLIB_CT_GENERIC_ERR_359                       ((BPLib_Status_t) -359) // CT error
#define BPLIB_CT_GENERIC_ERR_360                       ((BPLib_Status_t) -360) // CT error
#define BPLIB_CT_GENERIC_ERR_361                       ((BPLib_Status_t) -361) // CT error
#define BPLIB_CT_GENERIC_ERR_362                       ((BPLib_Status_t) -362) // CT error
#define BPLIB_CT_GENERIC_ERR_363                       ((BPLib_Status_t) -363) // CT error
#define BPLIB_CT_GENERIC_ERR_364                       ((BPLib_Status_t) -364) // CT error
#define BPLIB_CT_GENERIC_ERR_365                       ((BPLib_Status_t) -365) // CT error
#define BPLIB_CT_GENERIC_ERR_366                       ((BPLib_Status_t) -366) // CT error
#define BPLIB_CT_GENERIC_ERR_367                       ((BPLib_Status_t) -367) // CT error
#define BPLIB_CT_GENERIC_ERR_368                       ((BPLib_Status_t) -368) // CT error
#define BPLIB_CT_GENERIC_ERR_369                       ((BPLib_Status_t) -369) // CT error
*/

/* Administrative Record Processor Errors */
#define BPLIB_ARP_UNK_REC_TYPE_ERR                     ((BPLib_Status_t) -370)

/*
#define BPLIB_ARP_GENERIC_ERR_373                      ((BPLib_Status_t) -373)
#define BPLIB_ARP_GENERIC_ERR_374                      ((BPLib_Status_t) -374)
#define BPLIB_ARP_GENERIC_ERR_375                      ((BPLib_Status_t) -375)
#define BPLIB_ARP_GENERIC_ERR_376                      ((BPLib_Status_t) -376)
#define BPLIB_ARP_GENERIC_ERR_377                      ((BPLib_Status_t) -377)
#define BPLIB_ARP_GENERIC_ERR_378                      ((BPLib_Status_t) -378)
#define BPLIB_ARP_GENERIC_ERR_379                      ((BPLib_Status_t) -379)
#define BPLIB_ARP_GENERIC_ERR_380                      ((BPLib_Status_t) -380)
#define BPLIB_ARP_GENERIC_ERR_381                      ((BPLib_Status_t) -381)
#define BPLIB_ARP_GENERIC_ERR_382                      ((BPLib_Status_t) -382)
#define BPLIB_ARP_GENERIC_ERR_383                      ((BPLib_Status_t) -383)
#define BPLIB_ARP_GENERIC_ERR_384                      ((BPLib_Status_t) -384)
#define BPLIB_ARP_GENERIC_ERR_385                      ((BPLib_Status_t) -385)
#define BPLIB_ARP_GENERIC_ERR_386                      ((BPLib_Status_t) -386)
#define BPLIB_ARP_GENERIC_ERR_387                      ((BPLib_Status_t) -387)
#define BPLIB_ARP_GENERIC_ERR_388                      ((BPLib_Status_t) -388)
#define BPLIB_ARP_GENERIC_ERR_389                      ((BPLib_Status_t) -389)
*/

/** @} */

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* BPLIB_API_TYPES_H */
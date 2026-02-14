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

#ifndef BPLIB_CFG_H
#define BPLIB_CFG_H

#ifdef __cplusplus
extern "C" {
#endif


/*
** Macros
*/

/** 
 * \brief Maximum length of strings
 */

#ifndef BPLIB_MAX_IP_LENGTH
#define BPLIB_MAX_IP_LENGTH                 40  /* Maximum length of an IPv6 address string */
#endif
#ifndef BPLIB_MAX_STR_LENGTH
#define BPLIB_MAX_STR_LENGTH                32  /* Maximum length for a generic string */
#endif

/**
 * \brief Configuration array constraints
 */
#ifndef BPLIB_MAX_NUM_BUNDLE_QUEUES
#define BPLIB_MAX_NUM_BUNDLE_QUEUES             16  /* Maximum number of queues */
#endif
#ifndef BPLIB_MAX_NUM_MIB_SETS
#define BPLIB_MAX_NUM_MIB_SETS                  10  /* Maximum number of MIB sets of source configs and counters */
#endif
#ifndef BPLIB_MAX_NUM_EID_PATTERNS_PER_MIB_SET
#define BPLIB_MAX_NUM_EID_PATTERNS_PER_MIB_SET   4  /* Maximum number of EID patterns that can map to a source MIB set index */
#endif
#ifndef BPLIB_MAX_NUM_LATENCY_POLICY_SETS
#define BPLIB_MAX_NUM_LATENCY_POLICY_SETS       10  /* Maximum number of latency policy sets */
#endif
#ifndef BPLIB_MAX_NUM_STORE_SET
#define BPLIB_MAX_NUM_STORE_SET                 10  /* Maximum number of storage policy sets. */
#endif
#ifndef BPLIB_MAX_NUM_CRS
#define BPLIB_MAX_NUM_CRS                       10  /* Maximum number of Compressed Reporting CRS Entries */
#endif
#ifndef BPLIB_MAX_AUTH_SOURCE_EIDS
#define BPLIB_MAX_AUTH_SOURCE_EIDS              10  /* Maximum number of authorized source EID patterns */
#endif
#ifndef BPLIB_MAX_AUTH_CUSTODIAN_EIDS
#define BPLIB_MAX_AUTH_CUSTODIAN_EIDS           10  /* Maximum number of authorized custodian EID patterns */
#endif
#ifndef BPLIB_MAX_AUTH_CUSTODY_SOURCE_EIDS
#define BPLIB_MAX_AUTH_CUSTODY_SOURCE_EIDS      10  /* Maximum number of authorized custody source EID patterns */
#endif
#ifndef BPLIB_MAX_AUTH_REPORT_TO_EIDS
#define BPLIB_MAX_AUTH_REPORT_TO_EIDS           10  /* Maximum number of report-to EID patterns */
#endif
#ifndef BPLIB_MAX_NUM_STORE_EIDS
#define BPLIB_MAX_NUM_STORE_EIDS                10  /* Maximum number of EID patterns per storage policy set */
#endif

/** 
 * \brief Maximum number of contacts that can be running at once
 *          This drives the number of entries in the CLA configuration
 *          tables, as well as the number of CLA In/Out tasks in BPNode
 */
#ifndef BPLIB_MAX_NUM_CONTACTS
#define BPLIB_MAX_NUM_CONTACTS                  1
#endif

/**
 * \brief Maximum number of destination EID patterns per contact
 */
#ifndef BPLIB_MAX_CONTACT_DEST_EIDS
#define BPLIB_MAX_CONTACT_DEST_EIDS             3
#endif

/** 
 * \brief Maximum number of channels that can be running at once
 *          This drives the number of entries in the channel and ADU proxy configuration
 *          tables, as well as the number of ADU In/Out tasks in BPNode
 */
#ifndef BPLIB_MAX_NUM_CHANNELS
#define BPLIB_MAX_NUM_CHANNELS                  2
#endif

/** 
 * \brief Maximum number of extension blocks per bundle.
 */
#ifndef BPLIB_MAX_NUM_EXTENSION_BLOCKS
#define BPLIB_MAX_NUM_EXTENSION_BLOCKS          4
#endif

/**
 * \brief Maximum number of canonical blocks per bundle
 *        this is one more than BPLIB_MAX_NUM_EXTENSION_BLOCKS
 *        because it includes all extension blocks plus the payload block
 */
#ifndef BPLIB_MAX_NUM_CANONICAL_BLOCKS
#define BPLIB_MAX_NUM_CANONICAL_BLOCKS          (BPLIB_MAX_NUM_EXTENSION_BLOCKS + 1)
#endif

/**
 * \brief This is the EID scheme for this instance of a DTN node
 */
#ifndef BPLIB_LOCAL_EID_SCHEME
#define BPLIB_LOCAL_EID_SCHEME                  BPLIB_EID_SCHEME_IPN
#endif

/**
 * \brief This is the EID IPN/SSP format for this instance of a DTN node
 */
#ifndef BPLIB_LOCAL_EID_IPN_SSP_FORMAT
#define BPLIB_LOCAL_EID_IPN_SSP_FORMAT          BPLIB_EID_IPN_SSP_FORMAT_TWO_DIGIT
#endif

/**
 * \brief This is the EID allocator for this instance of a DTN node
 */
#ifndef BPLIB_LOCAL_EID_ALLOCATOR
#define BPLIB_LOCAL_EID_ALLOCATOR               0
#endif

/**
 * \brief This is the EID node number for this instance of a DTN node
 */
#ifndef BPLIB_LOCAL_EID_NODE_NUM
#define BPLIB_LOCAL_EID_NODE_NUM                100
#endif

/**
 * \brief This is the EID service number for this instance of a DTN node. This is not
 *        really used by BPLib, since local delivery sends bundles to other services on
 *        this node, but is included just in case a BPNode implementation does end up
 *        needing it.
 */
#ifndef BPLIB_LOCAL_EID_SERVICE_NUM
#define BPLIB_LOCAL_EID_SERVICE_NUM             0
#endif

/**
 * \brief This reflects whether the system bplib is running on is big endian. Note that
 *        regardless of this, CBOR encoded bundles are big-endian
 */
#ifndef BPLIB_SYS_BIG_ENDIAN
#define BPLIB_SYS_BIG_ENDIAN                    false
#endif

/**
 *  \brief This is the absolute maximum size a bundle is allowed to be.
 */
#ifndef BPLIB_MAX_BUNDLE_LEN
#define BPLIB_MAX_BUNDLE_LEN                    8192
#endif

/**
 * \brief This is the absolute maximum size a bundle payload is allowed to be. The real
 *        max that channels will use is defined in the channel configurations but if
 *        those configurations have MaxBundlePayloadSize values that are larger than this
 *        value, the configurations will be rejected. This value must be smaller than
 *        \ref BPLIB_MAX_BUNDLE_LEN
 */
#ifndef BPLIB_MAX_PAYLOAD_SIZE
#define BPLIB_MAX_PAYLOAD_SIZE                  4096
#endif

/**
 * \brief This is the absolute maximum lifetime a bundle can have. Channel configurations
 *        that specify a lifetime greater than this value will be rejected and bundles
 *        received by Storage that have a lifetime greater than this value will have their
 *        functional lifetime truncated to this value.
 */
#ifndef BPLIB_MAX_LIFETIME_ALLOWED
#define BPLIB_MAX_LIFETIME_ALLOWED              0xfffffffe
#endif

/**
 *  \brief This is the maximum retransmit timeout allowed in the contacts configuration
 */
#ifndef BPLIB_MAX_RETRANSMIT_ALLOWED
#define BPLIB_MAX_RETRANSMIT_ALLOWED            86000
#endif

/**
 *  \brief This is the maximum CS time trigger allowed in the contacts configuration. 
 *         TODO revisit value in 7.1
 */
#ifndef BPLIB_MAX_CS_TIME_TRIGGER_ALLOWED
#define BPLIB_MAX_CS_TIME_TRIGGER_ALLOWED       86000
#endif

/**
 *  \brief This is the maximum CS size trigger allowed in the contacts configuration. 
 *         TODO revisit value in 7.1 
 */
#ifndef BPLIB_MAX_CS_SIZE_TRIGGER_ALLOWED
#define BPLIB_MAX_CS_SIZE_TRIGGER_ALLOWED       86000
#endif

/**
 *  \brief Name of this entity. This should  unambiguously identify the node within 
 *         the network
 */
#define BPLIB_SYSTEM_NODE_NAME                  "BPNode Development"

/**
 *  \brief Name of the primary manager of this node
 */
#define BPLIB_SYSTEM_NODE_OWNER                 "NASA GSFC"

/**
 *  \brief Name of the underlying OS or executive controlling the resources of this node
 */
#define BPLIB_SYSTEM_SOFWARE_EXEC               "cFS"

/**
 *  \brief Version of the software executive
 */
#define BPLIB_SYSTEM_SOFTWARE_EXEC_VERSION      "cfe equuleus-rc1+dev235"

/**
 *  \brief List of all CLAs currently supported by this node
 */
#define BPLIB_SUPPORTED_CLAS                    "UDP"

/**
 *  \brief Maximum number of bundle bytes allowed in storage at any given time
 */
#define BPLIB_MAX_STORED_BUNDLE_BYTES            4000000000     /* 4 gigabytes */

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* BPLIB_CFG_H */

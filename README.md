# Bundle Protocol Library (BPLib)

## 1. Overview

```
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@...............@@@@@@@@@@@@@@@ @@@@@@@................@@@@@@@@@@ @@@@@@@@@@.......@@@@@@@@.....@@
@@................@@@@@@@@@@@@@@  @@@@@@................@@@@@@@@@@  @@@@@@@@.........@@@@@@@.....@@
@@.....@@@@@@......@@@@@@@@@@ @@@   @@@@@@@@@@.....@@@@@@@@@@@@ @@@   @@@@@@..........@@@@@@.....@@
@@.....@@@@@@@......@@@@@@@@@  @@@   @@@@@@@@@.....@@@@@@@@@@@@  @@@   @@@@@......@....@@@@@.....@@
@@.....@@@@@@@@.....@@@@@  @@@  @@@  @@@@@@@@@.....@@@@@@@@  @@@  @@@  @@@@@......@.....@@@@.....@@
@@.....@@@@@@@@.....@@@@@@  @@@  @@  @@@@@@@@@.....@@@@@@@@@  @@@  @@  @@@@@......@@.....@@@.....@@
@@.....@@@@@@@@.....@@@@@@  @@@  @@  @@@@@@@@@.....@@@@@@@@@  @@@  @@  @@@@@......@@@.....@@.....@@
@@.....@@@@@@@......@@@@@  @@@  @@   @@@@@@@@@.....@@@@@@@@  @@@  @@   @@@@@......@@@.....@@.....@@
@@.....@@@@@........@@@@@@@@@  @@   @@@@@@@@@@.....@@@@@@@@@@@@  @@   @@@@@@......@@@@.....@.....@@
@@.................@@@@@@@@@@ @@  @@@@@@@@@@@@.....@@@@@@@@@@@@ @@  @@@@@@@@......@@@@@..........@@
@@...............@@@@@@@@@@@@@@@ @@@@@@@@@@@@@.....@@@@@@@@@@@@@@@ @@@@@@@@@......@@@@@@........@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
```

The Bundle Protocol library (BPLib) is a C-language implementation of the 
[RFC-9171 Bundle Protocol version 7]( https://www.rfc-editor.org/rfc/rfc9171.html) that
targets embedded space flight applications via [BPNode](https://github.com/nasa/bp). It
also follows the CCSDS Compressed Bundle Status Reporting and Custody Signaling Orange Book
that is currently in draft. Currently only a subset of these specifications are implemented,
with the goal of implementing the full specifications in future builds.

The following features are currently implemented:
- Bundle creation, forwarding, reception, and delivery using CBOR for encoding and decoding
- The standard RFC-9171 extension blocks: previous node, age, and hop count
- Directives and telemetry needed for monitoring and controlling channels and contacts
- Initial Management Information Base (MIB) configurations, counters, and reports
- Bundle storage in persistent memory
- Custody transfer, including support for Compressed Custody Signals and Custody Transfer Extension Blocks

The [BPNode](https://github.com/nasa/bp) application handles most of the convergence layer
interactions and currently supports sending and receiving bundles over UDP and over the
cFS Software Bus. Additional Convergence Layer Adapters (CLAs) are planned for future builds.

BPLib contains no threads and relies entirely on the calling application for its execution 
context and implements a thread-safe blocking I/O model where shared resources are 
mutex-protected. Memory is allocated up-front by the calling application in a single pool
that BPLib uses as it moves bundles between threads to perform various ingress, processing, 
and egress operations. A sqlite3 database provides persistent storage to bundles that cannot be
immediately delivered and monitors bundles in storage for potential delivery or lifetime
expiration.

## 2. Repository Structure

This repository is organized into the following key directories:

- AA (Application Agent): Provides application and administrative services associated with monitoring and control, such as status monitoring or configuration control
  - ARP (Administrative Record Processor): Handles the processing and generation of new administrative record bundles
  - AS (Administrative Statistics): Manages the Management Information Base Counters and Reports telemetry packets
  - FWP (Framework Proxy): Provides function definitions to interface to an underlying framework for services such as time management, event handling, telemetry forwarding, etc.
  - NC (Node Configuration): Handles node configuration requests via directives and configuration tables
- app: Source files for bpcat, the standalone executable prototype
- BPA (Bundle Protocol Agent): Executes BPv7-related activities and functions
  - BI (Bundle Interface): Handles bundle reception, validation, and forwarding
  - CT (Custody Transfer): Processes custodial bundles and custody signals and manages the Custody Transfer Database (CTDB)
  - EBP (Extension Block Processor): Processes the standard RFC-9171 extension blocks
  - PDB (Policy Database): Manages node policy for bundle acceptance and bundle priority
  - PI (Payload Interface): Handles bundle creation and delivery, along with the channel interface
  - STOR (Storage): Maintains the bundle database in persistent memory
- CI (Core Infrastructure): Common low-level utilities used across other modules
  - CBOR (Concise Binary Object Representation): Encodes and decodes bundles
  - CRC (Cyclic Redundancy Check): Calculates fixed length codes for arbitrary data for error detection and uniqueness checks
  - EID (Endpoint ID): Helper functions for BPv7 endpoint ID handling
  - EM (Event Management): Handles node-generated event messages
  - MEM (Memory Allocator): Allocates and deallocates memory using an externally-provided fixed-length memory pool. Several algorithm implementation options are provided as different compilation options. By default, BPNode uses the cFE memory pool implementation while bpcat just uses malloc and ignores the provided memory pool.
  - PL (Performance Logger): Glue layer between the framework performance logging system and BPLib
  - QM (Queue Management): Provides several types of queues to move bundles between tasks
  - RBT (Red-Black Tree): Implements a generic red-black tree data structure
  - TIME (Time Management): Handles the generation of DTN time using the underlying framework's time system
- CLA (Convergence Layer Adapter): Glue layer between BPLib and the external convergence layer
- docs: Official documentation can be found in the BPNode repo, but this directory contains developer notes, doxygen source files, and deprecated documentation
- inc: High-level header definitions. Mission customization options are most likely to be found in the `inc/bplib_cfg.h` file

## 2. Prerequisites

1. The build has been tested on both Ubuntu 20.04.6 LTS and Ubuntu 22.04 with a standard __cmake__ build system and __gcc__ toolchain.

For Ubuntu 20.04.6 LTS, the following package versions are required:
- cmake version 3.22.1
- pkg-config 0.29.1
- gcc 9.4.0
- libsqlite3-dev 3.31.1

For Ubuntu 22.04, the following package versions are required:
- cmake version 3.22.1
- pkg-config 0.29.2
- gcc 11.4.0
- libsqlite3-dev 3.37.2

2. Create and store an install directory
Location and install directory name are customizable. The intent here is to make the various code
snippets easy to copy-paste.

```
cd ~
mkdir bp_install
cd bp_install
export INSTALL_DIR=$(pwd)
```

3. QCBOR 1.5.1
```
# Navigate to the install directory
cd $INSTALL_DIR

# Clone the QCBOR repo
git clone https://github.com/laurencelundblade/QCBOR.git

# Navigate to the QCBOR repo
cd QCBOR

# Switch to the correct version of QCBOR
git checkout v1.5.1

# Build and install QCBOR
sudo make uninstall
cmake -DBUILD_SHARED_LIBS=ON -S . -B build
cmake --build build
sudo make install
```

## 3. Building BPLib with BPNode within cFS

To build BPLib within the cFS architecture, see the [BPNode](https://github.com/nasa/bp/tree/main/docs) documentation.

## 5. Building BPLib as a Standalone

Please note that the provided standalone executable, bpcat, is provided only as a prototype 
and does not contain the full functionality of the library. For a comprehensive 
implementation of bplib, please see the cFS app, BPNode, in the section above.

1. Clone the repositories
```
# Navigate to the install directory
cd $INSTALL_DIR

# Clone the BPLib repository
git clone https://github.com/nasa/bplib

cd bplib

# Clone the OSAL repository
git clone https://github.com/nasa/OSAL osal

```

If you wish to modify the contact configurations, such as the UDP address or destination EID mappings, you should edit `$INSTALL_DIR/bplib/app/src/bpcat_nc.c`

3. Define build environment variables

```
export NasaOsal_DIR="${INSTALL_DIR}/bplib/osal-staging/usr/local/lib/cmake"
export BPLIB_OS_LAYER=OSAL

# Choose a build configuration - choose between Debug or Release
export MATRIX_BUILD_TYPE=Release

# Define bplib build directory
export BPLIB_BUILD="${INSTALL_DIR}/bplib/bplib-build-matrix-${MATRIX_BUILD_TYPE}-POSIX"
```

4. Build and Install OSAL

```
# Navigate to the bplib directory
cd $INSTALL_DIR/bplib

# Install OSAL
cd osal/
cmake -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DOSAL_SYSTEM_BSPTYPE=generic-linux \
      -DCMAKE_BUILD_TYPE="${MATRIX_BUILD_TYPE}" \
      -DOSAL_OMIT_DEPRECATED=TRUE \
      -DENABLE_UNIT_TESTS=TRUE \
      -DOSAL_CONFIG_DEBUG_PERMISSIVE_MODE=ON -B ../osal-build
cd ../osal-build
make DESTDIR=../osal-staging install
```

4. Build and Install BPLib

```
# Navigate to the cFS directory
cd $INSTALL_DIR/bplib

# Build OSAL with debug configurations
cmake -DCMAKE_VERBOSE_MAKEFILE=ON \
      -DCMAKE_BUILD_TYPE="${MATRIX_BUILD_TYPE}" \
      -DBPLIB_OS_LAYER=OSAL \
      -DCMAKE_PREFIX_PATH=/usr/local/lib/cmake -B "${BPLIB_BUILD}"
cd $BPLIB_BUILD
make all
```

5. Run unit tests
```
ctest --output-on-failure 2>&1 | tee ctest.log
```

## 6. Building a Standalone
```
# Run bpcat executable
cd $BPLIB_BUILD/app
./bpcat
```

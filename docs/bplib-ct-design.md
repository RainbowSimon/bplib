# Custody Transfer Design and Development Notes

## Miscellaneous Dev Notes

- There's a hacky workaround with locally created CCSs where that struct just gets copied into the Bundle->blob binary until it gets encoded. Probably should fix this at some point
- CTDB entries do not get deleted until Storage deletes the associated bundle, they just get marked as BPLib_CT_Transferred. If too many noncustodial bundles are detected in the CTDB, storage garbage collection is automatically run to clean up both storage and the CTDB and free up system memory
- Bundle retransmissions work best the first time, since the CCS most accurately can find holes in transmission sequences. If there's a high packet loss (ex: 10%), some of the retransmissions will also be dropped, and the likelihood that the CCS will be able to detect that missing bundle decreases. Those final bundles will just have to wait for the retransmission timers to expire to be sent again. In practice, this means a large batch of custodial transfers with a high packet loss rate will likely have a few stragglers that take a while to finalize the custodial transfer. This is expected behavior.
- CTDB entries are 128 bytes. If no active ingress or egress operations are happening, the BYTES_MEM_IN_USE telemetry value should equal BUNDLE_COUNT_IN_CUSTODY * 128

## CTDB Design

The CTDB uses memory blocks of size 128 to hold bundle information needed while its custody transfer to the downstream node is ongoing. The CTDB can be searched using two red-black trees, one indexing based on bundle ID, and one based on the sequence ID and sequence number combination assigned to the bundle's CTEB by this node. The first RBT is used when a deserialized bundle is available for reference, the second RBT is used when looking up values from a CCS in the CTDB to assign a custody signal to a bundle.

The Custody Transfer Database has a state for each entry to track the state of the bundle in the node. These states determine the custody transfer operations that can be performed for that bundle:

- Initialized: The bundle was received and not rejected, but the bundle is not in custodial storage yet and therefore custody is not officially accepted.
  - Bundles that can be immediately be delivered to the destination application never leave this state. These are the only custodial bundles that don't have to be stored. This CTDB entry is deleted by a job as the bundle leaves the node.
- In Custody: The bundle is in custodial storage but has not yet been sent out. This also means that a bundle sequence ID/number has not yet been assigned to this bundle and inserted into the sequence number Red-Black Tree
- Transmitted: The bundle has been sent to another node and a bundle sequence ID/number have been assigned. A custody signal has not yet been received
- Retransmitted: The bundle has been forwarded at least twice. In terms of state logic, nothing different from Transmitted occurs, but a different state is assigned in case this is useful to track in the future.
- Transferred: Custody has been accepted by the next node with a custody signal and the bundle has been marked for deletion in storage.
    - This entry is deleted by storage when it deletes the bundle
- Delivered: The bundle was delivered to the destination application and the bundle has been marked for deletion in storage.
    - This entry is deleted by storage when it deletes the bundle

NOTE: DO NOT DELETE A CTDB ENTRY BEFORE IT CAN BE DELETED FROM STORAGE. THIS LEADS TO WEIRD SYSTEM-WIDE BEHAVIOR.

## CCS Generation

CCSs will be generated upon the following triggers:

- CCS Size Trigger
    - Defined in the contact table, based on the predicted encoded CCS size
    - Note that the size calculation is super handwavey since it's impossible to predict the size of an encoded bundle with the provided information
- CCS Time Trigger
    - Defined in the contact table, based on the milliseconds elapsed since the CCS began to be populated
- Maximum CCSs in memory
    - When the maximum CCSs in progress is reached, the oldest is generated to allow for an in progress CCS to be used for a new bundle
- Maximum bundle sequence range length
    - When the max bundle sequence range length is reached (based on the size of the available array)

## Custody Criteria

Custody acceptance criteria are as follows:

- There's space available in storage
- There's space available in the CTDB
- There's enough memory left to create a CTDB entry
- No other errors occurred when initializing a CTDB entry
- No other errors occurred when storing the bundle
- The bundle was accepted by the Policy Database
    - This feature will be implemented in a future build

If the bundle is a duplicate of another bundle in the CTDB, an accepted custody signal is created, just in case the previous custody signal got lost, but the bundle is discarded.

## Testing

### How to Test Custody Transfer with Delays and Disruptions

To add a delay of 1 second and a packet loss rate of 10% (for example) to the loopback interface (127.0.0.1) on port 4551:

```
sudo tc qdisc add dev lo root handle 1: prio priomap 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
sudo tc qdisc add dev lo parent 1:1 handle 10: netem delay 1s loss 10%
sudo tc filter add dev lo protocol ip parent 1:0 prio 1 u32 match ip dport 4551 0xffff flowid 1:1
```

To remove the TC filter:

```
sudo tc qdisc del dev lo root
```

Rebooting the machine will also remove the filter

### Main Test Cases to Think About

- Custodial bundles created locally
- Custodial bundles created on a foreign node
- Custodial bundles forwarded to another node
- Custodial bundles stored and then delivered locally
- Custodial bundles delivered locally without being stored

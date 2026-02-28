# Custody Transfer Design and Development Notes

## Miscellaneous Dev Notes

- There's a hacky workaround with locally created CCSs where that struct just gets copied into the Bundle->blob binary until it gets encoded. Probably should fix this at some point
- CTDB entries do not get deleted until Storage deletes the associated bundle, they just get marked as BPLib_CT_Transferred. If too many noncustodial bundles are detected in the CTDB, storage garbage collection is automatically run to clean up both storage and the CTDB and free up system memory
- Bundle retransmissions work best the first time, since the CCS most accurately can find holes in transmission sequences. If there's a high packet loss (ex: 10%), some of the retransmissions will also be dropped, and the likelihood that the CCS will be able to detect that missing bundle decreases. Those final bundles will just have to wait for the retransmission timers to expire to be sent again. In practice, this means a large batch of custodial transfers with a high packet loss rate will likely have a few stragglers that take a while to finalize the custodial transfer. This is expected behavior.

## Testing

### How to Test Custody Transfer with Delays and Disruptions

To add a delay of 1 second and a packet loss rate of 10% to the loopback interface (127.0.0.1) on port 4551:

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

# Custody Transfer Design Notes

## How to Test Custody Transfer with Delays and Disruptions

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
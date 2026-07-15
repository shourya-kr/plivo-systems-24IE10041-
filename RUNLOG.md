# RUNLOG.md

## Experiment 1
* Profile: A (mild)
* Delay MS: 60
* Miss %: ~2%
* Overhead: 1.00x
* Change & Why: Ran the starter C code just to see what happens. It failed because there is zero backup for dropped packets, so every drop becomes a miss.

## Experiment 2
* Profile: A (mild)
* Delay MS: 60
* Miss %: 0.00%
* Overhead: 1.97x
* Change & Why: Added basic redundancy by sending the previous packet's payload along with the current one. Worked great for Profile A, but totally failed on Profile B because a burst loss of 2 packets breaks the chain.

## Experiment 3
* Profile: B (bursty)
* Delay MS: 60
* Miss %: 0.53%
* Overhead: 2.02x
* Change & Why: Switched to an XOR logic. Sent an XOR of packet (i-1) and (i-3) attached to the current packet. It fixed the burst drops, but the overhead was 2.02x, which is just slightly over the 2.0x limit.

## Experiment 4
* Profile: B (bursty)
* Delay MS: 60
* Miss %: 38.60%
* Overhead: 1.92x
* Change & Why: To fix the bandwidth issue, I made a rule to skip attaching the backup data on every 10th packet. Bandwidth passed nicely, but miss rate shot up. I realized 60ms is just too fast for the network lag on Profile B; the 3rd packet arrives too late to help.

## Experiment 5 (Final)
* Profile: B (bursty)
* Delay MS: 120
* Miss %: < 1.00%
* Overhead: 1.92x
* Change & Why: Increased the delay to 120ms. This gives the receiver enough time to actually wait for the backup packet to arrive through the network lag and do the math to recover the missing packets. Everything is VALID now!

# RUNLOG.md

## Experiment 1
* Profile: A (mild)
* Delay MS: 60
* Miss %: ~2.1%
* Overhead: 1.00x
* Change & Why: Ran the standard provided C baselines. Resulted in INVALID score because there was absolutely no redundancy handling the ~2% raw packet loss on Profile A. 

## Experiment 2
* Profile: A (mild)
* Delay MS: 60
* Miss %: 0.00%
* Overhead: 1.97x
* Change & Why: Modified architecture to implement a basic piggybacking system. Concatenated payload[i-1] to payload[i]. Valid run, but struggled when testing bursts.

## Experiment 3
* Profile: B (bursty)
* Delay MS: 55
* Miss %: >1.5%
* Overhead: 1.97x
* Change & Why: Tested the basic payload[i-1] on a bursty network. Failed because a burst of 2 loses a packet permanently since the payload is never resent in a third interval.

## Experiment 4
* Profile: B (bursty)
* Delay MS: 60
* Miss %: ~0.04%
* Overhead: 1.97x
* Change & Why: Moved to a Forward Error Correction (FEC) mechanism. The secondary payload is now an XOR of payload[i-1] and payload[i-3]. Handles bursts of 2 packets seamlessly and easily clears the threshold limit.
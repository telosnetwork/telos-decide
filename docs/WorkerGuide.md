# Trail Worker Guide

In this guide we will explore all the different interactions Workers can perform on the Trail Voting Platform.

### What is a Worker?

A worker on the Trail Voting Platform is an account that watches for jobs and completes them in return for payment. There are many different kinds of jobs and they become available based on certain types of activity within the platform.

### What Kinds of Jobs are Available?

In Trail v2.0.0 there are two kinds of jobs: rebalances and cleanups. Additional job types will become available as new features are added to Trail.

`Rebalance` jobs become available when voters cast votes on a ballot and then change their vote weight. This action prompts workers to recalculate that voter's weighted votes based on their new balance, and the worker is paid for performing this recalculation. Since a voter can participate on any number of different ballots, rebalance jobs become available on *every active vote by that voter* after a balance change.

Note that if a voter changes their balance again, another rebalance job will be available that will overwrite the work done by the previous recalculation if performed by a different worker. This way workers must always stay vigilant, even after rebalancing a vote. Rebalance work is always tracked, but isn't credited until the vote is cleaned at the end of the voting period.

`Cleanup` jobs become available after ballots close and the votes for that ballot are no longer needed on-chain. Workers are paid for cleaning up the expired votes and returning the committed RAM back to the original voter.

## Getting Started

Follow these steps to get started as a Trail Worker:

### 1. Find Work

Finding work to perform is as simple as watching the trailservice contract for specific actions.

Any time a voter casts a vote on a ballot and then changes their token balance, that vote need to be recalculated. Workers can watch for actions like `stake()`, `unstake()`, `transfer()`, etc. for rebalance work. However, since ballots have control over which token balance they read from (either liquid or staked) some of these actions may not warrant a rebalance job because the actual balanced used in calculating votes wasn't affected. 

For example, if a ballot reads a voter's staked balance when they vote, then regular `transfer()` actions won't make a rebalance job available (since the voter's staked balance wasn't affected by the transfer, only their liquid balance).

### 2. Perform Work

...

### 3. Getting Paid

Workers may call the `claimpayment()` action once every 24 hours to claim their earned payment for work performed since the last `claimpayment()` call.

When calling `claimpayment()` a worker must specify which registry they are claiming work for, as worker funds are managed on a registry-by-registry basis. Only work performed for ballots under that registry will be paid - this way its always clear where worker fund money is going and registries only need to worry about funding their own ballots.

For example, all ballots using the `TEST` registry will pay workers from the `TEST` registry's worker fund. Funds for the `CRAIG` registry will never be paid for work done on the `TEST` registry, and vice versa, ad infinitum.

#### How is Payment Calculated?

Payment is calculated based on 3 worker metrics: Rebalance Volume, Rebalance Count, and Cleanup Count.

`Rebalance Volume` is the total volume of rebalance work done for a registry, measured by the amount of raw tokens rebalanced (raw tokens being the pre-weighted amount).

`Rebalance Count` is the total number of valid rebalances done for a registry by the worker.

`Cleanup Count` is the total number of cleanups done for a registry by the worker.

Each metric is weighted evenly, and the percentage of work done in relation to the total work done on the registry is added together and divided by 3.

> For example:

Worker Fund: 500 TLOS

WorkerA: 

- Rebalance Vol: 500.00 TEST / 1000.00 TEST = 0.50 (50%)

- Rebalance Count: 5 / 75 = 0.0667 (6.7%)

- Cleanup Count: 10 / 432 = 0.0231 (2.3%)

Total Work Proportion = (0.5 + 0.0667 + 0.0231) / 3.0 (.2659 or 26.59%)
```
Worker Fund * Total Work Proportion = Total Payout

500 * .2659 = 132.95 TLOS Total Payout
```

### 4. Unregistering and Forfeiting Payment

...

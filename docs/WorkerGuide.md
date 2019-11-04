# Trail Worker Guide

In this guide we will explore all the different interactions Workers can perform on the Trail Voting Platform.

### What is a Worker?

A worker is an account that watches for jobs and completes them in return for payment. There are many different kinds of jobs and they become available based on certain types of activity within the platform.

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

Work can be performed by calling the appropriate worker action. Make sure to put your account name in the "worker" parameter.

Submit a `rebalance()` action to perform a rebalance on an active vote. The work will be logged on the vote, and when the vote is cleaned the total rebalance work will be credited to the worker's account.

If a vote is ready to be cleaned, call the `cleanupvote()` action to clean it up. Cleaning up votes submits all the reblanace work that was done on it as well.

### 3. Getting Paid

Workers may call the `claimpayment()` action to claim their earned payment for work performed since the last `claimpayment()` call.

When calling `claimpayment()` a worker must specify which treasury they are claiming work for, as worker funds are managed on a treasury-by-treasury basis. Only work performed for ballots under that treasury will be paid - this way its always clear where worker fund money is going and treasuries only need to worry about funding their own ballots.

For example, all ballots using the `TEST` treasury will pay workers from the `TEST` treasury's worker fund. Funds for the `CRAIG` treasury will never be paid for work done on the `TEST` treasury, and vice versa, ad infinitum.

#### How is Payment Calculated?

Payment is calculated based on 3 Work Metrics: Rebalance Volume, Rebalance Count, and Cleanup Count.

`Rebalance Volume` is the total volume of rebalance work done for a treasury, measured by the amount of raw tokens rebalanced (raw tokens being the pre-weighted amount).

`Rebalance Count` is the total number of valid rebalances done for a treasury by the worker.

`Cleanup Count` is the total number of cleanups done for a treasury by the worker.

Each metric is weighted evenly, and the total earned payout is equal to the total amount of work being claimed by the worker proportional to all the currently unclaimed work in the treasury. In other words, if a worker is claiming 15% of all unclaimed work from a treasury with 100 TLOS available in their worker payroll, the payout will be 15 TLOS.

#### Payment Decay

Approved work volume has a 1 day grace period to be claimed before it begins to decay at a rate of 1% per day.

After 10 days without claiming, the work may instead be claimed or forfeited by another worker.

### Example

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

### 4. Forfeiting Work

Work performed can optionally be forfeited at any time by the worker with the `forfeitwork()` action. This will delete all work done by the worker for the treasury and forfeit all payment they would otherwise receive. 

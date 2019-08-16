# Trail Developer Guide

In this Developer Guide, we will explore Trail's suite of platform services available to any prospective contract developer. We will follow an example throughout the guide to help establish the general flow and lifecycle of running a ballot and/or registry. The Trail Contract API will be introduced in this guide as certain sections become relevant to the task at hand. For a more straightforward breakdown of Trail's Contract API without the example fluff, see the [Trail Contract API](ContractAPI.md) document.

## Understanding Trail's Role

Trail's primary role is to consolidate many of the common boilerplate functions of voting contracts into a single chain-wide service available to all accounts on the network.

In other words, this means developers can leave the "vote counting" up to Trail and instead write their contracts to listen to Trail for final ballot results. External Smart Contracts can then immediately act on these results and interpret them according to their intended use case.

### Trail Voting Platform Services

| Service | Description | Cost |
| --- | --- | --- |
| Ballots | Ballot hosting, managing, and results broadcasting. | 35 TLOS per ballot |
| Registries  | Registry creation and management. | 250 TLOS per registry |
| Committees | Committee creation and management. | 100 TLOS per committee |
| Archival | Archive important ballots. | 2 TLOS per day |
| Workers | Worker registration, work tracking, and profit incentives. | Free |
| Voters | Voter registration, token tracking, and unlimited voting. | Free |

More information about each service can be found in the relevent documentation.

-----

## Building Your External Contract

For this Developer Guide we will also walk through building a simple external contract to act on ballot results after ballot closure. 

In our contract, we will launch a new election ballot and then *contractually act* on the results broadcast by Trail's `bcastresults()` action. Upon hearing the broadcast, our contract will read the final results, determine the winning candidate, and then send an inline action back to Trail to update our committee with the election winner. This of course requires a registry, ballot, and committee to be set up on Trail beforehand, but we will walk though that in the setup guide below.

> Setup: [Example Setup Guide](../contracts/example/README.md)

> Contract:
[example.hpp](../contracts/example/include/example.hpp) / 
[example.cpp](../contracts/example/src/example.cpp)

-----

## Registries

...

### Registry Creation

...

### Registry Management

...

### Registry Funding

...

## Ballots

...

### Ballot Creation

...

#### Ballot Statuses

| Status | Description |
| --- | --- |
| setup | Ballot is being prepared. |
| voting | Ballot is currently open for voting. |
| closed | Ballot is closed and final results have been rendered. |
| cancelled | Ballot was cancelled during voting, awaiting deletion. |
| archived | Ballot is currently archived and can't be deleted. |

### Voting Methods

| Voting Method | Description | Raw Weight | Weighted Vote |
| --- | --- | --- | -- |
| 1acct1vote | Every voter gets 1 whole vote. Zero balances don't count. | 0.01 TEST | 1.00 TEST Each |
| 1tokennvote | Raw weight is applied to each option selected. | 3.00 TEST | 3.00 TEST Each |
| 1token1vote | Raw weight is split among all selections. | 3.00 TEST | 1.00 TEST Each |
| 1tsquare1v | Raw weight is squared and split among selections. At ballot closure each option's total will be square-rooted | 3.00 TEST | 9.00 TEST Each |
| quadratic | Raw weight is squared and split among selections. | 3.00 TEST | 9.00 Each |
| ranked | Votes are cast in order of preference, with the weighted total divided by the option's position. | 3.00 TEST | #1 = 3.00 TEST, #2 = 1.50 TEST, ... |

#### Ballot Settings

| Setting | Description |
| --- | --- |
| lightballot | Marks as a light ballot if true. |
| revotable | Allows revoting on the ballot if true. |
| votestake | Reads voter's staked balance for casting votes if true. |

### Ballot Management

...

### Ballot Closing

...

### Broadcasting Ballot Results

...

### Ballot Deletion

...

### Ballot Cancellation

...

### Archiving and Unarchiving

...

-----

## Voters

...

### Voter Registration

...

### Casting Votes

...

### Unvoting

...

### Staking and Unstaking

...

-----

## Workers

...

### Worker Registration

...

### Rebalancing Votes

...

### Cleaning Votes

...

### Claiming Worker Payout

...

-----

## Committees

...

### Committee Creation

...

### Committee Management

...

### External Contract Hooks

...

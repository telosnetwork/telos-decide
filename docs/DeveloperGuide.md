# Trail Developer Guide

In this Developer Guide, we will explore Trail's suite of platform services available to any prospective contract developer. We will follow an example throughout the guide to help establish the general flow and lifecycle of running a ballot and/or registry. The Trail Contract API will be introduced in this guide as certain sections become relevant to the task at hand. For a more straightforward breakdown of Trail's Contract API without the example fluff, see the [Trail Contract API](ContractAPI.md) document.

## Understanding Trail's Role

Trail's primary role is to consolidate many of the common boilerplate functions of voting contracts into a single chain-wide service available to all accounts on the network.

In other words, this means developers can leave the "vote counting" up to Trail and instead write their contracts to listen to Trail for final ballot results. External Smart Contracts can then immediately act on these results and interpret them according to their intended use case.

### Trail Voting Platform Services

| Service | Description | Cost |
| --- | --- | --- |
| Ballots | Ballot publishing, managing, and closing. | 35 TLOS |
| Registries | Registry creation and management. | 250 TLOS |
| Committees | Committee creation and management. | 100 TLOS |
| Archival | Archive important ballots. | 5 TLOS |
| Workers | Worker registration, payments, and incentivization. | Free |
| Voters | Voter registration, balances, and management. | Free |

More information about each service can be found in each service's relevent documentation.

-----

## Building External Contract

For this Developer Guide we will also build a simple external contract to act on ballot results when they're broadcast. 

In our contract, we will launch a new election ballot and then act on the results broadcasted by the `bcastresults()` action. Upon hearing the broadcast from Trail, our contract will read the final results, determine the winner, then send an inline action back to Trail to update a Committee with the election winner.

The source code for this contract can be found [here](../contracts/example/src/exmaple.cpp).

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

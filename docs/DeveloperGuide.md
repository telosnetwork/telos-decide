# Trail Developer Guide

In this Developer Guide, we will explore Trail's suite of platform services available to any prospective contract developer. We will follow an example throughout the guide to help establish the general flow and lifecycle of running a ballot and/or treasury. The Trail Contract API will be introduced in this guide as certain sections become relevant to the task at hand. For a more straightforward breakdown of Trail's Contract API without the example fluff, see the [Trail Contract API](ContractAPI.md) document.

## Understanding Trail's Role

Trail's primary purpose is to consolidate all the common boilerplate functions of voting contracts into a single chain-wide service available to organizations, developers, and general users.

For developers, this means they can leave the user registration, token management, and vote tracking up to Trail, and instead design their contracts to listen for Trail to broadcast final ballot results. External Smart Contracts can then *contractually* interpret and act on these broadcasts according to their intended use cases.

### Trail Voting Platform Services

| Service | Description | Cost |
| --- | --- | --- |
| Voting | Voter registration, token balances, voting, unvoting, staking, and unstaking. | Free |
| Ballots | Ballot hosting, managing, and results broadcasting. | 35 TLOS per ballot |
| Treasuries  | Treasury creation, locking/unlocking, verbose settings, worker fund, token minting, transferring, reclaiming, and burning. | 250 TLOS per treasury |
| Committees | Committee creation, management, security, and seat control. | 100 TLOS per committee |
| Archival | Archive important ballots to prevent deletion. | 2 TLOS per day |
| Workers | Worker registration, progress tracking, and profit incentives. | Free |


More information about each service can be found in the relevent documentation.

-----

## Building Your External Contract

For this Developer Guide we will also walk through building a simple external contract to act on ballot results after ballot closure. 

In our contract, we will launch a new election ballot and then *contractually act* on the results broadcast by Trail's `bcastresults()` action. Upon hearing the broadcast, our contract will read the final results, determine the winning candidate, and then send an inline action back to Trail to update our committee with the election winner. This of course requires a treasury, ballot, and committee to be set up on Trail beforehand, but we will walk though that in the setup guide below.

> Setup: [Example Setup Guide](../contracts/example/README.md)

> Contract:
[example.hpp](../contracts/example/include/example.hpp) / 
[example.cpp](../contracts/example/src/example.cpp)

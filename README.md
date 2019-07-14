# Trail Voting Platform

Trail is an on-chain voting platform for the Telos Blockchain Network that offers an extensive suite of voting services for both users and developers.

## Features

* `Custom Token Registries`

    Any user on the Telos Blockchain Network may create a custom token registry that automatically inherits Trail's entire suite of voting services and fraud prevention tools. These registries can also be customized to allow or disallow specific behaviors like transferring, burning, or reclaiming of tokens.

    During ballot creation, the ballot publisher selects a specfic token to use for counting votes. The default is the standard `VOTE` token - usable in all Telos Governance ballots, but Trail offers additional services for creating privately managed tokens that ballots can use for counting votes instead.

    For example, a user could create the `BOARD` registry, where one token represents one board seat at their company. Then ballots could be proposed by any `BOARD` holder for any votable topic, and all `BOARD` holders would be able to cast a vote on any such ballot.

* `Optional Light Ballots`

    Light ballots disable the on-chain vote tracking portion of the voting process - this allows for developers to track votes in an off-chain database that is built by an Iris or Demux style service instead. 
    
    This option enables Trail to power vastly more voting services by saving RAM costs for both the platform and voters, while at the same time retaining the complete traceability and auditability benefits offered by the blockchain.

* `Traceable Vote Integrity`

    Trail has implemented a custom rebalance system that detects changes to a user's staked resources and either updates all open votes for the user, or posts a new rebalance job for any user to complete and earn rewards. Future updates will allow for optional identity services to futher enhance voting and platform features.

* `Profitable Janitorial Services`

    Trail has several janitorial actions that incentivize users on the network to clean up old votes and maintain an optimized voting system.

## The TRAIL Token

Trail has a platform specific token that is only obtainable by performing various jobs and tasks that help optimize the platform by cleaning expired votes and rebalancing active vote receipts.

**Disclaimer:** *The TRAIL token is not an ICO nor should it be considered for investment or speculative purposes. It is a reward for user assistance in maintaining an efficiently run voting platform and will have evolving purposes as the platform grows.*

## Roadmap

### Q4 2019

- ...

### Q1 2020

- ...

### Down Yonder

- ...

## Contributing to Trail

Trail is an open-source voting platform where contributions and improvements are welcomed by the community.

To make a contribution, simply fork this repo and submit a PR for your changes. In order to avoid duplication of work, it's best to engage with the community first to determine what would be an acceptable addition to the platform.

Discussion happens mostly on Telegram, so feel free to join the [Telos Dapp Development](https://t.me/dappstelos) chat and pitch your idea!

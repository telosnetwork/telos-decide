# Trail Voting Platform

Trail is an on-chain voting platform for the Telos Blockchain Network that offers an extensive suite of voting services for users and developers.

## Features and Services

* `Secure Ballot Hosting`

    Any user on the Telos Blockchain Network may create and publish a public ballot that is secured and hosted through Trail. During ballot creation, the ballot publisher selects a specfic token to use for counting votes. The default is the standard `VOTE` token - usable in all Telos Governance ballots, but Trail offers additional services for creating privately managed tokens that ballots can use for counting votes instead.

* `Advanced Voting Methods`

    The Trail Voting Platform boasts an extensive set of voting methods and customization options. Voting Methods include but are not limited to: `1acct1vote`, `1tokennvote`, `1token1vote`, `1tsquare1v`, `quadratic`, and `ranked`. See the `Voting Methods` section of the `Trail Developer Guide` for more information on the many different voting methods available.

* `Custom Token Registries`

    Any user on the Telos Blockchain Network may create a custom token registry that automatically inherits Trail's entire suite of voting services. These registries can also be customized to allow or disallow specific behaviors like transferring, burning, or reclaiming of tokens.

* `Optional Light Ballots`

    Light ballots disable the on-chain vote tracking portion of the voting process - this allows for developers to track votes in an off-chain database that is built by an Iris or Demux style service instead. 
    
    This option enables Trail to power vastly more voting services by saving RAM costs for both the platform and voters, while at the same time retaining the complete traceability and auditability benefits offered by the blockchain.

* `Committee Management Tools`

    Trail offers a suite of committe creation and management tools that are available as a free service to any active token registry and its voters. Developers can also hook their external smart contracts into Trail's committee tools to enable complete on-chain management of committees and their members.

* `Traceable Vote Integrity`

    Trail has implemented a custom rebalance system that detects changes to a user's staked resources and either updates all open votes for the user, or posts a new rebalance job for any user to complete and earn rewards. Future updates will allow for optional identity services to futher enhance voting and platform features.

* `Profitable Worker Services`

    Trail has several janitorial actions that incentivize workers on the network to maintain an optimized voting system.

## Join the Firewatch

The Firewatch is a league of voters, developers, and stakeholders dedicated to running an optimized voting platform.

## The TRAIL Token

Trail has a platform specific token that is only obtainable by performing various jobs and tasks that help optimize the platform by cleaning expired votes, rebalancing active vote receipts, and completing and interacting with valid ballots.

**Disclaimer:** *The TRAIL token is not an ICO nor should it be considered for investment or speculative purposes. It is a reward for user assistance in maintaining an efficiently run voting platform and will have evolving purposes as the platform grows.*

## Roadmap

### Ongoing

- Platform Optimization
- Firewatch Incentivization

### Q4 2019

- Additional Worker Services
- Additional Voting Methods
- External Contract Examples

### Q1 2020

- Staking, Stake Voting

### Down Yonder

- Delegates, Delegate Voting
- Optional Identity Services

## Contributing to Trail

Trail is an open-source voting platform where contributions and improvements are welcomed by the community.

To make a contribution, simply fork this repo and submit a PR for your changes. In order to avoid duplication of work, it's best to engage with the community first to determine what would be an acceptable addition to the platform.

Discussion happens mostly on Telegram, so feel free to join the [Telos Dapp Development](https://t.me/dappstelos) chat and pitch your idea!

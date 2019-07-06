# Trail Voting Platform

Trail is an on-chain voting platform for the Telos Blockchain Network that offers a full suite of voting services for both users and developers. Trail is maintained as a completely open source project by [@craigbranscom](https://github.com/craigbranscom) for maximum transparency and auditability.

## Services

* `Public Ballot Creation`

    Any user on the Telos Blockchain Network can create a ballot that is publicly viewable and votable by all users on the platform.

    During Ballot creation, the Ballot publisher may specify which token they want to use for counting votes. In most cases this will be the default `VOTE` token, but Trail offers additional services for creating privately-managed voting tokens that Ballot publishers can use instead.

* `Casting Votes`

    All users who own a balance of `TLOS` are elligible to mirror their staked balance and receive `VOTE` tokens at a 1:1 ratio. These `VOTE` tokens are castable on any Ballot that is created for use with that token.

    In short, regular `TLOS` tokens do not exist on the Trail platform they are instead swapped with `VOTE` tokens and those `VOTE` tokens are what are used to cast votes on Telos Governance proposals.

    However, if users have balances of custom tokens and there are open ballots for those tokens, they may cast their votes on those Ballots as well.

* `Custom Voting Token Creation`

    Any user may create a custom voting token that automatically inherits Trail's entire suite of voting services and fraud prevention tools.

    For instance, a user could create a `BOARD` token to represent one board seat at their company. They would then own the entire `BOARD` token namespace within Trail and have the full authority to `mint` or `send` those tokens. Trail's verbose token interface even allows for additional token features (set at the time of token creation) such as `burning`, `seizing`, or `mutable_max`.

* `Custom Token Ballots`

    Once a user has created a custom voting token, any holder of that token may publish Ballots that are votable on by any other holder of that custom token.

    Continuing with the `BOARD` example, a board member could propose a vote for all `BOARD` holders. Ballots created for custom tokens operate exactly the same as Ballots running on the system-backed `VOTE` token, they simply use a different token to count votes.

* `Fraud Prevention`

    Trail has implemented a powerful Rebalance system that detects changes to a user's staked resources and automatically updates all open votes for the user. 

* `System Efficiency`

    Trail has several efficiency actions that are callable by any user on the network to help clean up old votes and maintain a lightweight voting system.

## Purpose of VOTE Token

Trail manages an internal token named `VOTE` that is used in place of the native system token. 

Trail uses the virtual `VOTE` token to avoid confusion that a user's `TLOS` are being held on the platform, and to allow for more efficient actions and therefore lower resource usage when interacting with the platform.

All users who own a balance of `TLOS` are eligible to mirror their staked balance for an equivalent number of `VOTE` tokens. Trail calculates a user's `VOTE` balance by simply issuing `1 VOTE` per `1 TLOS` of the user's staked rement (in no particular order)

* Proxy Registration, Proxy Voting, Proxy Management

* Cleanup Rewards (Task Market)

* Additional Categories

* Additional Token Settings

## Contributing to Trail

Trail is an open-source voting platform where contributions and improvements are welcomed by the community.

To make a contribution, simply fork this repo and submit a PR for your changes. In order to avoid duplication of work, it's best to engage with the community first to determine what would be an acceptable addition to the platform.

Discussion happens mostly on Telegram, so feel free to join the [Telos Dapp Development](https://t.me/dappstelos) chat and pitch your idea!
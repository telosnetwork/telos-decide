# Trail Service User Guide

Trail offers a comprehensive suite of blockchain-based voting services available to any prospective voter on the Telos Blockchain Network.

### Recommended User Resources

* 9 TLOS to CPU

* 1 TLOS to NET

* 10 KB of RAM

## Voter Registration

All users on the Telos Blockchain Network are eleigible to register their Telos accounts within Trail and participate in the voting process.

* `regvoter(name owner, symbol token_sym)`

    The regvoter action creates a zero-balance wallet for the given token symbol if a token registry of the given symbol exists.

    `owner` is the name of the user registering the new account.

    `token_sym` is the symbol of tokens being stored in the new balance.

All users who want to participate in core voting **must** sign the  `regvoter()` action and supply `VOTE,4` as the token symbol.

* `unregvoter(name owner, symbol token_sym)`

    The unregvoter action cancels an existing account balance from Trail and returns the RAM cost back to the user.

    `owner` is the name of the user unregistering the account.

    `token_sym` is the symbol of the token balance to unregister.

Note that an account must be empty of all tokens before being allowed to close. Closed accounts can be created again at any time by simply signing another `regvoter()` action.

## Voter Participation

Voter participation is key to the effectiveness of the Trail platform, therefore Trail has been designed with a streamlined voter interface that keeps the voting process simple for users but also allows for extensive account and resource management.

### 1. Casting Votes

* `castvote(name voter, name ballot_name, name option)`

    The castvote action will cast user's full `VOTE` balance on the given ballot option. Note that this **does not spend** the user's `VOTE` tokens, it only **copies** their full token weight onto the ballot.

    For example, if UserA has already registered with Trail and been credited with `50.0000 VOTE` to their balance, they may then sign a `castvote(UserA, testballot, yes)` action to cast their `50 VOTE` tokens towards the "yes" option on the ballot named "testballot". After casting their vote, the total "yes" votes on the ballot will have increased by `50 VOTE` and the UserA's balance still has all `50 VOTE` tokens in their account.

### 2. Cleaning Old Votes

* `cleanupvotes(name voter, uint16_t count, symbol voting_sym)`

    The cleanupvotes action is a way to clear out old votes and allows voters to reclaim their RAM that has been allocated for storing vote receipts.

    `voter` is the account for which old receipts will be deleted.

    `count` is the number of receipts the voter wishes to delete. This action will run until it deletes specified number of receipts, or until it reaches the end of the list. Passing in a hard number allows voters to carefully manage their NET and CPU expenditure.

    Note that Trail has been designed to be tolerant of users deleting their vote receipts. Trail will never delete a vote receipt that is still applicable to an open ballot. Because of this, the `cleanupvotes()` action may be called by anyone to clean up any user's list of vote receipts.

* `cleanhouse(name voter)`

    The cleanhouse action will attempt to clean out a voter's entire history of votes in one sweep. If a user has an exceptionally large backlog of uncleaned votes this action may fail to clean all of them within the minimum transaction time. If that happens it's better to call `cleanupvotes()` several times first to reduce the number of votes `cleanhouse()` will attempt to clean. 

    `voter` is the name of the account to clean votes for.

Note that this action requires no account authorization, meaning any user can call `cleanhouse()` for any other user to assist in cleaning their vote backlog. There is no security risk to doing this and is a great way to help the platform generally run smoother.
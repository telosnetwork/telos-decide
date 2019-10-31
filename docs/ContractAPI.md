# Trail Contract API

This guide outlines all the contract actions available on the Trail Voting Platform.

-----

## Admin Actions

### ACTION `setconfig()`

Sets the data in the contract's config singleton.

- string `trail_version`: the version number of the deployed Trail contract.

- bool `set_defaults`: if true, sets default config values.

```
cleos push action trailservice setconfig '["v2.0.0-RFC2", true]' -p trailservice
```

### ACTION `updatefee()`

Updates a config fee.

- name `fee_name`: the name of the fee to update.

- asset `fee_amount`: the new fee amount.

```
cleos push action trailservice updatefee '["ballot", "35.0000 TLOS"]' -p trailservice
```

### ACTION `updatetime()`

Updates a config time.

- name `time_name`: the name of the config time to update.

- uint32_t `length`: the length of time in seconds.

```
cleos push action trailservice updatetime '["balcooldown", 86400]' -p trailservice
```

-----

## Treasury Actions

### ACTION `newtreasury()`

Creates a new treasury with the given access method and max supply. 

- name `manager`: the account that will manage the new treasury.

- asset `max_supply`: the maximum number of tokens allowed in circulation at one time.

- name `access`: the access method for acquiring a balance of the treasury's tokens. The list of available access methods can be found in the [Treasury Guide](TreasuryGuide.md).

Required Authority: `manager`

Required Fee: `1000 TLOS`

```
cleos push action trailservice newtreasury '["youraccount", "2,TEST", "public"]' -p manager
```

### ACTION `edittrsinfo()`

Edits the title, description, and icon of a treasury.

- symbol `treasury_symbol`: the treasury to change.

- string `title`: the new title.

- string `description`: the new description.

- string `content`: the new content

Required Authority: `manager`

```
cleos push action trailservice edittrsinfo '["2,TEST", "New Title", "New Desc", "New Icon"]' -p manager
```

### ACTION `toggle()`

Toggles a treasury setting on and off.

- symbol `treasury_symbol`: the treasury to change.

- name `setting_name`: the name of the setting to toggle.

```
cleos push action trailservice toggle '["2,TEST", "transferable"]' -p manager
```

### ACTION `mint()`

Mints new tokens into circulation.

- name `to`: the account to receive the minted tokens.

- asset `quantity`: the amount of tokens to mint.

- string `memo`: a memo describing the minting event.

```
cleos push action trailservice mint '["testaccounta", "5.00 TEST", "testing mint"]' -p manager
```

### ACTION `transfer()`

Transfers a quantity of tokens from one voter to another.

- name `from`: the account sending the tokens.

- name `to`: the account receiving the tokens.

- asset `quantity`: the quantiy of tokens to transfer.

- string `memo`: a memo describing the transfer.

```
cleos push action trailservice transfer '["testaccounta", "testaccountb", "5.00 TEST", "test transfer"]' -p testaccounta
```

### ACTION `burn()`

Burns a quantity of tokens from the manager's liquid balance. Reduces the circulating supply of tokens.

- asset `quantity`: the quantity of tokens to burn.

- string `memo`: a memo describing the burn.

```
cleos push action trailservice burn '["5.00 TEST", "test burn"]' -p manager
```

### ACTION `reclaim()`

Reclaims tokens from a voter to the treasury manager.

- name `voter`: the name of the account from which to reclaim.

- asset `quantity`: the amount of tokens to reclaim.

- strin `memo`: a memo describing the reclamation.

```
cleos push action trailservice reclaim '["someaccount", "5.00 TEST", "reclaim memo"]' -p manager
```

### ACTION `mutatemax()`

Changes the max supply of a treasury.

- asset `new_max_supply`: the new max supply to set.

- string `memo`: a memo describing the max supply mutation.

```
cleos push action trailservice mutatemax '["500.00 TEST", "test mutate max"]' -p manager
```

### ACTION `setunlocker()`

Sets a new treasury unlocker. Only the new unlock_account@unlock_authority will be able to unlock the treasury.

- symbol `treasury_symbol`: the treasury for which to set the new unlocker.

- name `new_unlock_acct`: the name of the new unlocker account.

- name `new_unlock_auth`: the name of the new unlock authority.

Required Authority: `treasury.manager`

```
cleos push action trailservice setunlocker '["2,TEST", "someaccount", "active"]' -p unlock_acct@unlock_auth
```

### ACTION `lock()`

Locks a treasury, preventing changes to settings.

- symbol `treasury_symbol`: the treasury to lock.

Required Authority: `treasury.manager`

```
cleos push action trailservice lock '["2,TEST"]' -p manager
```

### ACTION `unlock()`

Unlocks a treasury, allowing changes to settings.

- symbol `treasury_symbol`: the treasury to unlock.

Required Authority: `unlocker_acct@unlocker_auth`

```
cleos push action trailservice unlock '["2,TEST"]' -p unlock_acct@unlock_auth
```

-----

## Payroll Actions

### ACTION `addfunds()`

Adds funds to a payroll. Funds will be charged from the voter's TLOS balance.

- name `from`: the voter account sending the funds.

- symbol `treasury_symbol`: the treasury receiving the funds.

- name `payroll_name`: the name of the payroll to receive the funds.

- asset `quantity`: the number of tokens to add to the specified payroll.

```
cleos push action trailservice addfunds '["testaccounta", "2,TEST", "workers", "50.0000 TLOS"]' -p publisher
```

### ACTION editpayrate()

Edits the pay rate of a payroll.

- name `payroll_name`: the name of the payroll to edit.

- symbol `treasury_symbol`: the name of the treasury the payroll belongs to.

- uint32_t `period_length`: the new length of the payroll's pay period in seconds.

- asset `per_period`: the amount of payroll funds to make available for claiming per period.

```
cleos push action trailservice editpayrate '["workers", "2,TEST", "86400", "50.0000 TLOS"]' -p publisher
```

--- 

## Ballot Actions

### ACTION `newballot()`

Creates a new ballot with the given initial options.

- name `ballot_name`: the name of the new ballot.

- name `category`: the category of the new ballot. The list of available categories can be found in the [Ballot Guide](BallotGuide.md).

- name `publisher`: the name of the account publishing the ballot.

- symbol `treasury_symbol`: the treasury symbol the ballot will use to count votes.

- name `voting_method`: the method of weighting votes. The list of available voting methods can be found in the [Ballot Guide](BallotGuide.md).

- vector(name) `initial_options`: a list of initial options to place on the ballot.

Required Fee: `30 TLOS`

Required Authority: `publisher`

```
cleos push action trailservice newballot '["ballot1", "poll", "testaccounta", "2,TEST", "quadratic", ["opt1", "opt2"]]' -p testaccounta
```

### ACTION `editdetails()`

Edits the title, description, and content of a ballot.

- name `ballot_name`: the name of the ballot to edit.

- string `title`: the new ballot title.

- string `description`: the new ballot description.

- string `content`: the new ballot content. This is typically an IPFS link or URI to additional information about the ballot.

```
cleos push action trailservice editdetails '["ballot1", "Ballot 1 Example", "Example Description", "somewebsite.io"]' -p testaccounta
```

### ACTION `togglebal()`

Toggles a ballot setting on or off.

- name `ballot_name`: the name of the ballot.

- name `setting_name`: the name of the setting to toggle.

```
cleos push action trailservice togglebal '["ballot1", "lightballot"]' -p testaccounta
```

### ACTION `editminmax`

Changes the minimum and maximum number of options a single voter can select on a ballot.

- name `ballot_name`: the name of the ballot to update.

- uint8_t `new_min_options`: the new minimum number of options a voter can select.

- uint8_t `new_max_options`: the new maximum number of options a voter can select.

```
cleos push action trailservice editminmax '["ballot1", 1, 3]' -p testaccounta
```

### ACTION `addoption()`

Adds an option to a ballot.

- name `ballot_name`: the name of the ballot to add an option to.

- name `new_option_name`: the name of the new ballot option.

```
cleos push action trailservice addoption '["ballot1", "opt3"]' -p testaccounta
```

### ACTION `rmvoption()`

Removes an option from a ballot.

- anme `ballot_name`: the name of the ballot containing the option to remove.

- name `option_name`: the option name to remove.

```
cleos push action trailservice rmvoption '["ballot1", "opt3"]' -p testaccounta
```

### ACTION `openvoting()`

Opens a ballot for voting and sets the ballot end time. All ballot settings and voting options must be finalized before pushing this action, as they cannot be altered once voting has begun. Ballots can, however, be cancelled by the publisher mid-vote at any time.

- name `ballot_name`: the name of the ballot to open for voting.

- time_point_sec `end_time`: the time point at which to end voting.

```
cleos push action trailservice openvoting '["ballot1", "2020-05-22T13:00:00"]' -p testaccounta
```

### ACTION `cancelballot()`

Cancels a running ballot.

- name `ballot_name`: the name of the ballot to cancel.

- string `memo`: a memo describing the cancellation.

```
cleos push action trailservice cancelballot '["ballot1", "cancellation reason here"]' -p testaccounta
```

### ACTION `deleteballot()`

Deletes a ballot.

- name `ballot_name`: the name of the ballot to delete.

```
cleos push action trailservice deleteballot '["ballot1"]' -p testaccounta
```

### ACTION `postresults()`

Posts the final results of a light ballot.

- name `ballot_name`: the name of the ballot to post results to.

- map(name, asset) `light_results`: the final results calcualted off-chain.

- uint32_t `total_voters`: the total number of unique voters who participated on the light ballot.

```
cleos push action trailservice postresults '["ballot1", [{"opt1": "25.00 TEST"},{"opt2", "15.00 TEST"}], 8]' -p trailservice
```

### ACTION `closevoting()`

Closes voting on a ballot and renders the final results.

- name `ballot_name`: the name of the ballot to close.

- bool `broadcast`: if true, will inline the broadcast() action with the ballot results and total voters.

```
cleos push action trailservice closevoting '["ballot1", true]' -p testaccounta
```

### ACTION `broadcast()`

Broadcasts ballot results and notifies the ballot publisher.

- name `ballot_name`: the name of the ballot being broadcast.

- map(name, asset) `final_results`: the final weighted ballot results.

- uint32_t `total_voters`: the total number of unique voters who participated on the ballot.

```
Inline from closeballot()
```

### ACTION `archive()`

Archives a ballot. A flat fee is charged up front per day of archival.

- name `ballot_name`: the name of the ballot to archive.

- time_point_sec `archived_until`: the time point at which the ballot can be unarchived.

Daily Fee: `3 TLOS`

```
cleos push action trailservice archive '["ballot1", "2020-09-08T23:41:00"]' -p testaccounta
```

### ACTION `unarchive()`

Unarchives a ballot. If force is true, will unarchive before the unarchival date and forfeit any remaining archival time.

- name `ballot_name`: the name of the ballot being archived.

- bool `force`: if true, forces the ballot to unarchive. Remaining time on the archival is lost if forced to unarchive.

```
cleos push action trailservice unarchive '["ballot1", false]' -p testaccounta
```

-----

## Voter Actions

### ACTION `regvoter()`

Registers a voter to a treasury. If the treasury is non-public then a referral is required.

- name `voter`: the name of the voter to register.

- symbol `treasury_symbol`: the treasury symbol registering the voter.

- name `OPTIONAL referrer`: an optional referrer. Some treasuries will require a referral in order to access.

```
cleos push action trailservice regvoter '["testaccountb", "2,TEST", null]' -p testaccountb
```

### ACTION `unregvoter()`

Unregisters a voter. Requires liquid and staked amount to be zero.

- name `voter`: the name of the voter to unregister.

- symbol `treasury_symbol`: the treasury symbol to which the voter is registered.

```
cleos push action trailservice unregvoter '["testaccountb", "2,TEST"]' -p testaccountb
```

### ACTION `castvote()`

Casts a vote on a ballot. If a vote already exists, and the ballot allows revoting, the old vote will be rolled back and the new vote applied.

- name `voter`: the name of the voter casting the vote.

- name `ballot_name`: the name of the ballot receiving the vote.

- vector(name) `options`: a list of options to vote for.

```
cleos push action trailservice castvote '["testaccountb", "ballot1", ["opt1", "opt2"]]' -p testaccountb
```

### ACTION `unvoteall()`

Unvotes all options from a single vote.

- name `voter`: the name of the voter who cast the vote.

- name `ballot_name`: the name of the ballot for which the vote was cast.

```
cleos push action trailservice unvoteall '["testaccountb", "ballot1"]' -p testaccountb
```

### ACTION `stake()`

Stakes a quantity of tokens to a voter's staked amount from their liquid amount.

- name `voter`: the name of the voter staking tokens.

- asset `quantity`: the quantity of tokens to stake.

```
cleos push action trailservice stake '["testaccountb", "5.00 TEST"]' -p testaccountb
```

### ACTION `unstake()`

Unstakes a quantity of tokens from a voter's staked amount to their liquid amount. 

- name `voter`: the name of the voter unstaking tokens.

- asset `quantity`: the quantity of tokens to unstake.

```
cleos push action trailservice unstake '["testaccounb", "5.00 TEST"]' -p testaccountb
```

-----

## Worker Actions

### ACTION `rebalance()`

Rebalances a single vote. If worker name is supplied, credits worker with rebalance work on vote. Subsequent rebalances on the same vote will overwrite the previous work performed. 

- name `voter`: the name of the voter whose vote is being rebalanced.

- name `ballot_name`: the name of the ballot that was voted for.

- name `OPTIONAL worker`: the name of the worker performing the rebalance.

```
cleos push action trailservice rebalance '["testaccountb", "ballot1", "testaccountc"]' -p testaccountc
```

### ACTION `cleanupvote()`

Cleans a single expired vote. If worker name is supplied, credits worker with cleanup. If rebalance work was done on the vote, that work is credited to the rebalance worker.

- name `voter`: the name of the voter whose vote to clean.

- name `ballot_name`: the name of the ballot on the vote to clean up.

- name `OPTIONAL worker`: the name of the worker performing the cleanup.

```
cleos push action trailservice cleanupvote '["testaccounta", "ballot1", "testaccountb"]' -p testaccountb
```

### ACTION `forfeitwork()`

Forfeits all unclaimed payments from a single worker.

- name `worker_name`: the name of the worker to unregister.

- symbol `treasury_symbol`: the treasury symbol from which to forfeit work.

Required Authority: `worker_name`

```
cleos push action trailservice forfeitwork '["testaccountb", "2,TEST"]' -p testaccountb
```

### ACTION `claimpayment()`

Claims a share of available payroll funds. Payments can be claimed for each treasury where work was performed.

- name `worker_name`: the name of the worker claiming a payment.

- symbol `treasury_symbol`: the treasury symbol for which work was done.

```
cleos push action trailservice claimpayment '["testaccountb", "2,TEST"]' -p testaccountb
```

### ACTION `withdraw()`

Withdraws a quantity of tokens from the voter's TLOS balance out of Trail.

- name `voter`: the voter account to withdraw from.

- asset `quantity`: the amount of TLOS to withdraw.

Required Authority: `voter`

```
cleos push action trailservice withdraw '["testaccounta", "5.0000 TLOS"]' -p testaccounta
```

-----

## Committee Actions

### ACTION `regcommittee()`

Registers a new committee and populates it with initial seats.

- name `committee_name`: the name of the new commitee.

- string `committee_title`: the title of the committee.

- symbol `treasury_symbol`: the treasury symbol that the committee will belong to.

- vector(name) `initial_seats`: a list of initial seat names to add to the committee.

- name `registree`: the name of the account registering the committee and paying the registration fee.

Required Fee: `100 TLOS`

Required Authority: `registree`

```
cleos push action trailservice regcommittee '["jedicouncil", "Telos Jedi Council", "2,TEST", ["seat1", "seat2"], "testaccountc"]' -p testaccountc
```

### ACTION `addseat()`

Adds a seat to the committee.

- name `committee_name`: the name of the committee.

- symbol `treasury_symbol`: the treasury symbol that the committee belongs to.

- name `new_seat_name`: the name of the new seat to add to the committee.

Required Authority: `updater_acct@updater_auth`

```
cleos push action trailservice addseat '["jedicouncil", "2,TEST", "seat3"]' -p testaccountc
```

### ACTION `removeseat()`

Removes a seat from a committee.

- name `committee_name`: the name of the committee.

- symbol `treasury_symbol`: the treasury symbol that the comittee belongs to.

- name `seat_name`: the name of the seat to remove from the committee.

Required Authority: `updater_acct@updater_auth`

```
cleos push action trailservice removeseat '["jedicouncil", "2,TEST", "seat3"]' -p testaccountc
```

### ACTION `assignseat()`

Assigns an account name to a seat name.

- name `committee_name`: the name of the committee.

- symbol `treasury_symbol`: the treasury symbol that the committee belongs to.

- name `seat_name`: the name of the seat being assigned.

- name `seat_holder`: the name of the new seat holder.

- string `memo`: a memo describing the seat assignment.

Required Authority: `updater_acct@updater_auth`

```
cleos push action trailservice assignseat '["jedicouncil", "2,TEST", "seat1", "yoda", "first jedi council election"]' -p testaccountc
```

### ACTION `setupdater()`

Sets a new updater account and updater authority. The committee updater is the only account allowed to add, remove, or assign seats, as well as set a new updater or delete the committee.

- name `committee_name`: the name of the committee.

- symbol `treasury_symbol`: the treasury symbol the committee belongs to.

- name `updater_account`: the new account name required for updating the committee.

- name `updater_auth`: the new authority name required for updating the committee.

- string `memo`: a memo describing the change.

Required Authority: `updater_acct@updater_auth`

```
cleos push action trailservice setupdater '["jedicouncil", "2,TEST", "testaccountb", "active", "hand off authority"]' -p testaccountc
```

### ACTION `delcommittee()`

Deletes a committee.

- name `committee_name`: the name of the committee to delete.

- symbol `treasury_symbol`: the treasury symbol that the committee belongs to.

- string `memo`: a memo for describing the committee deletion.

Required Authority: `updater_acct@updater_auth`

```
cleos push action trailservice delcommittee '["jedicouncil", "2,TEST", "order 66 executed"]' -p testaccountc
```

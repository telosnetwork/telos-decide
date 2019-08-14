# Trail Contract API

This guide outlines all the contract actions available on the Trail Voting Platform.

-----

## Admin Actions

The following actions manage the platform configuration.

### ACTION `setconfig()`

Sets the data in the contract's config singleton.

- `trail_version` is the version number of the deployed Trail contract.

- `set_defaults` sets default values if true.

    ```
    cleos push action trailservice setconfig '["v2.0.0-RFC1", true]' -p trailservice
    ```

### ACTION `updatefee()`

Updates a config fee.

- `fee_name` is the name of the fee to update.

- `fee_amount` is the new fee amount.

    ```
    cleos push action trailservice updatefee '["ballot", "35.0000 TLOS"]' -p trailservice
    ```

### ACTION `updatetime()`

Updates a config time.

- `time_name` is the name of the config time.

- `length` is the time length in seconds.

    ```
    cleos push action trailservice updatetime '["balcooldown", 86400]' -p trailservice
    ```

-----

## Registry Actions

The following actions are used to create and manage registries.

### ACTION `newregistry()`

Creates a new Registry with the given access method and max supply. 

- `manager` is the account that will manage the new registry.

- `max_supply` is the maximum number of tokens allowed in circulation at one time.

- `access` is the access method for acquiring a balance of the registry's tokens.

    ```
    cleos push action trailservice newregistry '["youraccount", "2,TEST", "public"]' -p manager
    ```

### ACTION `togglereg()`

Toggles a registry setting on and off.

- `registry_symbol` is the registry with the setting to toggle.

- `setting_name` is the name of the setting.

    ```
    cleos push action trailservice togglereg '["2,TEST", "transferable"]' -p manager
    ```

### ACTION `mint()`

Mints new tokens into circulation.

- `to` is the account to receive the minted tokens.

- `quantity` is the amount of tokens to mint.

- `memo` is a memo.

    ```
    cleos push action trailservice mint '["testaccounta", "5.00 TEST", "testing mint"]' -p manager
    ```

### ACTION `transfer()`

Transfers a quantity of tokens from one voter to another.

- `from` is the account sending the tokens.

- `to` is the account receiving the tokens.

- `quantity` is the quantiy of tokens to transfer.

- `memo` is a memo describing the transfer.

    ```
    cleos push action trailservice transfer '["testaccounta", "testaccountb", "5.00 TEST", "test transfer"]' -p testaccounta
    ```

### ACTION `burn()`

Burns a quantity of tokens from the manager's liquid balance. Reduces the circulating supply of tokens.

- `quantity` is the quantity of tokens to burn.

- `memo` is a memo describing the burn.

    ```
    cleos push action trailservice burn '["5.00 TEST", "test burn"]' -p manager
    ```

### ACTION `reclaim()`

Reclaims tokens from a voter to the registry manager.

- `voter` is the name of the account from which to reclaim.

- `quantity` is the amount of tokens to reclaim.

- `memo` is a memo describing the reclamation.

    ```
    cleos push action trailservice reclaim '["someaccount", "5.00 TEST", "reclaim memo"]' -p manager
    ```

### ACTION `mutatemax()`

Changes the max supply of a registry.

- `new_max_supply` is the new max supply to set.

- `memo` is a memo describing the mutation.

    ```
    cleos push action trailservice mutatemax '["500.00 TEST", "test mutate max"]' -p manager
    ```

### ACTION `setunlocker()`

Sets a new registry unlocker. Only the new unlock_account@unlock_authority will be able to unlock the registry.

- `registry_symbol` is the registry to set the new unlocker.

- `new_unlock_acct` is the new unlock account.

- `new_unlock_auth` is the new unlock authority.

    ```
    cleos push action trailservice setunlocker '["2,TEST", "someaccount", "active"]' -p unlock_acct@unlock_auth
    ```

### ACTION `lockreg()`

Locks a registry, preventing changes to settings.

- `registry_symbol` is the registry to lock.

    ```
    cleos push action trailservice lockreg '["2,TEST"]' -p manager
    ```

### ACTION `unlockreg()`

Unlocks a registry, allowing changes to settings.

- `registry_symbol` is the registry to unlock.

    ```
    cleos push action trailservice unlockreg '["2,TEST"]' -p unlock_acct@unlock_auth
    ```

### ACTION `addtofund()`

Adds funds to a registry's worker fund. Funds will be charged from the voter's TLOS balance.

- `registry_symbol` is the registry receiving the funds.

- `voter` is the voter account sending the funds.

- `quantity` is the number of tokens to add to the worker fund.

    ```
    cleos push action trailservice addtofund '["2,TEST", "testaccounta", "50.0000 TLOS"]' -p publisher
    ```

-----

## Ballot Actions

The following are actions for creating and managing ballots.

### ACTION `newballot()`

Creates a new ballot with the initial options.

- `ballot_name` is the name of the new ballot.

- `category` is the category of the new ballot.

- `publisher` is the ballot publisher.

- `registry_symbol` is the registry symbol the ballot will use to count votes.

- `voting_method` is the method of weighting votes.

- `initial_options` is a list of initial options to place on the ballot.

    ```
    cleos push action trailservice newballot '["ballot1", "poll", "testaccounta", "2,TEST", "quadratic", ["opt1", "opt2"]]' -p testaccounta
    ```

### ACTION `editdetails()`

Edits the Title, Description, and Info for a ballot.

- `ballot_name` is the name of the ballot.

- `title` is the new ballot title.

- `description` is the new ballot description.

- `ballot_info` is the new ballot info.

    ```
    cleos push action trailservice editdetails '["ballot1", "Ballot 1 Example", "Example Description", "somewebsite"]' -p testaccounta
    ```

### ACTION `togglebal()`

Toggles a ballot setting on or off.

- `ballot_name` is the name of the ballot.

- `setting_name` is the name of the setting to toggle.

    ```
    cleos push action trailservice togglebal '["ballot1", "lightballot"]' -p testaccounta
    ```

### ACTION `editmaxopts`

Changes the max number of options a single voter can vote for.

- `ballot_name` is the name of the ballot to change.

- `new_max_options` is the new max number of options a voter can select.

    ```
    cleos push action trailservice editmaxopts '["ballot1", 3]' -p testaccounta
    ```

### ACTION `addoption()`

Adds an option to a ballot.

- `ballot_name` is the name of the ballot to add an option to.

- `new_option_name` is the name of the new ballot option.

    ```
    cleos push action trailservice addoption '["ballot1", "opt3"]' -p testaccounta
    ```

### ACTION `rmvoption()`

Removes an option from a ballot.

- `ballot_name` is the name of the ballot to remove the option from.

- `option_name` is the option name to remove.

    ```
    cleos push action trailservice rmvoption '["ballot1", "opt3"]' -p testaccounta
    ```

### ACTION `readyballot()`

Readies a ballot for voting and sets the ballot end time.

- `ballot_name` is the name of the ballot to ready.

- `end_time` is the time point at which to end voting.

    ```
    cleos push action trailservice readyballot '["ballot1", "2019-08-08T23:41:00"]' -p testaccounta
    ```

### ACTION `cancelballot()`

Cancels a running ballot.

- `ballot_name` is the name of the ballot to cancel.

- `memo` is a memo describing the cancellation.

    ```
    cleos push action trailservice '["ballot1", "cancellation reason here"]' -p testaccounta
    ```

### ACTION `deleteballot()`

Deletes a ballot.

- `ballot_name` is the name of the ballot to delete.

    ```
    cleos push action trailservice deleteballot '["ballot1"]' -p testaccounta
    ```

### ACTION `postresults()`

Posts the final results of a light ballot.

- `ballot_name` is the name of the ballot.

- `light_results` is a map of the final results calcualted off-chain.

- `total_voters` is the total number of unique voters who voted on the light ballot.

    ```
    cleos push action trailservice postresults '["ballot1", [{"opt1": "25.00 TEST"},{"opt2", "15.00 TEST"}], 8]' -p trailservice
    ```

### ACTION `closeballot()`

Closes a ballot and renders the final results.

- `ballot_name` is the name of the ballot to close.

- `broadcast` will inline the bcastresults() action if true.

    ```
    cleos push action trailservice closeballot '["ballot1", true]' -p testaccounta
    ```

### ACTION `bcastresults()`

Broadcasts ballot results and notifies the ballot publisher.

- `ballot_name` is the name of the ballot being broadcast.

- `final_results` is a map of the final weighted ballot results.

- `total_voters` is the total number of unique voters who voted on the ballot.

    ```
    Inline from closeballot()
    ```

### ACTION `archive()`

Archives a ballot. A flat fee is charged per day of archival.

- `ballot_name` is the name of the ballot to archive.

- `archived_until` is the time point at which the ballot can be unarchived.

    ```
    cleos push action trailservice archive '["ballot1", "2019-09-08T23:41:00"]' -p testaccounta
    ```

### ACTION `unarchive()`

Unarchives a ballot. If force is true, will unarchive before the unarchival date.

- `ballot_name` is the name of the ballot being archived.

- `force` forces the ballot to unarchive if true, even before the unarchival date.

    ```
    cleos push action trailservice unarchive '["ballot1", false]' -p testaccounta
    ```

-----

## Voter Actions

The following actions create and manage voters.

### ACTION `regvoter()`

Registers a voter to a registry. If the registry is non-public then a referral is required.

- `voter` is the name of the voter to register.

- `registry_symbol` is the registry symbol registering the voter.

- `OPTIONAL referrer` is an optoinal referrer. Some registries will require a referral in order to access.

    ```
    cleos push action trailservice regvoter '["testaccountb", "2,TEST", null]' -p testaccountb
    ```

### ACTION `unregvoter()`

Unregisters a voter. Requires liquid and staked amount to be zero.

- `voter` is the name of the voter to unregister.

- `registry_symbol` is the registry symbol to which the voter is registered.

    ```
    cleos push action trailservice unregvoter '["testaccountb", "2,TEST"]' -p testaccountb
    ```

### ACTION `castvote()`

Casts a vote on a ballot. If a vote already exists it will be rolled back and the new vote applied.

- `voter` is the name of the voter casting the vote.

- `ballot_name` is the name of the ballot receiving the vote.

- `options` is a vector of options to vote for.

    ```
    cleos push action trailservice castvote '["testaccountb", "ballot1", ["opt1", "opt2"]]' -p testaccountb
    ```

### ACTION `unvoteall()`

Unvotes all options from a single vote.

- `voter` is the name of the voter who cast the vote.

- `ballot_name` is the name of the ballot for which the vote was cast.

    ```
    cleos push action trailservice unvoteall '["testaccountb", "ballot1"]' -p testaccountb
    ```

### ACTION `stake()`

Stakes a quantity of tokens to a voter's staked amount from their liquid amount.

- `voter` is the name of the voter staking tokens.

- `quantity` is the quantity of tokens to stake.

    ```
    cleos push action trailservice stake '["testaccountb", "5.00 TEST"]' -p testaccountb
    ```

### ACTION `unstake()`

Unstakes a quantity of tokens from a voter's staked amount to their liquid amount. 

- `voter` is the name of the voter unstaking tokens.

- `quantity` is the quantity of tokens to unstake.

    ```
    cleos push action trailservice unstake '["testaccounb", "5.00 TEST"]' -p testaccountb
    ```

-----

## Worker Actions

The following actions create and manage workers.

### ACTION `regworker()`

Registers a new worker.

- `worker_name` is the name of the worker. Worker name must match voter name.

    ```
    cleos push action trailservice regworker '["testaccountb"]' -p testaccountb
    ```

### ACTION `unregworker()`

Unregisters a worker and forfeits all unclaimed payments. Requires the authority of the worker.

- `worker_name` is the name of the worker to unregister.

    ```
    cleos push action trailservice unregworker '["testaccountb"]' -p testaccountb
    ```

### ACTION `claimpayment()`

Claims a share of worker funds. Payments can be claimed for each registry where work was performed.

- `worker_name` is the name of the worker claiming a payment.

- `registry_symbol` is the registry symbol for which work was done.

    ```
    cleos push action trailservice claimpayment '["testaccountb", "2,TEST"]' -p testaccountb
    ```

### ACTION `rebalance()`

Rebalances a single vote. If worker name is supplies, credits worker with rebalance work on vote.

- `voter` is the name of the voter whose vote is being rebalanced.

- `ballot_name` is the name of the ballot that was voted for.

- `OPTIONAL worker` is the name of the worker performing the rebalance.

    ```
    cleos push action trailservice rebalance '["testaccountb", "ballot1", "testaccountc"]' -p testaccountc
    ```

### ACTION `cleanupvote()`

Cleans a single expired vote. If worker name is supplied, credits worker with cleanup. If rebalance work was done on the vote, that work is credited to the rebalance worker.

- `voter` is the name of the voter whose vote is being cleaned.

- `ballot_name` is the name of the ballot that was voted for.

- `OPTIONAL worker` is the name of the worker performing the cleanup.

    ```
    cleos push action trailservice cleanupvote '["testaccounta", "ballot1", "testaccountb"]' -p testaccountb
    ```

### ACTION `withdraw()`

Withdraws a quantity of tokens from the voter's TLOS balance out of Trail.

- `voter` is the voter account to withdraw from.

- `quantity` is the amount of TLOS to withdraw.

    ```
    cleos push action trailservice withdraw '["testaccounta", "5.0000 TLOS"]' -p testaccounta
    ```

-----

## Committee Actions

The following actions are used for creating and managing committees.

### ACTION `regcommittee()`

Register a new committee and populates it with initial seats. Requires the authority of the registree.

- `committee_name` is the name of the new commitee.

- `committee_title` is the title of the committee.

- `registry_symbol` is the registry symbol that the committee will belong to.

- `initial_seats` is a vector of initial seat names to add to the committee.

- `registree` is the name of the account registering the committee.

    ```
    cleos push action trailservice regcommittee '["jedicouncil", "Coruscant Jedi Council", "2,TEST", ["seat1", "seat2"], "testaccountc"]' -p testaccountc
    ```

### ACTION `addseat()`

Adds a seat to the committee. Requires the authority of the committee updater.

- `committee_name` is the name of the committee.

- `registry_symbol` is the registry symbol that the committee belongs to.

- `new_seat_name` is the name of the new seat to add to the committee.

    ```
    cleos push action trailservice addseat '["jedicouncil", "2,TEST", "seat3"]' -p testaccountc
    ```

### ACTION `removeseat()`

Remove a seat from a committee. Requires the authority of the committee updater.

- `committee_name` is the name of the committee.

- `registry_symbol` is the registry symbol that the comittee belongs to.

- `seat_name` is the name of the seat to remove from the committee.

    ```
    cleos push action trailservice removeseat '["jedicouncil", "2,TEST", "seat3"]' -p testaccountc
    ```

### ACTION `assignseat()`

Assigns an account name to a seat name. Requires the authority of the committee updater.

- `committee_name` is the name of the committee.

- `registry_symbol` is the registry symbol that the committee belongs to.

- `seat_name` is the name of the seat being assigned.

- `seat_holder` is the name of the new seat holder.

- `memo` is a memo describing the seat assignment.

    ```
    cleos push action trailservice assignseat '["jedicouncil", "2,TEST", "seat1", "yoda", "first jedi council election"]' -p testaccountc
    ```

### ACTION `setupdater()`

Sets a new updater account and updater authority. The committee updater is the only account allowed to add, remove, or assign seats, as well as set a new updater or delete the committee.

- `committee_name` is the name of the committee.

- `registry_symbol` is the registry symbol the committee belongs to.

- `updater_account` is the new account name required for updating the committee.

- `updater_auth` is the new authority name required for updating the committee.

- `memo` is a memo describing the change.

    ```
    cleos push action trailservice setupdater '["jedicouncil", "2,TEST", "testaccountb", "active", "hand off authority"]' -p testaccountc
    ```

### ACTION `delcommittee()`

Deletes a committee. Requires the authority of the committee updater.

- `committee_name` is the name of the committee to delete.

- `registry_symbol` is the registry symbol that the committee belongs to.

- `memo` is a memo for describing the committee deletion.

    ```
    cleos push action trailservice delcommittee '["jedicouncil", "2,TEST", "order 66 executed"]' -p testaccountc
    ```

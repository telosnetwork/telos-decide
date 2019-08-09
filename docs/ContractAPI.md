# Trail Contract API

...

## Admin Actions

...

### ACTION `setconfig()`

Sets the data in the contract's config singleton.

- `trail_version` is the version number of the deployed Trail contract.

- `ballot_fee` is the fee collected for creating a new ballot with the `newballot()` action.

- `registry_fee` is the fee collected for creating a new registry with the `newregistry()` action.

- `archival_fee` is the fee collected for archiving a ballot with the `archive()` action.

- `min_ballot_length` is the minimum length of time (in seconds) that a ballot may be open for voting.

- `ballot_cooldown` is the length of time that must pass (in seconds) after closing before a ballot can be deleted.

    cleos push action trailservice setconfig '[...]' -p trailservice

## Registry Actions

...

### ACTION `newregistry()`

...

- `manager` is the account that will manage the new registry.

- `max_supply` is the maximum number of tokens allowed in circulation at one time.

- `access` is the access method for acquiring a balance of the registry's tokens.

    cleos push action trailservice newregistry '["youraccount", "2,TEST", "public"]' -p manager

### ACTION `togglereg()`

...

- `registry_symbol`

- `setting_name`

    cleos push action trailservice togglereg '["2,TEST", "transferable"]' -p manager

### ACTION `mint()`

...

- `to` is the account to receive the minted tokens.

- `quantity` is the amount of tokens to mint.

- `memo` is a memo.

    cleos push action trailservice mint '["testaccounta", "5.00 TEST", "testing mint"]' -p manager

### ACTION `transfer()`

...

- `from`

- `to`

- `quantity`

- `memo`

    cleos push action trailservice transfer '["testaccounta", "testaccountb", "5.00 TEST", "test transfer"]' -p testaccounta

### ACTION `burn()`

...

- `quantity`

- `memo`

    cleos push action trailservice burn '["5.00 TEST", "test burn"]' -p manager

### ACTION `reclaim()`

...

- `voter`

- `quantity`

- `memo`

    cleos push action trailservice reclaim '["someaccount", "5.00 TEST", "reclaim memo"]' -p manager

### ACTION `mutatemax()`

...

- `new_max_supply`

- `memo`

    cleos push action trailservice mutatemax '["500.00 TEST", "test mutate max"]' -p manager

### ACTION `setunlocker()`

...

- `registry_symbol`

- `new_unlock_acct`

- `new_unlock_auth`

    cleos push action trailservice setunlocker '["2,TEST", "someaccount", "active"]' -p unlock_acct@unlock_auth

### ACTION `lockreg()`

...

- `registry_symbol`

    cleos push action trailservice lockreg '["2,TEST"]' -p manager

### ACTION `unlockreg()`

...

- `registry_symbol`

    cleos push action trailservice unlockreg '["2,TEST"]' -p unlock_acct@unlock_auth

### ACTION `addtofund()`

...

- `registry_symbol`

- `voter`

- `quantity`

    cleos push action trailservice addtofund '["2,TEST", "voteraccount", "quantity"]' -p publisher

## Ballot Actions

...

### ACTION `newballot()`

...

- `ballot_name`

- `category`

- `publisher`

- `registry_symbol`

- `voting_method`

- `initial_options`

    cleos push action trailservice newballot '["ballot1", "poll", "testaccounta", "2,TEST", "quadratic", ["opt1", "opt2"]]' -p testaccounta

### ACTION `editdetails()`

...

- `ballot_name`

- `title`

- `description`

- `ballot_info`

    cleos push action trailservice editdetails '["ballot1", "Ballot 1 Example", "Example Description", "somewebsite"]' -p testaccounta

### ACTION `togglebal()`

...

- `ballot_name`

- `setting_name`

    cleos push action trailservice togglebal '["ballot1", "lightballot"]' -p testaccounta

### ACTION `editmaxopts`

...

- `ballot_name`

- `new_max_options`

    cleos push action trailservice editmaxopts '["ballot1", 3]' -p testaccounta

### ACTION `addoption()`

...

- `ballot_name`

- `new_option_name`

    cleos push action trailservice addoption '["ballot1", "opt3"]' -p testaccounta

### ACTION `rmvoption()`

...

- `ballot_name`

- `option_name`

    cleos push action trailservice rmvoption '["ballot1", "opt3"]' -p testaccounta

### ACTION `readyballot()`

...

- `ballot_name`

- `end_time`

    cleos push action trailservice readyballot '["ballot1", "2019-08-08T23:41:00"]' -p testaccounta

### ACTION `cancelballot()`

...

- `ballot_name`

- `memo`

    cleos push action trailservice '["ballot1", "cancellation reason here"]' -p testaccounta

### ACTION `deleteballot()`

...

- `ballot_name`

    cleos push action trailservice deleteballot '["ballot1"]' -p testaccounta

### ACTION `closeballot()`

...

- `ballot_name`

- `post_results`

    cleos push action trailservice closeballot '["ballot1", true]' -p testaccounta

### ACTION `postresults()`

...

- `ballot_name`

- `final_results`

- `voting_method`

- `total_voters`

    //requires trailservice@active
    cleos push action trailservice postresults '["ballot1", [{"opt1": "25.00 TEST"},{"opt2", "15.00 TEST"}], "quadratic", 8]' -p trailservice

### ACTION `archive()`

...

- `ballot_name`

- `archived_until`

    cleos push action trailservice archive '["ballot1", "2019-09-08T23:41:00"]' -p testaccounta

### ACTION `unarchive()`

...

- `ballot_name`

- `force_unarchive`

    cleos push action trailservice unarchive '["ballot1", false]' -p testaccounta

## Voter Actions

...

### ACTION `regvoter()`

- `voter`

- `registry_symbol`

- `referrer` OPTIONAL

    cleos push action trailservice regvoter '["testaccountb", "2,TEST", null]' -p testaccountb

### ACTION `unregvoter()`

...

- `voter`

- `registry_symbol`

    cleos push action trailservice unregvoter '["testaccountb", "2,TEST"]' -p testaccountb

### ACTION `castvote()`

...

- `voter`

- `ballot_name`

- `options`

    cleos push action trailservice castvote '["testaccountb", "ballot1", ["opt1", "opt2"]]' -p testaccountb

### ACTION `unvoteall()`

...

- `voter`

- `ballot_name`

    cleos push action trailservice unvoteall '["testaccountb", "ballot1"]' -p testaccountb

### ACTION `stake()`

...

- `voter`

- `quantity`

    cleos push action trailservice stake '["testaccountb", "5.00 TEST"]' -p testaccountb

### ACTION `unstake()`

...

- `voter`

- `quantity`

    cleos push action trailservice unstake '["testaccounb", "5.00 TEST"]' -p testaccountb

## Worker Actions

...

### ACTION `regworker()`

...

- `worker_name`

    cleos push action trailservice regworker '["testaccountb"]' -p testaccountb

### ACTION `unregworker()`

...

- `worker_name`

    cleos push action trailservice unregworker '["testaccountb"]' -p testaccountb

### ACTION `claimpayment()`

...

- `worker_name`

- `registry_symbol`

    cleos push action trailservice claimpayment '["testaccountb", "2,TEST"]' -p testaccountb

### ACTION `rebalance()`

...

    cleos push action trailservice rebalance ...

### ACTION `cleanupvote()`

...

    cleos push action trailservice cleanupvote ...

## Committee Actions

...

### ACTION `regcommittee()`

...

- `committee_name`

- `committee_title`

- `registry_symbol`

- `initial_seats`

- `registree`

    cleos push action trailservice regcommittee '["jedicouncil", "Coruscant Jedi Council", "2,TEST", ["seat1", "seat2"], "testaccountc"]' -p testaccountc

### ACTION `addseat()`

...

- `committee_name`

- `registry_symbol`

- `new_seat_name`

    cleos push action trailservice addseat '["jedicouncil", "2,TEST", "seat3"]' -p testaccountc

### ACTION `removeseat()`

...

- `committee_name`

- `registry_symbol`

- `seat_name`

    cleos push action trailservice removeseat '["jedicouncil", "2,TEST", "seat3"]' -p testaccountc

### ACTION `assignseat()`

...

- `committee_name`

- `registry_symbol`

- `seat_name`

- `seat_holder`

- `memo`

    cleos push action trailservice assignseat '["jedicouncil", "2,TEST", "seat1", "yoda", "first jedi council election"]' -p testaccountc

### ACTION `setupdater()`

...

- `committee_name`

- `registry_symbol`

- `updater_account`

- `updater_auth`

- `memo`

    cleos push action trailservice setupdater '["jedicouncil", "2,TEST", "testaccountb", "active", "hand off authority"]' -p testaccountc

### ACTION `delcommittee()`

...

- `committee_name`

- `registry_symbol`

- `memo`

    cleos push action trailservice delcommittee '["jedicouncil", "2,TEST", "order 66 executed"]' -p testaccountc


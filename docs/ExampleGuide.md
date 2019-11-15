# Example Guide

For this example, we will assume we have control of the `testaccounta` account on Telos Testnet and own a balance of `1200 TLOS`.

## 1. Deposit Funds

To pay for the necessary registration fees, we'll deposit our 1200 TLOS into Trail by simply sending a `transfer()` with `trailservice` as the recipient.

```
cleos push action eosio.token transfer '["testaccounta", "trailservice", "1200.0000 TLOS", "deposit"]' -p testaccounta
```

Trail will catch this transfer and create an account with your funds. From there, certain actions will require a fee (creating a treasury, committee, or ballot) that will be drawn from this balance. 

There is no TLOS fee for depositing to or withdrawing from Trail, and both can be done at any time.

## 2. Treasury Creation

To create the example treasury we simply call the `newtreasury()` action on the `trailservice` contract. We will supply the following arguments:

* manager: `testaccounta`

    We will make ourself the manager, since we want to have control over the treasury operations for now.

* max_supply: `500.0 EXMPL`

    We will limit the supply to 500.0 EXMPL, but we can adjust this later.

* access: `public`

    We will choose public, since we want anyone to be able to register themselves and join as a voter.

```
cleos push action trailservice newtreasury '["testaccounta", "500.0 EXMPL", "public"]' -p testaccounta
```

This will create a new public treasury with a max supply of 500.0 EXMPL tokens. Note that the treasury symbol is `1,EXMPL` since we chose to use 1 decimal place of precision and the EXMPL ticker.

Cost: `1000 TLOS`

## 3. Committee Registration

To create the example committee we will call the `regcommittee()` action on the `trailservice` contract. We will supply the following arguments:

* committee_name: `examplecmte`

    This will be the committee identifier used to locate the committee in the contract table.

* committee_title: "Example Committee"

    This is the human readable title of the committee.

* treasury_symbol: `1,EXMPL`

    The treasury symbol we created with the newtreasury() action. Don't forget the precision.

* initial_seats: `["seat1", "seat2", "seat3"]`

    The seat names initially available after successful committee creation. More can be added later.

* registree: `testaccounta`

    The account registering the committee. This account will be billed for the committee creation.

```
cleos push action trailservice regcommittee '["examplecmte", "Example Committee", "1,EXMPL", ["seat1", "seat2", "seat3"], "testaccounta"]' -p testaccounta
```

Cost: `100 TLOS`

## 4. Ballot Creation

To create the example ballot we will call the `newballot()` action on the `trailservice` contract. We will supply the following arguments:

* ballot_name: `examplebal`

    The name identifier for our ballot. This must be unique.

* category: `election`

    The category the ballot falls under. Since the winning candidate on our ballot is going to be the first member of our committee, the election category is most appropriate.

* publisher: `testaccounta`

    The account who will set up and launch the ballot, and has the ability to cancel or delete if desired.

* treasury_symbol: `1,EXMPL`

    The treasury that the ballot will be ran under. We will use the treasury we made earlier, and this means any `EXMPL` holder, including ourselves, will be able to cast a vote on this ballot.

* voting_method: `1token1vote`

    We will choose 1token1vote, since we want votes for multiple candidates to have the voter's weight cast evenly between the selections. This is a design choice, and any other voting method could have been selected.

* initial_options: `["testaccountx", "testaccounty", "testaccountz"]`

    We will place test accounts x, y, and z to be placed as options on the ballot. We could add more before opening up the ballot for voting, but won't for simplicity's sake.

```
cleos push action trailservice newballot '["examplebal", "election", "testaccounta", "1,EXMPL", "1token1vote", ["testaccountx", "testaccounty", "testaccountz"]]' -p testaccounta
```

Once the ballot has been registered, don't forget to actually begin voting be calling `openvoting()`. This can be done at any time, so we will wait until we finish setting up our example contract.

Cost: `30 TLOS`

## 5. Example Contract Build and Deployment

To build the example contract we will use the `build.sh` provided in this repository's contracts folder, but you can build with the `eosio-cpp` tool in the `eosio.cdt` if that is preferable.

```
mkdir build && mkdir build/example

chmod +x build.sh

./build.sh example
```

To deploy the example contract to our `testaccounta` account, we will use the provided `deploy.sh` script, but you can deploy manually with the `cleos set contract` command as well.

```
chmod +x deploy.sh

./deploy.sh example testnet
```

Once the contract is deployed we need to tell it to watch our new ballot and listen for the `broadcast()` action. To do this we simply call the `watchballot()` action. We will be pushing this transaction to the `testaccounta` account, since that's where we deployed our example contract. We'll give it the following arguments:

* ballot_name: `examplebal`

    Tells our example contract to watch the examplebal ballot on Trail.

* treasury_symbol: `1,EXMPL`

    The treasury symbol used on the ballot.

* committee_name: `examplecmte`

    The committee to update with the ballot winner.

* seat_name: `seat1`

    The name of the seat to assign the winner to.

```
cleos push action testaccounta watchballot '["examplebal", "1,EXMPL", "examplecmte", "seat1"]' -p testaccounta
```

Next, after telling our contract to watch our Trail ballot, we need to give it permission to execute an inline action to Trail's `assignseat()` action so we can immediately elect our winning candidate. To do this we want to add the virtual `eosio.code` permission to our contract's active authority with the `cleos set account permission` command, like so:

```
cleos set account permission testaccounta active --add-code -p testaccounta
```

## 6. Open Ballot Voting

Now that we have our example contract set up and watching our ballot, we can finally open up our ballot for voting. This is done by simply sending an `openvoting()` action to Trail.

Be aware that once voting has begun no further changes to the ballot may be performed. This is to preserve the integrity of the ballot and ensure voters can't be deceived by altering ballots mid-vote. Ballots may still be cancelled, however.

We will supply the following arguments to our `openvoting()` action:

* ballot_name: `examplebal`

    This is the name of the ballot to open for voting. Only the ballot publisher will be able to open ballots for voting.

* end_time: `2019-11-14T13:00:00`

    This is the time that voting will close in UTC time. Format is `YYYY-MM-DDTHH:MM:SS`

```
cleos push action trailservice openvoting '["examplebal", "2019-11-14T13:00:00"]' -p testaccounta
```

Now that voting has been opened on our ballot, all that's left to do is wait for votes to roll in. Results can be seen in real time, as the ballot tracks the live vote count for each ballot option.

## 7. Closing Out The Ballot

Once the end time on the ballot has been reached, it will immediately stop accepting votes. The ballot can be properly closed out and the results broadcasted by calling the `closevoting()` action. We will supply the following arguments:

* ballot_name: `examplebal`

    The name of the ballot to close.

* broadcast: `true`

    Since we want to broadcast the results back to our example contract, we will give true here.

Once complete, the ballot status will be changed to `closed` to show the ballot has been properly ended.

Since we chose to broadcast our results, an inline action has automatically been launched by Trail to its own `broadcast()` action that then notifies the ballot publisher account where our example contract is located.

The example contract we deployed earlier to the `testaccounta` account catches this `broadcast()` notification from Trail and parses the final ballot results to determine the winner. Once determiend, our contract automatically executes an inline action to Trail's `assignseat()` action, which emplaces the winner into `seat1` on the example committee.

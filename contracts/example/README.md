# Example Setup

### Registry Creation

To create the example registry we simply call the `newregistry()` action on the `trailservice` contract. We will supply the following arguments:

- manager: `testaccounta`

- max_supply: `500.0 EXMPL`

- access: `public`

```
cleos push action trailservice newregistry '["testaccounta", "500.0 EXMPL", "public"]' -p testaccounta
```

This will create a new public registry with a max supply of 500.0 EX tokens. Note that the registry symbol is `1,EXMPL` since we chose to use 1 decimal place of precision and the EXMPL ticker.

### Committee Registration

To create the example committee we will call the `regcommittee()` action on the `trailservice` contract. We will supply the following arguments:

- committee_name: `examplecmte`

- committee_title: "Example Committee"

- registry_symbol: `1,EXMPL`

- initial_seats: `["seat1", "seat2", "seat3"]`

- registree: `testaccounta`

```
cleos push action trailservice regcommittee '["examplecmte", "Example Committee", "1,EXMPL", ["seat1", "seat2", "seat3"], "testaccounta"]' -p testaccounta
```

### Ballot Registration

To create the example ballot we will call the `newballot()` action on the `trailservice` contract. We will supply the following arguments:

- ballot_name: `examplebal`

- category: `election`

- publisher: `testaccounta`

- registry_symbol: `1,EXMPL`

- voting_method: `1token1vote`

- initial_options: `["testaccountx", "testaccounty", "testaccountz"]`

```
cleos push action trailservice newballot '["examplebal", "election", "testaccounta", "1,EXMPL", "1token1vote", ["testaccountx", "testaccounty", "testaccountz"]]' -p testaccounta
```

Once the ballot has been registered, don't forget to actually begin voting be calling `readyballot()`. This can be done at any time, so we will wait until we finish setting up our example contract.

### Example Contract Build and Deployment

To build the example contract we will use the `build.sh` provided in this repository's contracts folder, but you can build with the `eosio-cpp` tool in the `eosio.cdt` if that is preferable.

```
cd contracts

chmod +x build.sh

./build.sh example
```

To deploy the example contract we will use the provided `deploy.sh` script, but you can deploy manually with the `cleos set contract` command as well.

```
cd contracts

chmod +x deploy.sh

./deploy.sh example testnet
```

Once the contract is deployed we need to tell it to watch our new ballot and listen for the `bcastresults()` action. To do this we simply call the `watchballot()` action. We will be pushing this transaction to the `testaccounta` account, since that's where we deployed our example contract. We'll give it the following arguments:

- ballot_name: `examplebal`

- registry_symbol: `1,EXMPL`

- committee_name: `examplecmte`

- seat_name: `seat1`

```
cleos push action testaccounta watchballot '["examplebal", "1,EXMPL", "examplecmte", "seat1"]' -p testaccounta
```

Lastly, after telling our contract to watch our Trail ballot, we need to give it permission to execute the inline action to Trail's `assignseat()` action so we can immediately elect our winning candidate. To do this we want to add the virtual `eosio.code` permission to our contract's active authority with the `cleos set account permission` command, like so:

```
cleos set account permission testaccounta active --add-code -p testaccounta
```

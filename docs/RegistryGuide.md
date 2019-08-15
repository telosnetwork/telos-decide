# Trail Registry Guide

In this guide we will explore all the different interactions a Registry Manager can perform on the Trail Voting Platform.

#### What is a Registry?

A Trail Registry is a place to keep track of data about a certain family of token. Information like the current supply of tokens, the max supply, the registry settings, and who the registry manager is are all tracked on the Token Registry. Note that tokens cannot exist on Trail without an accompanying Registry.

#### What is a Registry Symbol?

All Registries are identified by the symbol of the tokens they manage - also called a Registry Symbol. 

Note that Registry Symbols contain the precision amount of the token as well as the actual ticker name. So a `1.000 TEST` token would have a symbol of: `3,TEST`. The number 3 for the three decimal places of precision, followed by a comma with no space, then the ticker symbol in all caps.

## Getting Started

Follow these steps to get started creating your own Trail Registry:

### 1. Registry Setup

Any user on the Trail Voting Platform may create a new token registry, complete with all the features Trail provides. Before creating a registry, the creator needs to know a few things first: 

- **Manager:** the account that will be the manager of the registry. This account is allowed to do things like mint new tokens, change settings, or lock the registry.

- **Max Supply:** the total number of tokens allowed to exist at one time. The registry symbol will be extracted from this value (including the token precision).

- **Access:** the accessibility of the registry. This can be set to public, private, or invite. `Public` means that an account can register as a voter at any time. `Private` means the registry manager will have to approve every new voter before they can register. `Invite` means any voter can invite another voter, provided the inviter is a registered voter themselves.

### 2. Send `newregistry()` Transaction

Once the new settings are decided, the registry is ready for creation. Actually creating it involves sending a `newregistry()` action to the `trailservice` contract and giving it your arguments like so:

```
cleos push action trailservice newregistry '["craig.tf", "500.00 CRAIG", "public"]' -p craig.tf
```

In the above command we are creating the public `CRAIG` registry, managed by the `craig.tf` account, and with a max supply of 500.00 tokens.

Note that sending a `newregistry()` transaction can be done through almost any Telos block explorer, and even some wallets like Sqrl.

### 3. Managing A Registry

After the registry has been successfully created its ready to start holding ballots - but first we have to register voters and mint tokens, otherwise there won't be any voters with our tokens to actually *vote* on our ballots.

In order to mint tokens we have to have a voter to hold them, so lets go ahead and register ourselves. Registering new voters is done by sending a `regvoter()` transaction to the `trailservice` contract. Similar to preparing for sending the `newregistry()` transaction, we also need to know a few pieces of information first:

- **Voter:** the name of the account to register as a voter.

- **Registry Symbol:** the symbol of the registry to register for. This includes the token precision (the number of decimal places the token uses).

- **Referrer:** the account that is referring the new voter, if any. If no referrer, then give `null` as an argument.

Depending on the access method, a registry may require extra signatures from certain voters to qualify. Since we made our registry public we don't need to worry about referrals.

```
cleos push action trailservice regvoter '["craig.tf", "2,CRAIG", null]' -p craig.tf
```



-----

## Registry Table Breakdown

Registries manage a lot of data, so it's important for managers to understand how registries work and how to read registry information so they can best serve their voters.

Table: `registries`

Scope: `trailservice`

| Field | Type | Description |
| --- | --- | --- |
| supply | asset | Current circulating supply of tokens. |
| max_supply | asset | Maximum circulating supply of tokens. |
| voters | uint32 | Number of voters registered. |
| access | name | Access method for becoming a voter. |
| locked | boolean | Locked if true, unlocked if false. |
| unlock_acct | name | Account that can unlock the registry. |
| unlock_auth | name | Authority that can unlock the registry. |
| manager | name | Account that can edit and lock the registry. |
| settings | map | List of registry-wide settings with on/off states. |
| open_ballots | uint16 | Number of ballots open for voting. |
| worker_funds | asset | Amount of TLOS available to pay workers. |
| rebalanced_volume | asset | Volume of tokens rebalanced across all ballots. |
| rebalanced_count| uint32 | Number of rebalance events across all ballots. |
| cleaned_count | uint32 | Number of clean events across all ballots. |

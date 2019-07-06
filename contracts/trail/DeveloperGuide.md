# Trail Service Developer Guide

Trail offers a comprehensive suite of blockchain-based voting services available to any prospective contract developer on the Telos Blockchain Network.

## Understanding Trail's Role

Trail was designed to allow maximum flexibility for smart contract developers, while at the same time consolidating many of the boilerplate functions of voting contracts into a single service. 

This paradigm basically means contract writers can leave the "vote counting" up to Trail and instead focus on how they want to interpret the results of their ballot. Through cross-contract table lookup, any contract can view the results of a ballot and then have their contracts act based on those results.

## Ballot Lifecycle

Building a ballot on Trail is simple and gives great flexibility to the developer to build their ballots in a variety of ways. An example ballot and voting contract will be used throughout this guide, and is recommended as an established model for current Trail interface best practices.

### Recommended Developer Resources

* 49 TLOS to CPU

* 1 TLOS to NET

* 100 KB of RAM

### 1. Setting Up Your Contract (Optional)

While setting up an external contract is not required to utilize Trail's services, it is required if developers want to **contractually** act on ballot results. If you aren't interested in having a separate contract and just want to run a ballot or campaign, then proceed to step 2.

For our contract example, we will be making a simple document updater where updates are voted on by the community. The basic interface of our sample contract looks like this:

* `createdoc(namae doc_name, string text)`

    The createdoc action will create a new document, retrievable by the supplied `doc_name`. The text of the document is stored as a string (not ideal for production, but fine for this example).

* `makeproposal(name doc_name, string new_text)`

    The makeproposal action stores the new text retrievable by the document name, after first making sure the associated document exists. In a more complex example, this action could send an inline action to Trail's `createballot()` action to make the whole process atomic.

* `updatedoc(name doc_name, string new_text)`

    The updatedoc action simply updates our document with the new text provided in the proposal. This will be called after voting has completed to update our document with the text supplied in the proposal (if the proposal passes, if not the document will stay the same).

* `closeprop(name ballot_name)`

    The closeprop action is important for performing custom logic on ballot results, and simply sends an inline action to Trail's `closeballot()` action upon completion. The `status` field of the ballot allows ballot operators the ability to assign any context to status codes they desire. For example, a contract could interpret a status code of `7` to mean `REFUNDED`. However, status codes `0`, `1`, and `2` are reserved for meaning `SETUP`, `OPEN`, and `CLOSED`, respectively.

Note that we are never storing vote counts on our contract (remember, that's Trail's job).

### 2. Creating a Ballot

Trail offers extensive tools for any prospective user or developer to launch a ballot that is publicly votable by all TLOS token holders. Ballots can also be created that count votes based on custom user-crerated tokens, allowing teams or organizations to manage internal voting.

* `newballot(name ballot_name, name category, name publisher, string title, string description, string info_url, uint8_t max_votable_options, symbol voting_sym)`

    The createballot action will begin setup of a new ballot.

    `ballot_name` is the unique name of the ballot. No other ballot may concurrently share this name.

    `category` is the category the ballot falls under. A list of categories is below:

    * `proposal` : Users vote on a proposal by casting votes for either the YES, NO, or ABSTAIN options.

    * `election` : Users vote on candidate(s) from a set of candidate options.

    * `poll` : Users vote on a custom set of options. Polls can also be deleted mid-vote, and are the best way to quickly gauge the community's appetite for change.

    `publisher` is the account publishing the ballot. Only this account will be able to modify and close the ballot.  

    `title` is the title of the ballot.

    `description` is a brief explanantion of the ballot (ideally in markdown format). 

    `info_url` is critical to ensuring voters are able to know exactly what they are voting on when they cast their votes. This parameter should be a url to a webpage (or even better, an IPFS hash) that explains what the ballot is for, and should provide sufficient information for voters to make an informed decision.

    `max_votable_options` is the maximum number of options a single voter is allowed to cast on this ballot. For example, if an election was being run with 3 different candidates and the max votable options was set to 2, then voters may vote for no more than 2 of the 3 candidates on the ballot.

    `voting_sym` is the symbol to be used for counting votes. In order to cast a vote on the ballot, the voter must own a balance of tokens with the voting symbol.
    
    Note that in order to eliminate potential confusion with tokens on the `eosio.token` contract, the core voting symbol used by Trail is `VOTE` instead of `TLOS`, however since `VOTE` tokens are issued to all holders of `TLOS` at a 1:1 ratio based on the user's current stake, any user's `VOTE` balance will be functionally equivalent to their `TLOS` stake at any given time.
    
    Additionally, custom voting tokens must exist on Trail (by calling newtoken) before being able to create ballots that use them.

* `upsertinfo(name ballot_name, name publisher, string title, string description, string info_url)`

    The upsertinfo action allows the ballot publisher to replace the title, description, and info_url of the ballot.

    `ballot_name` is the name of the ballot to change.

    `publisher` is the name of the account that published the ballot. Only this account is authorized to reset the info on the ballot.

    `title` is the new ballot title.

    `description` is the new ballot description.

    `info_url` is the new ballot info url.


* `readyballot(name ballot_name, name publisher, uint32_t end_time)`

    The readyballot action is called by the ballot publisher when voting is ready to open. After calling this action no further changes may be made to the ballot.

    `ballot_name` is the name of the ballot to ready.

    `publisher` is the name of the account that published the ballot. Only this account is authorized to ready the ballot for voting.

    `end_time` is the time voting will close for the ballot. The end_time value is expressed as seconds since the Unix Epoch.


* `deleteballot(name ballot_name, name publisher)`

    The deleteballot action deletes an existing ballot. This action cannot be called during the voting process, and all ballots must wait for the ballot to "cool down" after the ballot's end time before being eligible for deletion. The current Ballot Cooldown period is `3 days`.

    `ballot_name` is the name the ballot to delete.

    `publisher` is the account that published the ballot. Only this account can delete the ballot.

In our custom contract example, the account that wishes to start a new ballot must simply call the `newballot()` action to begin the ballot setup process. Next, the same account would call `readyballot()` to open the ballot for voting.

### 3. Running A Ballot 

After ballot setup is complete, the only thing left to do is wait for users to begin casting their votes. All votes cast on Trail are live, so it's easy to see the state of the ballot as votes roll in. There are also a few additional features available for ballot runners that want to operate a more complex campaign. This feature set will grow with the development of Trail and as more complex versions of ballots are introduced to the system.

* `cancelballot()`

    The cancelballot action will cancel an existing proposal and mark its status as cancelled. Ballots that have been cancelled can no longer receive votes but remain visible in Trail so voters can remain informed on the status of the ballot. Only after the ballot has reached it's original end time *plus* the global ballot cooldown period may the ballot may be deleted as normal.

In our custom contract example, none of these actions are used (just to keep it simple). 

### 4. Closing A Ballot

Once a ballot has reached it's end time, it will automatically stop accepting votes. The final tally can be seen by querying the `ballots` table with the appropriate `ballot_name`.

* `closeballot(name ballot_name, name publisher, uint8_t new_status)`

    The closeballot action is used by publishers to close out a ballot and render a decision based on the results.

    `ballot_name` is the name of the ballot to close.

    `publisher` is the publisher of the ballot. Only this account may close the ballot.

    `new_status` is the resultant status after reaching a descision on the end state of the ballot. This number can represent any end state desired, but `0`, `1`, `2`, and `4` are reserved for `SETUP`, `OPEN`, `CLOSED`, and `ARCHIVED` respectively.

Back in our custom contract example, the `closeprop()` action would be called by the ballot publisher, where closeprop would perform a cross-contract table lookup to access the final ballot results. Then, based on the results of the ballot, the custom contract would determine whether the proposal passed or failed according to it's own standards, and update it's tables accordingly. Finally, the closeprop action would send an inline action to Trail's `closeballot()` action to (atomically) close out the ballot and assign a final status code for the ballot.

## Creating Custom Voting Tokens

Trail allows any Telos Blockchain Network user to create and manage their own custom voting tokens, which can be used to vote on any ballot that has been configured to count votes based on that token.

### About Token Registries

Trail stores metadata about its tokens in a custom data structure called a Token Registry. This stucture automatically updates when actions are called that would modify any part of the registry's state.

### 1. Registering a New Token

* `newtoken(name publisher, asset max_supply, token_settings settings, string info_url)`

    The newtoken action creates a new voting token in Trail, and initializes it with the given token settings.

    `publisher` is the name of the account that is creating the token registry. Only this account is authorized to issue new tokens, change settings, and execute other actions related to directly modifying the state of the registry.

    `max_supply` is the total number of tokens allowed to simultaneously exist in circulation. This action will fail if a token already exists with the same symbol in Trail.

    `settings` is a custom list of boolean settings designed for Trail's token registries. The currently supported settings are as follows:

    * `is_destructible` allows the registry to be erased completely by the publisher.
    * `is_proxyable` allows tokens to be proxied to users that have registered as a valid proxy for this registry. **In Development...**
    * `is_burnable` allows tokens to be burned from circulation by the publisher. 
    * `is_seizable` allows the tokens to be seized from token holders by the publisher. This setting is intended for publishers who want to operate a more controlled internal token voting system.
    * `is_max_mutable` allows the registry's max_supply field to be adjusted through an action call. Only the publisher can call the `changemax` action.
    * `is_liquid` allows the tokens to be transferred to other users. **In Development...**

    `info_url` is a url link (ideally an IPFS link) that prospective voters can follow to get more information about the token and its purpose.

### 2. Operating A Token Registry

Trail offers a wide range of token features designed to offer maximum flexibility and control for registry operators, while at the same time ensuring the transparency and security of registry operations for token holders. Registry operators can execute any of the following actions (provided their token settings allow it) to modify the state of token balances and the Registry itself.

* `mint(name publisher, name recipient, asset amount_to_mint)`

    The mint action is the only way for new tokens to be introduced into circulation.

    `publisher` is the name of the account that published the registry. Only the registry publisher has the authority to mint new tokens.

    `recipient` is the name of the recipient intended to receive the newly minted tokens.

    `amount_to_mint` is the amount of tokens to mint for the recipient. This action will fail if the number of tokens to mint would breach the max supply allowed by the registry.

* `burn(name publisher, asset amount_to_burn)`

    The burn action is used to burn tokens from a user's wallet. This action is only callable by the registry publisher, and if the registry allows token burning.

    `publisher` is the publisher of the registry. The publisher may only burn tokens from their own balance.

    `amount_to_burn` is the number of tokens to burn from circulation.

* `send(name sender, name recipient, asset amount, string memo)`

    The send action is very similar to the regular `eosio.token::transfer` in that it simply sends a specified number of tokens to another user. Note that this action has been purposefully named `send()` in order to avoid confusion with the regular `eosio.token::transfer` action. This action is callable by any token holder, but only if the registry's `is_liquid` setting is set to `true`.

    `sender` is the name of the account sending the tokens.

    `recipient` is the name of the account receiving the tokens.

    `amount` is the quantity of tokens to send.

    `memo` is a brief memo to send with the tokens.

Additionally, any registry that has `is_liquid` set to `true` in their token settings will automatically inhereit Trail's propietary `rebalance` system to eliminate vote washing and ensure the integrity of every vote.

* `seize(name publisher, name owner, asset amount_to_seize)`

    The seize action is called by the publisher to seize a balance of tokens from another account. Only the publisher can seize tokens, and only if the registry's settings allow it.

    `publisher` is the name of the account that published the registry. Only this account is authorized to execute the `seize()` action.

    `owner` is the name of the owner of the account balance being seized.

    `amount_to_seize` is the quantity of tokens to seize from the owner's balance.

Note that this action is mostly intended for use in internal voting systems, and Trail's native `VOTE` token is not subject to seizure.

* `changemax(name publisher, asset max_supply_delta)`

    The changemax action is called to adjust the maximum supply on a token registry. The max supply can never be changed to an amount below zero or the currently circulating supply.

    `publisher` is the name of the account that published the registry. Only this account is authoried to change the max supply.

    `max_supply_delta` is the delta to apply to the maximum supply. Giving a negative value will lower the supply, and giving a positive one will increase it.

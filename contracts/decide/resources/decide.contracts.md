<h1 class="contract">init</h1>

---
spec_version: "0.2.0"
title: Set Decide Config
summary: 'Set Decide Config Version'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/admin.png#9bf1cec664863bd6aaac0f814b235f8799fb02c850e9aa5da34e8a004bd6518e
---

{{$action.account}} sets the Telos Decide config to version {{app_version}}.

<h1 class="contract">updatefee</h1>

---
spec_version: "0.2.0"
title: Update Fee
summary: 'Update Platform Fee'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/admin.png#9bf1cec664863bd6aaac0f814b235f8799fb02c850e9aa5da34e8a004bd6518e
---

Trail Admin {{$action.account}} updates {{fee_name}} fee to {{fee_amount}}.

<h1 class="contract">updatetime</h1>

---
spec_version: "0.2.0"
title: Update Time Setting
summary: 'Update Platform Time Setting'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/admin.png#9bf1cec664863bd6aaac0f814b235f8799fb02c850e9aa5da34e8a004bd6518e
---

Trail Admin {{$action.account}} udpates the {{time_name}} time to {{length}} seconds.

<h1 class="contract">newtreasury</h1>

---
spec_version: "0.2.0"
title: Define New Treasury
summary: 'Define New Treasury'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/resource.png#3830f1ce8cb07f7757dbcf383b1ec1b11914ac34a1f9d8b065f07600fa9dac19
---

{{manager}} is defining a new treasury with a max supply of {{max_supply}} and an initial access method of {{access}}.

## Initial Settings

All settings on the new treasury will be initialized to false.

<h1 class="contract">edittrsinfo</h1>

---
spec_version: "0.2.0"
title: Edit Treasury Info
summary: 'Edit Treasury Info'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/resource.png#3830f1ce8cb07f7757dbcf383b1ec1b11914ac34a1f9d8b065f07600fa9dac19
---

{{$action.account}} sets the treasury title to {{title}}, the description to {{description}}, and the icon to {{icon}} for the {{treasury_symbol}} treasury.

<h1 class="contract">toggle</h1>

---
spec_version: "0.2.0"
title: Toggle Treasury Setting
summary: 'Toggle Treasury Setting'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/resource.png#3830f1ce8cb07f7757dbcf383b1ec1b11914ac34a1f9d8b065f07600fa9dac19
---

{{$action.account}} toggles the treasury setting {{setting_name}} for the {{treasury_symbol}} treasury.

<h1 class="contract">mint</h1>

---
spec_version: "0.2.0"
title: Mint Tokens
summary: 'Mint Treasury Tokens'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Treasury Manager {{$action.account}} mints {{quantity}} to {{to}} account.

<h1 class="contract">transfer</h1>

---
spec_version: "0.2.0"
title: Transfer Tokens
summary: 'Transfer Treasury Tokens'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/transfer.png#5dfad0df72772ee1ccc155e670c1d124f5c5122f1d5027565df38b418042d1dd
---

Voter {{$action.account}} transfers {{quantity}} to {{to}}.

<h1 class="contract">burn</h1>

---
spec_version: "0.2.0"
title: Burn Tokens
summary: 'Burn Treasury Tokens from Manager Balance'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Treasury Manager {{$action.account}} burns {{quantity}} from their account.

<h1 class="contract">reclaim</h1>

---
spec_version: "0.2.0"
title: Reclaim Tokens
summary: 'Reclaim Treasury Tokens from Voter'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Treasury manager {{$action.account}} reclaims {{quantity}} from {{voter}}.

<h1 class="contract">mutatemax</h1>

---
spec_version: "0.2.0"
title: Mutate Max Supply
summary: 'Mutate Treasury Max Supply'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Treasury Manager {{$action.account}} mutates the {{treasury_symbol}} treasury's max supply to {{new_max_supply}}.

<h1 class="contract">setunlocker</h1>

---
spec_version: "0.2.0"
title: Set Treasury Unlocker
summary: 'Set Treasury Unlocker Account and Permission'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Treasury Manager {{$action.account}} sets the {{treasury_symbol}} unlocker auth to {{new_unlock_acct}}@{{new_unlock_auth}}.

<h1 class="contract">lock</h1>

---
spec_version: "0.2.0"
title: Lock Treasury Settings
summary: 'Lock Treasury Settings'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Treasury Manager {{$action.account}} locks the {{treasury_symbol}} treasury.

<h1 class="contract">unlock</h1>

---
spec_version: "0.2.0"
title: Unlock Treasury Settings
summary: 'Unlock Treasury Settings'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Treasury Unlocker {{$action.account}} unlocks the {{treasury_symbol}} treasury.

<h1 class="contract">addfunds</h1>

---
spec_version: "0.2.0"
title: Add Payroll Funds
summary: 'Add to Treasury Payroll Funds'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

{{$action.account}} adds {{quantity}} to the {{treasury_symbol}}'s {{payroll_name}} payroll.

<h1 class="contract">editpayrate</h1>

---
spec_version: "0.2.0"
title: Edit Payroll Pay Rate
summary: 'Edit Payroll Pay Rate'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Treasury Manager {{$action.account}} changes {{treasury_symbol}}'s {{payroll_name}} pay rate to {{per_period}} per {{period_length}} second period.

<h1 class="contract">newballot</h1>

---
spec_version: "0.2.0"
title: New Treasury Ballot
summary: 'Create New Treasury Ballot'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Voter {{publisher}} creates the {{ballot_name}} ballot for the {{treasury_symbol}} treasury.

<h1 class="contract">editdetails</h1>

---
spec_version: "0.2.0"
title: Edit Ballot Info
summary: 'Edit Ballot Info'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Ballot publiser {{$action.account}} edits the ballot title to {{title}}, the description to {{description}}, and the content to {{content}}.

<h1 class="contract">togglebal</h1>

---
spec_version: "0.2.0"
title: Toggle Ballot Setting
summary: 'Toggle Ballot Setting'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Ballot publisher {{$action.account}} toggles the {{setting_name}} on the {{ballot_name}} ballot.

<h1 class="contract">editminmax</h1>

---
spec_version: "0.2.0"
title: Edit Ballot Min/Max
summary: 'Edit Ballot Min/Max Options'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Ballot publisher {{$action.account}} edits number of minimum vote options to {{new_min_options}}, and the max vote options to {{new_max_options}}. This means voters can select between {{new_min_options}} and {{new_max_options}} options when voting.

<h1 class="contract">addoption</h1>

---
spec_version: "0.2.0"
title: Add Ballot Option
summary: 'Add Ballot Option'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Ballot publisher {{$action.account}} adds the {{new_option_name}} option to the {{ballot_name}} ballot.

<h1 class="contract">rmvoption</h1>

---
spec_version: "0.2.0"
title: Remove Ballot Option
summary: 'Remove Ballot Option'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Ballot publisher {{$action.account}} removes the {{option_name}} option from the {{ballot_name}} ballot.

<h1 class="contract">openvoting</h1>

---
spec_version: "0.2.0"
title: Open Ballot Voting
summary: 'Open Ballot For Voting'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Ballot puboisher {{$action.account}} opens voting on {{ballot_name}} ballot until {{end_time}}.

<h1 class="contract">cancelballot</h1>

---
spec_version: "0.2.0"
title: Cancel Ballot
summary: 'Cancel Ballot'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Ballot publisher {{$action.account}} cancels the {{ballot_name}} ballot and forfeits all fees spent in its creation.

## Ballot Status Change

The {{ballot_name}} will be changed to the "cancelled" status. From there, it can be fully deleted with the deleteballot() action.

<h1 class="contract">deleteballot</h1>

---
spec_version: "0.2.0"
title: Delete Ballot
summary: 'Delete Ballot'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

{{$action.account}} deletes the {{ballot_name}} ballot.

<h1 class="contract">postresults</h1>

---
spec_version: "0.2.0"
title: Post Light Ballot Results
summary: 'Post Light Ballot Results'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Ballot publisher {{$action.account}} posts the results of the {{ballot_name}} light ballot.

<h1 class="contract">closevoting</h1>

---
spec_version: "0.2.0"
title: Close Ballot Voting
summary: 'Close Ballot Voting'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Ballot publisher {{$action.account}} closes the completed {{ballot_name}} ballot.

<h1 class="contract">broadcast</h1>

---
spec_version: "0.2.0"
title: Broadcast Ballot Results
summary: 'Broadcast Ballot Results'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Broadcasts the final ballot results to the ballot publisher's account.

<h1 class="contract">archive</h1>

---
spec_version: "0.2.0"
title: Archive Ballot
summary: 'Archive Ballot'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Ballot publisher {{$action.account}} archives the {{ballot_name}} ballot until {{archived_until}}.

<h1 class="contract">unarchive</h1>

---
spec_version: "0.2.0"
title: Unarchive Ballot
summary: 'Unarchive Ballot'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/token.png#207ff68b0406eaa56618b08bda81d6a0954543f36adc328ab3065f31a5c5d654
---

Ballot archiver {{$action.account}} unarchives the {{ballot_name}} ballot.

<h1 class="contract">regvoter</h1>

---
spec_version: "0.2.0"
title: Register Voter
summary: 'Register Voter'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/account.png#3d55a2fc3a5c20b456f5657faf666bc25ffd06f4836c5e8256f741149b0b294f
---

Register the new voter {{$action.account}} to the {{treasury_symbol}} treasury.

<h1 class="contract">unregvoter</h1>

---
spec_version: "0.2.0"
title: Unregister voter
summary: 'Unregister Voter'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/account.png#3d55a2fc3a5c20b456f5657faf666bc25ffd06f4836c5e8256f741149b0b294f
---

Unregister the {{$action.account}} voter from the {{treasury_symbol}} treasury.

<h1 class="contract">castvote</h1>

---
spec_version: "0.2.0"
title: Cast Vote
summary: 'Cast Vote on a Ballot'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/voting.png#db28cd3db6e62d4509af3644ce7d377329482a14bb4bfaca2aa5f1400d8e8a84
---

Voter {{$action.account}} casts votes for {{options}} on the {{ballot_name}} ballot.

<h1 class="contract">unvoteall</h1>

---
spec_version: "0.2.0"
title: Unvote All Options
summary: 'Unvote All Options'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/voting.png#db28cd3db6e62d4509af3644ce7d377329482a14bb4bfaca2aa5f1400d8e8a84
---

Voter {{$action.account}} unvotes all options on their vote.

<h1 class="contract">stake</h1>

---
spec_version: "0.2.0"
title: Stake Tokens
summary: 'Stake Tokens'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/voting.png#db28cd3db6e62d4509af3644ce7d377329482a14bb4bfaca2aa5f1400d8e8a84
---

Voter {{$action.account}} stakes {{quantity}}.

<h1 class="contract">unstake</h1>

---
spec_version: "0.2.0"
title: Unstake Tokens
summary: 'Unstake Tokens'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/voting.png#db28cd3db6e62d4509af3644ce7d377329482a14bb4bfaca2aa5f1400d8e8a84
---

Voter {{$action.account}} unstakes {{quantity}}.

<h1 class="contract">rebalance</h1>

---
spec_version: "0.2.0"
title: Rebalance Vote
summary: 'Rebalance Vote'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/voting.png#db28cd3db6e62d4509af3644ce7d377329482a14bb4bfaca2aa5f1400d8e8a84
---

Worker {{$action.account}} rebalances {{voter}}'s vote on the {{ballot_name}} ballot.

<h1 class="contract">cleanupvote</h1>

---
spec_version: "0.2.0"
title: Clean Vote
summary: 'Clean Vote'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/voting.png#db28cd3db6e62d4509af3644ce7d377329482a14bb4bfaca2aa5f1400d8e8a84
---

Worker {{$action.account}} cleans {{voter}}'s vote on the {{ballot_name}}.

<h1 class="contract">forfeitwork</h1>

---
spec_version: "0.2.0"
title: Forfeit Work
summary: 'Forfeit Work'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/voting.png#db28cd3db6e62d4509af3644ce7d377329482a14bb4bfaca2aa5f1400d8e8a84
---

Worker {{$action.account}} forfeits all currently commited work on the {{treasury_symbol}} treasury.

<h1 class="contract">claimpayment</h1>

---
spec_version: "0.2.0"
title: Claim Payment
summary: 'Claim Payment'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/account.png#3d55a2fc3a5c20b456f5657faf666bc25ffd06f4836c5e8256f741149b0b294f
---

Worker {{$action.account}} claims payment for all committed work on the {{treasury_symbol}} treasury.

<h1 class="contract">withdraw</h1>

---
spec_version: "0.2.0"
title: Withdraw TLOS
summary: 'Withdraw TLOS from Trail to Wallet'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/account.png#3d55a2fc3a5c20b456f5657faf666bc25ffd06f4836c5e8256f741149b0b294f
---

Account {{$action.account}} withdraws {{quantity}} from the Trail platform.

<h1 class="contract">regcommittee</h1>

---
spec_version: "0.2.0"
title: Register New Committee
summary: 'Register New Committee'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/account.png#3d55a2fc3a5c20b456f5657faf666bc25ffd06f4836c5e8256f741149b0b294f
---

Voter {{$action.account}} registers the {{committee_name}} committee for the {{treasury_symbol}} treasury.

<h1 class="contract">addseat</h1>

---
spec_version: "0.2.0"
title: Add Committee Seat
summary: 'Add Committee Seat'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/account.png#3d55a2fc3a5c20b456f5657faf666bc25ffd06f4836c5e8256f741149b0b294f
---

Committee updater {{$action.account}} adds the {{seat_name}} seat to the {{committee_name}} committee.

<h1 class="contract">removeseat</h1>

---
spec_version: "0.2.0"
title: Remove Committee Seat
summary: 'Remove Committee Seat'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/account.png#3d55a2fc3a5c20b456f5657faf666bc25ffd06f4836c5e8256f741149b0b294f
---

Committee updater {{$action.account}} removes the {{seat_name}} seat from the {{committee_name}} committee.

<h1 class="contract">assignseat</h1>

---
spec_version: "0.2.0"
title: Assign Committee Seat
summary: 'Assign Committee Seat'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/account.png#3d55a2fc3a5c20b456f5657faf666bc25ffd06f4836c5e8256f741149b0b294f
---

Committee updater {{$action.account}} assigns {{seat_holder}} to the {{seat_name}} seat in the {{committee_name}} committee.

<h1 class="contract">setupdater</h1>

---
spec_version: "0.2.0"
title: Set Committee Updater
summary: 'Set Committee Updater'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/account.png#3d55a2fc3a5c20b456f5657faf666bc25ffd06f4836c5e8256f741149b0b294f
---

Committee updater {{$action.account}} sets the updater to {{updater_acct}}@{{updater_auth}} for the {{committee_name}} committee.

<h1 class="contract">delcommittee</h1>

---
spec_version: "0.2.0"
title: Delete Committee
summary: 'Delete Committee'
icon: https://github.com/Telos-Foundation/images/raw/master/ricardian_assets/eosio.contracts/icons/account.png#3d55a2fc3a5c20b456f5657faf666bc25ffd06f4836c5e8256f741149b0b294f
---

Committee Updater {{$action.account}} deletes the committee.

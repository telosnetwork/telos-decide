## Trail Ballot Guide

In this guide we will explore all the different Ballot interactions that can be performed on the Trail Voting Platform.

### Ballot Creation

To create a new ballot, a voter can call the `newballot()` action.

#### Ballot Statuses

| Status | Description |
| --- | --- |
| setup | Ballot is being prepared. |
| voting | Ballot is currently open for voting. |
| closed | Ballot is closed and final results have been rendered. |
| cancelled | Ballot was cancelled during voting, awaiting deletion. |
| archived | Ballot is currently archived and can't be deleted. |

### Voting Methods

| Voting Method | Description | Raw Weight | Weighted Vote |
| --- | --- | --- | -- |
| 1acct1vote | Every voter gets 1 whole vote. Zero balances don't count. | 0.01 TEST | 1.00 TEST Each |
| 1tokennvote | Raw weight is applied to each option selected. | 3.00 TEST | 3.00 TEST Each |
| 1token1vote | Raw weight is split among all selections. | 3.00 TEST | 1.00 TEST Each |
| 1tsquare1v | Raw weight is squared and split among selections. At ballot closure each option's total will be square-rooted | 3.00 TEST | 9.00 TEST Each |
| quadratic | Raw weight is squared and split among selections. | 3.00 TEST | 9.00 Each |

#### Ballot Settings

| Setting | Description |
| --- | --- |
| lightballot | Marks as a light ballot if true. |
| revotable | Allows revoting on the ballot if true. |
| votestake | Reads voter's staked balance for casting votes if true. |

### Ballot Management

...

### Ballot Closing

...

### Broadcasting Ballot Results

...

### Ballot Deletion

...

### Ballot Cancellation

...

### Archiving and Unarchiving

...
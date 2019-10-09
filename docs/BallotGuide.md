## Trail Ballot Guide

...

### Ballot Creation

...

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
| ranked | Votes are cast in order of preference, with the weighted total divided by the option's position. | 3.00 TEST | #1 = 3.00 TEST, #2 = 1.50 TEST, ... |

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
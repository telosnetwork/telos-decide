## Trail Committee Guide

In this guide we will explore all the different Committee interactions that can be performed on the Trail Voting Platform.

### What is a Trail Committee?

A Trail Committee is a group of voters each with their own seat in the committee. Seats may be added and removed, and new members may be assigned to or released from a seat. The committee selects an updater account and authority that is allowed to make changes to the committee, allowing for decentralized control of the committee itself.

### Committee Creation

To create a new committee for a treasury, a voter must call the `newcommittee()` action.

### Adding and Removing Seats

Adding and removing seats is easy, simply call the respective action: `addseat()` or `removeseat()`.

### Committee Deletion

To delete a committee, the current updater account and authority must call the `delcommittee()` action.
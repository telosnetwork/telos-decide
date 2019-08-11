# Trail Developer Guide

In this Developer Guide, we will explore Trail's suite of platform services available to any prospective contract developer. We will follow an example throughout the guide to help establish the general flow and lifecycle of running a ballot and/or registry. The Trail Contract API will be introduced guide as certain sections become relevant to the task at hand. For a more straightforward breakdown of Trail's Contract API without the example fluff, see the [Trail Contract API](ContractAPI.md) document.

## Understanding Trail's Role

Trail's primary role is to consolidate many of the boilerplate functions of voting contracts into a single service, while also keeping the system flexible enough to offer verbose ballot and registry management. 

In other words, this design means developers can leave the "vote counting" up to Trail and instead write their contracts to interpret final ballot results posted by Trail. Through cross-contract table lookup, any contract can view the results of a ballot and then have their contracts act based on those results.

## Building External Contract

...


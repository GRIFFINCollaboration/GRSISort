# Contributing to GRSISort

Thank you for taking the time to contribute! :+1:

## Reporting an issue

Before reporting an issue, please check the wiki and the existing (open or closed) issues to see if you can find a solution.

If you were unable to find a solution to your problem, please run ErrorReport.sh, copy and paste the resulting form and fill out the missing information.

## Coding standards

Coding standards promote code that is presented in a standardized format and easier to understand.
For GRSISort, we base our coding standards on ROOT's [coding conventions](https://root.cern.ch/coding-conventions).
Please adhere to these standards. Additional standards are listed below.
* When casting variables, please use a `static_cast`. For example, use:
```
static_cast<Int_t> x
```
rather than 
```
(Int_t) x
```

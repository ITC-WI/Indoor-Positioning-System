Commit History
==============
For every push being made to the github repo in any branch the history
should be updated accordingly. Newer commits should come to the top.

## *NativeFramework initialised*

## Basic Scanning is set
1. Updates
   - start_scanning() and stop_scanning() are completely functional.
   - A circular load icon is implemented. This runs as long as the scanning is in progress.
   - Permissions are completely functional. At each instance user is asked permission if it isn't already given.
   - If Bluetooth is disabled, a dialog is thrown to ask user permission.
2. TODOs
   - start_scanning() should be implemented in a new thread.

Commit History
==============
For every push being made to the github repo in any branch the history
should be updated accordingly. Newer commits should come to the top.

## Chain of responsibility first design ready
1. Updates:
   - Chain of responsibility is now complete. Though testing remains.
   - Pointers to different filterHandlers were created. Their destruction is
     taken care of.
   - The Requests array is also set.
   - There were changes in the arrangement of code so every use occurs after
     a declaration. The JNI methods were shifted below the handler classes.
   - Filter calls were set. Though they are empty as of now.
   - Beacon was included in all the filter classes.
   - pointer logic in findBeacon and updateBeacon functions was improved.
2. TODOs:
   - Checking for possible memory leaks.
   - Testing realtime app.
   - Setup logging.
   - Error and exception handling remain.

## Beacon.h first design ready
1. Updates
   - Beacon class should now be completely functional. Though testing remains.
   - Several functions to find, add, update and remove beacons were added.
   - start pointer was made local to Beacon.h.
   - Construction and Destruction of beacons is completely handled
2. TODOs
   - Rigorously check for possible memory leaks.
   - Setup a logging environment.
   - Filter calls are not set up yet.
   - Chain of requests is yet to be set.
   - Error and exception handling remain.

## Chain of Responsibility (unfinished)
1. Updates
   - A chain of responsibility pattern is created using FilterHandler class.
   - The different requests would be tackled by Rssi, RssiToDistance, Distance and MultilaterationFilterHandler.
   - As of now there are five requests: RSSI_SMA, RSSI_KF, RSSI_TO_DISTANCE_PATH_LOSS, DISTANCE_KF and MULTILATERATE.
   - Each of the four filterHandler can call their respective filters which are given their own header file.
   - A beacon header and position header file is also created. It would be used to create objects for storing beacon
     state in a linked list and position state would have a single instance.
   - No changes in CMakeLists as other files are implemented as headers.
2. TODOs
   - Take care of Destroying objects.
   - Add filter calls in the filterHandlers.
   - Setup the chain of requests.
   - Complete the Beacon functionality in Beacon.h.
   - Setup logging in file (preferably csv).
   - Error and exception handling remaining.
   - Test the app so that it actually runs on a device.

## *NativeFramework initialised*
- Beacon handling functions(discovered,updated and lost) are declared. To be implemented.
- start_filtering() and initialise_scan() are declared. To be implemented.

## Basic Scanning is set
1. Updates
   - start_scanning() and stop_scanning() are completely functional.
   - A circular load icon is implemented. This runs as long as the scanning is in progress.
   - Permissions are completely functional. At each instance user is asked permission if it isn't already given.
   - If Bluetooth is disabled, a dialog is thrown to ask user permission.
2. TODOs
   - start_scanning() should be implemented in a new thread.

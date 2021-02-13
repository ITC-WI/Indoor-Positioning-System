Commit History
==============
For every push being made to the github repo in any branch the history
should be updated accordingly. Newer commits should come to the top.

## RSSI_SMA completed and code cleaned
1. Updates:
   - rssi_sma() defined and is now functional. It is implemented via STL queue.
   - Code was refactored. The Chain of Responsibility Design pattern(including the classes
     FilterHandler and all its children) was moved to FilterHandler.h file.
   - All JNI interface functions were moved to native-lib.h file. And from there simple c++ function
     were called. These functions are now defined in native-lib.cpp.
   - Unnecessary warnings and TODOs were removed.
   - A bug that caused the app to crash whenever a beacon was taken out of range was removed. The beacon
     scanning framework apart from permissions is now completely functional.
   - File now closes only when the activity is destroyed. So the user can run multiple start scanning
     stop scanning cycles without relaunching the app.

2. TODOs:
   - Error and exception handling remain. Though their utility seems very low now
     as file logging seems to be completely functional.
   - For the app to run properly certain permissions and services need to be active. The app
     does not take into account the case when Location services is not enabled. Also the Storage permission
     needs to be given by the user manually in the settings.
   - MainActivity still needs to be cleaned.

## Filtering Framework Ready
1. Updates:
   - File logging is set and complete.
   - If the file is not there a new file is created at
     "/storage/emulated/0/Download/IPS_test.txt". No requirements from the user side.
   - All memory leaks are taken care of. So the construction and destruction of
     objects is now complete. Though there is a possibility that some of the pointers
     may be initialised on heap and are therefore not cleared yet.
   - Realtime app is running and logging is going on expected.

2. TODOs:
   - Error and exception handling remain. Though their utility seems very low now
     as file logging seems to be completely functional.
   - start_filtering() does not reopen open the file if it has been closed once by
     stop_filtering(). This needs to be improved.

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

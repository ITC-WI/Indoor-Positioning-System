#include <jni.h>
#include <string>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include "Beacon.h"
#include "native-lib.h"

std::fstream file;//NOLINT

/**
* It is used for initialising all things on the filtering side.
*/
void initialise_scan() {
    //Currently Im outputting in a text file. It would be changed to csv.
    LOGI("Initialise()");
    file.open(path,std::ios::in|std::ios::out|std::ios::app);
    file<< std::fixed;
    file<< std::setprecision(2);
    file<<"\n\n";
    FILE("File opened");
    LOGI("%s", path);
    if(file.is_open()){
        LOGI("File Opened");
    }
    else{
        LOGI("File not opened");
    }

    //Initialise the handler pointers.
    rssiFilterHandler = new RssiFilterHandler();
    rssiToDistanceFilterHandler = new RssiToDistanceFilterHandler();
    distanceFilterHandler = new DistanceFilterHandler();
    multilaterationFilterHandler = new MultilaterationFilterHandler();

    //Set the chain of handlers.
    rssiFilterHandler ->SetNext(rssiToDistanceFilterHandler)->SetNext(distanceFilterHandler)->SetNext(multilaterationFilterHandler);
}

/**
 * Called when start scanning button is tapped.
 */
void start_filtering() {

    LOGI("filtering started");
    FILE("Scanning Started");
}

/**
 * Called when stop scanning button is tapped or when the activity is stopped.
 */
void stop_filtering() {

    LOGI("filtering stopped");
    FILE("Stopping Scan");

    //delete all remaining beacon objects.
    Beacon::removeAllBeacons();
    FILE("Scanning Stopped");
    file.flush();
}

/**
 * Called when the activity is destroyed.
 */
void exit() {

    Beacon::removeAllBeacons();

    //delete the filter handlers.
    delete rssiFilterHandler;
    delete rssiToDistanceFilterHandler;
    delete distanceFilterHandler;
    delete multilaterationFilterHandler;

    FILE("File closed");
    file.flush();
    file.close();
}

/**
 * Called from the corresponding method in java whenever a beacon is discovered.
 * It just adds a beacon object to the linked list of beacons.
 */
void IBeaconDiscovered(int major, int minor, float rssi) {

    //It returns a bool value that is not utilised here
    Beacon::addBeacon(major, minor, rssi);
    Beacon::printBeaconChain();
}

/**
 * Called from the corresponding function in java whenever a update is called.
 * It updates the rssis of various beacons with newly detected values.
 * @param majors
 * @param minors
 * @param rssis
 */
void IBeaconUpdated(int majors[], int minors[], float rssis[], int no_of_updates) {

    LOGI("size: %d",no_of_updates);
    //for each element in the array, update the corresponding beacon object in the list.
    for(int i=0; i<no_of_updates; i++){
        LOGI("Updating Beacon%d %d %d ",i, majors[i], minors[i]);
        //It returns a bool value that is not utilised here
        if(majors[i])
        Beacon::updateBeacon(majors[i],minors[i],rssis[i]);
    }

    //For now, whenever a  beacon is updated filter the rssis. Later this would run in an independent thread.
    filter();
}

/**
 * Called from the corresponding method in java whenever a beacon is lost.
 * It removes the beacon from the chain of active beacons.
 */
void IBeaconLost(int major, int minor) {
    //It returns a bool value that is not utilised here
    Beacon::removeBeacon(major,minor);
    Beacon::printBeaconChain();
}

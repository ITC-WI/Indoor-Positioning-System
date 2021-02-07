//
// Created by Auro on 29-01-2021.
//
#include <fstream>
#include <time.h>
#ifndef INDOOR_POSITIONING_SYSTEM_BEACON_H
#define INDOOR_POSITIONING_SYSTEM_BEACON_H

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "Beacon.h::", __VA_ARGS__))
#define FILE(...) file<< "Beacon.h::"<<now_ms()<<" "<<__VA_ARGS__<<std::endl

extern std::fstream file;

/* Return current time in milliseconds */
static double now_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000. + tv.tv_usec/1000.;
}

class Beacon {
private:
    Beacon(){
        next_beacon = nullptr;
        major = 0;
        minor = 0;
        rssi = 0;
        LOGI("Next beacon is nullptr");
        //TODO: Set all data to 0. Just in case.
    }

    ~Beacon(){
    }

    Beacon(int maj, int min, float r){
        Beacon();
        minor = min;
        major = maj;
        rssi = r;
    }

    /**
     * Returns a reference to the pointer to the beacon that is to be found.
     * The parameters are named as such so that there is no confusion
     * with the class members.
     * @param major
     * @param minor
     * @return Beacon*
     */
    static Beacon *&findBeacon(int maj, int min){
        if(start==nullptr){
            //This can never happen but use it for throwing an exception.
            LOGI("start is null");
            Beacon* beacon = nullptr;
            return beacon;
        }

        //reference to a pointer
        Beacon *&iterator = start;
        return findBeacon(iterator, maj, min);
    }

    static Beacon *&findBeacon(Beacon *iterator, int maj, int min){
        LOGI("Finding Beacon %d %d",maj,min);
        //This iterator is just to see how many checks are we performing.
        int i=0;

        do{
            if(((iterator->major) == maj)&& ((iterator->minor) == min)){
                return iterator;
            }
            else{
                //LOGI("i: %d",i);
                i++;
                iterator = iterator->next_beacon;
                findBeacon(iterator, maj, min);
            }
        }while (i<6);//Iterate only 6 times at max. This should work as long as
        // user has <=6 beacons in range. This condition is for testing
        // only. It would be modified later.
        Beacon *beacon = nullptr;
        return beacon;
    }

public:
    /**
     * Major stores the x coordinate of the beacon(in cm).
     */
    int major;
    /**
     * Minor stores the y coordinate of the beacon(in cm).
     */
    int minor;
    /**
     * The updated estimate of rssi.
     */
    float rssi;
    /**
     * Updated process noise covariance.
     */
    float rssi_process_covariance;
    /**
     * Updated observation noise covariance.
     */
    float rssi_noise_covariance;
    /**
     * The average rssi over sma_size.
     */
    float rssi_sma;
    /**
     * Values of rssi in the average stack currently. The total number of values to be in
     * the stack would be defined in the method for simple moving average.
     */
    int sma_size;
    /**
     * The updated estimate of distance (it has already been projected to the ground when
     * rssi was converted to distance).
     */
    float distance;

    /**
     * This beacon pointer maintains a linked list of beacons that are discovered but not
     * lost. Using this pointer we can provide faster access to beacon data.
     */
    Beacon *next_beacon{nullptr};

    /**
     * Points to the start of the beacon list.
     */
     static Beacon* start;

    //TODO: add the required data

    /**
     * Create a new beacon object and add it to the beacon linked list.
     * If start is null add it there. The parameters are named as such
     * so that there is no confusion with the class members.
     * @param maj
     * @param min
     * @param r
     * @return bool: To signify whether the object was created successfully.
     */
    static bool addBeacon(int maj, int min, float r){

        LOGI("Adding Beacon");
        if(start == nullptr){
            start = new Beacon(maj, min, r);
            LOGI("start initialised to %d",start);
        }

        else{
            Beacon *temp = start;
            start = new Beacon(maj, min, r);
            start ->next_beacon = temp;
        }

        FILE("Beacon added "<<maj<<" , "<<min);
        LOGI("Beacon Added %d %d %f", maj,min,r);

        return 0;
    }

    /**
     * Find the beacon from the current beacon list and update it.
     * The parameters are named as such so that there is no confusion
     * with the class members.
     * @param maj
     * @param min
     * @param r
     * @return bool: To signify whether the objects were updated successfully.
     */
    static bool updateBeacon(int maj, int min, float r){
        LOGI("Updating Beacon");
        FILE("Updating Beacon "<<maj<<" , "<<min);
        Beacon *beacon = findBeacon(maj,min);
        beacon->rssi =r;
        LOGI("Update %d %d %f", maj,min,r);
        //fout("%time, RSSI Update: , maj, min, r");
        FILE("New Beacon State:");
        file<<"maj: "<<beacon->major<<" , min: "<<beacon->minor<<" , rssi: "<<beacon->rssi<<std::endl;
        return 0;
    }

    /**
     * Destroys the beacon object and removes it from the beacon list.
     * The parameters are named as such so that there is no confusion
     * with the class members.
     * @param maj
     * @param min
     * @param r
     * @return bool: To signify whether the object was destroyed successfully.
     */
    static bool removeBeacon(int maj, int min){
        //reference to the pointer of Beacon object.
        Beacon *&beacon = findBeacon(maj, min);

        removeBeacon(beacon);
        return 0;
    }

    static bool removeBeacon(Beacon *&beacon){
        Beacon* beacon_ptr = beacon;
        FILE("Removing Beacon "<<beacon->major<<" , "<<beacon->minor);
        if((beacon -> next_beacon) == nullptr){
            //Delete the object that is pointed to by the beacon*
            //and then set the beacon* to nullptr.
            delete beacon_ptr;
            beacon = nullptr;
        }

        else{
            Beacon* temp = (beacon->next_beacon);
            delete beacon_ptr;
            beacon = temp;
        }
        return 0;
    }

    /**
     * If any beacon objects are remaining then remove them
     * @return
     */
    static bool removeAllBeacons(){
        while(start!=nullptr){
            printBeaconChain();
            removeBeacon(start);
        }
        return 0;
    }

    static void printBeaconChain(){
        FILE("Beacon Chain:");
        Beacon *chain = start;
        while(chain!= nullptr){
            int maj = chain->major;
            LOGI("%d ->",maj);
            file<<maj<<" -> ";
            chain = chain->next_beacon;
        }
        file<<"End of Chain"<<std::endl;
        LOGI("End of chain");
    }
};

//This initialises the start pointer at the beginning of the program.
Beacon *Beacon::start = nullptr;

#endif //INDOOR_POSITIONING_SYSTEM_BEACON_H

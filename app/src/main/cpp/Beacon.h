//
// Created by Auro on 29-01-2021.
//
#include <fstream>
#include <time.h>
#include <android/log.h>
#include <queue>
#ifndef INDOOR_POSITIONING_SYSTEM_BEACON_H
#define INDOOR_POSITIONING_SYSTEM_BEACON_H

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "Beacon.h::", __VA_ARGS__))
#define FILE(...) file<< "Beacon.h :: "<<now_ms()<<" "<<__VA_ARGS__<<std::endl

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
        rssi_sma = 0;
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

    static Beacon **findBeaconPtr(int maj, int min){
        Beacon **current = &start;

        for(int i=0; i<6; i++){
            if(checkRef(current, maj, min)){
                return current;
            }
            else{
                current = &((*current)->next_beacon);
            }
        }
        //Throw an exception here.
        return nullptr;
    }

    static Beacon *findBeacon(int maj, int min){
        Beacon *current = start;
        for(int i=0; i<6; i++){
            if(checkRef(current, maj, min)){
                return current;
            }
            else{
                current = current -> next_beacon;
            }
        }
        //Throw an exception here.
        return nullptr;
    }

    static inline bool checkRef(Beacon *beacon, int maj, int min){
        return (beacon->major == maj) && (beacon->minor == min);
    }

    static inline bool checkRef(Beacon **beacon, int maj, int min){
        return checkRef(*beacon, maj, min);
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
      * A queue that maintains a list of last n(SMA_SIZE: defined in rssi.h) rssi values
      */
     std::queue<int> rssi_queue;
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
        }

        else{
            Beacon *temp = start;
            start = new Beacon(maj, min, r);
            start ->next_beacon = temp;
        }

        FILE("Beacon_Added "<<maj<<" "<<min);
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
        LOGI("Updating Beacon %d %d %f", maj,min,r);
        FILE("Updating_Beacon "<<maj<<" "<<min);

        Beacon *beacon = findBeacon(maj,min);
        beacon->rssi =r;

        FILE("New_Beacon_State "<< beacon->major<<" "<<beacon->minor<<" "<<beacon->rssi);
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

        removeBeacon(findBeaconPtr(maj, min));
        return 0;
    }

    static bool removeBeacon(Beacon **beacon){

        FILE("Removing_Beacon "<<(*beacon)->major<<" "<<(*beacon)->minor);
        if(((*beacon) -> next_beacon) == nullptr){
            //Delete the object that is pointed to by the beacon*
            //and then set the beacon* to nullptr.
            delete (*beacon);
            (*beacon) = nullptr;
        }

        else{
            Beacon *temp = ((*beacon)->next_beacon);
            delete (*beacon);
            (*beacon) = temp;
        }
        return 0;
    }

    /**
     * If any beacon objects are remaining then remove them
     * @return
     */
    static bool removeAllBeacons(){
        while(start!=nullptr){
            removeBeacon(&start);
            printBeaconChain();
        }
        return 0;
    }

    static void printBeaconChain(){
        file << "Beacon.h :: " << now_ms() << " " << "Beacon_Chain ";
        Beacon *chain = start;
        while(chain!= nullptr){
            int maj = chain->major;
            LOGI("%d ->",maj);
            file<<maj<<" -> ";
            chain = chain->next_beacon;
        }
        file<<"End of Chain"<<std::endl;
        LOGI("End_of_chain");
    }
};

//This initialises the start pointer at the beginning of the program.
Beacon *Beacon::start = nullptr;

#endif //INDOOR_POSITIONING_SYSTEM_BEACON_H

//
// Created by Auro on 29-01-2021.
//

#ifndef INDOOR_POSITIONING_SYSTEM_BEACON_H
#define INDOOR_POSITIONING_SYSTEM_BEACON_H


class Beacon {
private:
    Beacon(){
        this->next_beacon = nullptr;
        //TODO: Set all data to 0. Just in case.
    }

    ~Beacon(){
    }

    Beacon(int major, int minor, float rssi){
        Beacon();
        this->minor = minor;
        this->major = major;
        this->rssi = rssi;
    }

    /**
     * Returns a reference to the pointer to the beacon that is to be found.
     * The parameters are named as such so that there is no confusion
     * with the class members.
     * @param major
     * @param minor
     * @return Beacon*
     */
    static Beacon *findBeacon(int maj, int min){
        if(start==nullptr){
            //This can never happen but use it for throwing an exception.
        }

        //reference to a pointer
        Beacon *iterator = start;
        return findBeacon(*iterator, maj, min);
    }

    static Beacon *findBeacon(Beacon &iterator, int maj, int min){
        //This iterator is just to see how many checks are we performing.
        int i=0;

        do{
            if(((iterator.major) == maj)&& ((iterator.minor) == min)){
                return &iterator;
            }
            else{
                iterator = *(start->next_beacon);
                findBeacon(iterator, maj, min);
            }
        }while (i<6);//Iterate only 6 times at max. This should work as long as
        // user has <=6 beacons in range. This condition is for testing
        // only. It would be modified later.
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
    Beacon *next_beacon;

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

        if(start == nullptr){
            start = new Beacon(maj, min, r);
        }

        else{
            Beacon *temp = start;
            start = new Beacon(maj, min, r);
            start ->next_beacon = temp;
        }
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
        Beacon *beacon = findBeacon(maj,min);
        beacon->rssi =r;
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
        Beacon *beacon = findBeacon(maj, min);

        removeBeacon(beacon);
    }

    static bool removeBeacon(Beacon *&beacon){
        if((beacon -> next_beacon) == nullptr){
            //Delete the object that is pointed to by the beacon*
            //and then set the beacon* to nullptr.
            delete beacon;
            beacon = nullptr;
        }

        else{
            Beacon *temp = (beacon->next_beacon);
            delete beacon;
            beacon = temp;
        }
    }

    /**
     * If any beacon objects are remaining then remove them
     * @return
     */
    static bool removeAllBeacons(){
        while(start!=nullptr){
            removeBeacon(start);
        }
    }
};

//This initialises the start pointer at the beginning of the program.
Beacon *Beacon::start = nullptr;

#endif //INDOOR_POSITIONING_SYSTEM_BEACON_H

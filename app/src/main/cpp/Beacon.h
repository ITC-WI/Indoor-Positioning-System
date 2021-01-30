//
// Created by Auro on 29-01-2021.
//

#ifndef INDOOR_POSITIONING_SYSTEM_BEACON_H
#define INDOOR_POSITIONING_SYSTEM_BEACON_H


class Beacon {
private:
    Beacon(){
        this->next_beacon = nullptr;

    }
    ~Beacon(){
    }

    Beacon(int major, int minor, float rssi){
        Beacon();
        this->minor = minor;
        this->major = major;
        this->rssi = rssi;
    }

    Beacon *findBeacon(int major, int minor){

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

    //TODO: add the required data

    static Beacon* addBeacon(int major, int minor, float rssi){
        return new Beacon(major, minor, rssi);
    }


};

#endif //INDOOR_POSITIONING_SYSTEM_BEACON_H

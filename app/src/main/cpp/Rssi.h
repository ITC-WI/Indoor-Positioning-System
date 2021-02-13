//
// Created by Auro on 29-01-2021.
//
#include "Beacon.h"
#ifndef INDOOR_POSITIONING_SYSTEM_RSSI_H
#define INDOOR_POSITIONING_SYSTEM_RSSI_H

#ifdef FILE
#undef FILE
#endif
#define FILE(...) file<< "Rssi.h::"<<now_ms()<<" "<<__VA_ARGS__<<std::endl

extern std::fstream file;
class Rssi {
public:

    static const int SMA_SIZE = 10;

    /**
     * Go through the beacon chain and filter the values.
     */
    static void kf(){

    };

    /**
     * Go through the beacon chain and filter the values.
     */
    static void sma(){
        Beacon *current = Beacon::start;
        FILE("sma() call");
        while (current!= nullptr){

            if((current->rssi_queue.size())<SMA_SIZE){
                current->rssi_sma = (current -> rssi_sma)*(current->rssi_queue.size())+(current->rssi);
                current ->rssi_queue.push(current->rssi);
                current ->rssi_sma /= (current -> rssi_queue.size());
            }

            else{
                current->rssi_sma = ((current -> rssi_sma)*(SMA_SIZE)+(current->rssi)-(current->rssi_queue.front()))/SMA_SIZE;
                current->rssi_queue.pop();
                current->rssi_queue.push(current->rssi);
            }

            FILE("Beacon{"<<current->major<<","<<current->minor<<"} sma: "<<current->rssi_sma);
            current = current ->next_beacon;
        }
    }
};


#endif //INDOOR_POSITIONING_SYSTEM_RSSI_H

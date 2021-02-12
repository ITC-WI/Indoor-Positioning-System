//
// Created by Auro on 12-02-2021.
//

#ifndef INDOOR_POSITIONING_SYSTEM_FILTERHANDLER_H
#define INDOOR_POSITIONING_SYSTEM_FILTERHANDLER_H

#include "Multilateration.h"
#include "Rssi.h"
#include "RssiToDistance.h"
#include "Distance.h"

static const int RSSI_SMA = 1;
static const int RSSI_KF = 2;
static const int RSSI_TO_DISTANCE_PATH_LOSS = 3;
static const int DISTANCE_KF = 4;
static const int MULTILATERATE =5;
static const int REQUESTS[] = {RSSI_SMA, RSSI_TO_DISTANCE_PATH_LOSS,
                               DISTANCE_KF, MULTILATERATE};

/**
 * Basic Interface for building chain of interfaces. For more info look here:
 * <a href = https://refactoring.guru/design-patterns/chain-of-responsibility/cpp/example#example-0>
 * Chain of Responsibility Design Pattern</a>
 * <b>What is a chain of responsibility?</b><br>
 * A Filter handler is the basic class/interface which defines two purely virtual functions
 * ({@link FilterHandler#SetNext()} and handle_request()). AbstractFilterHandler class is derived from FilterHandler
 * to implement the function SetNext. From here are derived various FilterHandlers each of which
 * define and use the handle_request() method in their own way.
 */
class FilterHandler{
public:

    /**
     * A pure virtual function. Must be defined in derived class.
     * Used to set the next handler in chain.
     * @param filterHandler
     * @return
     */
    virtual FilterHandler *SetNext(FilterHandler *filterHandler) = 0;

    /**
     * A pure virtual function. Must be defined in derived class.
     * Handles the request. The current request scheme is defined in the class.
     * @param request
     * @return
     */
    virtual int handle_request(int request) = 0;
};

class AbstractFilterHandler : public FilterHandler{
private:
    /**
     * Points to the next handler in chain.
     */
    FilterHandler *next_handler;

public:
    /**
     * Sets the next_handler pointer to 0x0.
     */
    AbstractFilterHandler() : next_handler(nullptr){}

    FilterHandler *SetNext(FilterHandler *filterHandler) override {
        this -> next_handler = filterHandler;
        return filterHandler;
    }

    /**
     * @param request
     * @return
     */
    int handle_request(int request) override {
        if (this->next_handler){
            return this->next_handler->handle_request(request);
        }

        return {};
    }
};

class RssiFilterHandler: public AbstractFilterHandler{
public:

    int handle_request(int request) override{
        if (request == RSSI_SMA){
            Rssi::sma();
            return 0;
        }
        else if(request == RSSI_KF){
            Rssi::kf();
            return 0;
        }
        else{
            return AbstractFilterHandler::handle_request(request);
        }
    }
} *rssiFilterHandler;

class RssiToDistanceFilterHandler : public AbstractFilterHandler{
public:
    int handle_request(int request) override{
        if (request == RSSI_TO_DISTANCE_PATH_LOSS){
            RssiToDistance::path_loss();
            return 0;
        }
        else{
            return AbstractFilterHandler::handle_request(request);
        }
    }
} *rssiToDistanceFilterHandler;

class DistanceFilterHandler : public AbstractFilterHandler{
    int handle_request(int request) override{
        if (request == DISTANCE_KF){
            Distance::kf();
            return 0;
        }
        else{
            return AbstractFilterHandler::handle_request(request);
        }
    }
} *distanceFilterHandler;

class MultilaterationFilterHandler : public AbstractFilterHandler{
    int handle_request(int request) override{
        if (request == MULTILATERATE){
            Multilateration::weighted_lsq();
            return 0;
        }
        else{
            return AbstractFilterHandler::handle_request(request);
        }
    }
} *multilaterationFilterHandler;

/**
 * Pass the requests to the filterHandlers.
 */
void filter(){
    for(int request:REQUESTS){
        int result = rssiFilterHandler->handle_request(request);
        if(result){
            //LOG into file that request was not properly handled.
        }
    }
}

#endif //INDOOR_POSITIONING_SYSTEM_FILTERHANDLER_H

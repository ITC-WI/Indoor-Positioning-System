#include <jni.h>
#include <string>
#include <android/log.h>
#include <cstdio>
#include <fstream>
#include <ctime>
#include <iomanip>
#include "Beacon.h"
#include "Rssi.h"
#include "RssiToDistance.h"
#include "Distance.h"
#include "Multilateration.h"

//A macro to print to the android info log.
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-lib::", __VA_ARGS__))
#define FILE(...) file<< "native-lib::"<<now_ms()<<" "<<__VA_ARGS__<<std::endl

std::fstream file;

/**
 * Basic Interface for building chain of interfaces. For more info look here:
 * <a href = https://refactoring.guru/design-patterns/chain-of-responsibility/cpp/example#example-0>
 * Chain of Responsibility Design Pattern</a>
 * This basically enables us to pass a chain of requests through a series of handlers
 * which would either handle the request or pass it onto the next handler.
 */
class FilterHandler{
public:
    //TODO: Add other constants if necessary.
    static const int RSSI_SMA = 1;
    static const int RSSI_KF = 2;
    static const int RSSI_TO_DISTANCE_PATH_LOSS = 3;
    static const int DISTANCE_KF = 4;
    static const int MULTILATERATE =5;
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

static const int REQUESTS[] = {FilterHandler::RSSI_SMA, FilterHandler::RSSI_TO_DISTANCE_PATH_LOSS,
                               FilterHandler::DISTANCE_KF, FilterHandler::MULTILATERATE};

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

/**
 * It is a test method only
 */
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_stringFromJNI(JNIEnv* env, jobject /* this */) {
    std::string hello = "Powered by C++";
    return env->NewStringUTF(hello.c_str());
}

/**
* It is used for initialising all things on the filtering side.
*/
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_initialise_1scan(JNIEnv *env, jobject thiz, jstring filePath) {
    // TODO: implement initialise_scan()
    //Currently Im outputting in a text file. It would be changed to csv.
    //Also the filename would contain the time and date it was created.

    LOGI("Initialise()");
    LOGI("Now ms%lf",now_ms());
    const char *path = env->GetStringUTFChars(filePath, nullptr);

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
 * This function is unused as of now.
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_start_1filtering(JNIEnv *env, jobject thiz) {
    // TODO: implement start_filtering()
    LOGI("filtering started");
}

/**
 * This function is unused as of now.
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_stop_1filtering(JNIEnv *env, jobject thiz) {
    // TODO: implement stop_filtering()
    LOGI("filtering stopped");

    //delete all remaining beacon objects.
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
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_IBeaconDiscovered(JNIEnv *env, jobject thiz, jint major, jint minor, jfloat rssi) {

    //It returns a bool value that is not utilised here
    Beacon::addBeacon(major, minor, rssi);
    Beacon::printBeaconChain();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_IBeaconsUpdated(JNIEnv *env, jobject thiz, jintArray jmajors, jintArray jminors, jfloatArray jrssis) {

    //jintArray or jfloatArray are java types and have to copied into c++ types.

    /**
     * This is always equal to the number of objects in the Beacon list.
     */
    int no_of_updates = env ->GetArrayLength(jmajors);

    //Initialise the c++ arrays.
    int majors[no_of_updates];
    int minors[no_of_updates];
    float rssis[no_of_updates];


    //Copy the java arrays into c++ arrays.
    env-> GetIntArrayRegion(jmajors, 0, no_of_updates, majors);
    env-> GetIntArrayRegion(jminors, 0, no_of_updates, minors);
    env-> GetFloatArrayRegion(jrssis, 0, no_of_updates, rssis);


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

extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_IBeaconLost(JNIEnv *env, jobject thiz, jint major, jint minor, jfloat rssi) {
    //It returns a bool value that is not utilised here
    Beacon::removeBeacon(major,minor);
    Beacon::printBeaconChain();
}


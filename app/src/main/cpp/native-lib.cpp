#include <jni.h>
#include <string>
#include <android/log.h>
#include <stdio.h>
#include "Beacon.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-lib::", __VA_ARGS__))

extern FILE *file;
/**
 * pointer to a constant.
 */
const Beacon* start;
extern "C"
//The following is a test method only.
JNIEXPORT jstring JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_stringFromJNI(JNIEnv* env, jobject /* this */) {
    std::string hello = "Powered by C++";
    return env->NewStringUTF(hello.c_str());
}

/**
*
*/
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_initialise_1scan(JNIEnv *env, jobject thiz) {
    // TODO: implement initialise_scan()
    //Open the file
    file = fopen("test.txt","w+");
    if (file != NULL)
    {
        fputs("HELLO WORLD!\n", file);
        fflush(file);
        fclose(file);
    }
    else{
        LOGI("No file created");
    }
    LOGI("scan initialised");

    //Initialise the start pointer.
    start = nullptr;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_start_1filtering(JNIEnv *env, jobject thiz) {
    // TODO: implement start_filtering()
    LOGI("filtering started");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_stop_1filtering(JNIEnv *env, jobject thiz) {
    // TODO: implement stop_filtering()
    LOGI("filtering stopped");
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_IBeaconDiscovered(JNIEnv *env, jobject thiz, jint major, jint minor, jfloat rssi) {
    // TODO: implement IBeaconDiscovered()
    if (start== nullptr){
        start = Beacon::addBeacon(major,minor,rssi);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_IBeaconsUpdated(JNIEnv *env, jobject thiz, jintArray majors, jintArray minors, jfloatArray rssis) {
    // TODO: implement IBeaconUpdated()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_IBeaconLost(JNIEnv *env, jobject thiz, jint major, jint minor, jfloat rssi) {
    // TODO: implement IBeaconLost()
}



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
            //RSSI.method_name();
            return 0;
        }
        else if(request == RSSI_KF){
            //RSSI.method_name();
            return 0;
        }
        else{
            return AbstractFilterHandler::handle_request(request);
        }
    }
};

class RssiToDistanceFilterHandler : public AbstractFilterHandler{
public:
    int handle_request(int request) override{
        if (request == RSSI_TO_DISTANCE_PATH_LOSS){
            //RSSI_TO_DISTANCE.method_name();
            return 0;
        }
        else{
            return AbstractFilterHandler::handle_request(request);
        }
    }
};

class DistanceFilterHandler : public AbstractFilterHandler{
    int handle_request(int request) override{
        if (request == DISTANCE_KF){
            //Distance.method_name();
            return 0;
        }
        else{
            return AbstractFilterHandler::handle_request(request);
        }
    }
};

class MultilaterationFilterHandler : public AbstractFilterHandler{
    int handle_request(int request) override{
        if (request == MULTILATERATE){
            //Multilateration.method_name();
            return 0;
        }
        else{
            return AbstractFilterHandler::handle_request(request);
        }
    }
};
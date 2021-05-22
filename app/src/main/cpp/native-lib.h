//
// Created by Auro on 12-02-2021.
//

#ifndef INDOOR_POSITIONING_SYSTEM_NATIVE_LIB_H
#define INDOOR_POSITIONING_SYSTEM_NATIVE_LIB_H
#include <android/log.h>
#include <string>
#include "FilterHandler.h"

//A macro to print to the android info log.
#ifdef LOGI
#undef LOGI
#endif
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-lib::", __VA_ARGS__))
#ifdef FILE
#undef FILE
#endif
#define FILE(...) file<< "native-lib :: "<<now_ms()<<" "<<__VA_ARGS__<<std::endl

const char *path = nullptr;

void initialise_scan();
void start_filtering();
void stop_filtering();
void exit();
void IBeaconDiscovered(int, int, float);
void IBeaconUpdated(int[], int[], float[], int);
void IBeaconLost(int, int);

/**
* It is used for initialising all things on the filtering side.
*/
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_TrilaterationActivity_initialise_1scan(JNIEnv *env, __unused jobject thiz, jstring filePath) {
    path = env->GetStringUTFChars(filePath, nullptr);
    initialise_scan();
}

/**
 * Called when start scanning button is tapped.
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_TrilaterationActivity_start_1filtering(__unused JNIEnv *env, __unused jobject thiz) {
    start_filtering();
}

/**
 * Called when stop scanning button is tapped or when the activity is stopped.
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_TrilaterationActivity_stop_1filtering(__unused JNIEnv *env, __unused jobject thiz) {
    stop_filtering();
}

/**
 * Called when the activity is destroyed.
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_TrilaterationActivity_exit(__unused JNIEnv *env, __unused jobject thiz) {
    exit();
}

/**
 * Called from the corresponding method in java whenever a beacon is discovered.
 * It just adds a beacon object to the linked list of beacons.
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_TrilaterationActivity_IBeaconDiscovered(__unused JNIEnv *env, __unused jobject thiz, jint major, jint minor, jfloat rssi){
    IBeaconDiscovered(major, minor, rssi);
}

/**
 * Called from the corresponding function in java whenever a update is called.
 * It updates the rssis of various beacons with newly detected values.
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_TrilaterationActivity_IBeaconsUpdated(JNIEnv *env, __unused jobject thiz, jintArray jmajors, jintArray jminors, jfloatArray jrssis) {

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
    IBeaconUpdated(majors, minors, rssis, no_of_updates);
}

/**
 * Called from the corresponding method in java whenever a beacon is lost.
 * It removes the beacon from the chain of active beacons.
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_TrilaterationActivity_IBeaconLost(__unused JNIEnv *env, __unused jobject thiz, jint major, jint minor) {
    IBeaconLost(major, minor);
}
#endif //INDOOR_POSITIONING_SYSTEM_NATIVE_LIB_H

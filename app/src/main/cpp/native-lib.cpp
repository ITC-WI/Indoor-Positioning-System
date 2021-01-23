#include <jni.h>
#include <string>

extern "C"
//The following is a test method only.
JNIEXPORT jstring JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_stringFromJNI(JNIEnv* env, jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

/**
*
*/
extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_initialise_1scan(JNIEnv *env, jobject thiz) {
    // TODO: implement initialise_scan()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_start_1filtering(JNIEnv *env, jobject thiz) {
    // TODO: implement start_filtering()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_IBeaconDiscovered(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jobject i_beacon,
                                                                            jobject region) {
    // TODO: implement IBeaconDiscovered()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_IBeaconUpdated(JNIEnv *env, jobject thiz,
                                                                         jobject i_beacons,
                                                                         jobject region) {
    // TODO: implement IBeaconUpdated()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_indoor_1positioning_1system_MainActivity_IBeaconLost(JNIEnv *env, jobject thiz,
                                                                      jobject i_beacon,
                                                                      jobject region) {
    // TODO: implement IBeaconLost()
}
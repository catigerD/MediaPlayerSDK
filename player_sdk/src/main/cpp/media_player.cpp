#include <jni.h>
#include <string>
#include "CallJavaMgr.h"
#include "MediaPlayer.h"
#include "MediaStatus.h"

extern "C" {
#include "libavformat/avformat.h"
}

JavaVM *globalJVM = nullptr;
MediaStatus *status = nullptr;
CallJavaMgr *callJavaMgr = nullptr;
MediaPlayer *mediaPlayer = nullptr;

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *unused) {
    globalJVM = vm;
    JNIEnv *env = nullptr;
    if (globalJVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dengchong_player_1sdk_MediaPlayer_n_1prepare(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    mediaPlayer->prepare(url);
    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dengchong_player_1sdk_MediaPlayer_n_1initJVM(JNIEnv *env, jobject instance) {
    status = new MediaStatus;
    callJavaMgr = new CallJavaMgr(globalJVM, env, instance);
    mediaPlayer = new MediaPlayer(status, callJavaMgr);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dengchong_player_1sdk_MediaPlayer_n_1start(JNIEnv *env, jobject instance) {
    if (mediaPlayer != nullptr) {
        mediaPlayer->start();
    }
}
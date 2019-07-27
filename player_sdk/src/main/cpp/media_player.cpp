#include <jni.h>
#include <string>
#include "CallJavaMgr.h"
#include "MediaPlayer.h"
#include "MediaStatus.h"

extern "C" {
#include "libavformat/avformat.h"
}

JavaVM *globalJVM = nullptr;
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
    if (mediaPlayer != nullptr) {
        mediaPlayer->stop();
        delete mediaPlayer;
        mediaPlayer = nullptr;
    }
    mediaPlayer = new MediaPlayer(callJavaMgr);
    mediaPlayer->prepare(url);
    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dengchong_player_1sdk_MediaPlayer_n_1initJVM(JNIEnv *env, jobject instance) {
    callJavaMgr = new CallJavaMgr(globalJVM, env, instance);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dengchong_player_1sdk_MediaPlayer_n_1start(JNIEnv *env, jobject instance) {
    if (mediaPlayer != nullptr) {
        mediaPlayer->start();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dengchong_player_1sdk_MediaPlayer_n_1stop(JNIEnv *env, jobject instance) {
    if (mediaPlayer != nullptr) {
        mediaPlayer->stop();
        delete mediaPlayer;
        mediaPlayer = nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dengchong_player_1sdk_MediaPlayer_n_1pause(JNIEnv *env, jobject instance) {
    if (mediaPlayer != nullptr) {
        mediaPlayer->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dengchong_player_1sdk_MediaPlayer_n_1resume(JNIEnv *env, jobject instance) {
    if (mediaPlayer != nullptr) {
        mediaPlayer->resume();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dengchong_player_1sdk_MediaPlayer_n_1seek(JNIEnv *env, jobject instance, jint time) {
    if (mediaPlayer != nullptr) {
        mediaPlayer->seek(time);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_dengchong_player_1sdk_MediaPlayer_n_1duration(JNIEnv *env, jobject instance) {
    if (mediaPlayer != nullptr) {
        return mediaPlayer->duration;
    }
    return -1;
}
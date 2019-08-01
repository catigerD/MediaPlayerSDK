//
// Created by dengchong on 2019-07-24.
//

#include "CallJavaMgr.h"

CallJavaMgr::CallJavaMgr(JavaVM *vm, JNIEnv *env, jobject jo) :
        javaVM(vm),
        jniEnv(env),
        jobj(jniEnv->NewGlobalRef(jo)) {
    jclass clz = env->GetObjectClass(jobj);
    if (clz == nullptr) {
        LOGE("env->GetObjectClass(jobj) is null");
        return;
    }
    mid_prepared = env->GetMethodID(clz, "callPrepared", "()V");
    mid_completed = env->GetMethodID(clz, "callCompleted", "()V");
    mid_time_info = env->GetMethodID(clz, "callTimeInfo", "(II)V");
    mid_render = env->GetMethodID(clz, "callRender", "(II[B[B[B)V");
}

CallJavaMgr::~CallJavaMgr() {
    mid_time_info = nullptr;
    mid_completed = nullptr;
    mid_prepared = nullptr;

    jniEnv->DeleteGlobalRef(jobj);
    jniEnv = nullptr;
    javaVM = nullptr;
}

void CallJavaMgr::callPrepared() {
    if (mid_prepared == nullptr) {
        return;
    }
    JNIEnv *env;
    bool isAttach = false;
    int ret;
    ret = javaVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (JNI_OK != ret) {
        if (javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return;
        }
        isAttach = true;
    }
    env->CallVoidMethod(jobj, mid_prepared);
    if (isAttach) {
        javaVM->DetachCurrentThread();
    }
}

void CallJavaMgr::callCompleted() {
    if (mid_completed == nullptr) {
        return;
    }
    JNIEnv *env;
    bool isAttach = false;
    int ret;
    ret = javaVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (JNI_OK != ret) {
        if (javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return;
        }
        isAttach = true;
    }
    env->CallVoidMethod(jobj, mid_completed);
    if (isAttach) {
        javaVM->DetachCurrentThread();
    }
}

void CallJavaMgr::callTimeInfo(int cur, int total) {
    if (mid_time_info == nullptr) {
        return;
    }
    JNIEnv *env;
    bool isAttach = false;
    int ret;
    ret = javaVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (JNI_OK != ret) {
        if (javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return;
        }
        isAttach = true;
    }
    env->CallVoidMethod(jobj, mid_time_info, cur, total);
    if (isAttach) {
        javaVM->DetachCurrentThread();
    }
}

void CallJavaMgr::callRender(int width, int height, char *y, char *u, char *v) {
    if (mid_render == nullptr) {
        return;
    }
    JNIEnv *env;
    bool isAttach = false;
    int ret;
    ret = javaVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (JNI_OK != ret) {
        if (javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return;
        }
        isAttach = true;
    }
    jbyteArray yArray = env->NewByteArray(width * height);
    env->SetByteArrayRegion(yArray, 0, width * height, reinterpret_cast<const jbyte *>(y));
    jbyteArray uArray = env->NewByteArray(width / 2 * height / 2);
    env->SetByteArrayRegion(uArray, 0, width / 2 * height / 2, reinterpret_cast<const jbyte *>(u));
    jbyteArray vArray = env->NewByteArray(width / 2 * height / 2);
    env->SetByteArrayRegion(vArray, 0, width / 2 * height / 2, reinterpret_cast<const jbyte *>(v));

    env->CallVoidMethod(jobj, mid_render, width, height, yArray, uArray, vArray);

    env->DeleteLocalRef(yArray);
    env->DeleteLocalRef(uArray);
    env->DeleteLocalRef(vArray);
    if (isAttach) {
        javaVM->DetachCurrentThread();
    }
}






//
// Created by dengchong on 2019-07-24.
//

#include "CallJavaMgr.h"

CallJavaMgr::CallJavaMgr(JavaVM *vm, JNIEnv *env, jobject jo) :
        mVM(vm),
        mEnv(env),
        mObj(mEnv->NewGlobalRef(jo)) {
    jclass clz = env->GetObjectClass(mObj);
    if (clz == nullptr) {
        LOGE("env->GetObjectClass(mObj) is null");
        return;
    }
    mid_prepared = env->GetMethodID(clz, "callPrepared", "()V");
    mid_completed = env->GetMethodID(clz, "callCompleted", "()V");
    mid_time_info = env->GetMethodID(clz, "callTimeInfo", "(II)V");
    mid_render = env->GetMethodID(clz, "callRender", "(II[B[B[B)V");

    mEnv->DeleteLocalRef(clz);
}

CallJavaMgr::~CallJavaMgr() {
    mEnv->DeleteGlobalRef(mObj);
}

void CallJavaMgr::callPrepared() {
    callJavaMethod(mid_prepared);
}

void CallJavaMgr::callCompleted() {
    callJavaMethod(mid_completed);
}

void CallJavaMgr::callTimeInfo(int cur, int total) {
    callJavaMethod(mid_time_info, cur, total);
}

void CallJavaMgr::callRender(int width, int height, char *y, char *u, char *v) {
    if (!mid_render) {
        return;
    }
    JNIEnv *env;
    bool isAttach = false;
    int ret;
    ret = mVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (JNI_OK != ret) {
        if (mVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
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

    env->CallVoidMethod(mObj, mid_render, width, height, yArray, uArray, vArray);

    env->DeleteLocalRef(yArray);
    env->DeleteLocalRef(uArray);
    env->DeleteLocalRef(vArray);
    if (isAttach) {
        mVM->DetachCurrentThread();
    }
}

template<typename... Args>
void CallJavaMgr::callJavaMethod(const jmethodID &mid, const Args &... args) {
    if (!mid) {
        return;
    }
    JNIEnv *env;
    bool isAttach = false;
    int ret;
    ret = mVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (JNI_OK != ret) {
        if (mVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return;
        }
        isAttach = true;
    }
    env->CallVoidMethod(mObj, mid, args...);
    if (isAttach) {
        mVM->DetachCurrentThread();
    }
}






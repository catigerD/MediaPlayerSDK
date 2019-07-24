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
}

CallJavaMgr::~CallJavaMgr() {
    jniEnv->DeleteGlobalRef(jobj);
}

void CallJavaMgr::callPrepared(ThreadType type) {
    if (mid_prepared == nullptr) {
        return;
    }
    switch (type) {
        case THREAD_MAIN:
            jniEnv->CallVoidMethod(jobj, mid_prepared);
            break;
        case THREAD_CHILD:
            JNIEnv *env;
            if (javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
                LOGE("AttachCurrentThread fail");
                return;
            }
            env->CallVoidMethod(jobj, mid_prepared);
            javaVM->DetachCurrentThread();
            break;
    }
}

void CallJavaMgr::callCompleted(ThreadType type) {
    if (mid_completed == nullptr) {
        return;
    }
    switch (type) {
        case THREAD_MAIN:
            jniEnv->CallVoidMethod(jobj, mid_completed);
            break;
        case THREAD_CHILD:
            JNIEnv *env;
            if (javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
                LOGE("AttachCurrentThread fail");
                return;
            }
            env->CallVoidMethod(jobj, mid_completed);
            javaVM->DetachCurrentThread();
            break;
    }
}





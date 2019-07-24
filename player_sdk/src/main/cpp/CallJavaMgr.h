//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_CALLJAVAMGR_H
#define MEDIAPLAYERSDK_CALLJAVAMGR_H

#include <jni.h>
#include "AndroidLog.h"

enum ThreadType {
    THREAD_MAIN,
    THREAD_CHILD,
};

class CallJavaMgr {
private:
    JavaVM *javaVM = nullptr;
    JNIEnv *jniEnv = nullptr;
    jobject jobj = nullptr;

    jmethodID mid_prepared = nullptr;
    jmethodID mid_completed = nullptr;

public:
    CallJavaMgr(JavaVM *vm, JNIEnv *env, jobject jo);

    ~CallJavaMgr();

    //todo 使用可变参数优化代码？
    void callPrepared(ThreadType type);

    void callCompleted(ThreadType type);
};


#endif //MEDIAPLAYERSDK_CALLJAVAMGR_H

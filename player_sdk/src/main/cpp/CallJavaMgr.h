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
    JavaVM *mVM;
    JNIEnv *mEnv;
    jobject mObj;

    jmethodID mid_prepared;
    jmethodID mid_completed;
    jmethodID mid_time_info;
    jmethodID mid_render;

public:
    CallJavaMgr(JavaVM *vm, JNIEnv *env, jobject jo);

    ~CallJavaMgr();

    //todo 使用可变参数优化代码？
    void callPrepared();

    void callCompleted();

    void callTimeInfo(int cur, int total);

    void callRender(int width, int height, char *y, char *u, char *v);
};


#endif //MEDIAPLAYERSDK_CALLJAVAMGR_H

//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_ANDROIDLOG_H
#define MEDIAPLAYERSDK_ANDROIDLOG_H

#include <android/log.h>
#include <string>

using namespace std;

class ALog {
public:
    static const bool DEBUG = true;
    static const string TAG;

    static void d(const string format, initializer_list<const char *> args = {}) {
        if (DEBUG) {
            __android_log_print(ANDROID_LOG_DEBUG, TAG.c_str(), format.c_str(), args);
        }
    }

    static void e(const string format, initializer_list<const char *> args = {}) {
        if (DEBUG) {
            __android_log_print(ANDROID_LOG_ERROR, TAG.c_str(), format.c_str(), args);
        }
    }
};

#endif //MEDIAPLAYERSDK_ANDROIDLOG_H


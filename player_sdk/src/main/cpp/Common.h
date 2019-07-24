//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_COMMON_H
#define MEDIAPLAYERSDK_COMMON_H

extern "C" {
#include "libavutil/time.h"
};

const unsigned SLEEP_TIME = 1000 * 100;//单位微秒

inline void sleep() {
    av_usleep(SLEEP_TIME);
}

#endif //MEDIAPLAYERSDK_COMMON_H

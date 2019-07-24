//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_AUDIOMGR_H
#define MEDIAPLAYERSDK_AUDIOMGR_H

extern "C" {
#include "libavcodec/avcodec.h"
};

class AudioMgr {
private:

public:
    AudioMgr();

    ~AudioMgr();

    int streamIndex = -1;
    AVCodecParameters *codecParameters = nullptr;
    AVCodecContext *codecContext = nullptr;
};


#endif //MEDIAPLAYERSDK_AUDIOMGR_H

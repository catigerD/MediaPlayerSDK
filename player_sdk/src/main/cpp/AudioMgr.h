//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_AUDIOMGR_H
#define MEDIAPLAYERSDK_AUDIOMGR_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/time.h"
};

#include "PacketQueue.h"
#include "MediaStatus.h"
#include "AndroidLog.h"

class AudioMgr {
private:
    friend void *decodeLoop(void *data);

public:
    AudioMgr(MediaStatus *status);

    ~AudioMgr();

    int streamIndex = -1;
    AVCodecParameters *codecParameters = nullptr;
    AVCodecContext *codecContext = nullptr;

    MediaStatus *status;
    PacketQueue *pktQueue;

    //decode相关
    pthread_t decodeTid;
    bool decodeAPacketFinish = true;

    void decode();
};


#endif //MEDIAPLAYERSDK_AUDIOMGR_H

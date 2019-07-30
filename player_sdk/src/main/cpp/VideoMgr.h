//
// Created by dengchong on 2019-07-30.
//

#ifndef MEDIAPLAYERSDK_VIDEOMGR_H
#define MEDIAPLAYERSDK_VIDEOMGR_H

extern "C" {
#include "libavcodec/avcodec.h"
};

#include "MediaStatus.h"
#include "PacketQueue.h"
#include "Common.h"
#include "AndroidLog.h"

class VideoMgr {

public:
    VideoMgr(MediaStatus *status, int index, AVCodecParameters *avCodecParameters);

    ~VideoMgr();

    void start();

    void setCodecContext(AVCodecContext *context) {
        if (codecContext != nullptr) {
            avcodec_close(codecContext);
            avcodec_free_context(&codecContext);
        }
        codecContext = context;
    }

    AVCodecParameters *getCodecParameters() {
        return avCodecParameters;
    }

    int getIndex() {
        return index;
    }

    void putPacket(AVPacket *packet) {
        packetQueue.putAVPacket(packet);
    }

    int getSize() {
        return packetQueue.size();
    }

private:
    friend void *startVideoThread(void *);

    void decode();

private:
    MediaStatus *mediaStatus;
    PacketQueue packetQueue;

    int index;
    AVCodecParameters *avCodecParameters;
    AVCodecContext *codecContext;

    pthread_t startTid;
};

void *startVideoThread(void *);


#endif //MEDIAPLAYERSDK_VIDEOMGR_H

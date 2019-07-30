//
// Created by dengchong on 2019-07-30.
//

#ifndef MEDIAPLAYERSDK_VIDEOMGR_H
#define MEDIAPLAYERSDK_VIDEOMGR_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};

#include "MediaStatus.h"
#include "PacketQueue.h"
#include "Common.h"
#include "AndroidLog.h"
#include "FrameQueue.h"
#include <array>

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

    void clearFrame() {
        frameQueue.clearFrame();
    }

private:
    static const unsigned MAX_FRAME_SIZE;

    friend void *startVideoThread(void *);

    void decode();

    bool needScale(AVFrame *frame) {
        return AV_PIX_FMT_YUV420P == frame->format;
    }

    bool scaleYUV(AVFrame *inFrame, AVFrame **outFrame);

    void startPlay();

    friend void *playThread(void *data);

    void play();

private:
    MediaStatus *mediaStatus;
    PacketQueue packetQueue;

    int index;
    AVCodecParameters *avCodecParameters;
    AVCodecContext *codecContext;

    pthread_t startTid;
    FrameQueue frameQueue;

    pthread_t playTid;
};

void *startVideoThread(void *);

void *playThread(void *data);


#endif //MEDIAPLAYERSDK_VIDEOMGR_H

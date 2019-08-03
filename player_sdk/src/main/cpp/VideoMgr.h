//
// Created by dengchong on 2019-07-30.
//

#ifndef MEDIAPLAYERSDK_VIDEOMGR_H
#define MEDIAPLAYERSDK_VIDEOMGR_H

extern "C" {
#include "libavformat/avformat.h"
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
#include "CallJavaMgr.h"
#include <memory>

class VideoMgr {

public:
    VideoMgr(shared_ptr<MediaStatus> &status, CallJavaMgr *callJavaMgr, int index, AVStream *stream);

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
        if (stream != nullptr) {
            return stream->codecpar;
        }
        return nullptr;
    }

    int getIndex() {
        return index;
    }

    void putPacket(shared_ptr<AVPacket> packet) {
        packetQueue.push(packet);
    }

    void setAudioClock(double *audio_clock) {
        this->audio_clock = audio_clock;
    }

    int getSize() {
        return packetQueue.size();
    }

    void seek() {
        packetQueue.clear();
        frameQueue.clear();
    }

private:
    static const unsigned MAX_FRAME_SIZE;
    static const double DEFAULT_DELAY_TIME;
    static const double SYNCHRONIZE_SCOPE_MIN;
    static const double SYNCHRONIZE_SCOPE_MID;
    static const unsigned SYNCHRONIZE_SCOPE_MAX;

    friend void *startVideoThread(void *);

    void decode();

    bool needScale(const shared_ptr<AVFrame> &frame) const {
        return AV_PIX_FMT_YUV420P == frame->format;
    }

    bool scaleYUV(shared_ptr<AVFrame> &inFrame, shared_ptr<AVFrame> &outFrame);

    friend void *playThread(void *data);

    void play();

    void render(shared_ptr<AVFrame> frame);

    double getDiffTime(shared_ptr<AVFrame> frame);

    double getDelayTime(double diff);

private:
    shared_ptr<MediaStatus> &mediaStatus;
    CallJavaMgr *callJavaMgr;
    PacketQueue packetQueue;

    int index;
    AVStream *stream;
    AVCodecContext *codecContext;

    pthread_t startTid;
    shared_ptr<AVPacket> packet;
    shared_ptr<AVFrame> frame;
    shared_ptr<SwsContext> swsContext;
    FrameQueue frameQueue;

    pthread_t playTid;
    double *audio_clock;
    double video_clock;
    double delayTime;
};

void *startVideoThread(void *);

void *playThread(void *data);


#endif //MEDIAPLAYERSDK_VIDEOMGR_H

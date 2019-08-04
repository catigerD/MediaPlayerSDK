//
// Created by dengchong on 2019-07-30.
//

#ifndef MEDIAPLAYERSDK_VIDEOMGR_H
#define MEDIAPLAYERSDK_VIDEOMGR_H

#include "Common.h"
#include "FrameQueue.h"
#include "MediaMgr.h"
#include "AudioMgr.h"

class VideoMgr : public MediaMgr {

public:
    VideoMgr(shared_ptr<CallJavaMgr> &callJavaMgr, shared_ptr<MediaStatus> &status, int index,
             shared_ptr<AVFormatContext> &formatContext);

    ~VideoMgr();

    void start();

    int getStreamIndex() {
        return index;
    }

    void setAudioMgr(shared_ptr<AudioMgr> &audioMgr) {
        this->audioMgr = audioMgr;
    }

    void putPacket(const shared_ptr<AVPacket> &packet) {
        packetQueue->push(packet);
    }

    void seek() {
        packetQueue->clear();
        frameQueue->clear();
    }

private:
    static const unsigned MAX_FRAME_SIZE;
    static const double DEFAULT_DELAY_TIME;
    static const double SYNCHRONIZE_SCOPE_MIN;
    static const double SYNCHRONIZE_SCOPE_MID;
    static const unsigned SYNCHRONIZE_SCOPE_MAX;

    friend void *videoStartThread(void *);

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
    pthread_t startTid;
    shared_ptr<FrameQueue> frameQueue;

    shared_ptr<AVPacket> packet;
    shared_ptr<AVFrame> frame;
    shared_ptr<SwsContext> swsContext;

    pthread_t playTid;
    shared_ptr<AudioMgr> audioMgr;
    double delayTime;
};

void *videoStartThread(void *);

void *playThread(void *data);


#endif //MEDIAPLAYERSDK_VIDEOMGR_H

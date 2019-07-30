//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_MEDIAPLAYER_H
#define MEDIAPLAYERSDK_MEDIAPLAYER_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
};

#include <string>
#include "AndroidLog.h"
#include "AudioMgr.h"
#include "CallJavaMgr.h"
#include "PacketQueue.h"
#include "AndroidLog.h"
#include "Common.h"
#include "VideoMgr.h"
#include <memory>

using namespace std;

class MediaPlayer {
public:
    MediaPlayer(CallJavaMgr *callJavaMgr);

    ~MediaPlayer();

    int duration = -1;

    void prepare(const string urlParam);

    void start();

    void stop();

    void pause();

    void resume();

    void seek(int time);

    static const unsigned MAX_PACKET_SIZE;

private:

    friend void *readPacket(void *data);

    friend void *prepareThread(void *data);

    int openCodec(AVCodecParameters *avCodecParameters, AVCodecContext **avCodecContext);

    void prepareFfmpeg();

private:

    MediaStatus status;
    //回调 java 层
    CallJavaMgr *callJavaMgr;

    string url;
    AVFormatContext *formatContext;
    //audio相关
    AudioMgr *audioMgr;
    shared_ptr<VideoMgr> videoMgr;

    pthread_t readPktTid;
    pthread_t prepareTid;
    //seek
    pthread_mutex_t seek_mutex;
};

void *prepareThread(void *data);

#endif //MEDIAPLAYERSDK_MEDIAPLAYER_H

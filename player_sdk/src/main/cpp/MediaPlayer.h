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

using namespace std;

class MediaPlayer {
private:
    const unsigned MAX_PACKET_SIZE = 100;

    friend void *readPacket(void *data);

    friend void *prepareThread(void *data);

private:
    string url;
    AVFormatContext *formatContext = nullptr;

    //回调 java 层
    CallJavaMgr *callJavaMgr = nullptr;

    //audio相关
    AudioMgr *audioMgr;

    MediaStatus status;

    pthread_t readPktTid;
    pthread_t prepareTid;

    void prepareFfmpeg();

    //seek
    pthread_mutex_t seek_mutex;

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
};

void *prepareThread(void *data);

#endif //MEDIAPLAYERSDK_MEDIAPLAYER_H

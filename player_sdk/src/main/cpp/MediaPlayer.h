//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_MEDIAPLAYER_H
#define MEDIAPLAYERSDK_MEDIAPLAYER_H

#include <string>
#include "AndroidLog.h"
#include "AudioMgr.h"
#include "CallJavaMgr.h"
#include "PacketQueue.h"
#include "AndroidLog.h"
#include "Common.h"
#include "VideoMgr.h"
#include <AVWrap.h>

using namespace std;

class MediaPlayer {

public:
    MediaPlayer(shared_ptr<CallJavaMgr> &callJavaMgr);

    ~MediaPlayer();

    void prepare(const string urlParam);

    void start();

    void stop();

    void pause();

    void resume();

    int getDuration() {
        if (formatCtx) {
            return static_cast<int>(formatCtx->duration / AV_TIME_BASE);
        }
        return 0;
    }

    void seek(int time);

private:
    //回调 java 层
    shared_ptr<CallJavaMgr> &callJavaMgr;
    shared_ptr<MediaStatus> status;

    string url;
    shared_ptr<AVFormatContext> formatCtx;
    bool init;

    shared_ptr<AudioMgr> audioMgr;
    shared_ptr<VideoMgr> videoMgr;

    pthread_t prepareTid;
    pthread_t startTid;
    //seek
    pthread_mutex_t seek_mutex;

private:
    friend void *prepareThread(void *data);

    void prepareFfmpeg();

    friend void *startThread(void *data);

    void readPacket();
};

void *prepareThread(void *data);

void *startThread(void *data);

#endif //MEDIAPLAYERSDK_MEDIAPLAYER_H

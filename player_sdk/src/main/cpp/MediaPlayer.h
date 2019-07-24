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

private:
    string url;
    AVFormatContext *formatContext = nullptr;

    //回调 java 层
    CallJavaMgr *callJavaMgr = nullptr;

    MediaStatus *status;

    //audio相关
    AudioMgr *audioMgr;

    pthread_t readPktTid;

    void prepareFfmpeg();

public:
    MediaPlayer(MediaStatus *status, CallJavaMgr *callJavaMgr);

    ~MediaPlayer();

    void prepare(const string urlParam);

    void start();
};


#endif //MEDIAPLAYERSDK_MEDIAPLAYER_H

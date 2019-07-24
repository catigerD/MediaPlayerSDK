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

using namespace std;

class MediaPlayer {
private:
    string url;
    AVFormatContext *formatContext = nullptr;

    //audio相关
    AudioMgr audioMgr;

    //回调 java 层
    CallJavaMgr *callJavaMgr = nullptr;

    void prepareFfmpeg();

public:
    MediaPlayer(CallJavaMgr *callJavaMgr);

    ~MediaPlayer();

    void prepare(const string urlParam);
};


#endif //MEDIAPLAYERSDK_MEDIAPLAYER_H

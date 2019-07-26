//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_AUDIOMGR_H
#define MEDIAPLAYERSDK_AUDIOMGR_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/time.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "libswresample/swresample.h"
};

#include "PacketQueue.h"
#include "MediaStatus.h"
#include "AndroidLog.h"
#include <assert.h>

class AudioMgr {
private:
    friend void *startThread(void *data);

    friend void pcmSimpleBufferQueueCallback(
            SLAndroidSimpleBufferQueueItf caller,
            void *pContext
    );

    //decode相关
    pthread_t startTid;
    bool decodeAPacketFinish = true;
    int sample_rate = 0;
    uint8_t *swrBuf = nullptr;

    //OpenSL
    //引擎接口
    SLObjectItf engineObject;
    SLEngineItf engineItf;
    //混响器
    SLObjectItf outputMixObject;
    SLEnvironmentalReverbItf environmentalReverbItf;
    SLEnvironmentalReverbSettings environmentalReverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;//不要忘记
    //播放器
    SLObjectItf playObject;
    SLPlayItf playItf;
    //缓冲器队列
    SLAndroidSimpleBufferQueueItf androidSimpleBufferQueueItf;

    void initOpenSLESEnv();

    void createEngine();

    void createBufferQueueAudioPlayer();

    void destroyOpenSLES();

    //解码相关资源
    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;
    SwrContext *swrContext = nullptr;
    void releaseDecodeRes();

    int decode(uint8_t **outputBuf, int *size);

    SLuint32 getCurrentSimpleRate();

public:
    AudioMgr(MediaStatus *status, AVStream *stream);

    ~AudioMgr();

    MediaStatus *status;
    PacketQueue *pktQueue;

    int streamIndex = -1;
    AVStream *stream = nullptr;
    AVCodecContext *codecContext = nullptr;

    void start();

    void stop();

    void pause();

    void resume();
};


#endif //MEDIAPLAYERSDK_AUDIOMGR_H

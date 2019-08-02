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
#include "CallJavaMgr.h"
#include "Common.h"

class AudioMgr {
private:
    friend void *startThread(void *data);

    friend void pcmSimpleBufferQueueCallback(
            SLAndroidSimpleBufferQueueItf caller,
            void *pContext
    );

    //音频属性
    int64_t duration;

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

    //回调 java 层
    CallJavaMgr *callJavaMgr;

    void callJavaTimeInfo(int cur, int total);

public:
    AudioMgr(CallJavaMgr *callJavaMgr, MediaStatus *status, int index, AVStream *stream, int64_t duration);

    ~AudioMgr();

    MediaStatus *status;
    AVStream *stream = nullptr;

    PacketQueue *pktQueue;
    int streamIndex;
    AVCodecContext *codecContext = nullptr;
    //当前播放时间
    int clock = 0;
    //减少 java 层调用间隔
    int last_clock = 0;

    void start();

    void stop();

    void pause();

    void resume();

    void seek();
};

void pcmSimpleBufferQueueCallback(
        SLAndroidSimpleBufferQueueItf caller,
        void *pContext
);


#endif //MEDIAPLAYERSDK_AUDIOMGR_H

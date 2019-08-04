//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_AUDIOMGR_H
#define MEDIAPLAYERSDK_AUDIOMGR_H

extern "C" {
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};

#include "Common.h"
#include "MediaMgr.h"
#include "assert.h"

class AudioMgr : public MediaMgr {


public:
    AudioMgr(shared_ptr<CallJavaMgr> &callJavaMgr, shared_ptr<MediaStatus> &status, int index,
             shared_ptr<AVFormatContext> &formatContext);

    ~AudioMgr();

    void start();

    void stop();

    void pause();

    void resume();

    void seek() {
        clock = 0;
        last_clock = 0;
        packetQueue->clear();
        avcodec_flush_buffers(codecContext.get());
    }

private:
    friend void *audioStartThread(void *data);

    void initOpenSLESEnv();

    void createEngine();

    void createBufferQueueAudioPlayer();

    void startDecode();

    void destroyOpenSLES();

    friend void pcmSimpleBufferQueueCallback(
            SLAndroidSimpleBufferQueueItf caller,
            void *pContext
    );

    int decode();

    void play();

    SLuint32 getCurrentSimpleRate();

    void callJavaTimeInfo(int cur, int total);

private:

    pthread_t startTid;
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

    //音频某些格式一个 packet 可能包含多个 frame
    bool gotPacket;
    int sample_rate;
    shared_ptr<uint8_t> convertBuffer;

    //减少 java 层调用间隔
    double last_clock = 0;

    //解码相关资源
    shared_ptr<AVPacket> packet;//正在解码的 packet
    shared_ptr<AVFrame> frame;//解码完整的 frame
    shared_ptr<SwrContext> swrContext;
};

void *audioStartThread(void *data);

void pcmSimpleBufferQueueCallback(
        SLAndroidSimpleBufferQueueItf caller,
        void *pContext
);


#endif //MEDIAPLAYERSDK_AUDIOMGR_H

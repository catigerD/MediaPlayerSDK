//
// Created by dengchong on 2019-07-24.
//

#include "AudioMgr.h"

AudioMgr::AudioMgr(shared_ptr<CallJavaMgr> &callJavaMgr, shared_ptr<MediaStatus> &status, int index,
                   shared_ptr<AVFormatContext> &formatContext)
        : MediaMgr(callJavaMgr, status, index, formatContext),
          gotPacket(true),
          sample_rate(formatContext->streams[index]->codecpar->sample_rate),
          convertBuffer(shared_ptr<uint8_t>(new uint8_t[sample_rate * 2 * 2])) {

}

AudioMgr::~AudioMgr() {
    destroyOpenSLES();
}


void AudioMgr::start() {
    pthread_create(&startTid, nullptr, audioStartThread, this);
}

void *audioStartThread(void *data) {
    AudioMgr *audioMgr = static_cast<AudioMgr *>(data);
    audioMgr->initOpenSLESEnv();
    audioMgr->startDecode();
    pthread_exit(&audioMgr->startTid);
}

void AudioMgr::initOpenSLESEnv() {
    createEngine();
    createBufferQueueAudioPlayer();
}

void AudioMgr::startDecode() {
    if (androidSimpleBufferQueueItf) {
        //启动缓冲队列回调
        pcmSimpleBufferQueueCallback(androidSimpleBufferQueueItf, this);
    }
}

void AudioMgr::createEngine() {
    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineItf);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // create output mix, with environmental reverb specified as a non-required interface
    //创建输出混合，将环境混响指定为非必需接口
    const SLInterfaceID outputMixIIDs[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean outputMixReqs[1] = {SL_BOOLEAN_FALSE};
    result = (*engineItf)->CreateOutputMix(engineItf, &outputMixObject, 1, outputMixIIDs, outputMixReqs);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &environmentalReverbItf);
    if (SL_RESULT_SUCCESS == result) {
        result = (*environmentalReverbItf)->SetEnvironmentalReverbProperties(environmentalReverbItf,
                                                                             &environmentalReverbSettings);
        (void) result;
    }
    // ignore unsuccessful result codes for environmental reverb, as it is optional for this example
}

void AudioMgr::createBufferQueueAudioPlayer() {
    SLresult result;
    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue androidSimpleBufferQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM pcmFormat = {
            SL_DATAFORMAT_PCM,
            2,
            getCurrentSimpleRate(),//需根据codecParams.sample_rates设置
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_RIGHT | SL_SPEAKER_FRONT_LEFT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource dataSource = {&androidSimpleBufferQueue, &pcmFormat};

    // configure audio sink音频接收器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink dataSink = {&outputMix, nullptr};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID playIIds[2] = {SL_IID_BUFFERQUEUE,
                                       SL_IID_PLAYBACKRATE/*码率切换更平滑*/};
    const SLboolean playReqs[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineItf)->CreateAudioPlayer(engineItf, &playObject, &dataSource, &dataSink, 2, playIIds, playReqs);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the player
    result = (*playObject)->Realize(playObject, SL_BOOLEAN_FALSE);
    // this will always succeed on Android, but we check result for portability to other platforms
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the buffer queue interface
    result = (*playObject)->GetInterface(playObject, SL_IID_BUFFERQUEUE, &androidSimpleBufferQueueItf);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // register callback on the buffer queue
    result = (*androidSimpleBufferQueueItf)->RegisterCallback(androidSimpleBufferQueueItf, pcmSimpleBufferQueueCallback,
                                                              this);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the startPlay interface
    result = (*playObject)->GetInterface(playObject, SL_IID_PLAY, &playItf);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // set the player's state to playing
    result = (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
}

void AudioMgr::destroyOpenSLES() {
    if (playObject != nullptr) {
        (*playObject)->Destroy(playObject);
        playItf = nullptr;
        androidSimpleBufferQueueItf = nullptr;
    }
    if (outputMixObject != nullptr) {
        (*outputMixObject)->Destroy(outputMixObject);
        environmentalReverbItf = nullptr;
    }
    if (engineObject != nullptr) {
        (*engineObject)->Destroy(engineObject);
        engineItf = nullptr;
    }
}

void AudioMgr::callJavaTimeInfo(int cur, int total) {
    if (callJavaMgr != nullptr) {
        callJavaMgr->callTimeInfo(cur, total);
    }
}

void pcmSimpleBufferQueueCallback(
        SLAndroidSimpleBufferQueueItf caller,
        void *pContext
) {
    auto *audioMgr = static_cast<AudioMgr *>(pContext);
    audioMgr->play();
}

void AudioMgr::play() {
    int size = decode();
    LOGI("pcmSimpleBufferQueueCallback: size : %d", size);
    if (size > 0) {
        clock += (double) size / (sample_rate * 2 * 2);
        if (clock - last_clock > 0.1) {
            callJavaTimeInfo(static_cast<int>(clock), static_cast<int>(formatContext->duration / AV_TIME_BASE));
            last_clock = clock;
        }
        (*androidSimpleBufferQueueItf)->Enqueue(
                androidSimpleBufferQueueItf, convertBuffer.get(), static_cast<SLuint32>(size));
    }
}

int AudioMgr::decode() {
    int samples = 0;
    int errorCode = 0;
    int count = 0;
    while (status && !status->exit) {

        if (status && (status->seek || status->pause)) {
            sleep();
            continue;
        }

        status->decode = true;

        if (!hasData()) {
            status->decode = false;
            sleep();
            continue;
        }

        if (gotPacket) {
            if (!packetQueue->pop(packet)) {
                status->decode = false;
                continue;
            }
            errorCode = avcodec_send_packet(codecContext.get(), packet.get());
            if (errorCode != 0) {
                LOGE("audio avcodec_send_packet failed ... ,error msg : %s", av_err2str(errorCode));
                gotPacket = true;
                status->decode = false;
                continue;
            }
        }
        frame = AVWrap::allocAVFrame();
        errorCode = avcodec_receive_frame(codecContext.get(), frame.get());
        if (errorCode != 0) {
            LOGI("audio avcodec_receive_frame failed ...  ,error msg : %s", av_err2str(errorCode));
            gotPacket = true;
            status->decode = false;
            continue;
        }
        swrContext = AVWrap::allocSwrContext(frame);
        if (!swrContext) {
            LOGI("swrContext init failed ... ,error msg");
            gotPacket = true;
            status->decode = false;
            continue;
        }
        gotPacket = false;
        uint8_t *tempBuf = convertBuffer.get();
        samples = swr_convert(swrContext.get(),
                              &tempBuf,
                              frame->nb_samples,
                              reinterpret_cast<const uint8_t **>(&frame->data),
                              frame->nb_samples);

        double cur_time = packet->pts * av_q2d(formatContext->streams[index]->time_base);
        if (cur_time < clock) {
            cur_time = clock;
        }
        clock = cur_time;
        status->decode = false;
        LOGI("AudioMgr::decode() : count %d", count++);

        break;
    }
    return samples * 2 * 2;
}

SLuint32 AudioMgr::getCurrentSimpleRate() {
    SLuint32 rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void AudioMgr::stop() {
    if (playItf) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_STOPPED);
    }
}

void AudioMgr::pause() {
    if (playItf) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PAUSED);
    }
}

void AudioMgr::resume() {
    if (playItf) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    }
}

void AudioMgr::seek() {
    clock = 0;
    last_clock = 0;
    packetQueue->clear();
    avcodec_flush_buffers(codecContext.get());
}

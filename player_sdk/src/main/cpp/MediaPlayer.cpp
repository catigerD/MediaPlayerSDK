//
// Created by dengchong on 2019-07-24.
//

#include "MediaPlayer.h"

MediaPlayer::MediaPlayer(shared_ptr<CallJavaMgr> &callJavaMg)
        : callJavaMgr(callJavaMg),
          status(make_shared<MediaStatus>()),
          url(),
          formatCtx(),
          init(false),
          audioMgr(),
          videoMgr(),
          prepareTid(),
          startTid(),
          seek_mutex() {
    pthread_mutex_init(&seek_mutex, nullptr);
}

MediaPlayer::~MediaPlayer() {
    pthread_mutex_destroy(&seek_mutex);
}

void MediaPlayer::prepare(const string urlParam) {
    url = urlParam;
    pthread_create(&prepareTid, nullptr, prepareThread, this);
}

void *prepareThread(void *data) {
    auto *mediaPlayer = static_cast<MediaPlayer *>(data);
    mediaPlayer->prepareFfmpeg();
    pthread_exit(&mediaPlayer->prepareTid);
}

void MediaPlayer::prepareFfmpeg() {
    int ret;//保存函数操作结果码

    //1.注册协议，复用，编解码器
    av_register_all();
    avformat_network_init();

    //2.读取文件头信息，填充AVInputFormat到 AVFormatContext 结构体
    formatCtx = AVWrap::allocAVFormatContext();//分配内存
    AVFormatContext *tempFormatCtx = formatCtx.get();
    if (!formatCtx) {
        LOGE("avformat_alloc_context() fail for url : %s", url.c_str());
        return;
    }

    ret = avformat_open_input(&tempFormatCtx, url.c_str(), nullptr, nullptr);
    if (ret != 0) {
        LOGE("avformat_open_input() fail for url : %s , error msg : %s", url.c_str(), av_err2str(ret));
        return;
    }

    //3.读取流信息
    ret = avformat_find_stream_info(tempFormatCtx, nullptr);
    if (ret < 0) {
        LOGE("avformat_find_stream_info() fail for url : %s , error msg : %s", url.c_str(), av_err2str(ret));
        return;
    }

    for (int i = 0; i < formatCtx->nb_streams; i++) {
        if (AVMEDIA_TYPE_AUDIO == formatCtx->streams[i]->codecpar->codec_type) {
            audioMgr = make_shared<AudioMgr>(callJavaMgr, status, i, formatCtx);

        } else if (AVMEDIA_TYPE_VIDEO == formatCtx->streams[i]->codecpar->codec_type) {
            videoMgr = make_shared<VideoMgr>(callJavaMgr, status, i, formatCtx);
        }
    }

    if (!audioMgr || !videoMgr) {
        LOGE("audioMgr or videoMgr is null for url : %s", url.c_str());
        return;
    }

    videoMgr->setAudioMgr(audioMgr);

    //4.获取解码器并打开
    if (!audioMgr->openCodec()) {
        return;
    }
    if (!videoMgr->openCodec()) {
        return;
    }

    callJavaMgr->callPrepared();
    LOGI("准备好了！！！");

    init = true;
}

void MediaPlayer::start() {
    pthread_create(&startTid, nullptr, startThread, this);
}

void *startThread(void *data) {
    MediaPlayer *mediaPlayer = static_cast<MediaPlayer *>(data);
    mediaPlayer->readPacket();
    pthread_exit(&mediaPlayer->startTid);
}

void MediaPlayer::readPacket() {
    if (!init) {
        return;
    }

    int acount = 0;
    int vcount = 0;
    int ret = 0;
    audioMgr->start();
    videoMgr->start();

    while (!status->exit) {

        if (status->seek || status->pause) {
            sleep();
            continue;
        }
        status->read = true;

        if (!videoMgr->canFill() || !videoMgr->canFill()) {
            sleep();
            continue;
        }

        shared_ptr<AVPacket> packet = AVWrap::allocAVPacket();

        pthread_mutex_lock(&seek_mutex);
        ret = av_read_frame(formatCtx.get(), packet.get());
        pthread_mutex_unlock(&seek_mutex);
        if (ret == 0) {
            if (packet->stream_index == audioMgr->getStreamIndex()) {
                audioMgr->putPacket(packet);
                LOGI("MediaPlayer::readPacket() audio : count %d", acount++);
            } else if (packet->stream_index == videoMgr->getStreamIndex()) {
                videoMgr->putPacket(packet);
                LOGI("MediaPlayer::readPacket() video : count %d", vcount++);
            }
        } else {
            //无数据包了，等待数据包消耗完成
            while (!status->exit) {
                if (audioMgr->hasData() || videoMgr->hasData()) {
                    sleep();
                } else {
                    status->exit = true;
                }
            }
        }
    }

    callJavaMgr->callCompleted();
    status->read = false;
}

void MediaPlayer::stop() {
    status->exit = true;

    if (audioMgr != nullptr) {
        audioMgr->stop();
    }

    while (status->read || status->decode) {
        sleep();
    }
    sleep();
}

void MediaPlayer::pause() {
    status->pause = true;
    if (audioMgr != nullptr) {
        audioMgr->pause();
    }
}

void MediaPlayer::resume() {
    status->pause = false;
    if (audioMgr != nullptr) {
        audioMgr->resume();
    }
}

//耗时，需要子线程中调用
void MediaPlayer::seek(int time) {
    if (formatCtx->duration < 0) {
        //直播流
        return;
    }
    if (time < 0 || time > formatCtx->duration / AV_TIME_BASE) {
        return;
    }
    if (audioMgr == nullptr) {
        return;
    }
    if (videoMgr == nullptr) {
        return;
    }
    if (!status->seek) {
        status->seek = true;
        int64_t rel = time * AV_TIME_BASE;
        pthread_mutex_lock(&seek_mutex);
        audioMgr->seek();
        videoMgr->seek();
//        avformat_seek_file(formatCtx, audioMgr->streamIndex, INT64_MIN, rel, INT64_MAX, 0);
        avformat_seek_file(formatCtx.get(), -1, INT64_MIN, rel, INT64_MAX, 0);
        pthread_mutex_unlock(&seek_mutex);
        status->seek = false;
    }
}



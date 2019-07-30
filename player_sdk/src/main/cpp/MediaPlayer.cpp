//
// Created by dengchong on 2019-07-24.
//

#include "MediaPlayer.h"

const unsigned MediaPlayer::MAX_PACKET_SIZE = 100;

MediaPlayer::MediaPlayer(CallJavaMgr *callJavaMg) :
        status(),
        callJavaMgr(callJavaMg),
        formatContext(nullptr),
        audioMgr(nullptr),
        videoMgr(nullptr) {
    pthread_mutex_init(&seek_mutex, nullptr);
}

MediaPlayer::~MediaPlayer() {
    delete audioMgr;
    audioMgr = nullptr;

    pthread_mutex_destroy(&seek_mutex);
    callJavaMgr = nullptr;
}

void MediaPlayer::prepare(const string urlParam) {
    url = urlParam;
    pthread_create(&prepareTid, nullptr, prepareThread, this);
}

void *prepareThread(void *data) {
    MediaPlayer *mediaPlayer = static_cast<MediaPlayer *>(data);
    mediaPlayer->prepareFfmpeg();
    pthread_exit(&mediaPlayer->prepareTid);
}

void MediaPlayer::prepareFfmpeg() {
    int ret = 0;//保存函数操作结果码

    //1.注册协议，复用，编解码器
    av_register_all();
    avformat_network_init();

    //2.读取文件头信息，填充AVInputFormat到 AVFormatContext 结构体
    formatContext = avformat_alloc_context();//分配内存,可能存在分配内存错误情况
    if (formatContext == nullptr) {
        LOGE("avformat_alloc_context() fail for url : %s", url.c_str());
        return;
    }
    ret = avformat_open_input(&formatContext, url.c_str(), nullptr, nullptr);
    if (ret != 0) {
        LOGE("avformat_open_input() fail for url : %s , error msg : %s", url.c_str(), av_err2str(ret));
        return;
    }

    //3.读取流信息
    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret < 0) {
        LOGE("avformat_find_stream_info() fail for url : %s , error msg : %s", url.c_str(), av_err2str(ret));
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (AVMEDIA_TYPE_AUDIO == formatContext->streams[i]->codecpar->codec_type) {
            duration = formatContext->duration / AV_TIME_BASE;
            audioMgr = new AudioMgr(callJavaMgr, &status, i, formatContext->streams[i],
                                    formatContext->duration / AV_TIME_BASE);

        } else if (AVMEDIA_TYPE_VIDEO == formatContext->streams[i]->codecpar->codec_type) {
            videoMgr = make_shared<VideoMgr>(&status, i, formatContext->streams[i]->codecpar);
        }
    }

    if (audioMgr == nullptr) {
        LOGE("audioMgr.streamIndex < 0 for url : %s", url.c_str());
        return;
    }

    if (videoMgr == nullptr) {
        LOGE("videoMgr.streamIndex < 0 for url : %s", url.c_str());
        return;
    }

    //4.获取解码器并打开
    ret = openCodec(audioMgr->stream->codecpar, &audioMgr->codecContext);
    if (ret != 0) {
        LOGE("openCodec audio fail for url : %s", url.c_str());
        return;
    }
    AVCodecContext *vCodecContext = nullptr;
    ret = openCodec((*videoMgr).getCodecParameters(), &vCodecContext);
    if (ret != 0) {
        LOGE("openCodec video fail for url : %s", url.c_str());
        return;
    }
    (*videoMgr).setCodecContext(vCodecContext);

    callJavaMgr->callPrepared();
    LOGI("准备好了！！！");
}

int MediaPlayer::openCodec(AVCodecParameters *avCodecParameters, AVCodecContext **avCodecContext) {
    AVCodec *codec = avcodec_find_decoder(avCodecParameters->codec_id);
    if (codec == nullptr) {
        LOGE("avcodec_find_decoder() fail for url : %s", url.c_str());
        return -1;
    }
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (codecContext == nullptr) {
        LOGE("avcodec_alloc_context3() fail for url : %s", url.c_str());
        return -1;
    }
    int ret = 0;
    ret = avcodec_parameters_to_context(codecContext, avCodecParameters);
    if (ret < 0) {
        LOGE("avcodec_parameters_to_context() fail for url : %s , error msg : %s", url.c_str(), av_err2str(ret));
        return -1;
    }
    *avCodecContext = codecContext;
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret != 0) {
        LOGE("avcodec_open2() fail for url : %s , error msg : %s", url.c_str(), av_err2str(ret));
        return -1;
    }
    return 0;
}

void *readPacket(void *data) {
    MediaPlayer *mediaPlayer = static_cast<MediaPlayer *>(data);

    if (mediaPlayer->audioMgr == nullptr) {
        return nullptr;
    }

    int ret = 0;
    int count = 0;
    int vcount = 0;
    mediaPlayer->audioMgr->start();
    mediaPlayer->videoMgr.operator*().start();

    while (!mediaPlayer->status.exit) {
        if (mediaPlayer->status.seek) {
            sleep();
            continue;
        }
        mediaPlayer->status.read = true;
        if (mediaPlayer->audioMgr->pktQueue->size() > mediaPlayer->MAX_PACKET_SIZE
            || mediaPlayer->videoMgr.operator*().getSize() > mediaPlayer->MAX_PACKET_SIZE) {
            sleep();
            continue;
        }
        AVPacket *packet = av_packet_alloc();
        pthread_mutex_lock(&mediaPlayer->seek_mutex);
        ret = av_read_frame(mediaPlayer->formatContext, packet);
        pthread_mutex_unlock(&mediaPlayer->seek_mutex);
        if (ret == 0) {
            if (packet->stream_index == mediaPlayer->audioMgr->streamIndex) {
                mediaPlayer->audioMgr->pktQueue->putAVPacket(packet);
                LOGI("a --- 读取第 %d 帧包", count++);
            } else if (packet->stream_index == mediaPlayer->videoMgr.operator*().getIndex()) {
                mediaPlayer->videoMgr.operator*().putPacket(packet);
                LOGI("v --- 读取第 %d 帧包", vcount++);
            } else {
                av_packet_free(&packet);
                av_free(packet);
            }
        } else {
            av_packet_free(&packet);
            av_free(packet);
            //等待数据包消耗完成
            while (!mediaPlayer->status.exit) {
                if (mediaPlayer->audioMgr->pktQueue->size() > 0) {
                    sleep();
                } else {
                    mediaPlayer->status.exit = true;
                }
            }
        }
    }

    LOGI("读取完成----------");
    mediaPlayer->callJavaMgr->callCompleted();

    mediaPlayer->status.read = false;
    pthread_exit(&mediaPlayer->readPktTid);
}

void MediaPlayer::start() {
    pthread_create(&readPktTid, nullptr, readPacket, this);
}

void MediaPlayer::stop() {
    status.exit = true;

    if (audioMgr != nullptr) {
        audioMgr->stop();
    }

    while (status.read || status.decode) {
        sleep();
    }
    sleep();
}

void MediaPlayer::pause() {
    if (audioMgr != nullptr) {
        audioMgr->pause();
    }
}

void MediaPlayer::resume() {
    if (audioMgr != nullptr) {
        audioMgr->resume();
    }
}

//耗时，需要子线程中调用
void MediaPlayer::seek(int time) {
    if (formatContext->duration < 0) {
        //直播流
        return;
    }
    if (time < 0 || time > formatContext->duration / AV_TIME_BASE) {
        return;
    }
    if (audioMgr == nullptr) {
        return;
    }
    if (!status.seek) {
        status.seek = true;
        audioMgr->pktQueue->clearAVPacket();
        audioMgr->clock = 0;
        audioMgr->last_clock = 0;
        int64_t rel = time * AV_TIME_BASE;
        pthread_mutex_lock(&seek_mutex);
        avcodec_flush_buffers(audioMgr->codecContext);
//        avformat_seek_file(formatContext, audioMgr->streamIndex, INT64_MIN, rel, INT64_MAX, 0);
        avformat_seek_file(formatContext, -1, INT64_MIN, rel, INT64_MAX, 0);
        pthread_mutex_unlock(&seek_mutex);
        status.seek = false;
    }
}



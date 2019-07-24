//
// Created by dengchong on 2019-07-24.
//

#include "MediaPlayer.h"

MediaPlayer::MediaPlayer(MediaStatus *status, CallJavaMgr *callJavaMg) :
        callJavaMgr(callJavaMg),
        status(status) {
}

MediaPlayer::~MediaPlayer() {
    delete audioMgr;
}

void MediaPlayer::prepare(const string urlParam) {
    url = urlParam;
    prepareFfmpeg();
}

void MediaPlayer::prepareFfmpeg() {
    int ret = 0;//保存函数操作结果码

    //1.注册协议，复用，编解码器
    av_register_all();

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
            audioMgr = new AudioMgr(status);
            audioMgr->streamIndex = i;
            audioMgr->codecParameters = formatContext->streams[i]->codecpar;
            break;
        }
    }

    if (audioMgr->streamIndex < 0) {
        LOGE("audioMgr.streamIndex < 0 for url : %s", url.c_str());
        return;
    }

    //4.获取解码器并打开
    AVCodec *codec = avcodec_find_decoder(audioMgr->codecParameters->codec_id);
    if (codec == nullptr) {
        LOGE("avcodec_find_decoder() fail for url : %s", url.c_str());
        return;
    }
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (codecContext == nullptr) {
        LOGE("avcodec_alloc_context3() fail for url : %s", url.c_str());
        return;
    }
    ret = avcodec_parameters_to_context(codecContext, audioMgr->codecParameters);
    if (ret < 0) {
        LOGE("avcodec_parameters_to_context() fail for url : %s , error msg : %s", url.c_str(), av_err2str(ret));
        return;
    }
    audioMgr->codecContext = codecContext;
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret != 0) {
        LOGE("avcodec_open2() fail for url : %s , error msg : %s", url.c_str(), av_err2str(ret));
    }

    callJavaMgr->callPrepared(THREAD_MAIN);
    LOGI("准备好了！！！");
}

void *readPacket(void *data) {
    MediaPlayer *mediaPlayer = static_cast<MediaPlayer *>(data);

    if (mediaPlayer->audioMgr == nullptr) {
        return nullptr;
    }

    int ret = 0;
    int count = 0;
    mediaPlayer->audioMgr->decode();

    while (!mediaPlayer->status->exit) {
        if (mediaPlayer->audioMgr->pktQueue->size() > mediaPlayer->MAX_PACKET_SIZE) {
            sleep();
            continue;
        }
        AVPacket *packet = av_packet_alloc();
        ret = av_read_frame(mediaPlayer->formatContext, packet);
        if (ret == 0) {
            if (packet->stream_index == mediaPlayer->audioMgr->streamIndex) {
                mediaPlayer->audioMgr->pktQueue->putAVPacket(packet);
                LOGI("读取第 %d 帧包", count++);
            } else {
                av_packet_free(&packet);
                av_free(packet);
            }
        } else {
            av_packet_free(&packet);
            av_free(packet);
            //等待数据包消耗完成
            while (!mediaPlayer->status->exit) {
                if (mediaPlayer->audioMgr->pktQueue->size() > 0) {
                    sleep();
                } else {
                    mediaPlayer->status->exit = true;
                }
            }
        }
    }

    LOGI("读取完成----------");
    mediaPlayer->callJavaMgr->callCompleted(THREAD_CHILD);

    pthread_exit(&mediaPlayer->readPktTid);
}

void MediaPlayer::start() {
    pthread_create(&readPktTid, nullptr, readPacket, this);
}



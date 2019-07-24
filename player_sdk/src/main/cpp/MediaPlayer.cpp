//
// Created by dengchong on 2019-07-24.
//

#include "MediaPlayer.h"

MediaPlayer::MediaPlayer(CallJavaMgr *callJavaMg) : callJavaMgr(callJavaMg) {

}

MediaPlayer::~MediaPlayer() {

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
        ALog::e("avformat_alloc_context() fail for url : %s", {url.c_str()});
        return;
    }
    ret = avformat_open_input(&formatContext, url.c_str(), nullptr, nullptr);
    if (ret != 0) {
        ALog::e("avformat_open_input() fail for url : %s , error msg : %s", {url.c_str(), av_err2str(ret)});
        return;
    }

    //3.读取流信息
    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret < 0) {
        ALog::e("avformat_find_stream_info() fail for url : %s , error msg : %s", {url.c_str(), av_err2str(ret)});
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (AVMEDIA_TYPE_AUDIO == formatContext->streams[i]->codecpar->codec_type) {
            audioMgr.streamIndex = i;
            audioMgr.codecParameters = formatContext->streams[i]->codecpar;
            break;
        }
    }

    if (audioMgr.streamIndex < 0) {
        ALog::e("audioMgr.streamIndex < 0 for url : %s", {url.c_str()});
        return;
    }

    //4.获取解码器并打开
    AVCodec *codec = avcodec_find_decoder(audioMgr.codecParameters->codec_id);
    if (codec == nullptr) {
        ALog::e("avcodec_find_decoder() fail for url : %s", {url.c_str()});
        return;
    }
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (codecContext == nullptr) {
        ALog::e("avcodec_alloc_context3() fail for url : %s", {url.c_str()});
        return;
    }
    ret = avcodec_parameters_to_context(codecContext, audioMgr.codecParameters);
    if (ret < 0) {
        ALog::e("avcodec_parameters_to_context() fail for url : %s , error msg : %s", {url.c_str(), av_err2str(ret)});
        return;
    }
    audioMgr.codecContext = codecContext;
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret != 0) {
        ALog::e("avcodec_open2() fail for url : %s , error msg : %s", {url.c_str(), av_err2str(ret)});
    }

    callJavaMgr->callPrepared(THREAD_MAIN);
    ALog::d("准备好了！！！");
}

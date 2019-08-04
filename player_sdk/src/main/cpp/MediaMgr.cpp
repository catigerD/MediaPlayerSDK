//
// Created by dengchong on 2019-08-03.
//

#include "MediaMgr.h"

const unsigned MediaMgr::DEFAULT_MAX_QUEUE_SIZE = 20;

MediaMgr::MediaMgr(shared_ptr<CallJavaMgr> &callJavaMgr, shared_ptr<MediaStatus> &status, int index,
                   shared_ptr<AVFormatContext> &formatContext)
        : callJavaMgr(callJavaMgr),
          status(status),
          index(index),
          formatContext(formatContext),
          codecContext(),
          packetQueue(new PacketQueue(status)),
          clock(0) {

}

bool MediaMgr::openCodec() {
    AVStream *stream = formatContext->streams[index];

    AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        LOGE("avcodec_find_decoder() failed ...");
        return false;
    }

    codecContext = AVWrap::allocAVCodecContext(codec);
    if (!codecContext) {
        LOGE("avcodec_alloc_context3() failed ...");
        return false;
    }

    int ret = 0;
    ret = avcodec_parameters_to_context(codecContext.get(), stream->codecpar);
    if (ret < 0) {
        LOGE("avcodec_parameters_to_context() failed ...  , error msg : %s", av_err2str(ret));
        return false;
    }

    ret = avcodec_open2(codecContext.get(), codec, nullptr);
    if (ret != 0) {
        LOGE("avcodec_open2() failed ... , error msg : %s", av_err2str(ret));
        return false;
    }
    return true;
}

MediaMgr::~MediaMgr() {

}

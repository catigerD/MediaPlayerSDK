//
// Created by dengchong on 2019-08-03.
//

#ifndef MEDIAPLAYERSDK_AVWRAP_H
#define MEDIAPLAYERSDK_AVWRAP_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
};

#include <memory>

using namespace std;

inline void freeFormatContext(AVFormatContext *context) {
    avformat_close_input(&context);
    avformat_free_context(context);
}

inline void freeCodecContext(AVCodecContext *context) {
    avcodec_close(context);
    avcodec_free_context(&context);
}

inline void freeAVPacket(AVPacket *packet) {
    av_packet_free(&packet);
    av_free(packet);
}

inline void freeAVFrame(AVFrame *frame) {
    av_frame_free(&frame);
    av_free(frame);
}

inline void freeSwsContext(SwsContext *context) {
    sws_freeContext(context);
}

inline void freeSwrContext(SwrContext *context) {
    swr_free(&context);
}

class AVWrap {
public:
    static shared_ptr<AVFormatContext> allocAVFormatContext() {
        return shared_ptr<AVFormatContext>(avformat_alloc_context(), freeFormatContext);
    }

    static shared_ptr<AVCodecContext> allocAVCodecContext(AVCodec *codec) {
        return shared_ptr<AVCodecContext>(avcodec_alloc_context3(codec), freeCodecContext);
    }

    static shared_ptr<AVPacket> allocAVPacket() {
        return shared_ptr<AVPacket>(av_packet_alloc(), freeAVPacket);
    }

    static shared_ptr<AVFrame> allocAVFrame() {
        return shared_ptr<AVFrame>(av_frame_alloc(), freeAVFrame);
    }

    static shared_ptr<SwsContext> allocSwsContext() {
        return shared_ptr<SwsContext>(sws_alloc_context(), freeSwsContext);
    }

    static shared_ptr<SwsContext> allocSwsContext(const shared_ptr<AVFrame> &frame) {
        shared_ptr<SwsContext> swsContext(sws_getContext(frame->width,
                                                         frame->height,
                                                         static_cast<AVPixelFormat>(frame->format),
                                                         frame->width,
                                                         frame->height,
                                                         AV_PIX_FMT_YUV420P,
                                                         SWS_BICUBIC,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr), freeSwsContext);
        return swsContext;
    }

    static shared_ptr<SwrContext> allocSwrContext() {
        return shared_ptr<SwrContext>(swr_alloc(), freeSwrContext);
    }
};


#endif //MEDIAPLAYERSDK_AVWRAP_H

//
// Created by dengchong on 2019-07-30.
//

#include "VideoMgr.h"

const unsigned VideoMgr::MAX_FRAME_SIZE = 3;
const double VideoMgr::DEFAULT_DELAY_TIME = 0.04;
const double VideoMgr::SYNCHRONIZE_SCOPE_MIN = 0.003;
const double VideoMgr::SYNCHRONIZE_SCOPE_MID = 0.5;
const unsigned VideoMgr::SYNCHRONIZE_SCOPE_MAX = 10;

VideoMgr::VideoMgr(MediaStatus *status, CallJavaMgr *callJavaMgr, int index, AVStream *stream)
        : mediaStatus(status),
          callJavaMgr(callJavaMgr),
          packetQueue(status),
          index(index),
          stream(stream),
          codecContext(nullptr),
          frameQueue(status),
          audio_clock(nullptr),
          video_clock(0),
          delayTime(0) {

}

VideoMgr::~VideoMgr() {

}

void VideoMgr::start() {
    pthread_create(&startTid, nullptr, startVideoThread, this);
    pthread_create(&playTid, nullptr, playThread, this);
}

void *startVideoThread(void *data) {
    VideoMgr *videoMgr = static_cast<VideoMgr *>(data);
    videoMgr->decode();
    pthread_exit(&videoMgr->startTid);
}

void VideoMgr::decode() {
    int ret = 0;
    int count = 0;
    while (mediaStatus != nullptr && !mediaStatus->exit) {
        if (mediaStatus != nullptr && mediaStatus->seek) {
            sleep();
            continue;
        }
        if (packetQueue.size() == 0) {
            sleep();
            continue;
        }
        AVPacket *packet = nullptr;
        packetQueue.getAVPacket(&packet);
        ret = avcodec_send_packet(codecContext, packet);
        if (ret != 0) {
            av_packet_free(&packet);
            av_free(packet);
            continue;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret != 0) {
            av_frame_free(&frame);
            av_free(frame);
            av_packet_free(&packet);
            av_free(packet);
            continue;
        }
        AVFrame *outFrame = nullptr;
        if (needScale(frame)) {
            if (!scaleYUV(frame, &outFrame)) {
                av_frame_free(&frame);
                av_free(frame);
                av_packet_free(&packet);
                av_free(packet);
                continue;
            } else {
                av_frame_free(&frame);
                av_free(frame);
                av_packet_free(&packet);
                av_free(packet);
            }
        } else {
            outFrame = frame;
            av_packet_free(&packet);
            av_free(packet);
        }
        if (frameQueue.size() > MAX_FRAME_SIZE) {
            sleep();
            continue;
        }
        frameQueue.putFrame(outFrame);
        LOGI("frame添加视频帧，当前第 %d 帧", count++);
    }
}

bool VideoMgr::scaleYUV(AVFrame *inFrame, AVFrame **outFrame) {
    SwsContext *swsContext = sws_getContext(
            inFrame->width,
            inFrame->height,
            static_cast<AVPixelFormat>(inFrame->format),
            inFrame->width,
            inFrame->height,
            AV_PIX_FMT_YUV420P,
            SWS_BICUBIC,
            nullptr,
            nullptr,
            nullptr);

    if (swsContext == nullptr) {
        LOGE("swsContext == nullptr");
        return false;
    }
    AVFrame *yuvFrame = av_frame_alloc();
    int num = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, inFrame->width, inFrame->height, 1);
    uint8_t *buf = new uint8_t[num];
    int ret = av_image_fill_arrays(
            yuvFrame->data,
            yuvFrame->linesize,
            buf,
            AV_PIX_FMT_YUV420P,
            inFrame->width,
            inFrame->height,
            1);
    if (ret < 0) {
        LOGE("av_image_fill_arrays fail");
        delete[] buf;
        av_frame_free(&yuvFrame);
        av_free(yuvFrame);
        sws_freeContext(swsContext);
        return false;
    }
    sws_scale(
            swsContext,
            inFrame->data,
            inFrame->linesize,
            0,
            inFrame->height,
            yuvFrame->data,
            yuvFrame->linesize);
    *outFrame = yuvFrame;
    sws_freeContext(swsContext);
    return true;
}

void *playThread(void *data) {
    VideoMgr *videoMgr = static_cast<VideoMgr *>(data);
    videoMgr->play();
    pthread_exit(&videoMgr->playTid);
}

void VideoMgr::play() {
    int count = 0;
    while (mediaStatus != nullptr && !mediaStatus->exit) {
        if (mediaStatus != nullptr && mediaStatus->seek) {
            sleep();
            continue;
        }
        if (frameQueue.size() == 0) {
            sleep();
            continue;
        }
        AVFrame *frame = nullptr;
        if (frameQueue.getFrame(&frame)) {
            LOGI("frame消耗视频帧，当前第 %d 帧", count++);
            av_usleep(static_cast<unsigned int>(getDelayTime(getDiffTime(frame)) * 1000000));
            render(frame);
            av_frame_free(&frame);
            av_free(frame);
        }
    }
}

double VideoMgr::getDiffTime(AVFrame *frame) {
    int64_t pts = av_frame_get_best_effort_timestamp(frame);
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(stream->time_base);
    if (pts > 0) {
        video_clock = pts;
    }
    double diff = *audio_clock - video_clock;
    return diff;
}

double VideoMgr::getDelayTime(double diff) {
    if (diff > SYNCHRONIZE_SCOPE_MIN) {
        delayTime = delayTime * 2 / 3;
        if (delayTime < DEFAULT_DELAY_TIME / 2) {
            delayTime = DEFAULT_DELAY_TIME * 2 / 3;
        } else if (delayTime > DEFAULT_DELAY_TIME * 2) {
            delayTime = DEFAULT_DELAY_TIME * 2;
        }
    } else if (diff < -SYNCHRONIZE_SCOPE_MIN) {
        delayTime = delayTime * 3 / 2;
        if (delayTime < DEFAULT_DELAY_TIME / 2) {
            delayTime = DEFAULT_DELAY_TIME * 2 / 3;
        } else if (delayTime > DEFAULT_DELAY_TIME * 2) {
            delayTime = DEFAULT_DELAY_TIME * 2;
        }
    } else if (diff == SYNCHRONIZE_SCOPE_MIN) {

    }
    if (diff >= SYNCHRONIZE_SCOPE_MID) {
        delayTime = 0;
    } else if (diff <= -SYNCHRONIZE_SCOPE_MID) {
        delayTime = DEFAULT_DELAY_TIME * 2;
    }

    if (fabs(diff) >= SYNCHRONIZE_SCOPE_MAX) {
        delayTime = DEFAULT_DELAY_TIME;
    }
    return delayTime;
}

void VideoMgr::render(AVFrame *frame) {
    if (callJavaMgr != nullptr) {
        callJavaMgr->callRender(
                codecContext->width,
                codecContext->height,
                reinterpret_cast<char *>(frame->data[0]),
                reinterpret_cast<char *>(frame->data[1]),
                reinterpret_cast<char *>(frame->data[2]));
    }
}




//
// Created by dengchong on 2019-07-30.
//

#include <common/AVWrap.h>
#include "VideoMgr.h"

const unsigned VideoMgr::MAX_FRAME_SIZE = 3;
const double VideoMgr::DEFAULT_DELAY_TIME = 0.04;
const double VideoMgr::SYNCHRONIZE_SCOPE_MIN = 0.003;
const double VideoMgr::SYNCHRONIZE_SCOPE_MID = 0.5;
const unsigned VideoMgr::SYNCHRONIZE_SCOPE_MAX = 10;

VideoMgr::VideoMgr(shared_ptr<CallJavaMgr> &callJavaMgr, shared_ptr<MediaStatus> &status, int index,
                   shared_ptr<AVFormatContext> &formatContext)
        : MediaMgr(callJavaMgr, status, index, formatContext),
          frameQueue(make_shared<FrameQueue>(status)),
          delayTime(0) {

}

VideoMgr::~VideoMgr() {

}

void VideoMgr::start() {
    pthread_create(&startTid, nullptr, videoStartThread, this);
    pthread_create(&playTid, nullptr, playThread, this);
}

void *videoStartThread(void *data) {
    VideoMgr *videoMgr = static_cast<VideoMgr *>(data);
    videoMgr->decode();
    pthread_exit(&videoMgr->startTid);
}

void VideoMgr::decode() {
    int errorCode = 0;
    int count = 0;
    while (status && !status->exit) {
        if (status && status->seek) {
            sleep();
            continue;
        }
        if (!hasData()) {
            sleep();
            continue;
        }
        packetQueue->pop(packet);
        errorCode = avcodec_send_packet(codecContext.get(), packet.get());
        if (errorCode != 0) {
            LOGE("video avcodec_send_packet failed ... ,error msg : %s", av_err2str(errorCode));
            continue;
        }
        frame = AVWrap::allocAVFrame();
        errorCode = avcodec_receive_frame(codecContext.get(), frame.get());
        if (errorCode != 0) {
            LOGI("video avcodec_receive_frame failed ...  ,error msg : %s", av_err2str(errorCode));
            continue;
        }
        shared_ptr<AVFrame> outFrame;
        if (needScale(frame)) {
            if (!scaleYUV(frame, outFrame)) {
                continue;
            }
        } else {
            outFrame = frame;
        }
        if (frameQueue->size() > MAX_FRAME_SIZE) {
            sleep();
            continue;
        }
        frameQueue->put(outFrame);
        LOGI("VideoMgr::decode() : count %d", count++);
    }
}

bool VideoMgr::scaleYUV(shared_ptr<AVFrame> &inFrame, shared_ptr<AVFrame> &outFrame) {
    shared_ptr<SwsContext> swsContext = AVWrap::allocSwsContext(inFrame);
    if (!swsContext) {
        LOGE("swsContext == nullptr");
        return false;
    }
    shared_ptr<AVFrame> yuvFrame = AVWrap::allocAVFrame();
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
        return false;
    }
    sws_scale(
            swsContext.get(),
            inFrame->data,
            inFrame->linesize,
            0,
            inFrame->height,
            yuvFrame->data,
            yuvFrame->linesize);
    outFrame = yuvFrame;
    return true;
}

void *playThread(void *data) {
    VideoMgr *videoMgr = static_cast<VideoMgr *>(data);
    videoMgr->play();
    pthread_exit(&videoMgr->playTid);
}

void VideoMgr::play() {
    int count = 0;
    while (status && !status->exit) {
        if (status && (status->seek || status->pause)) {
            sleep();
            continue;
        }
        if (frameQueue->size() == 0) {
            sleep();
            continue;
        }
        shared_ptr<AVFrame> frame;
        if (frameQueue->get(frame)) {
            av_usleep(static_cast<unsigned int>(getDelayTime(getDiffTime(frame)) * 1000000));
            render(frame);
            LOGI("VideoMgr::play() : count %d", count++);
        }
    }
}

double VideoMgr::getDiffTime(shared_ptr<AVFrame> frame) {
    int64_t pts = av_frame_get_best_effort_timestamp(frame.get());
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(formatContext->streams[index]->time_base);
    if (pts > 0) {
        clock = pts;
    }
    double diff = audioMgr->getClock() - clock;
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

void VideoMgr::render(shared_ptr<AVFrame> frame) {
    if (callJavaMgr != nullptr) {
        callJavaMgr->callRender(
                codecContext->width,
                codecContext->height,
                reinterpret_cast<char *>(frame->data[0]),
                reinterpret_cast<char *>(frame->data[1]),
                reinterpret_cast<char *>(frame->data[2]));
    }
}




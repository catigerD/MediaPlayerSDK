//
// Created by dengchong on 2019-07-30.
//

#include "VideoMgr.h"

VideoMgr::VideoMgr(MediaStatus *status, int index, AVCodecParameters *avCodecParameters)
        : mediaStatus(status),
          packetQueue(status),
          index(index),
          avCodecParameters(avCodecParameters),
          codecContext(nullptr) {

}

VideoMgr::~VideoMgr() {

}

void VideoMgr::start() {
    pthread_create(&startTid, nullptr, startVideoThread, this);
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
        LOGI("消耗视频帧，当前第 %d 帧", count++);
    }
}



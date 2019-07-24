//
// Created by dengchong on 2019-07-24.
//

#include "AudioMgr.h"

AudioMgr::AudioMgr(MediaStatus *status) :
        status(status) {
    pktQueue = new PacketQueue(status);
}

AudioMgr::~AudioMgr() {
    delete pktQueue;
    avcodec_close(codecContext);
    avcodec_free_context(&codecContext);
}

void *decodeLoop(void *data) {
    AudioMgr *audioMgr = static_cast<AudioMgr *>(data);
    int ret = 0;
    while (!audioMgr->status->exit) {
        if (audioMgr->pktQueue->size() <= 0) {
            continue;
        }
        AVPacket *packet;
        if (!audioMgr->pktQueue->getAVPacket(&packet)) {
            continue;
        }
        if (audioMgr->decodeAPacketFinish) {
            ret = avcodec_send_packet(audioMgr->codecContext, packet);
            if (ret != 0) {
                LOGI("decode loop avcodec_send_packet fail ,error msg : %s", av_err2str(ret));
                av_packet_free(&packet);
                av_free(packet);
                audioMgr->decodeAPacketFinish = true;
                continue;
            }
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(audioMgr->codecContext, frame);
        if (ret != 0) {
            LOGI("decode loop avcodec_receive_frame fail ,error msg : %s", av_err2str(ret));
            av_frame_free(&frame);
            av_free(frame);
            av_packet_free(&packet);
            av_free(packet);
            audioMgr->decodeAPacketFinish = true;
            continue;
        }
        audioMgr->decodeAPacketFinish = false;
        //消耗 frame
        LOGI("消耗一帧，还剩 %d 帧", audioMgr->pktQueue->size());

        av_frame_free(&frame);
        av_free(frame);
        av_packet_free(&packet);
        av_free(packet);
    }

    pthread_exit(&audioMgr->decodeTid);
}

void AudioMgr::decode() {
    pthread_create(&decodeTid, nullptr, decodeLoop, this);
}

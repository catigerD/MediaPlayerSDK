//
// Created by dengchong on 2019-07-24.
//

#include "PacketQueue.h"

PacketQueue::PacketQueue(shared_ptr<MediaStatus> &status)
        : status(status),
          remove_count(0) {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
}

PacketQueue::~PacketQueue() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

bool PacketQueue::push(const shared_ptr<AVPacket> &packet) {
    Lock lock(&mutex);
    pktQueue.push_back(packet);
    pthread_cond_signal(&cond);
    return true;
}

bool PacketQueue::pop(shared_ptr<AVPacket> &packet) {
    Lock lock(&mutex);
    while (status && !status->exit) {
        if (status && status->seek) {
            sleep();
            continue;
        }
        if (!pktQueue.empty()) {
            packet = pktQueue.front();
            pktQueue.pop_front();
            return true;
        } else {
            pthread_cond_wait(&cond, &mutex);
        }
    }
    return false;
}

list<AVPacket *>::size_type PacketQueue::size() {
    Lock lock(&mutex);
    auto size = pktQueue.size();
    return size;
}

void PacketQueue::clear() {
    pthread_cond_signal(&cond);
    Lock lock(&mutex);
    pktQueue.clear();
}

void PacketQueue::removeBeforeAudioFrame(AVRational time_base, double audio_clock) {
    Lock lock(&mutex);
    auto cur = pktQueue.begin();
    for (; cur != pktQueue.end(); ++cur) {
        if (AV_NOPTS_VALUE != cur->get()->pts && AV_PKT_FLAG_KEY != cur->get()->flags) {
            double clock = cur->get()->pts * av_q2d(time_base);
            if (clock - audio_clock < -0.01) {
                pktQueue.erase(cur);
                LOGI("PacketQueue::removeBeforeAudioFrame remove video count : %d", remove_count++);
            }
        }
    }
}



//
// Created by dengchong on 2019-07-24.
//

#include "PacketQueue.h"

PacketQueue::PacketQueue(shared_ptr<MediaStatus> &status) : status(status) {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
}

PacketQueue::~PacketQueue() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

bool PacketQueue::push(const shared_ptr<AVPacket> &packet) {
    Lock lock(&mutex);
    pktQueue.push(packet);
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
            pktQueue.pop();
            return true;
        } else {
            pthread_cond_wait(&cond, &mutex);
        }
    }
    return false;
}

queue<AVPacket *>::size_type PacketQueue::size() {
    Lock lock(&mutex);
    auto size = pktQueue.size();
    return size;
}

void PacketQueue::clear() {
    pthread_cond_signal(&cond);
    Lock lock(&mutex);
    while (!pktQueue.empty()) {
        pktQueue.pop();
    }
}



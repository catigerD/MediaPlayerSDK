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

bool PacketQueue::push(shared_ptr<AVPacket> packet) {
    pthread_mutex_lock(&mutex);
    pktQueue.push(packet);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return true;
}

bool PacketQueue::pop(shared_ptr<AVPacket> &packet) {
    pthread_mutex_lock(&mutex);
    while (!status->exit) {
        if (pktQueue.size() > 0) {
            packet = pktQueue.front();
            pktQueue.pop();
            break;
        } else {
            pthread_cond_wait(&cond, &mutex);
        }
    }
    pthread_mutex_unlock(&mutex);
    return true;
}

queue<AVPacket *>::size_type PacketQueue::size() {
    pthread_mutex_lock(&mutex);
    auto size = pktQueue.size();
    pthread_mutex_unlock(&mutex);
    return size;
}

void PacketQueue::clear() {
    pthread_cond_signal(&cond);
    pthread_mutex_lock(&mutex);
    while (!pktQueue.empty()) {
        pktQueue.pop();
    }
    pthread_mutex_unlock(&mutex);
}



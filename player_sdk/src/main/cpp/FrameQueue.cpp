//
// Created by dengchong on 2019-07-30.
//

#include "FrameQueue.h"

FrameQueue::FrameQueue(shared_ptr<MediaStatus> &status)
        : status(status) {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
}

FrameQueue::~FrameQueue() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

void FrameQueue::put(const shared_ptr<AVFrame> &frame) {
    Lock lock(&mutex);
    frameQueue.push(frame);
    pthread_cond_signal(&cond);
}

bool FrameQueue::get(shared_ptr<AVFrame> &frame) {
    Lock lock(&mutex);
    while (status && !status->exit) {
        if (status && status->seek) {
            sleep();
            continue;
        }
        if (!frameQueue.empty()) {
            frame = frameQueue.front();
            frameQueue.pop();
            return true;
        } else {
            pthread_cond_wait(&cond, &mutex);
        }
    }
    return false;
}

void FrameQueue::clear() {
    pthread_cond_signal(&cond);
    Lock lock(&mutex);
    while (!frameQueue.empty()) {
        frameQueue.pop();
    }
}



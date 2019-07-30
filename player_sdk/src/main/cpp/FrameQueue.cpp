//
// Created by dengchong on 2019-07-30.
//

#include "FrameQueue.h"

FrameQueue::FrameQueue(MediaStatus *status)
        : status(status) {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
}

FrameQueue::~FrameQueue() {
    clearFrame();
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    status = nullptr;
}

void FrameQueue::putFrame(AVFrame *frame) {
    Lock lock(&mutex);
    frameQueue.push(frame);
    pthread_cond_signal(&cond);
}

bool FrameQueue::getFrame(AVFrame **outFrame) {
    Lock lock(&mutex);
    while (status != nullptr && !status->exit) {
        if (status != nullptr && status->seek) {
            sleep();
            continue;
        }
        if (!frameQueue.empty()) {
            *outFrame = frameQueue.front();
            frameQueue.pop();
            return true;
        } else {
            pthread_cond_wait(&cond, &mutex);
        }
    }
    return false;
}

void FrameQueue::clearFrame() {
    pthread_cond_signal(&cond);
    Lock lock(&mutex);
    while (!frameQueue.empty()) {
        AVFrame *frame = frameQueue.front();
        frameQueue.pop();
        av_frame_free(&frame);
        av_free(frame);
    }
}



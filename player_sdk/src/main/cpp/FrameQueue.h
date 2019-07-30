//
// Created by dengchong on 2019-07-30.
//

#ifndef MEDIAPLAYERSDK_FRAMEQUEUE_H
#define MEDIAPLAYERSDK_FRAMEQUEUE_H

extern "C" {
#include "libavcodec/avcodec.h"
};

#include <queue>
#include "MediaStatus.h"
#include "pthread.h"
#include "Lock.h"
#include "Common.h"

using namespace std;

class FrameQueue {
public:
    FrameQueue(MediaStatus *status);

    ~FrameQueue();

    void putFrame(AVFrame *frame);

    bool getFrame(AVFrame **outFrame);

    int size() {
        Lock lock(&mutex);
        return frameQueue.size();
    }

    void clearFrame();

private:
    MediaStatus *status;
    queue<AVFrame *> frameQueue;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
};


#endif //MEDIAPLAYERSDK_FRAMEQUEUE_H

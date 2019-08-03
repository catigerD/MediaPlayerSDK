//
// Created by dengchong on 2019-07-30.
//

#ifndef MEDIAPLAYERSDK_FRAMEQUEUE_H
#define MEDIAPLAYERSDK_FRAMEQUEUE_H

#include <queue>
#include "MediaStatus.h"
#include "pthread.h"
#include "Lock.h"
#include "Common.h"
#include "AVWrap.h"

using namespace std;

class FrameQueue {
public:
    FrameQueue(shared_ptr<MediaStatus> status);

    ~FrameQueue();

    void put(const shared_ptr<AVFrame> &frame);

    bool get(shared_ptr<AVFrame> &frame);

    int size() {
        Lock lock(&mutex);
        return frameQueue.size();
    }

    void clear();

private:
    shared_ptr<MediaStatus> status;
    queue<shared_ptr<AVFrame>> frameQueue;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
};


#endif //MEDIAPLAYERSDK_FRAMEQUEUE_H

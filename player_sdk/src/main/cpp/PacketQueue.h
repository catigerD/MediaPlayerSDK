//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_PACKETQUEUE_H
#define MEDIAPLAYERSDK_PACKETQUEUE_H

extern "C" {
#include "libavcodec/avcodec.h"
};

#include <queue>
#include <pthread.h>
#include "MediaStatus.h"
#include <memory>
#include "Common.h"
#include "Lock.h"

using namespace std;

class PacketQueue {
public:

    PacketQueue(shared_ptr<MediaStatus> &status);

    ~PacketQueue();

    bool push(const shared_ptr<AVPacket> &packet);

    bool pop(shared_ptr<AVPacket> &packet);

    queue<shared_ptr<AVPacket>>::size_type size();

    void clear();

private:
    shared_ptr<MediaStatus> &status;
    queue<shared_ptr<AVPacket>> pktQueue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

#endif //MEDIAPLAYERSDK_PACKETQUEUE_H

//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_PACKETQUEUE_H
#define MEDIAPLAYERSDK_PACKETQUEUE_H

extern "C" {
#include <libavcodec/avcodec.h>
};

#include <queue>
#include <pthread.h>
#include "MediaStatus.h"

using namespace std;

class PacketQueue {
private:
    queue<AVPacket *> pktQueue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    MediaStatus *status = nullptr;

public:

    PacketQueue(MediaStatus *status);

    ~PacketQueue();

    bool putAVPacket(AVPacket *packet);

    bool getAVPacket(AVPacket **packet);

    queue<AVPacket *>::size_type size();

    void clearAVPacket();
};


#endif //MEDIAPLAYERSDK_PACKETQUEUE_H

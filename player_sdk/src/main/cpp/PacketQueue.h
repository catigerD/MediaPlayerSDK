//
// Created by dengchong on 2019-07-24.
//

#ifndef MEDIAPLAYERSDK_PACKETQUEUE_H
#define MEDIAPLAYERSDK_PACKETQUEUE_H

extern "C" {
#include "libavcodec/avcodec.h"
};

#include <pthread.h>
#include "MediaStatus.h"
#include <memory>
#include "Common.h"
#include "Lock.h"
#include <list>
#include "AndroidLog.h"

using namespace std;

class PacketQueue {
public:

    PacketQueue(shared_ptr<MediaStatus> &status);

    ~PacketQueue();

    bool push(const shared_ptr<AVPacket> &packet);

    bool pop(shared_ptr<AVPacket> &packet);

    list<shared_ptr<AVPacket>>::size_type size();

    void clear();

    void removeBeforeAudioFrame(AVRational time_base, double audio_clock);

private:
    shared_ptr<MediaStatus> &status;
    list<shared_ptr<AVPacket>> pktQueue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    int remove_count;
};

#endif //MEDIAPLAYERSDK_PACKETQUEUE_H

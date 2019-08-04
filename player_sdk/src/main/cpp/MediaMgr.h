//
// Created by dengchong on 2019-08-03.
//

#ifndef MEDIAPLAYERSDK_MEDIAMGR_H
#define MEDIAPLAYERSDK_MEDIAMGR_H

#include "CallJavaMgr.h"
#include "MediaStatus.h"
#include "AVWrap.h"
#include "AndroidLog.h"
#include "PacketQueue.h"

class MediaMgr {

public:

    MediaMgr(shared_ptr<CallJavaMgr> &callJavaMgr, shared_ptr<MediaStatus> &status, int index,
             shared_ptr<AVFormatContext> &formatContext);

    virtual ~MediaMgr();

    bool openCodec();

    virtual bool canFill() const {
        return packetQueue->size() <= DEFAULT_MAX_QUEUE_SIZE;
    }

    bool hasData() const {
        return packetQueue->size() != 0;
    }

    int getStreamIndex() const {
        return index;
    }

    bool putPacket(shared_ptr<AVPacket> &packet) {
        return packetQueue->push(packet);
    }

    const double &getClock() const {
        return clock;
    }

protected:
    static const unsigned DEFAULT_MAX_QUEUE_SIZE;

    //回调 java 层
    shared_ptr<CallJavaMgr> &callJavaMgr;
    shared_ptr<MediaStatus> &status;
    int index;
    shared_ptr<AVFormatContext> &formatContext;
    shared_ptr<AVCodecContext> codecContext;

    shared_ptr<PacketQueue> packetQueue;

    double clock;
    double last_clock;

private:

};


#endif //MEDIAPLAYERSDK_MEDIAMGR_H

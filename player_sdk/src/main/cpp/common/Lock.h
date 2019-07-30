//
// Created by dengchong on 2019-07-29.
//

#ifndef MEDIAPLAYERSDK_LOCK_H
#define MEDIAPLAYERSDK_LOCK_H

#include "pthread.h"

class Lock {
public:
    Lock(pthread_mutex_t *mutex);

    ~Lock();
private:
    pthread_mutex_t *mutex;
};


#endif //MEDIAPLAYERSDK_LOCK_H

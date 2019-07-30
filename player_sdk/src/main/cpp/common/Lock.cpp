//
// Created by dengchong on 2019-07-29.
//

#include "Lock.h"

Lock::Lock(pthread_mutex_t *mutex)
        : mutex(mutex) {
    pthread_mutex_lock(mutex);
}

Lock::~Lock() {
    pthread_mutex_unlock(mutex);
}

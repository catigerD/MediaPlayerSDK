//
// Created by dengchong on 2019-07-13.
//

#ifndef AVPLAYER_ANDROIDLOG_H
#define AVPLAYER_ANDROIDLOG_H

#include "android/log.h"

#define LOG_DEBUG true

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"avplayer",FORMAT,##__VA_ARGS__)
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"avplayer",FORMAT,##__VA_ARGS__)

#endif //AVPLAYER_ANDROIDLOG_H

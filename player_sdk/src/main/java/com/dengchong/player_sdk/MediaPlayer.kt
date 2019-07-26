package com.dengchong.player_sdk

import com.dengchong.player_sdk.listener.MediaPlayerListener

object MediaPlayer {

    init {
        System.loadLibrary("avcodec-57")
        System.loadLibrary("avdevice-57")
        System.loadLibrary("avfilter-6")
        System.loadLibrary("avformat-57")
        System.loadLibrary("avutil-55")
        System.loadLibrary("postproc-54")
        System.loadLibrary("swresample-2")
        System.loadLibrary("swscale-4")
        System.loadLibrary("media-player")

        n_initJVM();
    }

    var listener: MediaPlayerListener? = null

    fun prepare(url: String) {
        n_prepare(url)
    }

    fun start() {
        n_start()
    }

    fun stop() {
        n_stop()
    }

    private fun callPrepared() {
        listener?.onPrepared()
    }

    private fun callCompleted() {
        listener?.onCompleted()
    }

    private external fun n_initJVM()

    private external fun n_prepare(url: String)

    private external fun n_start()

    private external fun n_stop()
}
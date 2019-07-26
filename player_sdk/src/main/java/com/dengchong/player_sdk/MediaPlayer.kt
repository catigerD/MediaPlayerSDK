package com.dengchong.player_sdk

import com.dengchong.player_sdk.listener.MediaPlayerListener
import com.dengchong.player_sdk.listener.UIMediaPlayerListener
import com.dengchong.player_sdk.utils.AThread

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

    private var uiListener: UIMediaPlayerListener? = null
    var listener: MediaPlayerListener? = null
        set(value) {
            field = value
            uiListener = UIMediaPlayerListener(listener)
        }

    fun prepare(url: String) {
        AThread.runOnWorkThread {
            n_prepare(url)
        }
    }

    fun start() {
        AThread.runOnWorkThread {
            n_start()
        }
    }

    fun stop() {
        AThread.runOnWorkThread {
            n_stop()
        }
    }

    fun pause() {
        AThread.runOnWorkThread {
            n_pause()
        }
    }

    fun resume() {
        AThread.runOnWorkThread {
            n_resume()
        }
    }

    fun seek(time: Int) {
        AThread.runOnUiThread {
            n_seek(time)
        }
    }

    private fun callPrepared() {
        uiListener?.onPrepared()
    }

    private fun callCompleted() {
        uiListener?.onCompleted()
    }

    private fun callTimeInfo(cur: Int, total: Int) {
        uiListener?.onTimeInfo(cur, total)
    }

    private external fun n_initJVM()

    private external fun n_prepare(url: String)

    private external fun n_start()

    private external fun n_stop()

    private external fun n_pause()

    private external fun n_resume()

    private external fun n_seek(time: Int)
}
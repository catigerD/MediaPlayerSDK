package com.dengchong.player_sdk.listener

import android.os.Handler
import android.os.Looper
import com.dengchong.player_sdk.utils.ALog

interface MediaPlayerListener {
    fun onPrepared()

    fun onCompleted()

    fun onTimeInfo(cur: Int, total: Int)
}

open class LogMediaPlayerListener : MediaPlayerListener {

    override fun onPrepared() {
        ALog.d("MediaPlayerListener: onPrepared()")
    }

    override fun onCompleted() {
        ALog.d("MediaPlayerListener: onCompleted()")
    }

    override fun onTimeInfo(cur: Int, total: Int) {
        ALog.d("MediaPlayerListener：onTimeInfo(cur: ${cur}, total: ${total})")
    }
}

class UIMediaPlayerListener(private val mediaPlayerListener: MediaPlayerListener?) {

    private val handler by lazy {
        Handler(Looper.getMainLooper())
    }

    fun onPrepared() {
        handler.post {
            mediaPlayerListener?.onPrepared()
        }
    }

    fun onCompleted() {
        handler.post {
            mediaPlayerListener?.onCompleted()
        }
    }

    fun onTimeInfo(cur: Int, total: Int) {
        handler.post {
            mediaPlayerListener?.onTimeInfo(cur, total)
        }
    }
}
package com.dengchong.player_sdk.listener

import android.os.Handler
import android.os.Looper
import com.dengchong.player_sdk.utils.ALog
import com.dengchong.player_sdk.utils.AThread

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

    fun onPrepared() {
        mediaPlayerListener?.apply {
            AThread.runOnUiThread {
                onPrepared()
            }
        }
    }

    fun onCompleted() {
        mediaPlayerListener?.apply {
            AThread.runOnUiThread {
                onCompleted()
            }
        }
    }

    fun onTimeInfo(cur: Int, total: Int) {
        mediaPlayerListener?.apply {
            AThread.runOnUiThread {
                onTimeInfo(cur, total)
            }
        }
    }
}
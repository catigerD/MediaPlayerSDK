package com.dengchong.player_sdk.utils

import android.util.Log

object ALog {

    private const val TAG = "media_player:Java"

    fun d(msg: String, tag: String = "") {
        Log.d("$TAG---$tag", msg)
    }

    fun e(msg: String, tag: String = "") {
        Log.e("$TAG---$tag", msg)
    }

    fun e(e: Throwable, tag: String = "") {
        Log.e("$TAG---$tag", Log.getStackTraceString(e))
    }
}
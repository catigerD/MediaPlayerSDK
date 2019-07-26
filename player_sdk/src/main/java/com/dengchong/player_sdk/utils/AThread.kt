package com.dengchong.player_sdk.utils

import android.os.Handler
import android.os.HandlerThread
import android.os.Looper

object AThread {

    private val handlerThread: HandlerThread = HandlerThread("work")
    private val workHandler: Handler
    private val uiHandler: Handler = Handler(Looper.getMainLooper())

    init {
        handlerThread.start()
        workHandler = Handler(handlerThread.looper)
    }

    fun runOnWorkThread(runnable: () -> Unit) {
        workHandler.post(runnable)
    }

    fun runOnUiThread(runnable: () -> Unit) {
        uiHandler.post(runnable)
    }
}
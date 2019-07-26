package com.dengchong.player_sdk.utils

/**
 * format times
 * @param this
 * @return
 */
fun Int.secdsToDateFormat(): String {
    val hours = (this / (60 * 60)).toLong()
    val minutes = (this % (60 * 60) / 60).toLong()
    val seconds = (this % 60).toLong()

    var sh = "00"
    if (hours > 0) {
        if (hours < 10) {
            sh = "0$hours"
        } else {
            sh = hours.toString() + ""
        }
    }
    var sm = "00"
    if (minutes > 0) {
        if (minutes < 10) {
            sm = "0$minutes"
        } else {
            sm = minutes.toString() + ""
        }
    }

    var ss = "00"
    if (seconds > 0) {
        if (seconds < 10) {
            ss = "0$seconds"
        } else {
            ss = seconds.toString() + ""
        }
    }

    return if (this >= 3600) "$sh:$sm:$ss" else "$sm:$ss"

}
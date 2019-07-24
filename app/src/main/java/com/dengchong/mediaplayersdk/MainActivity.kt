package com.dengchong.mediaplayersdk

import android.Manifest
import android.content.pm.PackageManager
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.dengchong.player_sdk.MediaPlayer
import com.dengchong.player_sdk.listener.MediaPlayerListener
import com.dengchong.player_sdk.utils.ALog
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        reqPermissions()
        // Example of a call to a native method
        btn_prepare.setOnClickListener {
            MediaPlayer.prepare(MUSIC_1)
        }

        MediaPlayer.listener = object : MediaPlayerListener {
            override fun onPrepared() {
                super.onPrepared()
                MediaPlayer.start()
                ALog.d("已经准备好了！！！")
            }

            override fun onCompleted() {
                super.onCompleted()
                ALog.d("已经播放完成了！！！")
            }
        }
    }

    private fun reqPermissions() {
        val ret = ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)
        if (PackageManager.PERMISSION_GRANTED != ret) {
            ActivityCompat.requestPermissions(
                this,
                arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE),
                1
            )
        }
    }

    companion object {
        private const val MUSIC_1 = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3"
        private val MUSIC_LOCAL_1 = Environment.getExternalStorageDirectory().absolutePath + "/ca12.mp4"
    }
}

package com.dengchong.player_sdk.render

import android.content.Context
import android.graphics.BitmapFactory
import android.opengl.GLES30
import android.opengl.GLSurfaceView
import android.opengl.GLUtils
import android.os.Handler
import android.os.Looper
import android.util.AttributeSet
import android.widget.Toast
import com.dengchong.player_sdk.R
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class MediaSurfaceView(context: Context, attributeSet: AttributeSet?) : GLSurfaceView(context, attributeSet) {

    init {
        setEGLContextClientVersion(3)
        setRenderer(MediaRender(context))
    }
}

class MediaRender(val context: Context) : GLSurfaceView.Renderer {

    private val vertexData = floatArrayOf(
        -1f, -1f, 0f, 0f,
        1f, -1f, 1f, 0f,
        -1f, 1f, 0f, 1f,
        1f, 1f, 1f, 1f
    )

    private var vertexBuffre: FloatBuffer
    private var VBO = IntArray(1)
    private var textures = IntArray(1)
    private var program = 0;
    private var vertexPosition = 0
    private var texturePostion = 0
    private var samplerTexture = 0

    init {
        vertexBuffre = ByteBuffer
            .allocateDirect(vertexData.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
            .put(vertexData)
        vertexBuffre.position(0)
    }

    override fun onDrawFrame(gl: GL10?) {
        if (program == 0) {
            return
        }
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT)
        GLES30.glClearColor(1.0f, 0.0f, 0.0f, 1.0f)

        GLES30.glUseProgram(program)

        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, VBO[0])
        GLES30.glEnableVertexAttribArray(vertexPosition)
        GLES30.glVertexAttribPointer(vertexPosition, 2, GLES30.GL_FLOAT, false, 4 * 4, 0)
        GLES30.glEnableVertexAttribArray(texturePostion)
        GLES30.glVertexAttribPointer(texturePostion, 2, GLES30.GL_FLOAT, false, 4 * 4, 2 * 4)

        GLES30.glActiveTexture(GLES30.GL_TEXTURE0)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textures[0])
        val bitmap = BitmapFactory.decodeResource(context.resources, R.mipmap.ic_launcher)
        if (bitmap != null) {
            GLUtils.texImage2D(GLES30.GL_TEXTURE_2D, 0, bitmap, 0)
            bitmap.recycle()
        }
        GLES30.glUniform1i(samplerTexture, 0)

        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_STRIP, 0, 4)

        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, 0)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0)
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        GLES30.glViewport(0, 0, width, height)
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        val vertexSource = ShaderUtil.readRawTxt(context, R.raw.vertex_shader)
        val fragmentSource = ShaderUtil.readRawTxt(context, R.raw.fragment_shader)
        program = ShaderUtil.createProgram(vertexSource, fragmentSource)
        if (program == 0) {
            Handler(Looper.getMainLooper()).post {
                Toast.makeText(context, "program == 0", Toast.LENGTH_SHORT).show()
            }
            return;
        }
        vertexPosition = GLES30.glGetAttribLocation(program, "position")
        texturePostion = GLES30.glGetAttribLocation(program, "texture_coords")
        samplerTexture = GLES30.glGetUniformLocation(program, "sTexture")

        GLES30.glGenBuffers(1, VBO, 0)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, VBO[0])
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, vertexData.size * 4, vertexBuffre, GLES30.GL_STATIC_DRAW)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0)

        GLES30.glGenTextures(1, textures, 0)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textures[0])
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_REPEAT)
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_REPEAT)
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR)
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, 0)
    }

}
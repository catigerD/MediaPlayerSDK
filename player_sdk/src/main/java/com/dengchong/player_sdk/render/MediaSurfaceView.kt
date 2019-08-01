package com.dengchong.player_sdk.render

import android.content.Context
import android.opengl.GLES30
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import com.dengchong.player_sdk.R
import com.dengchong.player_sdk.utils.ALog
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class MediaSurfaceView(context: Context, attrs: AttributeSet? = null) : GLSurfaceView(context, attrs) {

    private lateinit var renderer: MediaRender

    init {
        setEGLContextClientVersion(3)
        renderer = MediaRender(context)
        setRenderer(renderer)
        renderMode = RENDERMODE_CONTINUOUSLY
    }

    fun render(width: Int, height: Int, y: ByteArray, u: ByteArray, v: ByteArray) {
        renderer.setRenderParams(width, height, y, u, v)
        requestRender()
    }
}

class MediaRender(val context: Context) : GLSurfaceView.Renderer {

    private var vertexCoordsLocation = 0
    private var texcoordsLocation = 0
    private var samplerY = 0
    private var samplerU = 0
    private var samplerV = 0
    private var program = 0
    private var textures = IntArray(3)

    private var width = 0;
    private var height = 0;
    private var yData: ByteBuffer? = null
    private var uData: ByteBuffer? = null
    private var vData: ByteBuffer? = null

    private var vertexCoords = floatArrayOf(
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f
    )
    private var texcoords = floatArrayOf(
        0f, 1f,
        1f, 1f,
        0f, 0f,
        1f, 0f
    )

    private var vertexBuffer: FloatBuffer? = null
    private var texcoordsBuffer: FloatBuffer? = null

    init {
        vertexBuffer = ByteBuffer
            .allocateDirect(vertexCoords.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
            .put(vertexCoords)

        texcoordsBuffer = ByteBuffer
            .allocateDirect(texcoords.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
            .put(texcoords)
    }

    override fun onDrawFrame(gl: GL10?) {
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT)
        GLES30.glClearColor(1.0f, 0.0f, 0.0f, 0.0f)
        renderYUV()
        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_STRIP, 0, 4)
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        GLES30.glViewport(0, 0, width, height)
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        program = createProgram(VERTEX_SHADER, FRAGMENT_SHADER)
        if (program == 0) {
            return
        }
        vertexCoordsLocation = GLES30.glGetAttribLocation(program, "position")
        texcoordsLocation = GLES30.glGetAttribLocation(program, "texcoords")
        samplerY = GLES30.glGetUniformLocation(program, "sampler_y")
        samplerU = GLES30.glGetUniformLocation(program, "sampler_u")
        samplerV = GLES30.glGetUniformLocation(program, "sampler_v")

        GLES30.glGenTextures(3, textures, 0)
        for (i in 0 until 3) {
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textures[i])
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_REPEAT)
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_REPEAT)
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR)
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR)
        }
    }

    fun setRenderParams(width: Int, height: Int, y: ByteArray, u: ByteArray, v: ByteArray) {
        this.width = width
        this.height = height
        this.yData = ByteBuffer.wrap(y)
        this.uData = ByteBuffer.wrap(u)
        this.vData = ByteBuffer.wrap(v)
    }

    private fun renderYUV() {
        if (program == 0) {
            return
        }
        if (width < 0 || height < 0 || yData == null || vData == null || vData == null) {
            return
        }
        GLES30.glUseProgram(program)
        GLES30.glEnableVertexAttribArray(vertexCoordsLocation)
        GLES30.glVertexAttribPointer(vertexCoordsLocation, 2, GLES30.GL_FLOAT, false, 2 * 4, vertexBuffer)
        GLES30.glEnableVertexAttribArray(texcoordsLocation)
        GLES30.glVertexAttribPointer(texcoordsLocation, 2, GLES30.GL_FLOAT, false, 2 * 4, texcoordsBuffer)

        GLES30.glActiveTexture(GLES30.GL_TEXTURE0)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textures[0])
        GLES30.glTexImage2D(
            GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, width, height, 0,
            GLES30.GL_LUMINANCE, GLES30.GL_UNSIGNED_BYTE, yData
        )

        GLES30.glActiveTexture(GLES30.GL_TEXTURE1)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textures[1])
        GLES30.glTexImage2D(
            GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, width / 2, height / 2, 0,
            GLES30.GL_LUMINANCE, GLES30.GL_UNSIGNED_BYTE, uData
        )

        GLES30.glActiveTexture(GLES30.GL_TEXTURE2)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textures[2])
        GLES30.glTexImage2D(
            GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, width / 2, height / 2, 0,
            GLES30.GL_LUMINANCE, GLES30.GL_UNSIGNED_BYTE, vData
        )

        GLES30.glUniform1i(samplerY, 0)
        GLES30.glUniform1i(samplerU, 1)
        GLES30.glUniform1i(samplerV, 2)

        yData?.clear()
        uData?.clear()
        vData?.clear()
        yData = null
        uData = null
        vData = null
    }

}
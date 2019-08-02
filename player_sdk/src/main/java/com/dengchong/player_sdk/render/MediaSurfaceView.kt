package com.dengchong.player_sdk.render

import android.content.Context
import android.opengl.GLES30
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import android.widget.Toast
import com.dengchong.player_sdk.R
import com.dengchong.player_sdk.utils.ALog
import com.dengchong.player_sdk.utils.AThread
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class MediaSurfaceView(context: Context, attributeSet: AttributeSet?) : GLSurfaceView(context, attributeSet) {

    private var renderer: MediaRender

    init {
        setEGLContextClientVersion(3)
        renderer = MediaRender(context)
        setRenderer(renderer)
        renderMode = RENDERMODE_WHEN_DIRTY
    }

    fun render(width: Int, height: Int, yData: ByteArray, uData: ByteArray, vData: ByteArray) {
        renderer.setRenderParams(width, height, yData, uData, vData)
        requestRender()
    }
}

class MediaRender(val context: Context) : GLSurfaceView.Renderer {

    private val vertexData = floatArrayOf(
        -1f, -1f, 0f, 1f,
        1f, -1f, 1f, 1f,
        -1f, 1f, 0f, 0f,
        1f, 1f, 1f, 0f
    )

    private var vertexBuffre: FloatBuffer
    private var VBO = IntArray(1)
    private var textures = IntArray(3)
    private var program = 0;
    private var vertexPosition = 0
    private var texturePostion = 0
    private var samplerYPosition = 0;
    private var samplerUPosition = 0;
    private var samplerVPosition = 0;

    init {
        vertexBuffre = ByteBuffer
            .allocateDirect(vertexData.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
            .put(vertexData)
        vertexBuffre.position(0)
    }

    override fun onDrawFrame(gl: GL10?) {

        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT)
        GLES30.glClearColor(1.0f, 0.0f, 0.0f, 1.0f)
        render()
        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_STRIP, 0, 4)
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        GLES30.glViewport(0, 0, width, height)
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        val vertexSource = ShaderUtil.readRawTxt(context, R.raw.vertex_shader)
        val fragmentSource = ShaderUtil.readRawTxt(context, R.raw.fragment_shader)
        program = ShaderUtil.createProgram(vertexSource, fragmentSource)
        if (program == 0) {
            AThread.runOnUiThread {
                Toast.makeText(context, "program == 0", Toast.LENGTH_SHORT).show()
            }
            return;
        }
        vertexPosition = GLES30.glGetAttribLocation(program, "position")
        texturePostion = GLES30.glGetAttribLocation(program, "texture_coords")
        samplerYPosition = GLES30.glGetUniformLocation(program, "samplerY")
        samplerUPosition = GLES30.glGetUniformLocation(program, "samplerU")
        samplerVPosition = GLES30.glGetUniformLocation(program, "samplerV")

        GLES30.glGenBuffers(1, VBO, 0)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, VBO[0])
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, vertexData.size * 4, vertexBuffre, GLES30.GL_STATIC_DRAW)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0)

        GLES30.glGenTextures(3, textures, 0)
        for (i in 0 until 3) {
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textures[i])
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_REPEAT)
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_REPEAT)
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR)
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR)
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, 0)
        }
    }

    private var width = 0;
    private var height = 0;
    private var yBuffer: ByteBuffer? = null
    private var uBuffer: ByteBuffer? = null
    private var vBuffer: ByteBuffer? = null

    fun setRenderParams(width: Int, height: Int, yData: ByteArray, uData: ByteArray, vData: ByteArray) {
        this.width = width
        this.height = height
        yBuffer = ByteBuffer.wrap(yData)
        uBuffer = ByteBuffer.wrap(uData)
        vBuffer = ByteBuffer.wrap(vData)
    }

    fun render() {
        if (width == 0
            || height == 0
            || yBuffer == null
            || uBuffer == null
            || vBuffer == null

        ) {
            ALog.d("MediaRender render failed ...")
        }

        if (program == 0) {
            return
        }
        GLES30.glUseProgram(program)

        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, VBO[0])
        GLES30.glEnableVertexAttribArray(vertexPosition)
        GLES30.glVertexAttribPointer(vertexPosition, 2, GLES30.GL_FLOAT, false, 4 * 4, 0)
        GLES30.glEnableVertexAttribArray(texturePostion)
        GLES30.glVertexAttribPointer(texturePostion, 2, GLES30.GL_FLOAT, false, 4 * 4, 2 * 4)

        GLES30.glActiveTexture(GLES30.GL_TEXTURE0)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textures[0])
        GLES30.glTexImage2D(
            GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, width, height, 0, GLES30.GL_LUMINANCE,
            GLES30.GL_UNSIGNED_BYTE, yBuffer
        )
        GLES30.glActiveTexture(GLES30.GL_TEXTURE1)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textures[1])
        GLES30.glTexImage2D(
            GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, width / 2, height / 2, 0, GLES30.GL_LUMINANCE,
            GLES30.GL_UNSIGNED_BYTE, uBuffer
        )
        GLES30.glActiveTexture(GLES30.GL_TEXTURE2)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textures[2])
        GLES30.glTexImage2D(
            GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, width / 2, height / 2, 0, GLES30.GL_LUMINANCE,
            GLES30.GL_UNSIGNED_BYTE, vBuffer
        )

        GLES30.glUniform1i(samplerYPosition, 0)
        GLES30.glUniform1i(samplerUPosition, 1)
        GLES30.glUniform1i(samplerVPosition, 2)

        yBuffer?.clear()
        uBuffer?.clear()
        vBuffer?.clear()
        yBuffer = null
        uBuffer = null
        vBuffer = null
    }

}
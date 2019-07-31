package com.dengchong.player_sdk.render

import android.content.Context
import android.opengl.GLES30
import androidx.annotation.RawRes
import com.dengchong.player_sdk.utils.ALog
import java.io.BufferedReader
import java.io.InputStreamReader
import java.lang.StringBuilder

fun Context.getShaderSource(@RawRes idRes: Int): String {
    val result = StringBuilder()
    val inputStream = resources.openRawResource(idRes)
    val reader = BufferedReader(InputStreamReader(inputStream))
    while (reader.readLine().also { result.append(it).append('\n') } != null) {

    }
    return result.toString()
}

fun loadShader(shaderType: Int, shaderSource: String): Int {
    val shaderId = GLES30.glCreateShader(shaderType)
    if (shaderId == 0) {
        return 0
    }
    GLES30.glShaderSource(shaderId, shaderSource)
    checkGLError("glShaderSource")
    GLES30.glCompileShader(shaderId)
    var compile = IntArray(1)
    GLES30.glGetShaderiv(shaderId, GLES30.GL_COMPILE_STATUS, compile, 0)
    if (compile[0] == GLES30.GL_FALSE) {
        ALog.e("shader compile failed : ${GLES30.glGetShaderInfoLog(shaderId)}")
        return 0
    }
    return shaderId
}

fun createProgram(vertexSource: String, fragmentSource: String): Int {
    var program = 0;
    val vertexShader = loadShader(GLES30.GL_VERTEX_SHADER, vertexSource)
    if (vertexShader == 0) {
        return 0
    }
    val fragmentShader = loadShader(GLES30.GL_FRAGMENT_SHADER, fragmentSource)
    if (fragmentShader == 0) {
        return 0
    }
    program = GLES30.glCreateProgram()
    if (program == 0) {
        return 0;
    }
    GLES30.glAttachShader(program, vertexShader)
    checkGLError("glAttachShader")
    GLES30.glAttachShader(program, fragmentShader)
    checkGLError("glAttachShader")
    GLES30.glLinkProgram(program)
    val link = IntArray(1)
    GLES30.glGetProgramiv(program, GLES30.GL_LINK_STATUS, link, 0)
    if (link[0] == GLES30.GL_FALSE) {
        ALog.e("program link failed : ${GLES30.glGetProgramInfoLog(program)}")
        return 0;
    }
    return program;
}

fun checkGLError(method: String) {
    var error = GLES30.glGetError();
    ALog.e("GLES30.glGetError() ${method} error : ${error}")
}
package com.dengchong.player_sdk.render

import android.opengl.GLES30
import com.dengchong.player_sdk.utils.ALog

const val VERTEX_SHADER = "" +
        "attribute vec4 position;\n" +
        "attribute vec2 texcoords;\n" +
        "varying vec2 out_texcoords;\n" +
        "void main(){\n" +
        "    gl_Position = position;\n" +
        "    out_texcoords = texcoords;\n" +
        "}"

const val FRAGMENT_SHADER = "" +
        "precision mediump float;\n" +
        "varying vec2 out_texcoords;\n" +
        "uniform sampler2D sampler_y;\n" +
        "uniform sampler2D sampler_u;\n" +
        "uniform sampler2D sampler_v;\n" +
        "void main(){\n" +
        "    float y, u, v;\n" +
        "    y = texture2D(sampler_y, out_texcoords).r;\n" +
        "    u = texture2D(sampler_u, out_texcoords).r - 0.5;\n" +
        "    v = texture2D(sampler_v, out_texcoords).r - 0.5;\n" +
        "\n" +
        "    vec3 rgb;\n" +
        "    rgb.r = y + 1.403 * v;\n" +
        "    rgb.g = y - 0.344 * u - 0.714 * v;\n" +
        "    rgb.b = y + 1.770 * u;\n" +
        "\n" +
        "    gl_FragColor = vec4(rgb, 1);\n" +
        "}"


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
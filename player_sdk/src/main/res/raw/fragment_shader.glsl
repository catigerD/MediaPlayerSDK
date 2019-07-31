precision mediump float;
varying vec2 out_texcoords;
uniform sampler2D sampler_y;
uniform sampler2D sampler_u;
uniform sampler2D sampler_v;
void main(){
    float y, u, v;
    y = texture2D(sampler_y, out_texcoords).r;
    u = texture2D(sampler_u, out_texcoords).r - 0.5;
    v = texture2D(sampler_v, out_texcoords).r - 0.5;

    vec3 rgb;
    rgb.r = y + 1.403 * v;
    rgb.g = y - 0.344 * u - 0.714 * v;
    rgb.b = y + 1.770 * u;

    gl_FragColor = vec4(rgb, 1);
}
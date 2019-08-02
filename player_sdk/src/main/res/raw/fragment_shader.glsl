precision mediump float;
varying vec2 out_texture_coords;
uniform sampler2D sTexture;
void main(){
    gl_FragColor = texture2D(sTexture, out_texture_coords);
}
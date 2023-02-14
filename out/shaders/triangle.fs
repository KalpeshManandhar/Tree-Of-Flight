#version 330 core
out vec4 FragColor;
in vec3 out_col;
in vec2 tex_crd;

uniform sampler2D texture0;
void main(){
   FragColor = vec4(out_col, 1.0f) * texture(texture0,tex_crd);
}

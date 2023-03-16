#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;
out vec3 out_col;
out vec2 tex_crd;
uniform mat3 transform;
uniform vec3 colors;
void main(){
   vec3 vex = vec3(aPos.x,aPos.y,1.0);
   vex = transform * vex;
   gl_Position = vec4(vex.x,vex.y, aPos.z, 1.0);
   out_col = colors;
   tex_crd = texCoord;
}

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;
out vec3 out_col;
out vec2 tmp_pos;
out vec2 tex_crd;
uniform mat3 transform;
uniform vec3 colors;
void main(){
   vec3 vex = vec3(aPos.x,aPos.y,1.0);
   vex = transform * vex;
   gl_Position = vec4(vex.x,vex.y, aPos.z, 1.0);
   out_col = colors;
   int vert_number = gl_VertexID % 3;
	if(vert_number == 0)
		tmp_pos = vec2(0,-2);
	else if ( vert_number == 1)
		tmp_pos = vec2(sqrt(3),1);
	else
		tmp_pos = vec2(-sqrt(3),1);
   tex_crd = texCoord;
}

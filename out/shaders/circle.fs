#version 330 core
out vec4 FragColor;
in vec3 out_col;
in vec2 tmp_pos;
in vec2 tex_crd;

uniform sampler2D texture0;
void main(){
	float alpha  = 1.0f;
	if(tmp_pos.x * tmp_pos.x + tmp_pos.y * tmp_pos. y > 1.0)
		alpha = 0.f;
	FragColor = vec4(out_col, alpha) * texture(texture0,tex_crd);
}

#version 410 core
layout (location = 5) in vec2 position;
layout (location = 6) in vec2 texcoord;
out vec2 uv;
out VS_OUT { 
	vec2 texcoord; 
} vs_out;
void main(void) {
			gl_Position = vec4(position,1.0,1.0);
			vs_out.texcoord = texcoord; 
			uv = (vec2( gl_Position.x, - gl_Position.y ) + vec2(1.0) ) / vec2(2.0);
}
#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec3 iv3color;
layout(location = 2) in vec2 iv2coord;

uniform mat4 um4mvp;


out vec3 vv3color;
out vec2 vv2coord;
void main()
{
	gl_Position = um4mvp * vec4(iv3vertex, 1.0);
	vv3color = iv3color;
	vv2coord = iv2coord;

}
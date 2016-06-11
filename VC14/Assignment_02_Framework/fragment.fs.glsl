#version 410

in vec3 vv3color;
in vec2 vv2coord;

layout(location = 0) out vec4 fragColor;
uniform sampler2D tex_object; 
void main()
{
    vec4 texColor = texture(tex_object, vv2coord);
	if(texColor.r==1.0 && texColor.g == 1.0 && texColor.b==1.0)
		discard;
	if(texColor.a <0.5)
		discard;
	if(texColor.r==0.0 && texColor.g == 0.0 && texColor.b==0.0)
		discard;
	fragColor = texColor;
}
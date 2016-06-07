#version 410

in vec3 vv3color;
in vec2 vv2coord;

layout(location = 0) out vec4 fragColor;
uniform sampler2D tex_object; 
void main()
{
    fragColor = texture(tex_object, vv2coord);

}
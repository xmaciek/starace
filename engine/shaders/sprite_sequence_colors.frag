#version 450

layout( binding = 1 ) uniform sampler2D atlasTexture;

layout( location = 0 ) in vec4 inColor;
layout( location = 1 ) in vec2 inUV;

layout( location = 0 ) out vec4 outColor;

void main()
{
    vec4 color = inColor;
    color.a *= texture( atlasTexture, inUV ).r;
    outColor = color;
}

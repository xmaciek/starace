#version 450

#extension GL_ARB_separate_shader_objects : enable

layout( binding = 1 ) uniform sampler2D fontTexture;

layout( location = 0 ) in vec4 inColor;
layout( location = 1 ) in vec2 inUV;

layout( location = 0 ) out vec4 outColor;

void main()
{
    vec4 color = inColor;
    color.a *= texture( fontTexture, inUV ).r;
    outColor = color;
}

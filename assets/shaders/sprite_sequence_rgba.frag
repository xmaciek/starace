#version 450

layout( binding = 1 ) uniform sampler2D atlasTexture;

layout( location = 0 ) in vec4 inColor;
layout( location = 1 ) in vec2 inUV;

layout( location = 0 ) out vec4 outColor;

void main()
{
    outColor = texture( atlasTexture, inUV ).rgba * inColor;
}

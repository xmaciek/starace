#version 450

layout( binding = 1 ) uniform sampler2D textureSampler;

layout( location = 0 ) in vec4 fragmentColor;
layout( location = 1 ) in vec2 fragmentUV;

layout( location = 0 ) out vec4 outColor;

void main()
{
    vec4 color = texture( textureSampler, fragmentUV ).rgba * fragmentColor;
    if ( color.a == 0 ) discard;
    outColor = color;
}

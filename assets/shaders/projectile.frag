#version 450

layout( binding = 1 ) uniform sampler2D textureSampler;
layout( location = 0 ) in vec2 fragUV;
layout( location = 0 ) out vec4 outColor;

void main()
{
    outColor = texture( textureSampler, fragUV ).rgba;
}

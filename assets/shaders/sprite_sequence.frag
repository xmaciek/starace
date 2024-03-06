#version 450

layout( binding = 1 ) uniform sampler2D atlasTexture;

layout( location = 0 ) in vec4 inColor;
layout( location = 1 ) in vec2 inUV;
layout( location = 2 ) in flat int sampleRGBA;

layout( location = 0 ) out vec4 outColor;

void main()
{
    vec4 colorArr[ 2 ];
    colorArr[ 1 ] = texture( atlasTexture, inUV ).rgba;
    colorArr[ 0 ] = vec4( 1.0, 1.0, 1.0, colorArr[ 1 ].r );
    outColor = colorArr[ sampleRGBA ] * inColor;
}

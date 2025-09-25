#version 450

layout( binding = 1 ) uniform sampler2D atlasTexture[ 2 ];

layout( location = 0 ) in vec4 inColor;
layout( location = 1 ) in vec2 inUV;
layout( location = 2 ) in flat uint whichAtlas;
layout( location = 3 ) in flat uint sampleRGBA;

layout( location = 0 ) out vec4 outColor;

void main()
{
    vec4 colorSelect[ 2 ];
    colorSelect[ 1 ] = texture( atlasTexture[ whichAtlas ], inUV ).rgba;
    colorSelect[ 0 ] = vec4( 1.0, 1.0, 1.0, colorSelect[ 1 ].r );
    outColor = colorSelect[ sampleRGBA ] * inColor;
}

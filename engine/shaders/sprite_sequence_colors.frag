layout( binding = 1 ) uniform sampler2D atlasTexture[ 9 ];

layout( location = 0 ) in vec4 inColor;
layout( location = 1 ) in vec2 inUV;
layout( location = 2 ) in flat uint whichAtlas;

layout( location = 0 ) out vec4 outColor;

void main()
{
    outColor = texture( atlasTexture[ whichAtlas ], inUV ) * inColor;
}

#version 450

layout( binding = 1 ) uniform sampler2D tex[ 6 ];

layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in flat uint inID;
layout( location = 0 ) out vec4 outColor;

void main()
{
    outColor = texture( tex[ inID ], inUV );
}

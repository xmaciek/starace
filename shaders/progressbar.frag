#version 450

layout( location = 0 ) in vec4 colorA;
layout( location = 1 ) in vec4 colorB;
layout( location = 2 ) in vec2 xthreshold;

layout( location = 0 ) out vec4 outColor;

void main()
{
    vec4 color[ 2 ];
    color[ 0 ] = colorA;
    color[ 1 ] = colorB;
    outColor = color[ int( xthreshold.x > xthreshold.y ) ];
}

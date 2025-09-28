#version 450

layout( location = 0 ) in vec4 color;
layout( location = 1 ) in vec2 uv;

layout( location = 0 ) out vec4 outColor;

void main()
{
    float dist = distance( vec2( 0.5, 0.5 ), uv );
    outColor = vec4( color.rgb, dist * 0.618 );
}

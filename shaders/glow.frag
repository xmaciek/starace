#version 450

#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec4 color;
layout( location = 1 ) in vec2 uv;

layout( location = 0 ) out vec4 outColor;

void main()
{
    vec2 center = vec2( 0.5, 0.5 );
    vec2 a = abs( uv - center );
    float dist = sqrt( dot( a, a ) );
    outColor = vec4( color.rgb, dist * 0.618 );
}

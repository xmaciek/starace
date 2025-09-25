#version 450

layout( binding = 1 ) uniform sampler2D textureSampler;
layout( location = 0 ) out vec4 outColor;

layout( location = 0 ) in vec4 inColor;
layout( location = 1 ) in vec4 inSlice; // x,y,w,h
layout( location = 2 ) in vec2 inUV;


void main()
{
    vec2 uv = inSlice.xy + mod( inUV, inSlice.zw );
    vec3 rrr = texture( textureSampler, uv ).rrr;
    outColor = vec4( rrr, 1.0 ) * inColor;
}

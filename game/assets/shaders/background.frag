#version 450

layout( binding = 1 ) uniform sampler2D textureSampler;
layout( location = 0 ) out vec4 outColor;


layout( location = 0 ) in flat vec4 inColor;
layout( location = 1 ) in flat vec4 inUV;
layout( location = 2 ) in flat vec4 inGeometry;
layout( location = 3 ) in flat vec2 inViewport;
layout( location = 4 ) in vec2 inSlice;


void main()
{
    vec2 xy = mod( ( inSlice * inViewport ) / inGeometry.zw, 1.0 );
    vec2 uv = inUV.xy + xy * inUV.zw;
    vec3 rrr = texture( textureSampler, uv ).rrr;
    outColor = vec4( rrr, 1.0 ) * inColor;
}

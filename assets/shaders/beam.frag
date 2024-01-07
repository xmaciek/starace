#version 450

layout( location = 0 ) in flat vec4 inColor1;
layout( location = 1 ) in flat vec4 inColor2;
layout( location = 2 ) in float inU;

layout( location = 0 ) out vec4 outColor;

void main()
{
    float a = 0.5 - cos( inU * 3.14 * 2.0 ) * 0.5;
    float n = 0.5 - cos( clamp( inU, 0.333333, 0.666667 ) * 3.14 * 6 ) * 0.5;
    outColor = mix( inColor1, inColor2, n );
    outColor.a = a;

}

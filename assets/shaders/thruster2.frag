#version 450

struct ColorScheme {
    vec4 inner1;
    vec4 inner2;
    vec4 outter1;
    vec4 outter2;
};

layout( location = 0 ) in vec2 fragUV;
layout( location = 1 ) in flat ColorScheme fragColor;

layout( location = 0 ) out vec4 colorOut;

void main()
{
    vec4 color1 = mix( fragColor.inner1, fragColor.outter1, fragUV.x );
    vec4 color2 = mix( fragColor.inner2, fragColor.outter2, fragUV.y );
    colorOut = mix( color1, color2, fragUV.y );
}

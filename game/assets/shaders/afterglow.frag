#version 450

struct ColorScheme {
    vec4 inner1;
    vec4 inner2;
    vec4 outter1;
    vec4 outter2;
};

layout( location = 0 ) in vec2 fragUV;
layout( location = 1 ) in flat float cutoff;
layout( location = 2 ) in flat ColorScheme fragColor;

layout( location = 0 ) out vec4 colorOut;

void main()
{

    float dist = distance( fragUV, vec2( 0.5, 0.5 ) );
    if ( dist < cutoff ) discard;

    float alpha = clamp( 2.0 * ( 0.5 - dist ), 0.0, 1.0 );
    float calpha = smoothstep( 0.0, 1.0, alpha );
    if ( calpha <= 0.0 ) discard;

    float x = smoothstep( 0.0, 1.0, cutoff );
    float y = smoothstep( 0.0, 1.0, cutoff );
    vec4 color1 = mix( fragColor.inner1, fragColor.inner2, x );
    vec4 color2 = mix( fragColor.outter2, fragColor.outter2, y );
    vec4 color = mix( color1, color2, x * 2.0 );
    colorOut = vec4( color.rgb, calpha * 0.518 );

}

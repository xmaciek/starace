#version 450

layout( location = 0 ) in vec4 color;
layout( location = 1 ) in vec2 uv;
layout( location = 2 ) in float radius;

layout( location = 0 ) out vec4 outColor;

void main()
{
    vec2 a = abs( uv - vec2( 0.5, 0.5 ) );
    float dist = sqrt( dot( a, a ) );
    if ( dist < radius ) discard;

    float alpha = clamp( 2.0 * ( 0.5 - dist ), 0.0, 1.0 );
    float calpha = 1.0 - ( 0.5 + 0.5 * cos( alpha * 3.1415 ) );
    if ( calpha <= 0.0 ) discard;

    outColor = vec4( color.rgb, calpha * 0.618 );
}

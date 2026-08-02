struct ColorScheme {
    vec4 inner;
    vec4 outter;
};

layout( location = 0 ) in vec2 fragUV;
layout( location = 1 ) in flat ColorScheme fragColor;

layout( location = 0 ) out vec4 colorOut;

const vec4 REMOVE_ALPHA = vec4( 1.0, 1.0, 1.0, 0.0 );

void main()
{
    vec4 color = mix( fragColor.inner, fragColor.outter, 1.0 - fragUV.x );
    colorOut = mix( color * REMOVE_ALPHA, color, smoothstep( 0.0, 1.0, fragUV.y ) );
}

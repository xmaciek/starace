layout( binding = 1 ) uniform sampler2D atlasTexture;

layout( location = 0 ) in flat vec4 inColor;
layout( location = 1 ) in flat vec4 spriteUVWH;
layout( location = 2 ) in flat vec2 spriteExtent;
layout( location = 3 ) in flat vec2 quadExtent;
layout( location = 4 ) in flat vec2 range;
layout( location = 5 ) in vec2 inUV;
layout( location = 6 ) in flat uint sampleRGBA;

layout( location = 0 ) out vec4 outColor;

const vec2 zero = vec2( 0.0, 0.0 );
const vec2 one = vec2( 1.0, 1.0 );

vec2 scale( vec2 uv, vec2 oldMin, vec2 oldMax, vec2 newMin, vec2 newMax )
{
    return newMin + ( uv - oldMin ) / ( oldMax - oldMin ) * ( newMax - newMin );
}

vec2 slice9( vec2 uv, vec2 border )
{
    vec2 r[3];
    r[ 0 ] = scale( uv, zero,         border,       zero,        range );
    r[ 1 ] = scale( uv, border,       one - border, range,       one - range );
    r[ 2 ] = scale( uv, one - border, one,          one - range, one );
    int x = int( uv.x > border.x ) + int( uv.x > ( 1.0 - border.x ) );
    int y = int( uv.y > border.y ) + int( uv.y > ( 1.0 - border.y ) );
    return vec2( r[ x ].x, r[ y ].y );
}

void main()
{
    vec2 border = spriteExtent * range / quadExtent;
    vec2 uv9 = slice9( inUV, border );
    vec2 newUV = mix( spriteUVWH.xy, spriteUVWH.xy + spriteUVWH.zw, uv9 );

    vec4 colorSelect[ 2 ];
    colorSelect[ 1 ] = texture( atlasTexture, newUV ).rgba;
    colorSelect[ 0 ] = vec4( 1.0, 1.0, 1.0, colorSelect[ 1 ].r );
    outColor = colorSelect[ sampleRGBA ] * inColor;
}

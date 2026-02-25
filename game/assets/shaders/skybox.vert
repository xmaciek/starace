layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

layout( location = 0 ) out vec2 fragUV;
layout( location = 1 ) out flat uint texID;

const float uvmin = 0.00125;
const float uvmax = 0.99875;
const float s = 1000.0;

const vec4 WALL[ 6 ][ 4 ] = {
    { vec4( -s, -s,  s, 0.0 ), vec4(  s, -s,  s, 0.0 ), vec4(  s,  s,  s, 0.0 ), vec4( -s,  s,  s, 0.0 ) },
    { vec4( -s,  s, -s, 0.0 ), vec4(  s,  s, -s, 0.0 ), vec4(  s, -s, -s, 0.0 ), vec4( -s, -s, -s, 0.0 ) },
    { vec4( -s, -s, -s, 0.0 ), vec4( -s, -s,  s, 0.0 ), vec4( -s,  s,  s, 0.0 ), vec4( -s,  s, -s, 0.0 ) },
    { vec4(  s,  s, -s, 0.0 ), vec4(  s,  s,  s, 0.0 ), vec4(  s, -s,  s, 0.0 ), vec4(  s, -s, -s, 0.0 ) },
    { vec4( -s,  s, -s, 0.0 ), vec4( -s,  s,  s, 0.0 ), vec4(  s,  s,  s, 0.0 ), vec4(  s,  s, -s, 0.0 ) },
    { vec4( -s, -s, -s, 0.0 ), vec4(  s, -s, -s, 0.0 ), vec4(  s, -s,  s, 0.0 ), vec4( -s, -s,  s, 0.0 ) },
};

const vec2 UV[ 6 ][ 4 ] = {
    { vec2( uvmin, uvmin ), vec2( uvmax, uvmin ), vec2( uvmax, uvmax ), vec2( uvmin, uvmax ) },
    { vec2( uvmin, uvmax ), vec2( uvmax, uvmax ), vec2( uvmax, uvmin ), vec2( uvmin, uvmin ) },
    { vec2( uvmin, uvmin ), vec2( uvmax, uvmin ), vec2( uvmax, uvmax ), vec2( uvmin, uvmax ) },
    { vec2( uvmin, uvmax ), vec2( uvmax, uvmax ), vec2( uvmax, uvmin ), vec2( uvmin, uvmin ) },
    { vec2( uvmin, uvmin ), vec2( uvmax, uvmin ), vec2( uvmax, uvmax ), vec2( uvmin, uvmax ) },
    { vec2( uvmin, uvmin ), vec2( uvmax, uvmin ), vec2( uvmax, uvmax ), vec2( uvmin, uvmax ) },
};

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( WALL[ gl_InstanceIndex ][ gl_VertexIndex ].xyz, 1.0 );
    fragUV = UV[ gl_InstanceIndex ][ gl_VertexIndex ];
    texID = gl_InstanceIndex;
}

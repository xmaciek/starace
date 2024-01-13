#version 450

const uint INSTANCES = 320;

struct Beam {
    vec3 position;
    vec4 quat;
    vec3 displacement;
    vec4 color1;
    vec4 color2;
};

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    Beam beams[ INSTANCES ];
};

layout( location = 0 ) out flat vec4 colorOut1;
layout( location = 1 ) out flat vec4 colorOut2;
layout( location = 2 ) out float u;

const vec3 vertexPos[ 12 ] = {
    vec3( 0.0, -1.0, 0.0 ),
    vec3( 0.0, -1.0, 1.0 ),
    vec3( 0.0, 1.0, 1.0 ),
    vec3( 0.0, 1.0, 1.0 ),
    vec3( 0.0, 1.0, 0.0 ),
    vec3( 0.0, -1.0, 0.0 ),

    vec3( -1.0, 0.0, 0.0 ),
    vec3( 1.0, 0.0, 0.0 ),
    vec3( 1.0, 0.0, 1.0 ),
    vec3( 1.0, 0.0, 1.0 ),
    vec3( -1.0, 0.0, 1.0 ),
    vec3( -1.0, 0.0, 0.0 ),
};

void main()
{
    Beam beam = beams[ gl_InstanceIndex ];
    vec3 mult = vertexPos[ gl_VertexIndex ];
    vec3 pos = mult * vec3( beam.displacement.xy, -beam.displacement.z );
    pos += 2.0 * cross( beam.quat.xyz, cross( beam.quat.xyz, pos ) + beam.quat.w * pos );
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( beam.position + pos, 1.0 );

    colorOut1 = beam.color1;
    colorOut2 = beam.color2;
    u = ( 1.0 + mult.x + mult.y ) * 0.5;
}

#version 450

const uint INSTANCES = 320;

struct Tail {
    vec4 position[ 8 ];
};

layout( location = 0 ) out vec2 uv;

layout( binding = 0 ) uniform ubo {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraDirection;
    vec3 cameraUp;
    Tail tail[ INSTANCES ];
};

const float vertPos[ 2 ] = { -0.012075, 0.012075 };

void main()
{
    vec3 p = ( normalize( cross( cameraUp, cameraDirection ) ) * vertPos[ gl_VertexIndex % 2 ] ) + tail[ gl_InstanceIndex ].position[ gl_VertexIndex / 2 ].xyz;
    gl_Position = projectionMatrix
        * viewMatrix
        * vec4( p, 1.0 );
    uv = vec2( gl_VertexIndex % 2, float( gl_VertexIndex / 2 ) / 7.0 );
}

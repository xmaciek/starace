#version 450

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 vertexPosition[ 4 ];
    vec4 vertexUV[ 4 ];
};

layout( location = 0 ) out vec2 fragmentUV;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertexPosition[ gl_VertexIndex ].xyz, 1.0 );

    fragmentUV = vertexUV[ gl_VertexIndex ].xy;
}

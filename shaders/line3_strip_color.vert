#version 450

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 vertexPosition[ 32 ];
    vec4 vertexColor[ 32 ];
};

layout( location = 0 ) out vec4 fragmentColor;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertexPosition[ gl_VertexIndex ].xyz, 1.0 );

    fragmentColor = vertexColor[ gl_VertexIndex ];
}

#version 450

#extension GL_ARB_separate_shader_objects : enable

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 vertexPosition[ 48 ];
    vec4 vertexColor[ 48 ];
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

#version 450

#extension GL_ARB_separate_shader_objects : enable

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec2 vertexPosition[ 4 ];
    vec2 vertexUV[ 4 ];
    vec4 colorValue;
};

layout( location = 0 ) out vec4 fragmentColor;
layout( location = 1 ) out vec2 fragmentUV;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertexPosition[ gl_VertexIndex ].x, vertexPosition[ gl_VertexIndex ].y, 0.0, 1.0 );

    fragmentColor = colorValue;
    fragmentUV = vertexUV[ gl_VertexIndex ].xy;
}

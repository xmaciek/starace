#version 450

#extension GL_ARB_separate_shader_objects : enable

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 colorValue;
    vec4 vertice[ 4 ];
};

layout( location = 0 ) out vec4 fragmentColor;
layout( location = 1 ) out vec2 fragmentUV;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertice[ gl_VertexIndex ].xy, 0.0, 1.0 );

    fragmentColor = colorValue;
    fragmentUV = vertice[ gl_VertexIndex ].zw;
}

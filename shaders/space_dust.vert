#version 450

#extension GL_ARB_separate_shader_objects : enable

const uint INSTANCES = 100;

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 color;
    vec4 particleOffset;
    vec4 particle[ INSTANCES ];
};

layout( location = 0 ) out vec4 colorOut;

void main()
{
    vec4 vertex = particle[ gl_InstanceIndex ];
    vec4 vertexPos[2];
    vertexPos[ 0 ] = vertex;
    vertexPos[ 1 ] = vertex + particleOffset;
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vertexPos[ gl_VertexIndex ];

    colorOut = color;
}

#version 450

#extension GL_ARB_separate_shader_objects : enable

const uint INSTANCES = 48;

struct Sprite {
    vec4 color;
    vec4 xyuv[ 6 ];
};

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    Sprite sprites[ INSTANCES ];
};

layout( location = 0 ) out vec4 outColor;
layout( location = 1 ) out vec2 outUV;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( sprites[ gl_InstanceIndex ].xyuv[ gl_VertexIndex ].xy, 0.0, 1.0 );

    outColor = sprites[ gl_InstanceIndex ].color;
    outUV = sprites[ gl_InstanceIndex ].xyuv[ gl_VertexIndex ].zw;
}

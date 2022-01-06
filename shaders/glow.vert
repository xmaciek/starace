#version 450

#extension GL_ARB_separate_shader_objects : enable

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 color;
    vec4 xyuv[ 4 ];
};

layout( location = 0 ) out vec4 colorFrag;
layout( location = 1 ) out vec2 uv;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( xyuv[ gl_VertexIndex ].xy, 0.0, 1.0 );

    colorFrag = color;
    uv = xyuv[ gl_VertexIndex ].zw;
}

#version 450

#extension GL_ARB_separate_shader_objects : enable

const uint c_characterCount = 48;
const uint c_verticeCount = c_characterCount * 6;

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 color;
    vec4 xyuv[ c_verticeCount ];
};

layout( location = 0 ) out vec4 outColor;
layout( location = 1 ) out vec2 outUV;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( xyuv[ gl_VertexIndex ].xy, 0.0, 1.0 );

    outColor = color;
    outUV = xyuv[ gl_VertexIndex ].zw;
}

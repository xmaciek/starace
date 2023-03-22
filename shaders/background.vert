#version 450

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 colorValue;
    vec4 uvSlice;
    vec4 xyuv[ 4 ];
};

layout( location = 0 ) out vec4 outColor;
layout( location = 1 ) out vec4 outSlice;
layout( location = 2 ) out vec2 outUV;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( xyuv[ gl_VertexIndex ].xy, 0.0, 1.0 );

    outColor = colorValue;
    outSlice = uvSlice;
    outUV = xyuv[ gl_VertexIndex ].zw;
}

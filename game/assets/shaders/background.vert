#version 450

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 color;
    vec4 uv;
    vec4 geometry;
    vec2 viewport;
};

layout( location = 0 ) out flat vec4 outColor;
layout( location = 1 ) out flat vec4 outUV;
layout( location = 2 ) out flat vec4 outGeometry;
layout( location = 3 ) out flat vec2 outViewport;
layout( location = 4 ) out vec2 outSlice;

const vec2 vert[ 4 ] = {
    vec2( 0, 0 ),
    vec2( 0, 1 ),
    vec2( 1, 1 ),
    vec2( 1, 0 ),
};

void main()
{
    outColor = color;
    outUV = uv;
    outGeometry = geometry;
    outViewport = viewport;
    outSlice = vert[ gl_VertexIndex ];

    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( viewport * vert[ gl_VertexIndex ], 0.0, 1.0 );
}

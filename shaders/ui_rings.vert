#version 450

const vec2 vertmult[] = {
    vec2( 0.0, 0.0 ),
    vec2( 0.0, 1.0 ),
    vec2( 1.0, 1.0 ),
    vec2( 1.0, 1.0 ),
    vec2( 1.0, 0.0 ),
    vec2( 0.0, 0.0 ),
};

layout( binding = 0 ) uniform ubo {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 xywh;
    mat4 modelMatrix[ 3 ];
    vec4 color[ 3 ];
};

layout( location = 0 ) out vec4 fragmentColor;
layout( location = 1 ) out vec2 fragmentUV;
layout( location = 2 ) out flat int samplerId;

void main()
{
    vec2 vertPos = xywh.xy + xywh.zw * vertmult[ gl_VertexIndex ];
    vec2 uvPos = vertmult[ gl_VertexIndex ];

    gl_Position = projectionMatrix * viewMatrix * modelMatrix[ gl_InstanceIndex ] * vec4( vertPos, 0.0, 1.0 );

    fragmentColor = color[ gl_InstanceIndex ];
    fragmentUV = uvPos;
    samplerId = gl_InstanceIndex;
}

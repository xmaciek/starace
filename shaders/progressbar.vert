#version 450

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 vertice[ 4 ];
    vec4 colors[ 2 ];
    vec4 axis;
};

layout( location = 0 ) out vec4 colorA;
layout( location = 1 ) out vec4 colorB;
layout( location = 2 ) out vec2 xthreshold;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertice[ gl_VertexIndex ].xy, 0.0, 1.0 );

    colorA = colors[ 0 ];
    colorB = colors[ 1 ];

    vec2 span = ( vertice[ 2 ].xy - vertice[ 0 ].xy ) * axis.xy;
    float range = abs( span.x + span.y );

    vec2 startPos = vertice[ 0 ].xy * axis.xy;
    float pos = startPos.x + startPos.y;

    vec2 nowPos = vertice[ gl_VertexIndex ].xy * axis.xy;
    xthreshold.x = abs( ( nowPos.x + nowPos.y ) - pos ) / range;
    xthreshold.y = axis.w;
}

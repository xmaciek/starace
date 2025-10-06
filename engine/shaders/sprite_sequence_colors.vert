#version 450

const uint INSTANCES = 64;

const vec2 vertmult[] = {
    vec2( 0.0, 0.0 ),
    vec2( 0.0, 1.0 ),
    vec2( 1.0, 1.0 ),
    vec2( 1.0, 0.0 ),
};

struct Sprite {
    vec4 color;
    vec4 xywh;
    vec4 uvwh;
    uvec4 sampleInfo;
};

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    Sprite sprites[ INSTANCES ];
};

layout( location = 0 ) out flat vec4 outColor;
layout( location = 1 ) out vec2 outUV;
layout( location = 2 ) out flat uint outWhichAtlas;
layout( location = 3 ) out flat uint outSampleRGBA;

void main()
{
    vec2 vertPos = sprites[ gl_InstanceIndex ].xywh.xy + sprites[ gl_InstanceIndex ].xywh.zw * vertmult[ gl_VertexIndex ];
    vec2 uvPos = sprites[ gl_InstanceIndex ].uvwh.xy + sprites[ gl_InstanceIndex ].uvwh.zw * vertmult[ gl_VertexIndex ];
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertPos, 0.0, 1.0 );

    outColor = sprites[ gl_InstanceIndex ].color;
    outUV = uvPos;
    outWhichAtlas = sprites[ gl_InstanceIndex ].sampleInfo.x;
    outSampleRGBA = sprites[ gl_InstanceIndex ].sampleInfo.y;
}

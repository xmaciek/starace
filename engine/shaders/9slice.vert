const int INSTANCES = 16;

struct Instance {
    vec4 color;
    vec4 xywh;
    vec4 uvwh;
    vec2 spriteExtent;
    uint sampleRGBA;
    float range;
};

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;

    Instance instances[ INSTANCES ];
};

layout( location = 0 ) out flat vec4 outColor;
layout( location = 1 ) out flat vec4 outSpriteUVWH;
layout( location = 2 ) out flat vec2 outSpriteExtent;
layout( location = 3 ) out flat vec2 outQuadExtent;
layout( location = 4 ) out flat vec2 outRange;
layout( location = 5 ) out vec2 outUV;
layout( location = 6 ) out flat uint outSampleRGBA;

const vec2 vertmult[] = {
    vec2( 0.0, 0.0 ),
    vec2( 0.0, 1.0 ),
    vec2( 1.0, 1.0 ),
    vec2( 1.0, 0.0 ),
};

void main()
{
    Instance inst = instances[ gl_InstanceIndex ];
    vec2 vertPos = inst.xywh.xy + inst.xywh.zw * vertmult[ gl_VertexIndex ];
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertPos, 0.0, 1.0 );

    outColor = inst.color;
    outSpriteUVWH = inst.uvwh;
    outSpriteExtent = inst.spriteExtent;
    outQuadExtent = inst.xywh.zw;
    outRange = vec2( inst.range, inst.range );
    outUV = vertmult[ gl_VertexIndex ];
    outSampleRGBA = inst.sampleRGBA;
}

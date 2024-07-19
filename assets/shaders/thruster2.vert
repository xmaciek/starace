#version 450

struct ColorScheme {
    vec4 inner1;
    vec4 inner2;
    vec4 outter1;
    vec4 outter2;
};

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ColorScheme colorScheme;
};

layout( location = 0 ) in vec3 vertVert;
layout( location = 1 ) in vec2 vertUV;
layout( location = 2 ) in vec3 vertNormal;

layout( location = 0 ) out vec2 fragUV;
layout( location = 1 ) out flat ColorScheme fragColor;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertVert.xyz, 1.0 );

    float f = dot( vertNormal, vertNormal ) * 0.0000001;
    f += 1.0;

    fragUV = vertUV * f;
    fragColor = colorScheme;

}

#version 450

#extension GL_ARB_separate_shader_objects : enable

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

layout( location = 0 ) in vec3 vertVert;
layout( location = 1 ) in vec2 vertUV;
// layout( location = 2 ) in vec3 vertNormal;

layout( location = 0 ) out vec2 fragUV;
// layout( location = 1 ) out vec3 fragNormal;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertVert.xyz, 1.0 );

    fragUV = vertUV;
//     fragNormal = vertNormal;
}

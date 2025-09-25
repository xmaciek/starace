#version 450

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

layout( location = 0 ) in vec3 vertVert;
layout( location = 1 ) in vec2 vertUV;
layout( location = 2 ) in vec3 vertNormal;

layout( location = 0 ) out vec2 fragUV;
layout( location = 1 ) out vec3 fragNormal;
layout( location = 2 ) out vec3 fragVert;
layout( location = 3 ) out flat mat3 fragNormalMatrix;

void main()
{
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertVert.xyz, 1.0 );

    fragUV = vertUV;
    fragNormal = vertNormal;
    fragVert = ( modelMatrix * vec4( vertVert.xyz, 1.0 ) ).xyz;
    fragNormalMatrix = mat3( transpose( inverse( modelMatrix ) ) );
}

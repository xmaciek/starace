#version 450

const uint INSTANCES = 64;

struct Projectile {
    vec4 quat;
    vec4 positionScale;
};

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    Projectile projectiles[ INSTANCES ];
};


layout( location = 0 ) in vec3 vertVert;
layout( location = 1 ) in vec2 vertUV;

layout( location = 0 ) out vec2 fragUV;

vec3 quat2rotate( vec3 pos, vec4 quat )
{
    return 2.0 * cross( quat.xyz, cross( quat.xyz, pos ) + quat.w * pos );
}

void main()
{
    Projectile projectile = projectiles[ gl_InstanceIndex ];
    vec3 pos = vertVert * projectile.positionScale.w;
    pos += 2.0 * cross( projectile.quat.xyz, cross( projectile.quat.xyz, pos ) + projectile.quat.w * pos );
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( projectile.positionScale.xyz + pos, 1.0 );
    fragUV = vertUV;
}

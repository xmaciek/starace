#version 450

layout( binding = 1 ) uniform sampler2D textureSampler;

layout( location = 0 ) in vec2 fragUV;
layout( location = 1 ) in vec3 fragNormal;
layout( location = 2 ) in vec3 fragVert;
layout( location = 3 ) in flat mat3 fragNormalMatrix;

layout( location = 0 ) out vec4 colorOut;

const vec3 LIGHT_POSITION = vec3( 0.0, 1000.0, 0.0 );
const vec4 LIGHT_COLOR = vec4( 1.0, 1.0, 1.0, 1.0 );
const vec3 LIGHT_POSITION2 = vec3( 0.0, -1000.0, 0.0 );
const vec4 LIGHT_COLOR2 = vec4( 0.6, 0.6, 0.6, 1.0 );
const vec4 AMBIENT_COLOR = vec4( 0.3, 0.3, 0.3, 1.0 );

float computeLight( vec3 position, vec3 vert, vec3 normal, mat3 matrix )
{
    vec3 norm = normalize( matrix * normal );
    vec3 lightDir = normalize( position - vert );
    float diff = max( dot( norm, lightDir ), 0.0 );
    return diff;
}

void main()
{
    vec4 albedo = texture( textureSampler, fragUV );
    float diff = computeLight( LIGHT_POSITION, fragVert, fragNormal, fragNormalMatrix );
    float diff2 = computeLight( LIGHT_POSITION2, fragVert, fragNormal, fragNormalMatrix );
    vec4 diffuse = diff * LIGHT_COLOR + diff2 * LIGHT_COLOR2;
    colorOut = ( AMBIENT_COLOR + diffuse ) * albedo;

}

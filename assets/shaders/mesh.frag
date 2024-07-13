#version 450

layout( binding = 1 ) uniform sampler2D textureSampler;

layout( location = 0 ) in vec2 fragUV;
layout( location = 1 ) in vec3 fragNormal;
layout( location = 2 ) in vec3 fragVert;
layout( location = 3 ) in flat mat3 fragNormalMatrix;

layout( location = 0 ) out vec4 colorOut;

const vec3 LIGHT_POSITION = vec3( 0.0, 1000.0, 0.0 );
const vec4 LIGHT_COLOR = vec4( 1.0, 1.0, 1.0, 1.0 );
const vec4 AMBIENT_COLOR = vec4( 0.3, 0.3, 0.3, 1.0 );

void main()
{
    vec4 albedo = texture( textureSampler, fragUV );
    vec3 norm = normalize( fragNormalMatrix * fragNormal );
    vec3 lightDir = normalize( LIGHT_POSITION - fragVert );
    float diff = max( dot( norm, lightDir ), 0.0 );
    vec4 diffuse = diff * LIGHT_COLOR;
    colorOut = ( AMBIENT_COLOR + diffuse ) * albedo;

}

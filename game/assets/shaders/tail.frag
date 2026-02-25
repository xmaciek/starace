layout( binding = 1 ) uniform sampler2D textureSampler;
layout( location = 0 ) in vec2 uv;
layout( location = 0 ) out vec4 outColor;

void main()
{
    outColor = vec4( 1.0, 1.0, 1.0, texture( textureSampler, uv ).r );
}

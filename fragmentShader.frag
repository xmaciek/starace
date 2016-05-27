#version 330 core
out vec4 color;

flat in int fragmentStance;
in vec4 fragmentColor;
in vec2 fragmentUV;
uniform sampler2D sampler;


void main() {
    switch( fragmentStance ) {
        case 0: color = fragmentColor; break;
        case 1: color = texture( sampler, fragmentUV ).rgba; break;
        default: color = vec4( 1, 1, 1, 1 ); break;
    }
}


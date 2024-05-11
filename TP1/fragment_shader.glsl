#version 330 core

// Ouput data
out vec4 color;
in vec2 o_uv0;
// flat in int numText;

uniform sampler2D text;
in vec4 boundsAndheight;

void main(){
        color =texture(text,o_uv0);
}

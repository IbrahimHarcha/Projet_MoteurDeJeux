#version 330 core

// Ouput data
out vec4 color;
in vec2 uv2;
// flat in int numText;

uniform sampler2D text;
in vec4 boundsAndheight;

void main(){
        color = texture(text,uv2);
}

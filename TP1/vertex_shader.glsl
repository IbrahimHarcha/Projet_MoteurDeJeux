#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 uv;

uniform mat4 modelM;
uniform mat4 viewM; 
uniform int mode;
uniform mat4 projM;

uniform sampler2D HMPlan;

out vec2 uv2;
uniform vec3 heightBounds;


//TODO create uniform transformations matrices Model View Projection
// Values that stay constant for the whole mesh.

void main(){
        uv2=uv;
        gl_Position = projM*viewM*modelM*vec4(vertices_position_modelspace,1);
}


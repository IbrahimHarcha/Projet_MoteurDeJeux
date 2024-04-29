#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 uv;

uniform mat4 Model;
uniform mat4 Projection; 
uniform mat4 View;

out vec2 uv2;

uniform sampler2D heightMapPlan;
uniform int mode;

void main(){
        uv2=uv;

        mat4 MVP = Projection * View * Model;

        gl_Position = MVP * vec4(vertices_position_modelspace,1);
}


#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 uv;

uniform mat4 modelM;
uniform mat4 viewM; // o_uv0=uv;
uniform int mode;
uniform mat4 projM;

uniform sampler2D HMPlan;

out vec2 o_uv0;
uniform vec3 heightBounds;
out vec4 boundsAndheight;


//TODO create uniform transformations matrices Model View Projection
// Values that stay constant for the whole mesh.

void main(){
   
        o_uv0=uv;
        // //mode heightmap
        // if(mode==1){
        //         float height = texture(HMPlan, uv).r;
        //         boundsAndheight=vec4(heightBounds,height);
        
        //         //height plus vertices.y pour gérer les offset en y
        //         vec3 poswithheight=vec3(vertices_position_modelspace[0],height+vertices_position_modelspace.y,vertices_position_modelspace[2]);
        //         gl_Position =   projM*viewM*modelM*vec4(poswithheight,1);
        
        // //mode classique
        // }else{
        //         gl_Position =   projM*viewM*modelM*vec4(vertices_position_modelspace,1);
        //         boundsAndheight=vec4(0,0,0,0);

        // }
        gl_Position =   projM*viewM*modelM*vec4(vertices_position_modelspace,1);
        boundsAndheight=vec4(0,0,0,0);
      

}


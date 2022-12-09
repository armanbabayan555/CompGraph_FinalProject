#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tCoord;

out VS_OUT {
    vec2 texCoord;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float time;

void main(){
    gl_Position = vec4(position, 1.0f);
    vs_out.texCoord = tCoord;
}

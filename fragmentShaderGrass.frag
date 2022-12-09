#version 330 core

in vec2 texCoords;

out vec4 color;

uniform float Ka;
uniform float Kd;
uniform float Ks;

uniform vec3 lightDir;
uniform sampler2D heightMap;
uniform sampler2D grassDist;
uniform sampler2D grassText;

void main(){
    vec3 lightColor = vec3(0.0, 1.0, 0.0);
    vec3 Ia = Ka * lightColor;

 
    color = texture(grassText, texCoords);    
}

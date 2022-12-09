#version 330 core

in vec2 texCoords;
in vec3 normal;

out vec4 color;

uniform float Ka;
uniform float Kd;
uniform float Ks;

uniform vec3 lightDir;
uniform sampler2D heightMap;
uniform sampler2D terrainTexture;

void main(){
    vec3 lightColor = vec3(0.0, 1.0, 0.0);
    vec3 Ia = Ka * lightColor;
    vec3 lightDirection = normalize(-lightDir);
    float lambertian = max(0.0, dot(lightDirection, normalize(normal)));
    vec3 Id = Kd * lambertian * lightColor;


    color = vec4(normalize(vec4(Id , 1.0) * texture(terrainTexture, texCoords)).xyz, 1.0);
 
}

#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoord;
} gs_in[];

out vec2 texCoords;
out vec4 interPos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 rippleCentre;
uniform float time;

void main(){
    normal = transpose(inverse(mat3(model))) * normalize(cross(vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position), vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position)));
    
    vec4 position;
    for(int i = 0; i < 3; i++) {
        position = gl_in[i].gl_Position;
        float distance = distance(vec3(model * position), rippleCentre);
        float offset = 2 * distance / (time + 1) * pow(2, -9.0 * distance) * cos(44 * distance - 3 * time);
        gl_Position = projection * view * model * vec4(position.x, position.y + offset, position.z, 1.0);
        interPos = projection * view * model * vec4(position.x, position.y + offset, position.z, 1.0);
        texCoords = gs_in[i].texCoord;
        EmitVertex();
    }

    EndPrimitive();
}
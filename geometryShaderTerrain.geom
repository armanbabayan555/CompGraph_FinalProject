#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT{
    vec2 texCoord;
} gs_in[];

out vec2 texCoords;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    normal = transpose(inverse(mat3(model))) * normalize(cross(vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position), vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position)));

    // Output the normal texture and transformed vertex coordinates for each triangle
    for (int i = 0; i < 3; i++) {
        gl_Position = projection * view * model * gl_in[i].gl_Position;
        texCoords = gs_in[i].texCoord;
        EmitVertex();
    }
    EndPrimitive();
}

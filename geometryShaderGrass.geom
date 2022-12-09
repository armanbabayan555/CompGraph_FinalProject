#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT {
    vec2 texCoord;
} gs_in[];

out vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D grassDist;


void generatePositionAndEmitVertex(vec4 position, int sign, float size1, float size2, float texCoords_x, float texCoords_y) {

     gl_Position = projection * view * model * vec4(position.x + sign * size1 / 2, position.y + size2, position.z, 1.0);
     texCoords = vec2(texCoords_x, texCoords_y);
     EmitVertex();
}

void generateGrassObj(vec4 position, float size) {

    generatePositionAndEmitVertex(position, -1, size, size, 0.0, 0.0);
    generatePositionAndEmitVertex(position, -1, size, 0.0, 0.0, 1.0);
    generatePositionAndEmitVertex(position, 1, size, size, 1.0, 0.0);
    generatePositionAndEmitVertex(position, 1, size, 0.0, 1.0, 1.0);
    
    EndPrimitive();
}

void main(){
    
    vec4 centroidCoords = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3.0;
    vec2 position = vec2((centroidCoords.x + 1) / 2, 1.0 - (centroidCoords.z + 1) / 2);
    
    if (texture(grassDist, position).r > 0.6) {
        generateGrassObj(centroidCoords, 0.3);
    }
    
}

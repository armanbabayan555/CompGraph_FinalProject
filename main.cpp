#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <windows.h>
#include "FrameBuffer.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Triangle.h"

using namespace std;

const int width = 1920;
const int height = 1080;

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        printf("OpenGL error %i, at %s:%i - for %s\n", error, fname, line, stmt);
        abort();
    }
}

#define GL_CHECK(stmt) do { \
        stmt; \
        CheckOpenGLError(#stmt, __FILE__, __LINE__); \
    } while (0)

struct Surface {
    vector<float> coordinates;
    int size;
    vector<int> indexBuffer;
    int indexCount;
};

struct CallbackContext {
    Surface* surface;
    glm::mat4* model;
    Camera* cam;
    Shader* shader;
    glm::mat4* projection;
};

Surface GenerateIndexedTriangleStripPlane(int hVertices, int vVertices, float yCoordinate = 0.5) {
    float dH = 2.0f / (hVertices - 1);
    float dV = 2.0f / (vVertices - 1);

    Surface output;
    for (int i = 0; i < vVertices; ++i) {
        for (int j = 0; j < hVertices; ++j) {
            // These three floating point numbers are for position coordinates
            output.coordinates.push_back(j * dH - 1.0f);
            output.coordinates.push_back(yCoordinate); 
            output.coordinates.push_back(i * dV - 1.0f);
            // These floating point numbers are for UV coordinates, required for texture mapping
            output.coordinates.push_back(static_cast<float>(j) / (hVertices - 1));
            output.coordinates.push_back(1.0f - static_cast<float>(i) / (vVertices - 1));
        }
    }

    for (int i = 0; i < vVertices - 1; ++i) {
        for (int j = 0; j < hVertices; ++j) {
            output.indexBuffer.push_back(i * hVertices + j);
            output.indexBuffer.push_back((i + 1) * hVertices + j);

            if (i != vVertices - 2 && j == hVertices - 1) {
                output.indexBuffer.push_back((i + 1) * hVertices + j);
                output.indexBuffer.push_back((i + 1) * hVertices);
            }
        }
    }

    output.size = output.coordinates.size();
    output.indexCount = output.indexBuffer.size();

    return output;
}

Ray ConstructRayThroughPixel(const Camera& camera, int i, int j, glm::mat4 projection) {
    const float screenX = 2.0f * j / width - 1.0f;
    const float screenY = 1.0f - 2.0f * i / height;

    glm::vec4 screenCoords(screenX, screenY, 1.0f, 1.0f);

    glm::vec4 target = inverse(projection) * screenCoords;

    return { 
        camera.Position,
        glm::normalize(
            glm::vec3(inverse(camera.GetViewMatrix()) * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0.0f))
        )};
}

glm::vec4 getWaterSurfaceCoordVector(const Surface& waterSurface, const mat4& model, int i, int k) {
    return model * glm::vec4(
        waterSurface.coordinates[waterSurface.indexBuffer.at(i + k) * 5],
        waterSurface.coordinates[waterSurface.indexBuffer.at(i + k) * 5 + 1],
        waterSurface.coordinates[waterSurface.indexBuffer.at(i + k) * 5 + 2],
        1.0);
}

void rayCast(const Surface& waterSurface, const mat4& model, const Camera& cam, Shader& sh, int y, int x, glm::mat4 projection) {
    float tVal;
    Ray r = ConstructRayThroughPixel(cam, y, x, projection);
    for (int i = 0; i < waterSurface.indexCount - 2; ++i) {
        Triangle t;
        if (i % 2) {
            t.setVertices({ 
                getWaterSurfaceCoordVector(waterSurface, model, i, 2),
                getWaterSurfaceCoordVector(waterSurface, model, i, 0),
                getWaterSurfaceCoordVector(waterSurface, model, i, 1),
                });
        }
        else {
            t.setVertices({
                getWaterSurfaceCoordVector(waterSurface, model, i, 1),
                getWaterSurfaceCoordVector(waterSurface, model, i, 0),
                getWaterSurfaceCoordVector(waterSurface, model, i, 2),
                });
        }

        glm::vec3 n = t.getPlaneNormal();

        if (t.intersects(r, tVal) && !(isnan(n.x) && isnan(n.y) && isnan(n.z))) {
                vec3 intersection = r.p0 + tVal * r.dir;
                glfwSetTime(0.0);
                sh.bind();
                GL_CHECK(glUniform3f(glGetUniformLocation(sh.GetProgramId(), "rippleCentre"), intersection.x, intersection.y, intersection.z));
                sh.unbind();
        }
    }

}

int main() {
    
    if (!glfwInit()) {
        printf("Failed to initialize GLFW!");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Final Project", NULL, NULL);

    if (window == NULL) {
        printf("Failed to initialize window!");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    GLenum glewInitialize = glewInit();
    if (glewInitialize != GLEW_OK)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(glewInitialize));
    }


    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Water handling
    Surface waterSurface = GenerateIndexedTriangleStripPlane(100, 100, 0.25);
    Mesh waterMesh(waterSurface.coordinates.data(), waterSurface.size);
    waterMesh.AddLayout(3);
    waterMesh.AddLayout(2);
    waterMesh.BindIndexBuffer(waterSurface.indexBuffer.data(), waterSurface.indexCount);
    Texture waterDiffuse("WaterDiffuse.png");
    Shader waterShader("vertexShaderWater.vert",
        "geometryShaderWater.geom",
        "fragmentShaderWater.frag");


    // Terrain handling
    Surface terrain = GenerateIndexedTriangleStripPlane(100, 100);
    Mesh terrainMesh(terrain.coordinates.data(), terrain.size);
    terrainMesh.AddLayout(3);
    terrainMesh.AddLayout(2);
    terrainMesh.BindIndexBuffer(terrain.indexBuffer.data(), terrain.indexCount);
    Texture heightMap("TerrainHeightMap.png");
    Texture terrainDiffuse("TerrainDiffuse.png");
    Shader terrainShader("vertexShaderTerrain.vert",
        "geometryShaderTerrain.geom",
        "fragmentShaderTerrain.frag");

    // Grass handling
    Texture grassDistribution("GrassDistribution.png");
    Texture grassDiffuse("GrassDiffuse.png");
    Shader grassShader("vertexShaderTerrain.vert",
        "geometryShaderGrass.geom",
        "fragmentShaderGrass.frag");

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -4.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.5, 1.0, 1.0));

    Camera camera;

    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 1.f, 150.0f);

    CallbackContext ctx = {&waterSurface, &model, &camera, &waterShader, &projection};
    glfwSetWindowUserPointer(window, &ctx);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);


    auto keyboardCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        CallbackContext* ctx = static_cast<CallbackContext*>(glfwGetWindowUserPointer(window));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            ctx->cam->ProcessKeyboard(FORWARD, 0.02f);
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            ctx->cam->ProcessKeyboard(LEFT, 0.02f);
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            ctx->cam->ProcessKeyboard(BACKWARD, 0.02f);
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            ctx->cam->ProcessKeyboard(RIGHT, 0.02f);
        }
    };
    glfwSetKeyCallback(window, keyboardCallback);

    static bool camFreeze = false;
    

    auto cursorPosCallback = [](GLFWwindow* window, double xpos, double ypos) {

        static float curPosX = xpos, curPosY = ypos;
        if (xpos >= 0 && xpos < width && ypos >= 0 && ypos < height) {
            CallbackContext* ctx = static_cast<CallbackContext*>(glfwGetWindowUserPointer(window));
            float dx = xpos - curPosX;
            float dy = curPosY - ypos;
            if (!camFreeze)
                ctx->cam->ProcessMouseMovement(dx, dy);

            curPosX = xpos;
            curPosY = ypos;
        }
    };
    glfwSetCursorPosCallback(window, cursorPosCallback);

    auto mouseButtonCallback = [](GLFWwindow* window, int button, int action, int mods) {
        if (action == GLFW_PRESS) {
            if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                camFreeze = !camFreeze;
            }
            else if (button == GLFW_MOUSE_BUTTON_LEFT) {
                CallbackContext* ctx = static_cast<CallbackContext*>(glfwGetWindowUserPointer(window));
                double xPos;
                double yPos;
                glfwGetCursorPos(window, &xPos, &yPos);

                rayCast(*(ctx->surface), *(ctx->model), *(ctx->cam), *(ctx->shader), yPos, xPos, *(ctx->projection));
            }
        }   
    };
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    FrameBuffer frameBuffer(width, height);

    int defaultFrameBufferWidth, defaultFrameBufferHeight;
    glfwGetFramebufferSize(window, &defaultFrameBufferWidth, &defaultFrameBufferHeight);

    while (!glfwWindowShouldClose(window)) {
        frameBuffer.Bind();

        glViewport(0, 0, width, height);
        glClearColor(0.92f, 0.92f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 view = camera.GetInvertedCamera(0.0f);
        glEnable(GL_CULL_FACE);

        // Terrain
        heightMap.Bind(GL_TEXTURE0);
        terrainDiffuse.Bind(GL_TEXTURE1);
        terrainShader.bind();
        terrainShader.SetMat4("view", view);
        terrainShader.SetMat4("model", model);
        terrainShader.SetMat4("projection", projection);
        terrainShader.SetFloat("Ka", 0.3f);
        terrainShader.SetFloat("Kd", 1.0f);
        terrainShader.SetFloat("Ks", 1.0f);
        terrainShader.SetVec3("lightDir", { 0.0f, -1.0f, -1.0f });
        terrainShader.SetInteger("heightMap", 0);
        terrainShader.SetInteger("terrainTexture", 1);

        terrainMesh.DrawElements();


        // Grass
        heightMap.Bind(GL_TEXTURE0);
        grassDistribution.Bind(GL_TEXTURE1);
        grassDiffuse.Bind(GL_TEXTURE2);
        grassShader.bind();
        grassShader.SetMat4("view", view);
        grassShader.SetMat4("model", model);
        grassShader.SetMat4("projection", projection);
        grassShader.SetFloat("Ka", 0.3f);
        grassShader.SetFloat("Kd", 1.0f);
        grassShader.SetFloat("Ks", 1.0f);
        grassShader.SetVec3("lightDir", { 0.0f, -1.0f, -1.0f });
        grassShader.SetInteger("heightMap", 0);
        grassShader.SetInteger("grassDist", 1);
        grassShader.SetInteger("grassText", 2);

        terrainMesh.DrawElements();

        glDisable(GL_CULL_FACE);

        frameBuffer.Unbind();
        glViewport(0, 0, defaultFrameBufferWidth, defaultFrameBufferHeight);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        view = camera.GetViewMatrix();

        // Terrain
        heightMap.Bind(GL_TEXTURE0);
        terrainDiffuse.Bind(GL_TEXTURE1);
        terrainShader.bind();
        terrainShader.SetMat4("view", view);
        terrainShader.SetMat4("model", model);
        terrainShader.SetMat4("projection", projection);
        terrainShader.SetFloat("Ka", 0.3f);
        terrainShader.SetFloat("Kd", 1.0f);
        terrainShader.SetFloat("Ks", 1.0f);
        terrainShader.SetVec3("lightDir", { 0.0f, -1.0f, -1.0f });
        terrainShader.SetInteger("heightMap", 0);
        terrainShader.SetInteger("terrainTexture", 1);

        terrainMesh.DrawElements();

        // Water
        waterDiffuse.Bind(GL_TEXTURE0);
        frameBuffer.BindTexture(GL_TEXTURE1);
        waterShader.bind();
        waterShader.SetMat4("view", view);
        waterShader.SetMat4("model", model);
        waterShader.SetMat4("projection", projection);
        waterShader.SetFloat("time", glfwGetTime());
        waterShader.SetFloat("Ka", 0.3f);
        waterShader.SetFloat("Kd", 1.0f);
        waterShader.SetFloat("Ks", 1.0f);
        waterShader.SetVec3("lightDir", { 0.0f, -1.0f, -1.0f });
        waterShader.SetVec3("viewDir", camera.Position);
        waterShader.SetInteger("waterTex", 0);
        waterShader.SetInteger("reflectionTex", 1);

        waterMesh.DrawElements();

        // Grass
        heightMap.Bind(GL_TEXTURE0);
        grassDistribution.Bind(GL_TEXTURE1);
        grassDiffuse.Bind(GL_TEXTURE2);
        grassShader.bind();
        grassShader.SetMat4("view", view);
        grassShader.SetMat4("model", model);
        grassShader.SetMat4("projection", projection);
        grassShader.SetFloat("Ka", 0.3f);
        grassShader.SetFloat("Kd", 1.0f);
        grassShader.SetFloat("Ks", 1.0f);
        grassShader.SetVec3("lightDir", { 0.0f, -1.0f, -1.0f });
        grassShader.SetInteger("heightMap", 0);
        grassShader.SetInteger("grassDist", 1);
        grassShader.SetInteger("grassText", 2);

        terrainMesh.DrawElements();


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}

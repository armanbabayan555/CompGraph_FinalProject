#include "Triangle.h"
#include <iostream>
#define EPSILON 0.0001

using namespace std;

Triangle::Triangle(): vertices{{1.f, 0.f, -1.f}, {-1.f, 0.f, -1.f}, {0.f, 1.f, -1.f}}, color{128, 128, 128} {}

void Triangle::setColor(const Color& col) {
    color = col;
}

Color Triangle::getColor() const {
    return color;
}

void Triangle::setVertices(const vector<Vertex>& vertices) {
    if (vertices.size() != 3) {
        printf("Vertex size is not valid!");
    }
    this->vertices = vertices;
}

vector<Vertex> Triangle::getVertices() const {
    return vertices;
}

glm::vec3 Triangle::getPlaneNormal() const {
    return glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
}

float Triangle::getPlaneDistance() const {
    return abs(glm::dot(vertices[0], getPlaneNormal()));
}

Triangle::Triangle(const initializer_list<Vertex>& il, const Color& c) {
    if (il.size() != 3) {
        printf("Initializer list size is not valid!");
    }
    vertices = vector<Vertex>(3);
    copy(il.begin(), il.end(), vertices.begin());
    setColor(c);
}

bool Triangle::isInsideTriangle(const glm::vec3& point) const {
    for (int i = 0; i < 3; ++i) {
        if (glm::dot(glm::cross(
            vertices[(i + 1) % 3] - vertices[i],
            point - vertices[i]
        ), getPlaneNormal()) < 0.f) {
            return false;
        }
    }

    return true;
}

bool Triangle::intersects(const Ray& ray, float& t) const {
    float dirDotNormal = glm::dot(ray.dir, getPlaneNormal());
    
    if (abs(dirDotNormal - 0.f) < EPSILON) {
        return false;
    }
    
    float tTmp = (getPlaneDistance() - dot(ray.p0, getPlaneNormal())) / dirDotNormal;
    
    if (tTmp < 0.0f) {
        return false;
    }
    
    if (!isInsideTriangle(ray.p0 + tTmp * ray.dir)) {
        return false;
    }
    
    t = tTmp;
    
    return true;
}

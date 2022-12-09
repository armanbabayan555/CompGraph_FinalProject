#ifndef Triangle_h
#define Triangle_h
using namespace std;

#include <initializer_list>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <opencv2/opencv.hpp>

using Color = cv::Vec3b;
using Vertex = glm::vec3;

struct Ray {
    glm::vec3 p0;
    glm::vec3 dir;
};

class Triangle {
public:
    Triangle();
    Triangle(const initializer_list<Vertex>& il, const Color& c);
    void setColor(const Color& col);
    Color getColor() const;
    void setVertices(const vector<Vertex>& vertices);
    vector<Vertex> getVertices() const;
    bool intersects(const Ray& r, float& t) const;
    
private:
    vector<Vertex> vertices;
    Color color;
public:
    glm::vec3 getPlaneNormal() const;
    float getPlaneDistance() const;
public:
    bool isInsideTriangle(const glm::vec3& point) const;
};

#endif

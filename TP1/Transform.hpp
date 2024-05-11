#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

class Transform
{
private:
    mat4 localMat = mat4(1.0f);

public:
    // Global space information concatenate in matrix
    mat4 model = mat4(1.0f);
    vec3 t;

    void Translate(vec3 translatation)
    {
        localMat = translate(localMat, translatation);
        t += translatation;
        t.x*=localMat[1][1];
        t.y*=localMat[2][2];
        t.z*=localMat[3][3];

    }

    void Rotation(const vec3 axis, float angle)
    {
        localMat = rotate(localMat, angle, axis);
    }

    void Scale(vec3 scaling)
    {
        localMat = scale(localMat, scaling);
    }

    Transform(/* args */)
    {
        model = mat4(1.0f);
    }

    ~Transform()
    {
    }

    mat4 getLocalModel()
    {
        return localMat;
    }

    void setPosition(glm::vec3 pos)
    {
        t = pos;
    }

    glm::vec3 getPosition()
    {
        return t;
    }
};

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h> 

using namespace glm;

class Transform
{
private:
    mat4 localMat = mat4(1.0f);
    vec3 position = vec3(0.0f);

public:
    mat4 model = mat4(1.0f);

    void Translate(vec3 translation)
    {
        localMat = translate(localMat, translation);
        position += translation;
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

    vec3 getPosition()
    {
        return position;
    }

    void setPosition(vec3 newPosition)
    {
        vec3 translation = newPosition - position; 
        Translate(translation);
    }

    mat4 getLocalModel()
    {
        return localMat;
    }
};

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

class Transform
{
private:
    mat4 localMat = mat4(1.0f);
    vec3 prevPosition = vec3(0.0f);
public:
    // Global space information concatenate in matrix
    mat4 model = mat4(1.0f);
    vec3 t;

    void setLocalMatX(float res)
    {
        localMat[0][2] = res;
    }

    void Translate2(vec3 translation)
    {
        // Calcul de la translation relative par rapport à la position précédente
        vec3 relativeTranslation = translation - prevPosition;
        
        localMat = translate(localMat, relativeTranslation);
        t += relativeTranslation;
        prevPosition = t; // Mettre à jour la position précédente
    }


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

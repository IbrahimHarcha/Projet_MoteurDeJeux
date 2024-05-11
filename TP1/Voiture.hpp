#ifndef OBJECT_HPP
#define OBJECT_HPP
#include "Object.hpp"
#endif

class Voiture : public Object
{
private:
    /* data */
    vec3 vitesse;
    float vitesseMax ;
    float tpsMax ;
    int turn;
    bool stop;

public:
    Voiture(float vitessemax=0, float timemax=0,vec3 speed = vec3(0, 0, 0));
    ~Voiture();
    void setSpeed(vec3 speed);
    void addSpeed(vec3 speed);
    void mulSpeed(vec3 speed);
    vec3 getSpeed();
    void setTurn(int turn);
    int getTurn();
    void setStop(bool stop);
    bool getStop();
    void setVMax(float vMax);
    float getVMax();
    void setTMax(float t);
    float getTMax();
    void draw(GLuint programID);
};

Voiture::Voiture(float vitessemax, float timemax,vec3 speed)
{
    this->vitesse = speed;
    this->turn = 0;
    this->stop = false;
    this->vitesseMax=vitessemax;
    this->tpsMax=timemax;
}

Voiture::~Voiture()
{
}

void Voiture::setSpeed(vec3 speed)
{
    this->vitesse = speed;
}

void Voiture::addSpeed(vec3 speed)
{
    this->vitesse += speed;
}

void Voiture::mulSpeed(vec3 speed)
{
    this->vitesse *= speed;
}

vec3 Voiture::getSpeed()
{
    return this->vitesse;
}

void Voiture::setTurn(int turn)
{
    this->turn = turn;
}

int Voiture::getTurn()
{
    return this->turn;
}


void Voiture::setStop(bool stop)
{
    this->stop = stop;
}

bool Voiture::getStop()
{
    return this->stop;
}

void Voiture::setVMax(float vMax)
{
    vitesseMax = vMax ;
}
    
float Voiture::getVMax()
{
    return vitesseMax ;
}
    
void Voiture::setTMax(float t)
{
    tpsMax = t ;
}

float Voiture::getTMax()
{
    return tpsMax ;
}


    void Voiture::draw(GLuint programID)
    {
        if (texture != -1){
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(programID, "text"), 0);
            glBindTexture(GL_TEXTURE_2D, texture);
        }

    glUniformMatrix4fv(glGetUniformLocation(programID, "modelM"), 1, GL_FALSE, &(this->transform.model)[0][0]);
    glUniform1i(glGetUniformLocation(programID, "mode"), mode);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,        // attribute
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uv);
    glVertexAttribPointer(
        1,        // attribute
        2,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
        2,        // attribute
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );

    glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // Draw triangles directly from vertices array

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    for (Object *child : children)
    {
        child->draw(programID);
    }
};
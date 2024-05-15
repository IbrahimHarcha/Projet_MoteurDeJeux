#ifndef TERRAIN_HPP
#define TERRAIN_HPP
#include "Terrain.hpp"
#endif

class Cube : public Terrain
{
protected:
    
public:
    void draw(GLuint programID);
    void addCube(const Cube &newCube);
    void setSizeY(int sizeY) { this->sizeY = sizeY; };
    void setSizeX(int sizeX) { this->sizeX = sizeX; };

    bool first;

    void setFirst(bool first);
    bool getFirst();

    // void loadTexture(std::string pathtexture);
    
};

// void Cube::loadTexture(std::string pathtexture)
// {
//     texture = loadTexture2DFromFilePath(pathtexture, this->width, this->height, this->nrChannels);
// }
    void Cube::setFirst(bool first){
        this->first = first;
    }

    bool Cube::getFirst(){
        return this->first;
    }

    void Cube::addCube(const Cube &newCube) {
        // Créez une copie du nouveau cube
        Cube *cubeCopy = new Cube(newCube);

        // Ajoutez la copie à la liste des enfants
        this->children.push_back(cubeCopy);
    }

    void Cube::draw(GLuint programID)
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

    if(!first){
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    }
    // glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // Draw triangles directly from vertices array

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    for (Object *child : children)
    {
        child->draw(programID);
    }
};
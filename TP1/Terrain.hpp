#ifndef OBJECT_HPP
#define OBJECT_HPP
#include "Object.hpp"
#include <vector>

struct Triangle {
    inline Triangle () {
        v[0] = v[1] = v[2] = 0;
    }
    inline Triangle (const Triangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
    }
    inline Triangle (unsigned int v0, unsigned int v1, unsigned int v2) {
        v[0] = v0;   v[1] = v1;   v[2] = v2;
    }
    bool isValid() const {
        // Vérifie si les trois sommets sont différents et ne dépassent pas la taille des données
        return (v[0] != v[1] && v[0] != v[2] && v[1] != v[2]);
    }
    
    unsigned int & operator [] (unsigned int iv) { return v[iv]; }
    unsigned int operator [] (unsigned int iv) const { return v[iv]; }
    inline virtual ~Triangle () {}
    inline Triangle & operator = (const Triangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
        return (*this);
    }
    // membres :
    unsigned int v[3];
};

class Terrain : public Object
{
protected:
    GLuint hmap;
    int nX;
    int nY;
    int sizeX;
    int sizeY;
    float offsetX;
    float offsetZ;
    float offsetY;
    bool HeightMap;
    string path;

public:
    float offSetZ;
    Terrain(/* args */);
    Terrain(int nX, int nY, int sizeX, int sizeY, float offsetX, float offsetZ, bool HeightMap, string path = "", float offsetY = 0);

    ~Terrain();
    void setPlan(int nX, int nY, int sizeX, int sizeY, float offsetX, float offsetZ, float offsetY, bool HeightMap, string path = "");
    void changeResolution(int newResX, int newResY);
    void draw(GLuint programID, float camPosZ);

    // vector<vec3> getTerrainVertices()
    // {
    //     return getVertices();
    // }

    // vector<unsigned short> getTerrainIndices()
    // {
    //     return getIndices();
    // }

    float getOffsetY()
    {
        return this->offsetY;
    }

    float getOffsetX()
    {
        return this->offsetX;
    }

    float getOffsetZ()
    {
        return this->offSetZ;
    }


};

Terrain::Terrain(/* args */)
{
}

Terrain::Terrain(int nX, int nY, int sizeX, int sizeY, float offsetX, float offsetZ, bool HeightMap, string path, float offsetY)
{
    this->nX = nX;
    this->nY = nY;
    this->sizeX = sizeX;
    this->sizeY = sizeY;
    this->offsetX = offsetX;
    this->offsetY = offsetY;
    this->offsetZ = offsetZ;
    this->HeightMap = HeightMap;
    this->path = path;

    setPlan(nX, nY, sizeX, sizeY, offsetX, offsetZ, offsetY, HeightMap, path);
}

Terrain::~Terrain()
{
}

void Terrain::setPlan(int nX, int nY, int sizeX, int sizeY, float offsetX, float offsetZ, float offsetY, bool HeightMap, string path)
{
    this->offSetZ = offsetZ;
    this->vertices.clear();
    this->indices.clear();

    vector<vec2> texCoords;

    for (size_t i = 0; i <= nX; i++)
    {
        for (size_t j = 0; j <= nY; j++)
        {
            this->vertices.push_back(vec3((((float)sizeX / (float)nX) * i) - offsetX, -offsetY, (((float)sizeY / (float)nY) * j) - offsetZ));
            texCoords.push_back(vec2((float)i / (float)nX, (float)j / (float)nY));
        }
    }

    for (size_t i = 0; i < nX; i++)
    {
        for (size_t j = 0; j < nY; j++)
        {
            this->indices.push_back((i + 1) * (nX + 1) + j);
            this->indices.push_back((i + 1) * (nX + 1) + j + 1);
            this->indices.push_back(i * (nX + 1) + j + 1);

            this->indices.push_back(i * (nX + 1) + j);
            this->indices.push_back((i + 1) * (nX + 1) + j);
            this->indices.push_back(i * (nX + 1) + j + 1);

            texCoords.push_back(vec2((float)(i + 1) / (float)nX, (float)j / (float)nY));
            texCoords.push_back(vec2((float)(i + 1) / (float)nX, (float)(j + 1) / (float)nY));
            texCoords.push_back(vec2((float)(i) / (float)nX, (float)(j + 1) / (float)nY));
            texCoords.push_back(vec2((float)(i) / (float)nX, (float)j / (float)nY));
            texCoords.push_back(vec2((float)(i + 1) / (float)nX, (float)(j) / (float)nY));
            texCoords.push_back(vec2((float)(i) / (float)nX, (float)(j + 1) / (float)nY));
        }
    }
    this->UVs = texCoords;

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(glm::vec3), &this->vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uv);
    glBindBuffer(GL_ARRAY_BUFFER, uv);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(vec2), &texCoords[0], GL_STATIC_DRAW);

    if (HeightMap)
    {
        this->mode = 1;
        this->hmap = loadTexture2DFromFilePath(this->path);
    }

    // Generate a buffer for the this->indices as well
}

void Terrain::changeResolution(int newResX, int newResY)
{
    this->nX = newResX;
    this->nY = newResY;
    setPlan(this->nX, this->nY, this->sizeX, this->sizeY, this->offsetX, this->offSetZ, this->offsetY, this->HeightMap, this->path);
}

void Terrain::draw(GLuint programID, float camPosZ)
{
    // float distanceThreshold = 32.0f; 
    // if (std::abs(transform.getPosition().z - camPosZ) > distanceThreshold)
    // {
    //     std::cout<<"On ne dessine pas..."<<std::endl;
    //     // l'objet est trop éloigné le long de l'axe Z, ne le dessinez pas
    //     return;
    // }
    if (this->hmap != -1)
    {
        glActiveTexture(GL_TEXTURE0 + 1);
        glUniform1i(glGetUniformLocation(programID, "heightMapPlan"), 1);
        glBindTexture(GL_TEXTURE_2D, this->hmap);
    }
    Object::draw(programID, camPosZ);
}


#endif
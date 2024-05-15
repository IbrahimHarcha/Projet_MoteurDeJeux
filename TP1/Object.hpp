#include "Transform.hpp"
#include <vector>
#include <common/texture.hpp>
#include <common/objloader.hpp>

using namespace std;

const glm::vec3 g(0.0f, -9.81f, 0.0f);

class Object
{
protected:
    Object *parent = nullptr;
    vector<Object *> children;
    vector<vec3> vertices;
    vector<vec3> normals;
    vector<unsigned short> indices;
    vector<vec2> UVs;
    GLuint vertexbuffer;
    GLuint elementbuffer;
    GLuint normalbuffer;
    GLuint texture;
    int width;
    int height;
    int nrChannels;
    GLuint uv;
    int mode = 0;
    vector<vec3> cube; // boite englobante pour collision


    glm::vec3 velocity; // vitesse
    glm::vec3 acceleration; // accélération
    glm::vec3 force; // force
    float mass; // masse
    float k ; // rési air

public:
    Transform transform;
    Object();
    ~Object();
    void add(Object *child);
    virtual void updateTree();
    void loadTexture(std::string pathtexture);
    vector<vec3> getVertices();
    vector<vec2> getUVs();
    vector<unsigned short> getIndices();
    virtual void draw(GLuint programID);
    vector<Object *> getChilds();
    void removeChild(int pos);
    vector<vec3> boiteEnglobante();

    void loadObject(const std::string &filename, int typeObject);


    // getters et setters
    glm::vec3 getVelocity() { return velocity; }
    void setVelocity(glm::vec3 velocity) { this->velocity = velocity; }
    void updateVelocity(glm::vec3 velocity) { this->velocity += velocity; }
    glm::vec3 getAcceleration() { return acceleration; }
    void setAcceleration(glm::vec3 acceleration) { this->acceleration = acceleration; }
    void updateAcceleration(glm::vec3 acceleration) { this->acceleration += acceleration; }
    glm::vec3 getForce() { return force; }
    void setForce(glm::vec3 force){ this->force = force; };
    float getk() {return k ;}
    void setk(float k){ this-> k = k ;}
    float getMass() { return mass; }
    void setMass(float mass) { this->mass = mass; }

    void updatePhysics(float deltaTime);
    void applyForce(glm::vec3 force);
    glm::vec3 cForce(glm::vec3 vitesse);
    glm::vec3 getSize();


    void setSpecialVelocity(const glm::vec3& specialVel);

    void setParent(Object *parent)
    {
        this->parent = parent;
    }

};

Object::Object()
{
}

Object::~Object()
{
    glDeleteBuffers(1, &uv);
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &elementbuffer);
}
void Object::add(Object *child)
{
    child->parent = this;
    this->children.push_back(child);
}

void Object::removeChild(int pos)
{
    if (pos >= 0 && pos < children.size())
    {
        children[pos]->parent = nullptr;
        children.erase(children.begin() + pos);
    }
}

void Object::updateTree()
{
    if (this->parent)
        this->transform.model = this->parent->transform.model * this->transform.getLocalModel();
    else
        this->transform.model = this->transform.getLocalModel();

    for (auto &&child : children)
    {
        child->updateTree();
    }
}

glm::vec3 Object::getSize()
{
    float Xmin = 100; 
    float Ymin = 100;
    float Zmin = 100;
    float Xmax = -100;
    float Ymax = -100;
    float Zmax = -100;
    for (long unsigned int i = 0; i < vertices.size(); i++)
    {
        if (vertices[i][0] < Xmin)
        {
            Xmin = vertices[i][0];
        }
        if (vertices[i][1] < Ymin)
        {
            Ymin = vertices[i][1];
        }
        if (vertices[i][2] < Zmin)
        {
            Zmin = vertices[i][2];
        }
        if (vertices[i][0] > Xmax)
        {
            Xmax = vertices[i][0];
        }
        if (vertices[i][1] > Ymax)
        {
            Ymax = vertices[i][1];
        }
        if (vertices[i][2] > Zmax)
        {
            Zmax = vertices[i][2];
        }
    }
    return vec3(Xmax - Xmin, Ymax - Ymin, Zmax - Zmin);
}


glm::vec3 Object::cForce( glm::vec3 vitesse) {
    glm::vec3 forceGravite = mass * g;
    glm::vec3 forceResistance = -k*vitesse;
    glm::vec3 forceInit = getForce();

    return forceGravite + forceResistance + forceInit;
}

void Object::updatePhysics(float deltaTime)
{   

    glm::vec3 velocity = getVelocity();
    glm::vec3 pos = transform.getPosition();
    glm::vec3 force  = cForce(velocity);
    float masse = getMass();
    glm::vec3 acc = force/masse;
    glm::vec3 midPos = pos + (0.5f * deltaTime)*velocity;
    glm::vec3 midVel = velocity + (0.5f * deltaTime) * acc;
    glm::vec3 midForc = cForce(midVel);
    glm::vec3 midAcc = midForc/masse;
    glm::vec3 newPos = pos + deltaTime*midVel;
    glm::vec3 newVel = velocity+deltaTime*midAcc;
    //printf("position avant %f %f %f \n",transform.getPosition().x,transform.getPosition().y,transform.getPosition().z);
    transform.Translate(newPos-pos);
    transform.setPosition(transform.t);
    //printf("position après %f %f %f \n",transform.getPosition().x,transform.getPosition().y,transform.getPosition().z);
    setVelocity(newVel);
    setForce(cForce(newVel));
}


// Méthode permettant d'appliquer une force à l'objet
void Object::applyForce(glm::vec3 force)
{
    glm::vec3 currentForce = getForce();
    setForce(currentForce + force);
}

vector<vec3> Object::getVertices()
{
    return vertices;
}
vector<vec2> Object::getUVs()
{
    return UVs;
}

vector<Object *> Object::getChilds()
{
    return children;
}

vector<unsigned short> Object::getIndices()
{
    return indices;
}

void Object::draw(GLuint programID)
{
    // if(this->typeObject){

    // }
    if (texture != -1)
    {
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
        (void *)0);
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

    

    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_SHORT, (void *)0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    for (Object *child : children)
    {
        child->draw(programID);
    }
}

void Object::loadTexture(std::string pathtexture)
{
    texture = loadTexture2DFromFilePath(pathtexture, this->width, this->height, this->nrChannels);
}


void Object::loadObject(const std::string &filename, int typeObject)
{
    std::vector<std::vector<unsigned short>> triangles;
    if (typeObject == 0)
    { // off
        loadOFF(filename, this->vertices, this->indices, triangles);
        vector<vec2> uvs;

        long int taille = vertices.size();
        float theta, phi, u, v;
        for (int i = 0; i < taille; i++)
        {
            theta = acos(vertices[i][1]);
            phi = atan(vertices[i][2], vertices[i][0]);
            u = (phi / (2 * M_PI) + 0.5);
            v = (theta / M_PI);
            uvs.push_back(vec2(u, v));
        }
        this->UVs = uvs;
    }
    else if (typeObject == 1) // obj ...
    {
        std::vector<vec3> normals;
        bool test = loadOBJ(filename.c_str(), this->vertices,  this->UVs , this->normals);

        if(!test){
            std::cout<<"Erreur lors du chargement de l'objet"<<std::endl;
        }
    }

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uv);
    glBindBuffer(GL_ARRAY_BUFFER, uv);
    glBufferData(GL_ARRAY_BUFFER, this->UVs.size() * sizeof(vec2), &UVs[0], GL_STATIC_DRAW);
}


vector<vec3> Object::boiteEnglobante()
{
        float Xmin = 100; //initialisation de valeurs arbitraires
        float Ymin = 100;
        float Zmin = 100;
        float Xmax = -100;
        float Ymax = -100;
        float Zmax = -100;
        for(long unsigned int i=0; i<vertices.size() ; i++){ 
            if(vertices[i][0]<Xmin){
                Xmin = vertices[i][0];
            }
            if(vertices[i][1]<Ymin){
                Ymin = vertices[i][1];
            }
            if(vertices[i][2]<Zmin){
                Zmin = vertices[i][2];
            }
            if(vertices[i][0]>Xmax){
                Xmax = vertices[i][0];
            }
            if(vertices[i][1]>Ymax){
                Ymax= vertices[i][1];
            }
            if(vertices[i][2]>Zmax){
                Zmax = vertices[i][2];
            }
        }
        cube.push_back(vec3(Xmin,Ymin,Zmin));
        cube.push_back(vec3(Xmax,Ymax,Zmax));
        return cube;
}

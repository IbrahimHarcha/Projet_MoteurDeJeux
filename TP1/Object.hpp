#include <vector>
#include <common/texture.hpp>
#include <common/objloader.hpp>
#include "Transform.hpp" 

using namespace std;

class Object
{
protected:
    Object *parent = nullptr;
    vector<Object *> children;
    vector<vec3> vertices;
    vector<unsigned short> indices;
    GLuint vertexbuffer;
    GLuint elementbuffer;
    GLuint texture;
    GLuint uv;
    GLuint normalbuffer;

    int mode = 0;
    vector<vec2> UVs;
    std::vector<glm::vec3> normals;

    glm::vec3 velocity; // vitesse
    glm::vec3 acceleration; // accélération
    glm::vec3 force; // force
    float mass; // masse

public:
    Transform transform;

    // Vélocité spéciale
    glm::vec3 specialVelocity;
    bool specialVelocityEnabled = false;

    Object();
    ~Object();

    void add(Object &child);
    void loadObject(const std::string &filename);
    void update();
    void draw(GLuint programID);

    void draw2(GLuint programID);


    vector<vec3> getVertices();
    vector<unsigned short> getIndices();
    vector<Object *> getChilds();
    void loadTexture(std::string pathtexture);
    void setVertices(vector<vec3> vertices);
    void setIndices(vector<unsigned short> indices);

    void loadTextureCube(std::string texturePath);

    // getters et setters
    glm::vec3 getVelocity() { return velocity; }
    void setVelocity(glm::vec3 velocity) { this->velocity = velocity; }

    glm::vec3 getAcceleration() { return acceleration; }
    void setAcceleration(glm::vec3 acceleration) { this->acceleration = acceleration; }

    glm::vec3 getForce() { return force; }
    void setForce(glm::vec3 force){ this->force = force; };

    float getMass() { return mass; }
    void setMass(float mass) { this->mass = mass; }

    void updatePhysics(float deltaTime);
    void applyForce(glm::vec3 force);

    glm::vec3 getSize();


    void setSpecialVelocity(const glm::vec3& specialVel);

    void loadObjectCustom(const std::string& filename, bool invertUVs);
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

void Object::updatePhysics(float deltaTime)
{
    // la force de gravité (F = mg)
    glm::vec3 gravityForce = glm::vec3(0.0f, -9.81f * mass, 0.0f); // g = 9.81 m/s^2 (accélération gravitationnelle)
    
    // on calcul l'accélération...
    glm::vec3 totalForce = force + gravityForce;
    glm::vec3 acceleration = totalForce / mass;
    
    // Mise à jour de la vitesse en utilisant la méthode d'Euler explicite
    velocity += acceleration * deltaTime;
    
    // Si une vélocité spéciale a été définie, l'utiliser
    if (specialVelocityEnabled) {
        // Réinitialiser la vélocité 
        specialVelocityEnabled = false;
        velocity = specialVelocity;
    }
    
    // Mise à jour de la position en utilisant la méthode d'Euler explicite
    transform.setPosition(transform.getPosition() + velocity * deltaTime);
    
    // on reset la force pour le prochain pas de simulation
    force = glm::vec3(0.0f);
}

void Object::setSpecialVelocity(const glm::vec3& specialVel)
{
    specialVelocity = specialVel;
    specialVelocityEnabled = true;
}

// Méthode permettant d'appliquer une force à l'objet
void Object::applyForce(glm::vec3 force)
{
    glm::vec3 currentForce = getForce();
    setForce(currentForce + force);
}

void Object::setVertices(vector<vec3> vertices)
{
    this->vertices = vertices;
}

void Object::setIndices(vector<unsigned short> indices)
{
    this->indices = indices;
}

vector<vec3> Object::getVertices()
{
    return vertices;
}

vector<Object *> Object::getChilds()
{
    return children;
}

vector<unsigned short> Object::getIndices()
{
    return indices;
}

void Object::add(Object &child)
{
    child.parent = this;
    this->children.push_back(&child);
}

void Object::update()
{
    if (this->parent)
        this->transform.model = this->parent->transform.model * this->transform.getLocalModel();
    else
        this->transform.model = this->transform.getLocalModel();

    for (auto &&child : children)
    {
        child->update();
    }
}

void Object::draw(GLuint programID)
{
    if (texture != -1)
    {
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(programID, "text"), 0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    glUniformMatrix4fv(glGetUniformLocation(programID, "Model"), 1, GL_FALSE, &(this->transform.model)[0][0]);
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

void Object::loadObjectCustom(const std::string& filename, bool invertUVs)
{
    // Définir ici le chargement personnalisé de l'objet OBJ
    // Par exemple, vous pourriez modifier la façon dont les UV sont traitées en fonction du paramètre invertUVs

    // Utilisation de la fonction loadOBJ existante avec des modifications supplémentaires si nécessaire
    bool success = loadOBJ(filename.c_str(), vertices, UVs, normals);

    if (!success)
    {
        // Gérer les échecs de chargement de fichier, par exemple :
        std::cerr << "Erreur lors du chargement de l'objet OBJ : " << filename << std::endl;
        return;
    }

    // Appliquer des modifications supplémentaires si nécessaire en fonction des options personnalisées
    if (invertUVs)
    {
        // Inverser les coordonnées de texture si nécessaire
        for (auto& uv : UVs)
        {
            uv.y = 1.0f - uv.y;
        }
    }

    // Charger les données dans les tampons OpenGL comme précédemment
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uv);
    glBindBuffer(GL_ARRAY_BUFFER, uv);
    glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(vec2), &UVs[0], GL_STATIC_DRAW);
}

 void Object::loadObject(const std::string &filename)
{
    std::vector<std::vector<unsigned short>> triangles;
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
    this->UVs=uvs;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uv);
    glBindBuffer(GL_ARRAY_BUFFER, uv);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
}

void Object::loadTexture(std::string pathtexture)
{
    texture = loadTexture2DFromFilePath(pathtexture);
}


glm::vec3 Object::getSize() {
    // Initialisation des dimensions minimales et maximales avec les premières coordonnées du premier vertex
    float minX = vertices[0].x;
    float minY = vertices[0].y;
    float minZ = vertices[0].z;
    float maxX = vertices[0].x;
    float maxY = vertices[0].y;
    float maxZ = vertices[0].z;

    // Parcours de tous les vertices pour trouver les dimensions minimales et maximales
    for (const auto& vertex : vertices) {
        if (vertex.x < minX) minX = vertex.x;
        if (vertex.y < minY) minY = vertex.y;
        if (vertex.z < minZ) minZ = vertex.z;
        if (vertex.x > maxX) maxX = vertex.x;
        if (vertex.y > maxY) maxY = vertex.y;
        if (vertex.z > maxZ) maxZ = vertex.z;
    }

    // Calcul des dimensions de la boîte englobante
    float width = maxX - minX;
    float height = maxY - minY;
    float depth = maxZ - minZ;

    return vec3(width, height, depth);
}


void Object::draw2(GLuint programID)
{
    if (texture != -1)
    {
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(programID, "text"), 0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    glUniformMatrix4fv(glGetUniformLocation(programID, "Model"), 1, GL_FALSE, &(this->transform.model)[0][0]);
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
}
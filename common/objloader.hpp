#ifndef OBJLOADER_H
#define OBJLOADER_H
#include <string>

bool loadOBJ(
    const char *path,
    std::vector<glm::vec3> &out_vertices,
    std::vector<glm::vec2> &out_uvs,
    std::vector<glm::vec3> &out_normals);

bool loadAssImp(
    const char *path,
    std::vector<unsigned short> &indices,
    std::vector<glm::vec3> &vertices,
    std::vector<glm::vec2> &uvs,
    std::vector<glm::vec3> &normals);


bool loadOFF(const std::string &filename,
             std::vector<glm::vec3> &vertices,
             std::vector<unsigned short> &indices,
             std::vector<std::vector<unsigned short>> &triangles);



void load_model(const char *path,
                std::vector<unsigned short> &indices,
               std::vector<glm::vec3> &vertices,
               std::vector<glm::vec3> &normals,
               std::vector<glm::vec2> &uvs);

void set_buffer_data(unsigned int index);

#endif

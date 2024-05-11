#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <fstream>

#include <glm/glm.hpp>

#include "objloader.hpp"
#include "vboindexer.hpp"
#include "texture.hpp"

// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide :
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOBJ(
    const char *path,
    std::vector<glm::vec3> &out_vertices,
    std::vector<glm::vec2> &out_uvs,
    std::vector<glm::vec3> &out_normals)
{
    printf("Loading OBJ file %s...\n", path);

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
        getchar();
        return false;
    }

    while (1)
    {

        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader

        if (strcmp(lineHeader, "v") == 0)
        {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0)
        {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0)
        {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0)
        {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9)
            {
                printf("File can't be read by our simple parser :-( Try exporting with other options\n");
                fclose(file);
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
        else
        {
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }

    // For each vertex of each triangle
    for (unsigned int i = 0; i < vertexIndices.size(); i++)
    {

        // Get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        // Get the attributes thanks to the index
        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        glm::vec2 uv = temp_uvs[uvIndex - 1];
        glm::vec3 normal = temp_normals[normalIndex - 1];

        // Put the attributes in buffers
        out_vertices.push_back(vertex);
        out_uvs.push_back(uv);
        out_normals.push_back(normal);
    }
    fclose(file);
    return true;
}

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Mesh
	{
		unsigned int VAO, VBO1, VBO2, VBO3, EBO; // Buffer handles (Typically type: GLuint is used)
		
		std::vector<glm::vec3> vert_positions;
		std::vector<glm::vec3> vert_normals;
		std::vector<glm::vec2> tex_coords;
		std::vector<unsigned short> vert_indices;
		unsigned int tex_handle;
	};	
 



unsigned int num_meshes;
std::vector<Mesh> mesh_list;

void load_model(const char *path,
                std::vector<unsigned short> &indices,
               std::vector<glm::vec3> &vertices,
               std::vector<glm::vec3> &normals,
               std::vector<glm::vec2> &uvs)
{

    Assimp::Importer importer;
    const aiScene *scene = nullptr;
    scene = importer.ReadFile(path, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        std::cout << "Assimp importer.ReadFile (Error) -- " << importer.GetErrorString() << "\n";
    else
    {
        num_meshes = scene->mNumMeshes;
        mesh_list.clear();
        mesh_list.resize(num_meshes);

        aiMesh *mesh{};
        int indices_offset = 0; // Not being used yet... i.e. indices_offset += mesh->mNumVertices; is commented further down.

        // (1) Loop through all the model's meshes
        // -----------------------------------------------------
        for (unsigned int i = 0; i < num_meshes; ++i)
        {
            mesh = scene->mMeshes[i]; // http://assimp.sourceforge.net/lib_html/structai_mesh.html

            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex]; // http://assimp.sourceforge.net/lib_html/structai_material.html

            // This loop will only run once (i.e. there's only 1 texture per mesh)
           
            // (3) Loop through all mesh [i]'s vertices
            // ---------------------------------------------------
            for (unsigned int i2 = 0; i2 < mesh->mNumVertices; ++i2)
            {
                glm::vec3 position{};
                position.x = mesh->mVertices[i2].x;
                position.y = mesh->mVertices[i2].y;
                position.z = mesh->mVertices[i2].z;
                mesh_list[i].vert_positions.push_back(position);

                if (mesh->HasNormals())
                {
                    glm::vec3 normal{};
                    normal.x = mesh->mNormals[i2].x;
                    normal.y = mesh->mNormals[i2].y;
                    normal.z = mesh->mNormals[i2].z;
                    mesh_list[i].vert_normals.push_back(normal);
                }
                else
                    mesh_list[i].vert_normals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

                if (mesh->HasTextureCoords(0)) // Only slot [0] is in question.
                {
                    glm::vec2 tex_coords{};
                    tex_coords.x = mesh->mTextureCoords[0][i2].x;
                    tex_coords.y = mesh->mTextureCoords[0][i2].y;
                    mesh_list[i].tex_coords.push_back(tex_coords);
                }
                else
                    mesh_list[i].tex_coords.push_back(glm::vec2(0.0f, 0.0f));
            }
            // (4) Loop through all mesh [i]'s Indices
            // --------------------------------------------------
            for (unsigned int i3 = 0; i3 < mesh->mNumFaces; ++i3)
                for (unsigned int i4 = 0; i4 < mesh->mFaces[i3].mNumIndices; ++i4)
                    mesh_list[i].vert_indices.push_back(mesh->mFaces[i3].mIndices[i4] + indices_offset);

            // indices_offset += mesh->mNumVertices; // Disabled for tutorial: Model Loading (Part 1 of 3)

            // set_buffer_data(i); // Set up: VAO, VBO and EBO.
            indices.insert(indices.end(), mesh_list[i].vert_indices.begin(), mesh_list[i].vert_indices.end());
            normals.insert(normals.end(), mesh_list[i].vert_normals.begin(), mesh_list[i].vert_normals.end());
            vertices.insert(vertices.end(), mesh_list[i].vert_positions.begin(), mesh_list[i].vert_positions.end());
            uvs.insert(uvs.end(), mesh_list[i].tex_coords.begin(), mesh_list[i].tex_coords.end());

        }
    }
}

void set_buffer_data(unsigned int index)
	{
		glGenVertexArrays(1, &mesh_list[index].VAO);
		glGenBuffers(1, &mesh_list[index].VBO1); // Alternative to using 3 separate VBOs, instead use only 1 VBO and set glVertexAttribPointer's offset...
		glGenBuffers(1, &mesh_list[index].VBO2); // like was done in tutorial 3... Orbiting spinning cubes.
		glGenBuffers(1, &mesh_list[index].VBO3);
		glGenBuffers(1, &mesh_list[index].EBO);
 
		glBindVertexArray(mesh_list[index].VAO);		
 
		// Vertex Positions
		// ---------------------
		glBindBuffer(GL_ARRAY_BUFFER, mesh_list[index].VBO1);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh_list[index].vert_positions.size(), &mesh_list[index].vert_positions[0], GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(0); // Void pointer below is for legacy reasons. Two possible meanings: "offset for buffer objects" & "address for client state arrays"
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		
		// Vertex Normals
		// --------------------
		glBindBuffer(GL_ARRAY_BUFFER, mesh_list[index].VBO2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh_list[index].vert_normals.size(), &mesh_list[index].vert_normals[0], GL_STATIC_DRAW);
 
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
 
		// Texture Coordinates
		// ---------------------------
		glBindBuffer(GL_ARRAY_BUFFER, mesh_list[index].VBO3);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh_list[index].tex_coords.size(), &mesh_list[index].tex_coords[0], GL_STATIC_DRAW);
 
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		
		// Indices for: glDrawElements()
		// ---------------------------------------
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_list[index].EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh_list[index].vert_indices.size(), &mesh_list[index].vert_indices[0], GL_STATIC_DRAW);
 
		glBindVertexArray(0); 	// Unbind VAO
	}
 

bool loadOFF(const std::string &filename,
             std::vector<glm::vec3> &vertices,
             std::vector<unsigned short> &indices,
             std::vector<std::vector<unsigned short>> &triangles)
{
    bool convertToTriangles = true;
    bool randomize = false;

    std::ifstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open())
    {
        std::cout << filename << " cannot be opened" << std::endl;
        return false;
    }

    std::string magic_s;

    myfile >> magic_s;

    if (magic_s != "OFF")
    {
        std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
        myfile.close();
        return false;
    }

    int n_vertices, n_faces, dummy_int;
    myfile >> n_vertices >> n_faces >> dummy_int;

    vertices.resize(n_vertices);

    for (int v = 0; v < n_vertices; ++v)
    {
        glm::vec3 vertex;
        myfile >> vertex.x >> vertex.y >> vertex.z;
        if (std::isnan(vertex.x))
            vertex.x = 0.0;
        if (std::isnan(vertex.y))
            vertex.y = 0.0;
        if (std::isnan(vertex.z))
            vertex.z = 0.0;
        vertices[v] = vertex;
    }

    for (int f = 0; f < n_faces; ++f)
    {
        int n_vertices_on_face;
        myfile >> n_vertices_on_face;
        if (n_vertices_on_face == 3)
        {
            unsigned short _v1, _v2, _v3;
            std::vector<unsigned short> _v;
            myfile >> _v1 >> _v2 >> _v3;
            _v.push_back(_v1);
            _v.push_back(_v2);
            _v.push_back(_v3);
            triangles.push_back(_v);
            indices.push_back(_v1);
            indices.push_back(_v2);
            indices.push_back(_v3);
        }
        else if (n_vertices_on_face > 3)
        {
            std::vector<unsigned short> vhandles;
            vhandles.resize(n_vertices_on_face);
            for (int i = 0; i < n_vertices_on_face; ++i)
                myfile >> vhandles[i];

            if (convertToTriangles)
            {
                unsigned short k = (randomize) ? (rand() % vhandles.size()) : 0;
                for (unsigned short i = 0; i < vhandles.size() - 2; ++i)
                {
                    std::vector<unsigned short> tri(3);
                    tri[0] = vhandles[(k + 0) % vhandles.size()];
                    tri[1] = vhandles[(k + i + 1) % vhandles.size()];
                    tri[2] = vhandles[(k + i + 2) % vhandles.size()];
                    triangles.push_back(tri);
                    indices.push_back(tri[0]);
                    indices.push_back(tri[1]);
                    indices.push_back(tri[2]);
                }
            }
            else
            {
                // careful not triangles
                triangles.push_back(vhandles);
                for (int i = 0; i < vhandles.size(); ++i)
                    indices.push_back(vhandles[i]);
            }
        }
        else
        {
            std::cout << "OFFIO::open error : Face number " << f << " has " << n_vertices_on_face << " vertices" << std::endl;
            myfile.close();
            return false;
        }
    }

    myfile.close();
    return true;
}

bool loadOFF(const std::string &filename,
             std::vector<glm::vec3> &vertices,
             std::vector<unsigned short> &faces)
{
    bool convertToTriangles = true;
    bool randomize = false;

    std::ifstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open())
    {
        std::cout << filename << " cannot be opened" << std::endl;
        return false;
    }

    std::string magic_s;

    myfile >> magic_s;

    if (magic_s != "OFF")
    {
        std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
        myfile.close();
        return false;
    }

    int n_vertices, n_faces, dummy_int;
    myfile >> n_vertices >> n_faces >> dummy_int;

    vertices.resize(n_vertices);

    for (int v = 0; v < n_vertices; ++v)
    {
        glm::vec3 vertex;
        myfile >> vertex.x >> vertex.y >> vertex.z;
        if (std::isnan(vertex.x))
            vertex.x = 0.0;
        if (std::isnan(vertex.y))
            vertex.y = 0.0;
        if (std::isnan(vertex.z))
            vertex.z = 0.0;
        vertices[v] = vertex;
    }

    for (int f = 0; f < n_faces; ++f)
    {
        int n_vertices_on_face;
        myfile >> n_vertices_on_face;
        if (n_vertices_on_face == 3)
        {
            unsigned short _v1, _v2, _v3;
            std::vector<unsigned short> _v;
            myfile >> _v1 >> _v2 >> _v3;
            //            _v.push_back( _v1 );
            //            _v.push_back( _v2 );
            //            _v.push_back( _v3 );
            //            faces.push_back( _v );
            faces.push_back(_v1);
            faces.push_back(_v2);
            faces.push_back(_v3);
        }
        else if (n_vertices_on_face > 3)
        {
            std::vector<unsigned short> vhandles;
            vhandles.resize(n_vertices_on_face);
            for (int i = 0; i < n_vertices_on_face; ++i)
                myfile >> vhandles[i];

            if (convertToTriangles)
            {
                unsigned short k = (randomize) ? (rand() % vhandles.size()) : 0;
                for (unsigned short i = 0; i < vhandles.size() - 2; ++i)
                {
                    std::vector<unsigned short> tri(3);
                    tri[0] = vhandles[(k + 0) % vhandles.size()];
                    tri[1] = vhandles[(k + i + 1) % vhandles.size()];
                    tri[2] = vhandles[(k + i + 2) % vhandles.size()];
                    // faces.push_back(tri);
                    faces.push_back(tri[0]);
                    faces.push_back(tri[1]);
                    faces.push_back(tri[2]);
                }
            }
            else
            {
                // faces.push_back(vhandles);
                for (int i = 0; i < vhandles.size(); ++i)
                    faces.push_back(vhandles[i]);
            }
        }
        else
        {
            std::cout << "OFFIO::open error : Face number " << f << " has " << n_vertices_on_face << " vertices" << std::endl;
            myfile.close();
            return false;
        }
    }

    myfile.close();
    return true;
}

// #ifdef USE_ASSIMP // don't use this #define, it's only for me (it AssImp fails to compile on your machine, at least all the other tutorials still work)

// Include AssImp
#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

bool loadAssImp(
    const char *path,
    std::vector<unsigned short> &indices,
    std::vector<glm::vec3> &vertices,
    std::vector<glm::vec2> &uvs,
    std::vector<glm::vec3> &normals)
{
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(path, 0 /*aiProcess_JoinIdenticalVertices | aiProcess_SortByPType*/);

    if (!scene)
    {
        fprintf(stderr, importer.GetErrorString());
        getchar();
        return false;
    }

    const aiMesh *mesh = scene->mMeshes[0]; // In this simple example code we always use the 1rst mesh (in OBJ files there is often only one anyway)

    // Fill vertices positions
    vertices.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        aiVector3D pos = mesh->mVertices[i];
        vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
    }

    // Fill vertices texture coordinates
    uvs.resize(mesh->mNumVertices);

    // for(unsigned int i=0; i<mesh->mNumVertices; i++){

    //     aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.

    //     uvs.push_back(glm::vec2(UVW.x, UVW.y));
    // }

    // Fill vertices normals
    normals.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        aiVector3D n = mesh->mNormals[i];
        normals.push_back(glm::vec3(n.x, n.y, n.z));
    }

    // Fill face indices
    indices.reserve(3 * mesh->mNumFaces);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        // Assume the model has only triangles.
        indices.push_back(mesh->mFaces[i].mIndices[0]);
        indices.push_back(mesh->mFaces[i].mIndices[1]);
        indices.push_back(mesh->mFaces[i].mIndices[2]);
    }

    // The "scene" pointer will be deleted automatically by "importer"

    return true;
}

bool loadModel(const char *path,
               std::vector<unsigned short> &indices,
               std::vector<glm::vec3> &vertices,
               std::vector<glm::vec3> &normals,
               std::vector<glm::vec2> &uvs)
{
    std::vector<glm::vec3> Intermediairevertices;
    std::vector<glm::vec3> Intermediairenormals;
    std::vector<glm::vec2> Intermediaireuvs;
    if (loadOBJ(path, Intermediairevertices, Intermediaireuvs, Intermediairenormals))
        indexVBO(Intermediairevertices, Intermediaireuvs, Intermediairenormals, indices, vertices, uvs, normals);

    return true;
}

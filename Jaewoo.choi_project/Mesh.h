/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Mesh.h
Purpose: <This file contains the declaration of struct Mesh,vertex,
functionality required to load shader source; compile shader source; link
shader objects into a program object, validate program object; log msesages
from compiling linking, and validation steps; install program object in
appropriate shaders; and pass uniform variables from the client to the
program object.>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64 bit>
Project: <jaewoo.choi_CS350_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 04/11/2022
End Header --------------------------------------------------------*/
#pragma once
#include <vector>
#include <GL/glew.h> 
#include"math.h"
#include"glslshader.h"
#include "Camera.h"
#include "shader.h"

#define MAX_BONE_INFLUENCE 4

struct Global
{
    glm::vec3 attenuation = { 1.f,0.220f,2.0f };
    glm::vec3 fogColor = { 0.5f,0.5f, 0.5f};
    glm::vec3 ambient = { 0.0f,0.0f, 0.0f};
    GLfloat fogMin = 0.1f;
    GLfloat fogMax = 20.f;
};

struct Material
{
    glm::vec3 ambient = { 0,0,0 };
    glm::vec3 diffuse = { 1.f,1.f,1.f };
    glm::vec3 specular = { 1.f,1.f,1.f };
    glm::vec3 emissive = { 0.f,0.f,0.f };

};

struct DirLight
{
    glm::vec3 direction = { 1.f,1.f,1.f };
    glm::vec3 ambient =  { 0.05f, 0.05f, 0.05f };
    glm::vec3 diffuse =  { 0.8f, 0.8f, 0.8f };
    glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
    glm::vec3 position = { 0.f,1.f,1.f };

    GLfloat innerAngle = 15.f;
    GLfloat outerAngle = 20.f;
    GLfloat falloff = 1.f;

    GLfloat cutoff = 0.f;
    GLfloat outerCutOff = 0.f;

    //GLfloat constant = 0.f;
    //GLfloat linear = 0.f;
    //GLfloat quadratic = 0.f;

    GLfloat shininess = 2.f;
    int type = 0;
    //point
    //dir
    //spot
};

struct lampSet {
    glm::vec4 Ambient = { 1.f,1.f,1.f,1.f };
    glm::vec4 Diffuse = { 1.f,1.f,1.f,1.f };
    glm::vec4 Specular = { 1.f,1.f,1.f,1.f };
};

/*  Mesh function(s) */
enum MeshID { BUNNY, SPHERE, SPHERE4, CUBE2, SPHERE_MODIFIED, NUM_MESHES };
enum Primitive { TRIANGLE, LINE, FACE, NUM_MESH };
struct Vertex
{
   // Vertex(glm::vec3 pos, glm::vec3 nrm, glm::vec3 uv) : pos(pos), nrm(nrm), uv(uv) {}

    Vertex() { }

    glm::vec3 pos{ 0 }, nrm{ 0 };
    glm::vec2 uv{ 0 };
    glm::vec3 color{ 0 };

    glm::vec3 Tangent{ 0 };
    // bitangent
    glm::vec3 Bitangent{ 0 };
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE]{ 0 };
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE]{ 0 };
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

const int vertexSize = sizeof(Vertex);
const int indexSize = sizeof(int);


/*  The layouts for specifying the offsets of a vertex
    when it is copied into the graphics pipeline. */
struct VertexLayout
{
    int location;
    int size;
    int type;
    bool normalized;
    int offset;
};

const VertexLayout vLayout[] =
{
    { 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos) },
    { 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, nrm) },
    { 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv) },
    {3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, color) }

};

struct MinMax
{
    glm::vec3 min = { 0,0,0 };
    glm::vec3 max = { 0,0,0 };
};

struct LengthMinMax
{
    float lenX = 0;
    float lenY = 0;
    float lenZ = 0;
};


const int layoutSize = sizeof(VertexLayout);
const int numAttribs = sizeof(vLayout) / layoutSize;    // for now numAttribs = 1: only pos


typedef std::vector<Vertex> VertexBufferType;
typedef std::vector<int> IndexBufferType;
typedef std::vector<glm::vec3> FaceBufferType;

/*  Mesh format */
struct Mesh
{
    // mesh Data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;
    GLuint VAO;
    int color = 0;

    // constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    Mesh() : numVertices(0), numTris(0), numIndices(0), position{ 0 }, scale{ 1 }, rotation{ 0 }, VBO{ 0 }, VAO{ 0 }
    { }


    GLuint VAOL;
    GLuint VAOFL;
    GLuint VBO;
    GLuint EBO;
    GLuint VBOL;
    GLuint VBOFL;
    GLuint IBO;
    GLuint UBO;
    GLuint FBO;
    GLuint RBO;
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nrm));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        // ids
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
    GLuint ProgramID;
    GLuint SkyProgramID;
    /*  Storing the actual vertex/index data */
    VertexBufferType vertexBuffer;
    IndexBufferType indexBuffer;
    VertexBufferType faceBuffer;
    VertexBufferType vertexBufferForVertexNrm;
    VertexBufferType vertexBufferForFaceNrm;
    

    int meshtype = -1;

    int numVertices=0;
    int numTris=0;
    int numIndices=0;
    int numVerticesLine=0;
    int numVerticesFaceLine=0;
    


    GLSLShader renderProg{ GLSLShader() };

    GLuint diffuseMap, specularMap, emissionMap;
    GLint offset[2];
    GLint blockSize;
    GLint blockSize1;
    GLubyte* blockBuffer;
    GLint modelLoc = 0;
    GLint viewLoc = 0;
    GLint projectionLoc = 0;
    GLint LightLoc = 0; 
    GLint colorLoc = 0;
    GLint ViewPosLoc = 0;
    GLint TriOrLine = 0;

    GLint MatricesLOC;
    GLint DirLightLOC = 0;


    GLint diffuseLoc = 0;
    GLint specularLoc = 0;
    GLint shininessLoc = 0;
    GLuint texture = 0;

    GLenum DrawBuffers[6] = { GL_COLOR_ATTACHMENT0 };
    GLuint numBuffers = 6;
    std::vector<std::string> faces
    {
            "../textures/right.jpg",
            "../textures/left.jpg",
            "../textures/top.jpg",
            "../textures/bottom.jpg",
            "../textures/front.jpg",
            "../textures/back.jpg"
    };
    std::vector<glm::vec2> uvPlanar;
    std::vector<glm::vec2> uvCyndrical;
    std::vector<glm::vec2> uvSpherical;
    std::vector<glm::vec2> uvcube;


    glm::vec3 position{ 0,0,0 };
    glm::vec3 scale = { 1,1,1 };
    glm::vec3 rotation{ 0,0,0 };

    GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);
    GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path, const char* geometry_file_path);
    void init(const char* vertex_file_path, const char* fragment_file_path,  glm::vec3 Pos = { 0,0,0 }, glm::vec3 Scale = { 1,1,1 }, glm::vec3 Rotate = { 0,0,0 }, int numlamp=16);
    void initFrame(const char* vertex_file_path, const char* fragment_file_path, const char* geometry_file_path,  glm::vec3 Pos = { 0,0,0 }, glm::vec3 Scale = { 1,1,1 }, glm::vec3 Rotate = { 0,0,0 });
    void initSkyBox(const char* vertex_file_path, const char* fragment_file_path, const char* geometry_file_path,  glm::vec3 Pos = { 0,0,0 }, glm::vec3 Scale = { 1,1,1 }, glm::vec3 Rotate = { 0,0,0 });
    void initLight(const char* vertex_file_path, const char* fragment_file_path, glm::vec3 Pos = { 0,0,0 }, glm::vec3 Scale = { 1,1,1 }, glm::vec3 Rotate = { 0,0,0 }, bool obj = true);
    void initLamp(const char* vertex_file_path, const char* fragment_file_path, glm::vec3 Pos = { 0,0,0 }, glm::vec3 Scale = { 1,1,1 }, glm::vec3 Rotate = { 0,0,0 });
    void initLine(const char* vertex_file_path, const char* fragment_file_path, glm::vec3 Pos = { 0,0,0 }, glm::vec3 Scale = { 1,1,1 }, glm::vec3 Rotate = { 0,0,0 });
    void initOrbit(const char* vertex_file_path, const char* fragment_file_path, glm::vec3 Pos = { 0,0,0 }, glm::vec3 Scale = { 1,1,1 }, glm::vec3 Rotate = { 0,0,0 });
    
    
    std::vector<GLuint> cubemapTextureEnvironment;
    std::vector<GLuint> loadCubemap(std::vector<std::string> faces);

    void setTexture();
    void setTextureGrid();
    GLfloat* readPPM(const char* ppm_file_path, int& width, int& height);
    void SendVertexData();
    void SendVertexDataLight();
    void SendVertexDataForLine();
    void SendVertexDataForFaceLine();
    void SendVertexDataLamp();
    void setup_shdrpgm(std::string shader);
    void setup_mesh();
    void compute_matrix(float delta_time);
    void Draw(Shader& shader);
    void DrawLineBVH(Shader& shader);
    void DrawLine(Shader& shader);
    void drawSphere(Shader& shader);
    void drawLamp(glm::mat4 view, glm::mat4 projection, int numberLamp, std::vector<DirLight> dl, std::vector<lampSet> lampSetting);
    void drawLampForFrame(int numberLamp, std::vector<DirLight> dl);
    void drawLight(glm::mat4 view, glm::mat4 projection, glm::vec3 light_pos, glm::vec3 view_pos, std::vector<DirLight> dl,int numlamp, Global global, Material mater,int typeMapping, int shaderType, GLuint A, float B, GLuint C);
    void drawSkyBox(glm::mat4 view, glm::mat4 projection, Camera cam);
    void drawSkyBoxForFrame(glm::mat4 view, glm::mat4 projection, Camera cam);
    void drawFrame(glm::mat4 view, glm::mat4 projection, Camera cam, glm::vec3 view_pos, std::vector<DirLight> dl,int numlamp, Global global, Material mater,int typeMapping, int shaderType, GLuint A, float B, GLuint C);
    void drawLine(glm::vec3 color, glm::mat4 view, glm::mat4 projection, glm::vec3 light_pos, glm::vec3 view_pos, glm::vec2 type_);
    void drawOrbit(glm::vec3 color, glm::mat4 view, glm::mat4 projection, glm::vec3 light_pos, glm::vec3 view_pos, glm::vec2 type_);
    void drawFaceLine(glm::vec3 color, glm::mat4 view, glm::mat4 projection, glm::vec3 light_pos, glm::vec3 view_pos, glm::vec2 type_);
    
    void set_position(glm::vec3 pos)
    {
        position = pos;
        update_flag = true;
    }
    void set_rotation(glm::vec3 angle)
    {
        rotation = angle;
        update_flag = true;
    }
    glm::vec3& Get_position() { return position; }
    glm::vec3& Get_scale() { return scale; }
    glm::vec3& Get_Rotation() { return rotation; }

    int* get_stack_slice() { return stack_slice; }
    int stack_slice[2] = { 0 };
    bool update_flag = true;
    Camera camera;

    
    glm::mat4 projection = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
    };
    glm::mat4 view = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
    };
};

Mesh LoadOBJ(const char* path);
Mesh ReverseLoadOBJ(const char* path);
void ReadOBJ(const char* path, Mesh& mesh, MinMax& m, LengthMinMax& lm);
void ReverseReadOBJ(const char* path, Mesh& mesh, MinMax& m, LengthMinMax& lm);
Mesh CalculateMesh(Mesh& mesh, MinMax& m, LengthMinMax& lm);
Mesh CreatePlane(int stacks, int slices);
Mesh CreateCube(int stacks, int slices, glm::vec3 center, MinMax m);
Mesh CreateCube(glm::vec3 center, MinMax m, int color);
Mesh CreateSphere(int stacks, int slices, glm::vec3 center, float radius);
Mesh CreateSphere(int stacks, int slices, glm::vec3 center, float radius, int color);
Mesh CreateTorus(int stacks, int slices, float startAngle, float endAngle);
Mesh CreateCylinder(int stacks, int slices);
Mesh CreateCone(int stacks, int slices);
Mesh CreateOrbit(int num);

void BuildIndexBuffer(int stacks, int slices, Mesh& mesh);
void addVertex(Mesh& mesh, const Vertex& v);
void addIndex(Mesh& mesh, int index);

void CalculateMinMax(Vertex v, MinMax& m);
void MoveToOrigin(Mesh& mesh,glm::vec3& origin, MinMax& m);
/******************************************************************************/
/*  Pre-defined shapes                                                        */
/******************************************************************************/



#pragma once

#ifndef MODEL_H
#define MODEL_H

#include "glhelper.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.h"
#include "Mesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

static unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

struct BoundingBox {
    glm::vec3 center;
    MinMax m;
    BoundingBox() {}
    BoundingBox(glm::vec3 center, MinMax m_)
        : center(center), m(m_) {}

    bool intersectsWith(const BoundingBox& other) const {
        if (m.max.x < other.m.min.x || other.m.max.x < m.min.x) return false;
        if (m.max.y < other.m.min.y || other.m.max.y < m.min.y) return false;
        if (m.max.z < other.m.min.z || other.m.max.z < m.min.z) return false;
        return true;
    }

    double getVolume() const {
        return (m.max.x - m.min.x) * (m.max.y - m.min.y) * (m.max.z - m.min.z);
    }

    BoundingBox mergeWith(const BoundingBox& other) const {
        glm::vec3 new_center;
        MinMax new_m;
        new_m.max.x = this->m.max.x >= other.m.max.x ? this->m.max.x : other.m.max.x;
        new_m.max.y = this->m.max.y >= other.m.max.y ? this->m.max.y : other.m.max.y;
        new_m.max.z = this->m.max.z >= other.m.max.z ? this->m.max.z : other.m.max.z;

        new_m.min.x = this->m.min.x <= other.m.min.x ? this->m.min.x : other.m.min.x;
        new_m.min.y = this->m.min.y <= other.m.min.y ? this->m.min.y : other.m.min.y;
        new_m.min.z = this->m.min.z <= other.m.min.z ? this->m.min.z : other.m.min.z;

        new_center = (new_m.max + new_m.min)/2.f;

        return BoundingBox(new_center, new_m);
    }
};

struct BoundingSphere {
    glm::vec3 center;
    double radius;
    BoundingSphere(glm::vec3 center, double radius_)
        : center(center), radius(radius_) {}

    
    bool containsPoint(glm::vec3 center_) const {
        double dx = this->center.x - center_.x;
        double dy = this->center.y - center_.y;
        double dz = this->center.z - center_.z;
        return dx * dx + dy * dy + dz * dz <= radius * radius;
    }

    bool intersects(const BoundingSphere& other) const {
        double dx = this->center.x - other.center.x;
        double dy = this->center.y - other.center.y;
        double dz = this->center.z - other.center.z;
        double distance = sqrt(dx * dx + dy * dy + dz * dz);
        return distance < this->radius + other.radius;
    }

    //BoundingSphere mergeWith(const BoundingSphere& other) const {
    //    double dx = this->x - other.x;
    //    double dy = this->y - other.y;
    //    double dz = this->z - other.z;
    //    double distance = sqrt(dx * dx + dy * dy + dz * dz);
    //    if (distance >= this->radius + other.radius) {
    //        if (this->radius > other.radius) {
    //            return *this;
    //        }
    //        else {
    //            return other;
    //        }
    //    }
    //    double new_radius = (this->radius + other.radius + distance) / 2.0;
    //    double ratio = (new_radius - this->radius) / distance;
    //    double new_x = this->x + dx * ratio;
    //    double new_y = this->y + dy * ratio;
    //    double new_z = this->z + dz * ratio;
    //    return BoundingSphere(new_x, new_y, new_z, new_radius);
    //}
};

struct BVHNode {
    BoundingBox bbox;
    BVHNode* left_child = nullptr;
    BVHNode* right_child = nullptr;
    int level = 0;
    int object_id = -1;
    BVHNode() {}
    int getlevel() const {
        if (isLeaf()) {
            return 0;
        }
        else {
            return 1 + std::max(left_child->getlevel(), right_child->getlevel());
        }
    }
    BVHNode(BoundingBox bbox, BVHNode* left_child, BVHNode* right_child, int object_id)
        : bbox(bbox), left_child(left_child), right_child(right_child), object_id(object_id) {}

    static int isNoLeaf(vector<BVHNode*> nodes, int size) {
        int remain = 0;
        for (int i = 0; i < size; i++)
        {
            if (nodes[i] != nullptr)
                remain++;
        }
        return remain;
    }

    bool isLeaf() const {
        if (object_id != -1)
            return true;
        return false;
    }

    static BVHNode* buildBVH(const vector<BoundingBox>& object_bboxes, vector<int>& object_ids,int type);
    static BVHNode* buildBVHTopDown(const vector<BoundingBox>& object_bboxes, vector<int>& object_ids,int type);


    void deleteBVH() {
        if (isLeaf()) {
            delete this;
            return;
        }
        left_child->deleteBVH();
        right_child->deleteBVH();
        delete this;
    }

    int intersectsWith(const BoundingBox& ray, double& t_min) const {
        if (!bbox.intersectsWith(ray)) return -1;
        if (isLeaf()) return object_id;
        double left_t_min = numeric_limits<double>::max(), right_t_min = numeric_limits<double>::max();
        int left_id = left_child->intersectsWith(ray, left_t_min);
        int right_id = right_child->intersectsWith(ray, right_t_min);
        if (left_id != -1 && left_t_min < t_min) {
            t_min = left_t_min;
            return left_id;
        }
        if (right_id != -1 && right_t_min < t_min) {
            t_min = right_t_min;
            return right_id;
        }
        return -1;
    }
};

class Model
{
public:
    // model data 
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>    meshes;
    string directory;
    bool isline = false;
    glm::vec3 center{};
    MinMax m;
    float CentroidRadius = 0;
    float ritterRadius = 0;
    float LarssonsRadius = 0;
    float PCARadius = 0;
    glm::mat3 C{};
    // constructor, expects a filepath to a 3D model.
    Model() {}
    Model(string const& path, bool line = false) : isline(line)
    {
        loadModel(path, line);
    }

    // draws the model, and thus all its meshes
    void Draw(Shader& shader, bool line = false);

    void FindNearestNeighbour(std::vector<Model> models);
private:
    LengthMinMax lm;
    std::vector<glm::vec3> PCA(aiMesh* mesh, glm::vec3 optimalDirection)
    {
        std::vector<glm::vec3> XYZ_MINMAX;
        XYZ_MINMAX.resize(6);
        float XminProj = FLT_MAX;
        float YminProj = FLT_MAX;
        float ZminProj = FLT_MAX;
        float XmaxProj = -FLT_MAX;
        float YmaxProj = -FLT_MAX;
        float ZmaxProj = -FLT_MAX;
        glm::vec3 Min{};
        glm::vec3 Max{};
        glm::vec3 v{};
        glm::vec3 Xaxis = { optimalDirection.x,0,0 };
        glm::vec3 Yaxis = { 0,optimalDirection.y,0 };
        glm::vec3 Zaxis = { 0,0,optimalDirection.z };

        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            v = glm::vec3{ mesh->mVertices[i].x ,mesh->mVertices[i].y,mesh->mVertices[i].z };
            float XprojDist = glm::dot(Xaxis, v);
            float YprojDist = glm::dot(Yaxis, v);
            float ZprojDist = glm::dot(Zaxis, v);
            if (XminProj > XprojDist)
            {
                XminProj = XprojDist;
                XYZ_MINMAX[0] = v;
            }
            if (XmaxProj <= XprojDist)
            {
                XmaxProj = XprojDist;
                XYZ_MINMAX[1] = v;
            }

            if (YminProj > YprojDist)
            {
                YminProj = YprojDist;
                XYZ_MINMAX[2] = v;
            }
            if (YmaxProj <= YprojDist)
            {
                YmaxProj = YprojDist;
                XYZ_MINMAX[3] = v;
            }

            if (ZminProj > ZprojDist)
            {
                ZminProj = ZprojDist;
                XYZ_MINMAX[4] = v;
            }
            if (ZmaxProj <= ZprojDist)
            {
                ZmaxProj = ZprojDist;
                XYZ_MINMAX[5] = v;
            }
        }
        return XYZ_MINMAX;
    }
    std::vector<glm::vec3> Ritter(aiMesh* mesh)
    {
        std::vector<glm::vec3> XYZ_MINMAX;
        XYZ_MINMAX.resize(6);
        float XminProj = FLT_MAX;
        float YminProj = FLT_MAX;
        float ZminProj = FLT_MAX;
        float XmaxProj = -FLT_MAX;
        float YmaxProj = -FLT_MAX;
        float ZmaxProj = -FLT_MAX;
        glm::vec3 Min{};
        glm::vec3 Max{};
        glm::vec3 v{};
        glm::vec3 Xaxis = { 1,0,0 };
        glm::vec3 Yaxis = { 0,1,0 };
        glm::vec3 Zaxis = { 0,0,1 };

        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            v = glm::vec3{ mesh->mVertices[i].x ,mesh->mVertices[i].y,mesh->mVertices[i].z };
            float XprojDist = glm::dot(Xaxis, v);
            float YprojDist = glm::dot(Yaxis, v);
            float ZprojDist = glm::dot(Zaxis, v);
            if (XminProj > XprojDist)
            {
                XminProj = XprojDist;
                XYZ_MINMAX[0] = v;
            }
            if (XmaxProj <= XprojDist)
            {
                XmaxProj = XprojDist;
                XYZ_MINMAX[1] = v;
            }

            if (YminProj > YprojDist)
            {
                YminProj = YprojDist;
                XYZ_MINMAX[2] = v;
            }
            if (YmaxProj <= YprojDist)
            {
                YmaxProj = YprojDist;
                XYZ_MINMAX[3] = v;
            }

            if (ZminProj > ZprojDist)
            {
                ZminProj = ZprojDist;
                XYZ_MINMAX[4] = v;
            }
            if (ZmaxProj <= ZprojDist)
            {
                ZmaxProj = ZprojDist;
                XYZ_MINMAX[5] = v;
            }
        }
        return XYZ_MINMAX;
    }
    std::vector<glm::vec3> Larssons(aiMesh* mesh)
    {
        std::vector<glm::vec3> XYZ_MINMAX;
        XYZ_MINMAX.resize(6);
        float XminProj = FLT_MAX;
        float YminProj = FLT_MAX;
        float ZminProj = FLT_MAX;
        float XmaxProj = -FLT_MAX;
        float YmaxProj = -FLT_MAX;
        float ZmaxProj = -FLT_MAX;
        glm::vec3 Min{};
        glm::vec3 Max{};
        glm::vec3 v{};
        glm::vec3 Xaxis = { 1,0,0 };
        glm::vec3 Yaxis = { 0,1,0 };
        glm::vec3 Zaxis = { 0,0,1 };
        for (int j = 0; j < 2; j++)
        {
            if (j == 0)
            {
                Xaxis = { 1,1,0 };
                Yaxis = { 0,0,1 };
                Zaxis = { 0,1,1 };
            }
            else
            {
                Xaxis = { 1,-1,0 };
                Yaxis = { 1,0,-1 };
                Zaxis = { 0,1,-1 };
            }

            for (int i = 0; i < mesh->mNumVertices; i++)
            {
                v = glm::vec3{ mesh->mVertices[i].x ,mesh->mVertices[i].y,mesh->mVertices[i].z };
                float XprojDist = glm::dot(Xaxis, v);
                float YprojDist = glm::dot(Yaxis, v);
                float ZprojDist = glm::dot(Zaxis, v);
                if (XminProj > XprojDist)
                {
                    XminProj = XprojDist;
                    XYZ_MINMAX[0] = v;
                }
                if (XmaxProj <= XprojDist)
                {
                    XmaxProj = XprojDist;
                    XYZ_MINMAX[1] = v;
                }

                if (YminProj > YprojDist)
                {
                    YminProj = YprojDist;
                    XYZ_MINMAX[2] = v;
                }
                if (YmaxProj <= YprojDist)
                {
                    YmaxProj = YprojDist;
                    XYZ_MINMAX[3] = v;
                }

                if (ZminProj > ZprojDist)
                {
                    ZminProj = ZprojDist;
                    XYZ_MINMAX[4] = v;
                }
                if (ZmaxProj <= ZprojDist)
                {
                    ZmaxProj = ZprojDist;
                    XYZ_MINMAX[5] = v;
                }
            }
        }
        return XYZ_MINMAX;
    }
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path, bool isline)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene, isline);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene, bool isline)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            if (isline == false)
                meshes.push_back(processMesh(mesh, scene));
            else
                meshes.push_back(processMeshLine(mesh, scene));

        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene, isline);

        }

    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.

            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.nrm = vector;
            }

            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.pos = vector;

            if (vertex.pos.x < m.min.x)
                m.min.x = vertex.pos.x;
            if (vertex.pos.y < m.min.y)
                m.min.y = vertex.pos.y;
            if (vertex.pos.z < m.min.z)
                m.min.z = vertex.pos.z;
            if (vertex.pos.x > m.max.x)
                m.max.x = vertex.pos.x;
            if (vertex.pos.y > m.max.y)
                m.max.y = vertex.pos.y;
            if (vertex.pos.z > m.max.z)
                m.max.z = vertex.pos.z;


            // texture coordinates

            glm::vec2 UV = glm::vec2(0, 0);
            glm::vec3 absVector = abs(vector);
            //+-X
            if (absVector.x >= absVector.y && absVector.x >= absVector.z)
            {
                if (vector.x > 0.0f)
                {
                    UV.x = vector.z / absVector.x;
                }
                else
                {
                    UV.x = -vector.z / absVector.x;
                }
                //pos.y
                UV.y = vector.y / absVector.x;
            }
            //+-Y
            else if (absVector.y >= absVector.x && absVector.y >= absVector.z)
            {
                if (vector.y < 0.0f)
                {
                    UV.y = vector.z / absVector.y;
                }
                else
                {
                    UV.y = -vector.z / absVector.y;
                }
                UV.x = -vector.x / absVector.y;
            }
            //+-Z
            else if (absVector.z >= absVector.x && absVector.z >= absVector.y)
            {
                if (vector.z < 0.0f) {
                    UV.x = vector.x / absVector.z;
                }
                else {
                    UV.x = -vector.x / absVector.z;
                }
                UV.y = vector.y / absVector.z;
            }
            vertex.uv = glm::vec2{ (UV + glm::vec2(1)).x / 2.0, (UV + glm::vec2(1)).y / 2.0 };

            //if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            //{
            //    glm::vec2 vec;
            //    // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            //    // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            //    vec.x = mesh->mTextureCoords[0][i].x;
            //    vec.y = mesh->mTextureCoords[0][i].y;
            //    vertex.uv = vec;
            //    // tangent
            //    vector.x = mesh->mTangents[i].x;
            //    vector.y = mesh->mTangents[i].y;
            //    vector.z = mesh->mTangents[i].z;
            //    vertex.Tangent = vector;
            //    // bitangent
            //    vector.x = mesh->mBitangents[i].x;
            //    vector.y = mesh->mBitangents[i].y;
            //    vector.z = mesh->mBitangents[i].z;
            //    vertex.Bitangent = vector;
            //}
            //else
            //    vertex.uv = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        //lm.lenX = m.max.x - m.min.x;
        //lm.lenY = m.max.y - m.min.y;
        //lm.lenZ = m.max.z - m.min.z;


        float res = 0;
        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            center.x += vertices[i].pos.x;
            center.y += vertices[i].pos.y;
            center.z += vertices[i].pos.z;
        }
        center = center / static_cast<float>(mesh->mNumVertices);
        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            C[0].x += (vertices[i].pos.x - center.x) * (vertices[i].pos.x - center.x);
            C[0].y += (vertices[i].pos.x - center.x) * (vertices[i].pos.y - center.y);
            C[0].z += (vertices[i].pos.x - center.x) * (vertices[i].pos.z - center.z);

            //C[1].x += (vertices[i].pos.y - center.y) * (vertices[i].pos.x - center.x);
            C[1].y += (vertices[i].pos.y - center.y) * (vertices[i].pos.y - center.y);
            C[1].z += (vertices[i].pos.y - center.y) * (vertices[i].pos.z - center.z);

            //C[2].x += (vertices[i].pos.z - center.z) * (vertices[i].pos.x - center.x);
            //C[2].y += (vertices[i].pos.z - center.z) * (vertices[i].pos.y - center.y);
            C[2].z += (vertices[i].pos.z - center.z) * (vertices[i].pos.z - center.z);

        }
        C[1].x = C[0].y;
        C[2].x = C[0].z;
        C[2].y = C[1].z;

        C[0] /= static_cast<float>(mesh->mNumVertices);
        C[1] /= static_cast<float>(mesh->mNumVertices);
        C[2] /= static_cast<float>(mesh->mNumVertices);
        glm::mat3 J{};
        glm::mat3 J_{};
        glm::mat3 Aprime{};
        glm::mat3 V{ 1,0,0,0,1,0,0,0,1 };
        glm::vec3 Vdirection{};
        int p = 0;
        int q = 2;
        double Beta = (C[q][q] - C[p][p]) / (2 * C[p][q]);
        double t_ = (glm::sign(Beta)) / (abs(Beta) + sqrt(Beta * Beta + 1));
        double cos_ = (1.f) / (sqrt(t_ * t_ + 1));
        double sin_ = cos_ * t_;
        J[0] = { cos_,sin_,0 };
        J[1] = { -sin_,cos_,0 };
        J[2] = { 0,0,1 };
        J_[0] = { cos_,-sin_,0 };
        J_[1] = { sin_,cos_,0 };
        J_[2] = { 0,0,1 };
        Aprime = J_ * C * J;
        V = V * J;
        if (Aprime[0][0] > Aprime[1][1])
        {

            if (Aprime[0][0] > Aprime[2][2])
                Vdirection = V[0];
            if (Aprime[2][2] > Aprime[0][0])
                Vdirection = V[2];
        }
        else if (Aprime[0][0] < Aprime[1][1])
        {
            if (Aprime[1][1] > Aprime[2][2])
                Vdirection = V[1];
            if (Aprime[2][2] > Aprime[1][1])
                Vdirection = V[2];
        }


        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            res = glm::length(vertices[i].pos - center);
            //vertices[i].pos.x -= center.x;
            //vertices[i].pos.y -= center.y;
            //vertices[i].pos.z -= center.z;
            if (CentroidRadius < res)
                CentroidRadius = res;
        }

        //m.min -= center;
        //m.max -= center;

        std::vector<glm::vec3> xyz_minmax{};

        for (int i = 0; i < 3; i++)
        {
            if (i == 0)
                xyz_minmax = Ritter(mesh);
            else if (i == 1)
                xyz_minmax = Larssons(mesh);
            else
                xyz_minmax = PCA(mesh, Vdirection);

            float resPair = 0;
            float Xpair = glm::length(xyz_minmax[0] - xyz_minmax[1]);
            float Ypair = glm::length(xyz_minmax[2] - xyz_minmax[3]);
            float Zpair = glm::length(xyz_minmax[4] - xyz_minmax[5]);
            resPair = Xpair > Ypair ? Xpair : Ypair;
            resPair = resPair > Zpair ? resPair : Zpair;
            resPair /= 2.f;
            glm::vec3 newPoint{};
            glm::vec3 newCenter{};
            bool isgrow = false;
            float newRadius = 0;
            for (int i = 0; i < mesh->mNumVertices; i++)
            {
                if (glm::length(vertices[i].pos - center) > resPair)
                {
                    newPoint = vertices[i].pos;
                    isgrow = true;
                }
            }
            if (i == 0)
            {
                if (isgrow)
                {
                    glm::vec3 dHat = Normalize(newPoint - center);
                    glm::vec3 pPrime = center - resPair * dHat;
                    newRadius = glm::length(newPoint - pPrime) * 0.5f;
                    newCenter = center + (newRadius - resPair) * dHat;
                    center = newCenter;
                    ritterRadius = newRadius;
                }
                else
                    ritterRadius = resPair;
            }
            else if (i == 1)
            {
                if (isgrow)
                {
                    glm::vec3 dHat = Normalize(newPoint - center);
                    glm::vec3 pPrime = center - resPair * dHat;
                    newRadius = glm::length(newPoint - pPrime) * 0.5f;
                    newCenter = center + (newRadius - resPair) * dHat;
                    center = newCenter;
                    LarssonsRadius = newRadius;
                }
                else
                    LarssonsRadius = resPair;
            }
            else
            {
                if (isgrow)
                {
                    glm::vec3 dHat = Normalize(newPoint - center);
                    glm::vec3 pPrime = center - resPair * dHat;
                    newRadius = glm::length(newPoint - pPrime) * 0.5f;
                    newCenter = center + (newRadius - resPair) * dHat;
                    center = newCenter;
                    PCARadius = newRadius;
                }
                else
                    PCARadius = resPair;
            }
        }


        // walk through each of the mesh's vertices
        //for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        //{
        //    // positions

        //    vertices[i].pos.x = 2 * ((vertices[i].pos.x - m.min.x) / lm.lenX) - 1.f;
        //    vertices[i].pos.y = 2 * ((vertices[i].pos.y - m.min.y) / lm.lenY) - 1.f;
        //    vertices[i].pos.z = 2 * ((vertices[i].pos.z - m.min.z) / lm.lenZ) - 1.f;
        //    if (lm.lenZ == 0)
        //        vertices[i].pos.z = 0;
        //}


        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        //// 3. normal maps
        //std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        //textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        //// 4. height maps
        //std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        //textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    Mesh processMeshLine(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        vector<Vertex> tmpvertices;
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;
        m.min = glm::vec3{ FLT_MAX };
        m.max = glm::vec3{ -FLT_MAX };

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.

            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.nrm = vector;
            }

            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.pos = vector;

            if (vertex.pos.x < m.min.x)
                m.min.x = vertex.pos.x;
            if (vertex.pos.y < m.min.y)
                m.min.y = vertex.pos.y;
            if (vertex.pos.z < m.min.z)
                m.min.z = vertex.pos.z;

            if (vertex.pos.x > m.max.x)
                m.max.x = vertex.pos.x;
            if (vertex.pos.y > m.max.y)
                m.max.y = vertex.pos.y;
            if (vertex.pos.z > m.max.z)
                m.max.z = vertex.pos.z;


            // texture coordinates

            glm::vec2 UV = glm::vec2(0, 0);
            glm::vec3 absVector = abs(vector);
            //+-X
            if (absVector.x >= absVector.y && absVector.x >= absVector.z)
            {
                if (vector.x > 0.0f)
                {
                    UV.x = vector.z / absVector.x;
                }
                else
                {
                    UV.x = -vector.z / absVector.x;
                }
                //pos.y
                UV.y = vector.y / absVector.x;
            }
            //+-Y
            else if (absVector.y >= absVector.x && absVector.y >= absVector.z)
            {
                if (vector.y < 0.0f)
                {
                    UV.y = vector.z / absVector.y;
                }
                else
                {
                    UV.y = -vector.z / absVector.y;
                }
                UV.x = -vector.x / absVector.y;
            }
            //+-Z
            else if (absVector.z >= absVector.x && absVector.z >= absVector.y)
            {
                if (vector.z < 0.0f) {
                    UV.x = vector.x / absVector.z;
                }
                else {
                    UV.x = -vector.x / absVector.z;
                }
                UV.y = vector.y / absVector.z;
            }
            vertex.uv = glm::vec2{ (UV + glm::vec2(1)).x / 2.0, (UV + glm::vec2(1)).y / 2.0 };

            tmpvertices.push_back(vertex);
        }

        //lm.lenX = m.max.x - m.min.x;
        //lm.lenY = m.max.y - m.min.y;
        //lm.lenZ = m.max.z - m.min.z;

        glm::vec3 center_ = { 0,0,0 };

        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            center_.x += tmpvertices[i].pos.x;
            center_.y += tmpvertices[i].pos.y;
            center_.z += tmpvertices[i].pos.z;
        }
        center_ = center_ / static_cast<float>(mesh->mNumVertices);

        //for (int i = 0; i < mesh->mNumVertices; i++)
        //{
        //    tmpvertices[i].pos.x -= center_.x;
        //    tmpvertices[i].pos.y -= center_.y;
        //    tmpvertices[i].pos.z -= center_.z;
        //}

        //m.min -= center_;
        //m.max -= center_;
        vertices.resize(17);
        vertices[0].pos = { m.min.x ,m.min.y ,m.min.z };
        vertices[1].pos = { m.min.x ,m.min.y ,m.max.z };
        vertices[2].pos = { m.min.x ,m.max.y ,m.max.z };
        vertices[3].pos = { m.min.x ,m.max.y ,m.min.z };
        vertices[4].pos = { m.min.x ,m.min.y ,m.min.z };

        vertices[5].pos = { m.max.x ,m.min.y ,m.min.z };
        vertices[6].pos = { m.max.x ,m.max.y ,m.min.z };
        vertices[7].pos = { m.min.x ,m.max.y ,m.min.z };
        vertices[8].pos = { m.min.x ,m.max.y ,m.max.z };
        vertices[9].pos = { m.max.x ,m.max.y ,m.max.z };

        vertices[10].pos = { m.max.x ,m.max.y ,m.min.z };
        vertices[11].pos = { m.max.x ,m.min.y ,m.min.z };
        vertices[12].pos = { m.max.x ,m.min.y ,m.max.z };
        vertices[13].pos = { m.max.x ,m.max.y ,m.max.z };

        vertices[14].pos = { m.max.x ,m.min.y ,m.max.z };
        vertices[15].pos = { m.min.x ,m.min.y ,m.max.z };
        vertices[16].pos = { m.min.x ,m.min.y ,m.min.z };

        for (int i = 0; i < 17; i++)
        {
            center += vertices[i].pos;
        }
        center /= 17.f;
        // walk through each of the mesh's vertices
        //for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        //{
        //    // positions

        //    tmpvertices[i].pos.x = 2 * ((tmpvertices[i].pos.x - m.min.x) / lm.lenX) - 1.f;
        //    tmpvertices[i].pos.y = 2 * ((tmpvertices[i].pos.y - m.min.y) / lm.lenY) - 1.f;
        //    tmpvertices[i].pos.z = 2 * ((tmpvertices[i].pos.z - m.min.z) / lm.lenZ) - 1.f;
        //    if (lm.lenZ == 0)
        //        tmpvertices[i].pos.z = 0;
        //}


        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < 17; i++)
        {
            //aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            //for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(i);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        //// 3. normal maps
        //std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        //textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        //// 4. height maps
        //std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        //textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }
};



unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = 0;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
#endif
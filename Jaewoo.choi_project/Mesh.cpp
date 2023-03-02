/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Mesh.cpp
Purpose: <This file contains the definition of struct Mesh,vertex,
functionality required to load shader source; compile shader source; link
shader objects into a program object, validate program object; log msesages
from compiling linking, and validation steps; install program object in
appropriate shaders; and pass uniform variables from the client to the
program object.>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64 bit>
Project: <jaewoo.choi_CS350_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>, LoadShaders()some function from file of the professor
Creation date: 04/11/2022
End Header --------------------------------------------------------*/
#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "Mesh.h"
#include "glhelper.h"
#include "stb_image.h"
/*  Function prototype(s) */

Mesh CreatePlane(int stacks, int slices)
{
    Mesh mesh;
    mesh.stack_slice[0] = stacks;
    mesh.stack_slice[1] = slices;
    for (int stack = 0; stack <= stacks; ++stack)
    {
        float row = (float)stack / stacks;

        for (int slice = 0; slice <= slices; ++slice)
        {
            float col = (float)slice / slices;

            Vertex v;

            v.pos = glm::vec3(col - 0.5f, row - 0.5f, 0.0f);
            v.nrm = glm::vec3(0.0f, 0.0f, 1.0f);
            v.uv = glm::vec2(col, row);

            addVertex(mesh, v);
        }
    }

    BuildIndexBuffer(stacks, slices, mesh);

    return mesh;
}

Mesh CreateCube(int stacks, int slices)
{
    Mesh planeMesh = CreatePlane(stacks, slices);
    Mesh mesh;
    mesh.stack_slice[0] = stacks;
    mesh.stack_slice[1] = slices;
    Vec3 const translateArray[] =
    {
        Vec3(+0.0f, +0.0f, +0.5f), // Z+
        Vec3(+0.0f, +0.0f, -0.5f), // Z-
        Vec3(+0.5f, +0.0f, +0.0f), // X+
        Vec3(-0.5f, +0.0f, +0.0f), // X-
        Vec3(+0.0f, +0.5f, +0.0f), // Y+
        Vec3(+0.0f, -0.5f, +0.0f), // Y-
    };

    Vec2 const rotateArray[] =
    {
        Vec2(+0.0f, +0.0f),             // Z+
        Vec2(+0.0f, (float)+PI),        // Z-
        Vec2(+0.0f, (float)+HALF_PI),   // X+       
        Vec2(+0.0f, (float)-HALF_PI),   // X-
        Vec2((float)-HALF_PI, +0.0f),   // Y+
        Vec2((float)+HALF_PI, +0.0f)    // Y-
    };


    /*  Transform the plane to 6 positions to form the faces of the cube */
    for (int i = 0; i < 6; ++i)
    {
        Mat4 transformMat = Translate(translateArray[i]) * Rotate(rotateArray[i][YINDEX], { 0,1,0 }) * Rotate(rotateArray[i][XINDEX], { 1,0,0 });

        for (int j = 0; j < planeMesh.numVertices; ++j)
        {
            Vertex v;
            v.pos = Vec3(transformMat * Vec4(planeMesh.vertexBuffer[j].pos, 1.0));
            v.nrm = Vec3(transformMat * Vec4(planeMesh.vertexBuffer[j].nrm, 1.0));
            v.uv = planeMesh.vertexBuffer[j].uv;

            v.pos = RoundDecimal(v.pos);
            v.nrm = RoundDecimal(v.nrm);

            addVertex(mesh, v);
        }

        for (int j = 0; j < planeMesh.numIndices; ++j)
            addIndex(mesh, planeMesh.indexBuffer[j] + planeMesh.numVertices * i);
    }

    return mesh;
}

Mesh CreateSphere(int stacks, int slices)
{
    Mesh mesh;
    mesh.stack_slice[0] = stacks;
    mesh.stack_slice[1] = slices;
    for (int stack = 0; stack <= stacks; ++stack)
    {
        float row = (float)stack / stacks;
        float beta = PI * (row - 0.5f);

        for (int slice = 0; slice <= slices; ++slice)
        {
            float col = (float)slice / slices;
            float alpha = col * PI * 2.0f;
            Vertex v;
            v.uv.x = col;
            v.uv.y = row * (-1.0f);

            v.pos.x = 0.5f * sin(alpha) * cos(beta);
            v.pos.y = 0.5f * sin(beta);
            v.pos.z = 0.5f * cos(alpha) * cos(beta);

            v.nrm.x = v.pos.x;
            v.nrm.y = v.pos.y;
            v.nrm.z = v.pos.z;

            v.nrm /= 0.5;

            addVertex(mesh, v);
        }
    }

    BuildIndexBuffer(stacks, slices, mesh);

    return mesh;
}

Mesh CreateCylinder(int stacks, int slices)
{
    Mesh mesh;
    Vertex vertex;
    mesh.stack_slice[0] = stacks;
    mesh.stack_slice[1] = slices;
    /// For the body

    for (int i = 0; i <= stacks; i++)
    {
        float row = static_cast<float>(i) / stacks;

        for (int j = 0; j <= slices; j++)
        {
            float col = static_cast<float>(j) / slices;
            float alpha = col * 2.0f * PI;
            float sinAlpha = sin(alpha);
            float cosAlpha = cos(alpha);

            vertex.uv.x = row;
            vertex.uv.y = col;

            vertex.pos.x = 0.5f * sinAlpha;
            vertex.pos.y = row - 0.5f;
            vertex.pos.z = 0.5f * cosAlpha;

            vertex.nrm.x = vertex.pos.x / 0.5f;
            vertex.nrm.y = 0;
            vertex.nrm.z = vertex.pos.z / 0.5f;

            addVertex(mesh, vertex);
        }
    }
    BuildIndexBuffer(stacks, slices, mesh);

    int vertex_size = static_cast<int>(mesh.vertexBuffer.size());

    /// For the caps

    for (int i = 0; i <= stacks; i++)
    {
        if (i == 0)
        {
            vertex.pos = Vec3(0.0, 0.5f, 0.0);
            vertex.nrm.x = vertex.pos.x / 0.5f;
            vertex.nrm.y = vertex.pos.y / 0.5f;
            vertex.nrm.z = vertex.pos.z / 0.5f;
        }
        else if (i == 1)
        {
            vertex.pos = Vec3(0.0, -0.5f, 0.0);
            vertex.nrm.x = vertex.pos.x / 0.5f;
            vertex.nrm.y = vertex.pos.y / 0.5f;
            vertex.nrm.z = vertex.pos.z / 0.5f;
        }

        addVertex(mesh, vertex);

        float row = static_cast<float>(i) / stacks;

        for (int j = 0; j <= slices; j++)
        {
            float col = static_cast<float>(j) / slices;
            float alpha = col * 2.0f * PI;
            float sinAlpha = sin(alpha);
            float cosAlpha = cos(alpha);

            vertex.uv.x = row;
            vertex.uv.y = col;

            if (i == 0)
            {
                vertex.pos = Vec3(0.5 * sinAlpha, 0.5f, 0.5f * cosAlpha);

                vertex.nrm.x = vertex.pos.x / 0.5f;
                vertex.nrm.y = vertex.pos.y / 0.5f;
                vertex.nrm.z = vertex.pos.z / 0.5f;
            }
            else if (i == 1)
            {
                vertex.pos = Vec3(0.5f * sinAlpha, -0.5f, 0.5f * cosAlpha);

                vertex.nrm.x = vertex.pos.x / 0.5f;
                vertex.nrm.y = vertex.pos.y / 0.5f;
                vertex.nrm.z = vertex.pos.z / 0.5f;
            }

            addVertex(mesh, vertex);
        }
    }

    for (int i = 0; i <= stacks; i++)
    {
        vertex_size += (i * (slices + 2));

        for (int j = 1; j < slices; j++)
        {
            addIndex(mesh, vertex_size);
            addIndex(mesh, vertex_size + j);
            addIndex(mesh, vertex_size + j + 1);
        }
        addIndex(mesh, vertex_size);
        addIndex(mesh, vertex_size + slices);
        addIndex(mesh, vertex_size + 1);
    }

    return mesh;
}

Mesh CreateTorus(int stacks, int slices, float startAngle, float endAngle)
{
    Mesh mesh;
    mesh.stack_slice[0] = stacks;
    mesh.stack_slice[1] = slices;
    for (int stack = 0; stack <= stacks; ++stack)
    {
        float row = (float)stack / stacks; // [0.0, 1.0]

        float alpha = PI + startAngle + (row * (endAngle - startAngle)); // [0.0, 2¥ð]

        float sinAlpha = sin(alpha);
        float  cosAlpha = cos(alpha);

        for (int slice = 0; slice <= slices; ++slice)
        {
            float col = (float)slice / slices;

            float beta = col * PI * 2.0f;

            Vertex center;
            center.pos = Vec3(0, 0, 0);

            Vertex v;
            v.uv.x = col;
            v.uv.y = row;

            v.pos.x = -(0.35f + (0.15f * cos(beta))) * sinAlpha;
            v.pos.y = -0.15f * sin(beta);
            v.pos.z = -(0.35f + (0.15f * cos(beta))) * cosAlpha;

            v.nrm = v.pos - center.pos;

            v.nrm /= 0.15f;
            addVertex(mesh, v);
        }

    }

    BuildIndexBuffer(stacks, slices, mesh);
    return mesh;
}

Mesh CreateCone(int stacks, int slices)
{
    Mesh mesh;
    Vertex v;
    Mesh top;
    //Mesh bottom;
    mesh.stack_slice[0] = stacks;
    mesh.stack_slice[1] = slices;
    float col = 0;
    float alpha = 0;
    for (int stack = 0; stack <= stacks; ++stack)
    {
        float row = ((float)stack / stacks) - 0.5f; // [0.0, 1.0]

        for (int slice = 0; slice <= slices; ++slice)
        {
            col = (float)slice / slices;
            alpha = col * PI * 2.0f;

            // side
            v.uv.x = row;
            v.uv.y = col;

            v.pos.x = 0.5f * (0.5f - row) * sin(alpha);
            v.pos.y = row;
            v.pos.z = 0.5f * (0.5f - row) * cos(alpha);


            v.nrm.x = v.pos.x / 0.5f;
            v.nrm.y = 0.0;
            v.nrm.z = v.pos.z / 0.5f;

            addVertex(mesh, v);
        }
    }
    BuildIndexBuffer(stacks, slices, mesh);


    return  mesh;
}

Mesh CreateOrbit(int num)
{
    Mesh mesh;
    Vertex v;
    float radius = 1.5;
    float theta = TWO_PI / (float)num;
    for (int i = 0; i <= num; i++)
    {
        v.pos = { radius * cos(theta * i),0,radius * sin(theta * i) };
        mesh.vertexBufferForVertexNrm.push_back(v);
        if (i != 0 && i != num) {
            mesh.vertexBufferForVertexNrm.push_back(v);
            ++mesh.numVerticesLine;
        }
        ++mesh.numVerticesLine;
    }

    return mesh;
}


void BuildIndexBuffer(int stacks, int slices, Mesh& mesh)
{
    //@todo: IMPLEMENT ME
    int p0 = 0, p1 = 0, p2 = 0;
    int p3 = 0, p4 = 0, p5 = 0;

    int stride = slices + 1;
    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            int current_row = i * stride;
            /*  You need to compute the indices for the first triangle here */
            /*  ... */
            p0 = current_row + j;
            p1 = p0 + 1;
            p2 = p1 + stride;
            // triangle <p3, p4, p5>
            /*  Ignore degenerate triangle */
            if (!DegenerateTri(mesh.vertexBuffer[p0].pos,
                mesh.vertexBuffer[p1].pos,
                mesh.vertexBuffer[p2].pos))
            {
                /*  Add the indices for the first triangle */
                addIndex(mesh, p0);
                addIndex(mesh, p1);
                addIndex(mesh, p2);
            }


            /*  You need to compute the indices for the second triangle here */
            /*  ... */
            p3 = p2;
            p4 = p3 - 1;
            p5 = p0;
            /*  Ignore degenerate triangle */
            if (!DegenerateTri(mesh.vertexBuffer[p3].pos,
                mesh.vertexBuffer[p4].pos,
                mesh.vertexBuffer[p5].pos))
            {
                /*  Add the indices for the second triangle */
                addIndex(mesh, p3);
                addIndex(mesh, p4);
                addIndex(mesh, p5);
            }
        }
    }
}

void addVertex(Mesh& mesh, const Vertex& v)
{
    mesh.vertexBuffer.push_back(v);
    ++mesh.numVertices;
}

void addIndex(Mesh& mesh, int index)
{
    mesh.indexBuffer.push_back(index);
    ++mesh.numIndices;

    if (mesh.numIndices % 3 == 0)
        ++mesh.numTris;
}



void Mesh::setup_shdrpgm(std::string shader)
{
    std::string vert = "../shaders/";
    std::string frag = "../shaders/";
    vert = vert + shader + ".vert";
    frag = frag + shader + ".frag";

    std::vector<std::pair<GLenum, std::string>> shdr_files;
    shdr_files.push_back(std::make_pair(GL_VERTEX_SHADER, vert));
    shdr_files.push_back(std::make_pair(GL_FRAGMENT_SHADER, frag));
    renderProg.CompileLinkValidate(shdr_files);
    if (GL_FALSE == renderProg.IsLinked())
    {
        std::cout << "Unable to compile/link/validate shader programs" << "\n";
        std::cout << renderProg.GetLog() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void Mesh::init(const char* vertex_file_path, const char* fragment_file_path, glm::vec3 Pos, glm::vec3 Scale, glm::vec3 Rotate, int numlamp)
{

    position = Pos;
    scale = Scale;
    rotation = Rotate;
    stbi_set_flip_vertically_on_load(true);
    LoadShaders(vertex_file_path, fragment_file_path);


    glUseProgram(ProgramID);
    //setTexture();


    glUniform1i(glGetUniformLocation(ProgramID, "NUMBER_OF_POINT_LIGHTS"), numlamp);



  
    colorLoc = glGetUniformLocation(ProgramID, "color");

    diffuseLoc = glGetUniformLocation(ProgramID, "diffuse");
    specularLoc = glGetUniformLocation(ProgramID, "specular");
    shininessLoc = glGetUniformLocation(ProgramID, "shininess");


    MatricesLOC = glGetUniformBlockIndex(ProgramID, "Matrices");
    glUniformBlockBinding(ProgramID, MatricesLOC, 0);//
    glGetActiveUniformBlockiv(ProgramID, MatricesLOC, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    SendVertexData();


}


void Mesh::initSkyBox(const char* vertex_file_path, const char* fragment_file_path, const char* geometry_file_path, glm::vec3 Pos, glm::vec3 Scale, glm::vec3 Rotate)
{
    //init Frame buffer

    position = Pos;
    scale = Scale;
    rotation = Rotate;

    stbi_set_flip_vertically_on_load(true);

    SkyProgramID = LoadShaders(vertex_file_path, fragment_file_path);
    glUseProgram(SkyProgramID);

    cubemapTextureEnvironment.resize(faces.size());

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        glGenTextures(1, &cubemapTextureEnvironment[i]);
        glBindTexture(GL_TEXTURE_2D, cubemapTextureEnvironment[i]);
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);//window width,height 1600, 1000
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID[i], 0);
    }
    GLint a = glGetUniformLocation(SkyProgramID, "material.right");
    glUniform1i(a, 0);
    glUniform1i(glGetUniformLocation(SkyProgramID, "material.left"), 1);
    glUniform1i(glGetUniformLocation(SkyProgramID, "material.top"), 2);
    glUniform1i(glGetUniformLocation(SkyProgramID, "material.bottom"), 3);
    glUniform1i(glGetUniformLocation(SkyProgramID, "material.front"), 4);
    glUniform1i(glGetUniformLocation(SkyProgramID, "material.back"), 5);

    MatricesLOC = glGetUniformBlockIndex(SkyProgramID, "Matrices");
    glUniformBlockBinding(SkyProgramID, MatricesLOC, 0);//
    glGetActiveUniformBlockiv(SkyProgramID, MatricesLOC, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    SendVertexData();


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

}


void Mesh::initLamp(const char* vertex_file_path, const char* fragment_file_path, glm::vec3 Pos, glm::vec3 Scale, glm::vec3 Rotate)
{

    camera = { { 0.f, 0.f, 10.0f } };
    position = Pos;
    scale = glm::vec3{ 0.2f, 0.2f, 0.2f } ;
    rotation = Rotate;

    LoadShaders(vertex_file_path, fragment_file_path);
    glUseProgram(renderProg.GetHandle());

    modelLoc = glGetUniformLocation(renderProg.GetHandle(), "model");
    viewLoc = glGetUniformLocation(renderProg.GetHandle(), "view");
    projectionLoc = glGetUniformLocation(renderProg.GetHandle(), "projection");
    //TODO:outer


    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glm::mat4 model = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
    };

    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    SendVertexDataLamp();
}

void Mesh::initLine(const char* vertex_file_path, const char* fragment_file_path, glm::vec3 Pos, glm::vec3 Scale, glm::vec3 Rotate)
{
    camera = { { 0.f, 0.f, 10.0f } };
    position = Pos;
    scale = Scale;
    rotation = Rotate;

    LoadShaders(vertex_file_path, fragment_file_path);

    glUseProgram(renderProg.GetHandle());

    /*  Obtain the locations of the variables in the shaders with the given names */
    modelLoc = glGetUniformLocation(renderProg.GetHandle(), "model");
    viewLoc = glGetUniformLocation(renderProg.GetHandle(), "view");
    colorLoc = glGetUniformLocation(renderProg.GetHandle(), "color");
    projectionLoc = glGetUniformLocation(renderProg.GetHandle(), "projection");
    LightLoc = glGetUniformLocation(renderProg.GetHandle(), "lightPos");
    ViewPosLoc = glGetUniformLocation(renderProg.GetHandle(), "viewPos");

    SendVertexDataForLine();
    SendVertexDataForFaceLine();
    /*  Bind framebuffer to 0 to render to the screen (by default already 0) */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /*  Initially drawing using filled mode */

    /*  Hidden surface removal */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void Mesh::initOrbit(const char* vertex_file_path, const char* fragment_file_path, glm::vec3 Pos, glm::vec3 Scale, glm::vec3 Rotate)
{
    camera = { { 0.f, 0.f, 10.0f } };
    position = Pos;
    scale = Scale;
    rotation = Rotate;

    LoadShaders(vertex_file_path, fragment_file_path);

    glUseProgram(renderProg.GetHandle());

    /*  Obtain the locations of the variables in the shaders with the given names */
    modelLoc = glGetUniformLocation(renderProg.GetHandle(), "model");
    viewLoc = glGetUniformLocation(renderProg.GetHandle(), "view");
    colorLoc = glGetUniformLocation(renderProg.GetHandle(), "color");
    projectionLoc = glGetUniformLocation(renderProg.GetHandle(), "projection");
    LightLoc = glGetUniformLocation(renderProg.GetHandle(), "lightPos");
    ViewPosLoc = glGetUniformLocation(renderProg.GetHandle(), "viewPos");

    SendVertexDataForLine();
    /*  Bind framebuffer to 0 to render to the screen (by default already 0) */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /*  Initially drawing using filled mode */

    /*  Hidden surface removal */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

GLuint Mesh::LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::string Line;
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else {
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
        //getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::string Line;
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex Shader
    //printf("Compiling shader : %s\n", vertex_file_path);
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    int InfoLogLengthPlusOne = 0;
    if (InfoLogLength > 0) {
        InfoLogLengthPlusOne = InfoLogLength + 1;
        std::vector<char> VertexShaderErrorMessage(InfoLogLengthPlusOne);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }


    // Compile Fragment Shader
    //printf("Compiling shader : %s\n", fragment_file_path);
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        InfoLogLengthPlusOne = InfoLogLength + 1;
        std::vector<char> FragmentShaderErrorMessage(InfoLogLengthPlusOne);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }


    // Link the program
    //printf("Linking program\n");
    ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        InfoLogLengthPlusOne = InfoLogLength + 1;
        std::vector<char> ProgramErrorMessage(InfoLogLengthPlusOne);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    std::vector<std::pair<GLenum, std::string>> shdr_files;
    shdr_files.push_back(std::make_pair(GL_VERTEX_SHADER, vertex_file_path));
    shdr_files.push_back(std::make_pair(GL_FRAGMENT_SHADER, fragment_file_path));
    renderProg.CompileLinkValidate(shdr_files);
    if (GL_FALSE == renderProg.IsLinked())
    {
        std::cout << "Unable to compile/link/validate shader programs" << "\n";
        std::cout << renderProg.GetLog() << std::endl;
        std::exit(EXIT_FAILURE);
    }



    return ProgramID;
}

GLuint Mesh::LoadShaders(const char* vertex_file_path, const char* fragment_file_path, const char* geometry_file_path)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::string Line;
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else {
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
        //getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::string Line;
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex Shader
    //printf("Compiling shader : %s\n", vertex_file_path);
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    int InfoLogLengthPlusOne = 0;
    if (InfoLogLength > 0) {
        InfoLogLengthPlusOne = InfoLogLength + 1;
        std::vector<char> VertexShaderErrorMessage(InfoLogLengthPlusOne);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }


    // Compile Fragment Shader
    //printf("Compiling shader : %s\n", fragment_file_path);
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        InfoLogLengthPlusOne = InfoLogLength + 1;
        std::vector<char> FragmentShaderErrorMessage(InfoLogLengthPlusOne);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }


    // Link the program
    //printf("Linking program\n");
    ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        InfoLogLengthPlusOne = InfoLogLength + 1;
        std::vector<char> ProgramErrorMessage(InfoLogLengthPlusOne);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    std::vector<std::pair<GLenum, std::string>> shdr_files;
    shdr_files.push_back(std::make_pair(GL_VERTEX_SHADER, vertex_file_path));
    shdr_files.push_back(std::make_pair(GL_FRAGMENT_SHADER, fragment_file_path));
    renderProg.CompileLinkValidate(shdr_files);
    if (GL_FALSE == renderProg.IsLinked())
    {
        std::cout << "Unable to compile/link/validate shader programs" << "\n";
        std::cout << renderProg.GetLog() << std::endl;
        std::exit(EXIT_FAILURE);
    }



    return ProgramID;
}

void Mesh::Draw(Shader& shader)
{
    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;

    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++); // transfer unsigned int to string

        // now set the sampler to the correct texture unit
        GLint Loc = glGetUniformLocation(shader.ID, (name).c_str());
        glUniform1i(Loc, i);
        // and finally bind the texture
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}


void Mesh::draw(glm::mat4 view, glm::mat4 projection, glm::vec3 light_pos, glm::vec3 view_pos, std::vector<DirLight> dl, int numlamp,
    Global global, Material mater, std::vector<GLuint> cubemapTexture, bool reflect, bool refract, int indexFresnel,
    std::vector<float> refractiveIndex, std::vector<float> fresnelC, float FresnelPower)
{
    glUseProgram(ProgramID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    
    glDepthMask(GL_TRUE);
    glm::mat4 model = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
    };
    
    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    glBindBuffer(GL_UNIFORM_BUFFER, UBO);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));

    glBindBufferBase(GL_UNIFORM_BUFFER, MatricesLOC, UBO);


    glBindVertexArray(VAO);

    glUniform4fv(colorLoc, 1, ValuePtr(glm::vec3{ 1.f,1.f,1.f }));//


    glUniform1i(glGetUniformLocation(ProgramID, "NUMBER_OF_POINT_LIGHTS"), numlamp);
    glUniform1f(glGetUniformLocation(ProgramID, "Reflection_bool"), reflect);
    glUniform1f(glGetUniformLocation(ProgramID, "Refraction_bool"), refract);
    glUniform3f(glGetUniformLocation(ProgramID, "viewPos"), view_pos.x, view_pos.y, view_pos.z);


    glUniform1f(glGetUniformLocation(ProgramID, "refractiveIndex"), refractiveIndex[indexFresnel]);
    glUniform1f(glGetUniformLocation(ProgramID, "fresnelC"), fresnelC[indexFresnel]);
    glUniform1f(glGetUniformLocation(ProgramID, "FresnelPower"), FresnelPower);
    
    glUniform3f(glGetUniformLocation(ProgramID, "attenuation"), global.attenuation.x, global.attenuation.y, global.attenuation.z);
    glUniform3f(glGetUniformLocation(ProgramID, "fogColor"), global.fogColor.x, global.fogColor.y, global.fogColor.z);
    glUniform3f(glGetUniformLocation(ProgramID, "globalAmbient"), global.ambient.x, global.ambient.y, global.ambient.z);
    glUniform1f(glGetUniformLocation(ProgramID, "fogMin"), global.fogMin);
    glUniform1f(glGetUniformLocation(ProgramID, "fogMax"), global.fogMax);

    //glUniform3f(glGetUniformLocation(renderProg.GetHandle(), "material.diffuse"), mater.ambient.x, mater.ambient.y, mater.ambient.z);
    //glUniform3f(glGetUniformLocation(renderProg.GetHandle(), "material.specular"), mater.specular.x, mater.specular.y, mater.specular.z);
    glUniform1f(glGetUniformLocation(ProgramID, "material.shininess"), 32.f);
    //glUniform3f(glGetUniformLocation(renderProg.GetHandle(), "material.emissive"), mater.ambient.x, mater.ambient.y, mater.ambient.z);

    glUniform3f(glGetUniformLocation(ProgramID, "material.diffuse"), mater.diffuse.x, mater.diffuse.y, mater.diffuse.z);
    glUniform3f(glGetUniformLocation(ProgramID, "material.specular"), mater.specular.x, mater.specular.y, mater.specular.z);
    glUniform3f(glGetUniformLocation(ProgramID, "material.ambient"), mater.ambient.x, mater.ambient.y, mater.ambient.z);
    glUniform3f(glGetUniformLocation(ProgramID, "material.emissive"), mater.emissive.x, mater.emissive.y, mater.emissive.z);

    //TODO
   GLint a = glGetUniformLocation(ProgramID, "material.right");
   glUniform1i(a, 0);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, cubemapTexture[0]);
   glUniform1i(glGetUniformLocation(ProgramID, "material.left"), 1);
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, cubemapTexture[1]);
   glUniform1i(glGetUniformLocation(ProgramID, "material.top"), 2);
   glActiveTexture(GL_TEXTURE2);
   glBindTexture(GL_TEXTURE_2D, cubemapTexture[2]);
   glUniform1i(glGetUniformLocation(ProgramID, "material.bottom"), 3);
   glActiveTexture(GL_TEXTURE3);
   glBindTexture(GL_TEXTURE_2D, cubemapTexture[3]);
   glUniform1i(glGetUniformLocation(ProgramID, "material.front"), 4);
   glActiveTexture(GL_TEXTURE4);
   glBindTexture(GL_TEXTURE_2D, cubemapTexture[4]);
   glUniform1i(glGetUniformLocation(ProgramID, "material.back"), 5);
   glActiveTexture(GL_TEXTURE5);
   glBindTexture(GL_TEXTURE_2D, cubemapTexture[5]);



    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}

void Mesh::drawLight(glm::mat4 view, glm::mat4 projection, glm::vec3 light_pos, glm::vec3 view_pos, std::vector<DirLight> dl, int numlamp, Global global, Material mater, int typeMapping, int shaderType, GLuint A, float B, GLuint C)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(ProgramID);


    glm::mat4 model = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
    };

    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));




    glBindBuffer(GL_UNIFORM_BUFFER, UBO);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));

    glBindBufferBase(GL_UNIFORM_BUFFER, MatricesLOC, UBO);


    glBindVertexArray(VAO);

    glUniform4fv(colorLoc, 1, ValuePtr(glm::vec3{ 1.f,1.f,1.f }));//
    glm::vec3 attenuation = { 1.f,0.220f,0.2f };
    glm::vec3 fogColor = { 0.f,1.f,1.f };

    GLfloat fogMin = 0.1f;
    GLfloat fogMax = 20.f;
    //glUniform3fv(LightLoc, 1, ValuePtr(light_pos));//
    glUniform3fv(ViewPosLoc, 1, ValuePtr(view_pos));//



    glUniform1i(glGetUniformLocation(ProgramID, "NUMBER_OF_POINT_LIGHTS"), numlamp);
    glUniform1i(glGetUniformLocation(ProgramID, "typeMapping"), typeMapping);
    glUniform3f(glGetUniformLocation(ProgramID, "attenuation"), global.attenuation.x, global.attenuation.y, global.attenuation.z);
    glUniform3f(glGetUniformLocation(ProgramID, "fogColor"), global.fogColor.x, global.fogColor.y, global.fogColor.z);
    glUniform3f(glGetUniformLocation(ProgramID, "globalAmbient"), global.ambient.x, global.ambient.y, global.ambient.z);
    glUniform1f(glGetUniformLocation(ProgramID, "fogMin"), global.fogMin);
    glUniform1f(glGetUniformLocation(ProgramID, "fogMax"), global.fogMax);
    glUniform1f(glGetUniformLocation(ProgramID, "material.shininess"), 32.f);
    glUniform3f(glGetUniformLocation(ProgramID, "material.diffuse"), mater.diffuse.x, mater.diffuse.y, mater.diffuse.z);
    glUniform3f(glGetUniformLocation(ProgramID, "material.specular"), mater.specular.x, mater.specular.y, mater.specular.z);
    glUniform3f(glGetUniformLocation(ProgramID, "material.ambient"), mater.ambient.x, mater.ambient.y, mater.ambient.z);
    glUniform3f(glGetUniformLocation(ProgramID, "material.emissive"), mater.emissive.x, mater.emissive.y, mater.emissive.z);

    char buf[3] = {};
    std::string tmpA = "pointLights[";
    std::string tmpB;
    std::string pos = tmpB + "].position";
    std::string dir = tmpB + "].direction";
    std::string cut = tmpB + "].cutOff";
    std::string out = tmpB + "].outerCutOff";
    std::string amb = tmpB + "].ambient";
    std::string dif = tmpB + "].diffuse";
    std::string spe = tmpB + "].specular";
    std::string con = tmpB + "].constant";
    std::string lin = tmpB + "].linear";
    std::string qua = tmpB + "].quadratic";
    std::string typ = tmpB + "].type";
    std::string inA = tmpB + "].innerAngle";
    std::string outA = tmpB + "].outerAngle";
    std::string fall = tmpB + "].falloff";

    for (int i = 0; i < numlamp; i++)
    {
        _itoa(i, buf, 10);
        tmpB = tmpA + buf;
        pos = tmpB + "].position";
        dir = tmpB + "].direction";
        amb = tmpB + "].ambient";
        dif = tmpB + "].diffuse";
        spe = tmpB + "].specular";
        con = tmpB + "].attenuation";
        cut = tmpB + "].cutOff";
        out = tmpB + "].outerCutOff";
        typ = tmpB + "].type";
        inA = tmpB + "].innerAngle";
        outA = tmpB + "].outerAngle";
        fall = tmpB + "].falloff";


        auto test = glGetUniformLocation(ProgramID, dif.c_str());
        glUniform3f(glGetUniformLocation(ProgramID, pos.c_str()), dl[i].position.x, dl[i].position.y, dl[i].position.z);
        glUniform3f(glGetUniformLocation(ProgramID, dir.c_str()), dl[i].direction.x, dl[i].direction.y, dl[i].direction.z);
        glUniform3f(glGetUniformLocation(ProgramID, amb.c_str()), dl[i].ambient.x, dl[i].ambient.y, dl[i].ambient.z);
        glUniform3f(glGetUniformLocation(ProgramID, dif.c_str()), dl[i].diffuse.x, dl[i].diffuse.y, dl[i].diffuse.z);
        glUniform3f(glGetUniformLocation(ProgramID, spe.c_str()), dl[i].specular.x, dl[i].specular.y, dl[i].specular.z);
        glUniform1f(glGetUniformLocation(ProgramID, cut.c_str()), dl[i].cutoff);
        glUniform1f(glGetUniformLocation(ProgramID, out.c_str()), dl[i].outerCutOff);
        glUniform1i(glGetUniformLocation(ProgramID, typ.c_str()), dl[i].type);
        glUniform1f(glGetUniformLocation(ProgramID, inA.c_str()), dl[i].innerAngle);
        glUniform1f(glGetUniformLocation(ProgramID, outA.c_str()), dl[i].outerAngle);
        glUniform1f(glGetUniformLocation(ProgramID, fall.c_str()), dl[i].falloff);
    }



    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
    //glDepthFunc(GL_LESS);

    glBindVertexArray(0);
    //glBindBuffer(GL_DRAW_FRAMEBUFFER, 0);
    //glDrawBuffer(GL_BACK_LEFT);
}

void Mesh::drawFrame(glm::mat4 view, glm::mat4 projection, Camera cam, glm::vec3 view_pos, std::vector<DirLight> dl, int numlamp, Global global, Material mater, int typeMapping, int shaderType, GLuint A, float B, GLuint C)
{
    //glEnable(GL_DEPTH_TEST);
    //

    //glm::mat4 model = glm::mat4(1.0f);
    //glm::mat4 _view = glm::mat4(1.0f);
    //glm::mat4 _projection = glm::mat4(1.0f);
    //glm::mat4 tmp = glm::mat4(1.0f);

    ////TODO
    //glBindVertexArray(VAO);
    //glm::mat4 skyBoxTransform[6];
    //skyBoxTransform[0] = (glm::perspective(glm::radians(90.0f), 1.f, 0.1f, 100.f) * glm::lookAt(glm::vec3(0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));//right
    //skyBoxTransform[1] = (glm::perspective(glm::radians(90.0f), 1.f, 0.1f, 100.f) * glm::lookAt(glm::vec3(0), glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));//left
    //skyBoxTransform[2] = (glm::perspective(glm::radians(90.0f), 1.f, 0.1f, 100.f) * glm::lookAt(glm::vec3(0), glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));//bottom
    //skyBoxTransform[3] = (glm::perspective(glm::radians(90.0f), 1.f, 0.1f, 100.f) * glm::lookAt(glm::vec3(0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));//top
    //skyBoxTransform[4] = (glm::perspective(glm::radians(90.0f), 1.f, 0.1f, 100.f) * glm::lookAt(glm::vec3(0), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0)));//back
    //skyBoxTransform[5] = (glm::perspective(glm::radians(90.0f), 1.f, 0.1f, 100.f) * glm::lookAt(glm::vec3(0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0)));//front
    //for (int i = 0; i < 6; i++)
    //{
    //    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    //    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cubemapTexture[i], 0);
    //    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //    

    //    model = glm::mat4(1.0f);
    //    

    //    cam.Update(0);
    //    if (i == 0)
    //        cam.yaw += 90.0f;
    //    else if (i == 1)
    //        cam.yaw += 90.0f;
    //    else if (i == 2)
    //        cam.yaw += 90.0f;
    //    else if (i == 3)
    //        cam.pitch += 90.0f;
    //    else if (i == 4)
    //        cam.pitch += 180.0f;
    //    else if (i == 5)
    //    {
    //        cam.pitch += 90.0f;
    //        cam.yaw += 90.0f;
    //        cam.Update(0);
    //    }
    //    _projection = skyBoxTransform[i];
    //    _view = cam.GetViewMatrix();

    //    tmp = glm::mat4(1);

    //    glBindBuffer(GL_UNIFORM_BUFFER, UBO);

    //    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &tmp[0].x);
    //    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(_projection));
    //    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));

    //    glBindBufferBase(GL_UNIFORM_BUFFER, MatricesLOC, UBO);

    //    drawSkyBoxForFrame();
    //    drawLampForFrame(numlamp,dl);

    //   // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //}

}

void Mesh::drawSkyBox(glm::mat4 view, glm::mat4 projection, Camera cam)
{
    // second render pass: draw as normal
    // ----------------------------------

    glUseProgram(SkyProgramID);


    glm::mat4 model = {
       1,0,0,0,
       0,1,0,0,
       0,0,1,0,
       0,0,0,1
    };

    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, { 1.f,1.f,1.f });


    // cubes
    
    glBindVertexArray(VAO);
    for (int i = 0; i < faces.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, cubemapTextureEnvironment[i]);
        //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cubemapTexture[i], 0);
    }


    glBindBuffer(GL_UNIFORM_BUFFER, UBO);

    glm::mat4 tmp = glm::mat4(glm::mat3(view));
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &tmp[0].x);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));

    glBindBufferBase(GL_UNIFORM_BUFFER, MatricesLOC, UBO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);



}

void Mesh::drawSkyBoxForFrame(glm::mat4 view, glm::mat4 projection, Camera cam)
{
    // second render pass: draw as normal
    // ----------------------------------

    glDisable(GL_DEPTH_TEST);
    glUseProgram(SkyProgramID);


    glm::mat4 model = {
       1,0,0,0,
       0,1,0,0,
       0,0,1,0,
       0,0,0,1
    };

    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, { 1.f,1.f,1.f });


    // cubes

    glBindVertexArray(VAO);
    for (int i = 0; i < faces.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, cubemapTextureEnvironment[i]);
        //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cubemapTexture[i], 0);
    }


    glBindBuffer(GL_UNIFORM_BUFFER, UBO);

    glm::mat4 tmp = glm::mat4(glm::mat3(view));
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &tmp[0].x);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));

    glBindBufferBase(GL_UNIFORM_BUFFER, MatricesLOC, UBO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);



}

void Mesh::drawLine(glm::vec3 color, glm::mat4 view, glm::mat4 projection, glm::vec3 light_pos, glm::vec3 view_pos, glm::vec2 type_)
{
    glUseProgram(renderProg.GetHandle());
    glm::mat4 model = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
    model = glm::translate(model, glm::vec3{ 0,0.1,0 });
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(45.0f), glm::vec3(0.0f, 0.5f, 0.0f));
    model = glm::scale(model, glm::vec3{1.f,1.f,1.f});
    //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 0.5f, 0.0f));



    glBindVertexArray(VAOL);
    glUniform4fv(colorLoc, 1, ValuePtr(color));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(LightLoc, 1, ValuePtr(light_pos));
    glUniform3fv(ViewPosLoc, 1, ValuePtr(view_pos));
    glUniform2fv(TriOrLine, 1, ValuePtr(type_));

    //glDrawElements(GL_LINES, numIndicesLine, GL_UNSIGNED_INT, nullptr);
    //glLineWidth(1.f);
    //glVertexAttrib3f(0.1f, 0.1f, 0.99f, 1.f); // blue color for lines
    glDrawArrays(GL_LINES, 0, numVerticesLine);
    glBindVertexArray(0);

}

void Mesh::drawOrbit(glm::vec3 color, glm::mat4 view, glm::mat4 projection, glm::vec3 light_pos, glm::vec3 view_pos, glm::vec2 type_)
{
    glUseProgram(renderProg.GetHandle());
    glm::mat4 model = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
    model = glm::translate(model, glm::vec3{ 0,0,0 });
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(45.0f), glm::vec3(0.0f, 0.5f, 0.0f));
    model = glm::scale(model, glm::vec3{ 1,1,1 });
    //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 0.5f, 0.0f));



    glBindVertexArray(VAOL);
    glUniform4fv(colorLoc, 1, ValuePtr(color));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(LightLoc, 1, ValuePtr(light_pos));
    glUniform3fv(ViewPosLoc, 1, ValuePtr(view_pos));
    glUniform2fv(TriOrLine, 1, ValuePtr(type_));

    //glDrawElements(GL_LINES, numIndicesLine, GL_UNSIGNED_INT, nullptr);
    //glLineWidth(1.f);
    //glVertexAttrib3f(0.1f, 0.1f, 0.99f, 1.f); // blue color for lines
    glDrawArrays(GL_LINES, 0, numVerticesLine);
    glBindVertexArray(0);

}

void Mesh::drawFaceLine(glm::vec3 color, glm::mat4 view, glm::mat4 projection, glm::vec3 light_pos, glm::vec3 view_pos, glm::vec2 type_)
{
    glUseProgram(renderProg.GetHandle());
    glm::mat4 model = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
    model = glm::translate(model, glm::vec3{ 0,0.1,0 });
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(45.0f), glm::vec3(0.0f, 0.5f, 0.0f));
    model = glm::scale(model, glm::vec3{ 1.f,1.f,1.f });
    //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 0.5f, 0.0f));



    glBindVertexArray(VAOFL);
    glUniform4fv(colorLoc, 1, ValuePtr(color));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(LightLoc, 1, ValuePtr(light_pos));
    glUniform3fv(ViewPosLoc, 1, ValuePtr(view_pos));
    glUniform2fv(TriOrLine, 1, ValuePtr(type_));


    glDrawArrays(GL_LINES, 0, numVerticesFaceLine);
    glBindVertexArray(0);

}

void Mesh::drawLamp(glm::mat4 view, glm::mat4 projection, int numberLamp, std::vector<DirLight> dl, std::vector<lampSet> lampSetting)
{
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glDisable(GL_DEPTH_TEST);
    glUseProgram(renderProg.GetHandle());

    glBindVertexArray(VAO);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindBuffer(GL_UNIFORM_BUFFER, UBO);

    for (int i = 0; i < numberLamp; i++)
    {
        glm::mat4 model = {
       1,0,0,0,
       0,1,0,0,
       0,0,1,0,
       0,0,0,1
        };
        glm::vec3 Pos = { dl[i].position.x,dl[i].position.y,dl[i].position.z };
        model = glm::translate(model, Pos);
        model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, { 0.1f,0.1f ,0.1f });

        glUniform4f(glGetUniformLocation(renderProg.GetHandle(), "diffuse"), lampSetting[i].Diffuse.x, lampSetting[i].Diffuse.y, lampSetting[i].Diffuse.z, lampSetting[i].Diffuse.w);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);

}

void Mesh::drawLampForFrame(int numberLamp, std::vector<DirLight> dl)
{
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(renderProg.GetHandle());

    glBindVertexArray(VAO);


    for (int i = 0; i < numberLamp; i++)
    {
        glm::mat4 model = {
       1,0,0,0,
       0,1,0,0,
       0,0,1,0,
       0,0,0,1
        };
        glm::vec3 Pos = { dl[i].position.x,dl[i].position.y,dl[i].position.z };
        model = glm::translate(model, Pos);
        model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, { 0.2f,0.2f ,0.2f });

        //glUniform4f(glGetUniformLocation(renderProg.GetHandle(), "diffuse"), lampSetting[i].Diffuse.x, lampSetting[i].Diffuse.y, lampSetting[i].Diffuse.z, lampSetting[i].Diffuse.w);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    }
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
}


void Mesh::SendVertexData()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    /*  Copy vertex attributes to GPU */
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(numVertices) * static_cast <GLsizeiptr>(vertexSize), &vertexBuffer[0], GL_DYNAMIC_DRAW);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    /*  Copy vertex indices to GPU */
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(numIndices) * static_cast <GLsizeiptr>(indexSize), &indexBuffer[0], GL_DYNAMIC_DRAW);
    //5
    glGenBuffers(1, &UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);


    /*  Send vertex attributes to shaders */
    for (int i = 0; i < numAttribs; ++i)
    {
        glEnableVertexAttribArray(vLayout[i].location);
        glVertexAttribPointer(vLayout[i].location, vLayout[i].size, vLayout[i].type, vLayout[i].normalized, vertexSize, (void*)(uintptr_t)vLayout[i].offset);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Mesh::SendVertexDataForLine()
{
    glEnableVertexAttribArray(0);
    glGenVertexArrays(1, &VAOL);
    glBindVertexArray(VAOL);

    glGenBuffers(1, &VBOL);
    glBindBuffer(GL_ARRAY_BUFFER, VBOL);
    /*  Copy vertex attributes to GPU */
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(numVerticesLine) * static_cast <GLsizeiptr>(vertexSize), &vertexBufferForVertexNrm[0], GL_DYNAMIC_DRAW);


    /*  Send vertex attributes to shaders */
    for (int i = 0; i < numAttribs; ++i)
    {
        glEnableVertexAttribArray(vLayout[i].location);
        glVertexAttribPointer(vLayout[i].location, vLayout[i].size, vLayout[i].type, vLayout[i].normalized, vertexSize, (void*)(uintptr_t)vLayout[i].offset);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void Mesh::SendVertexDataForFaceLine()
{
    glEnableVertexAttribArray(0);
    glGenVertexArrays(1, &VAOFL);
    glBindVertexArray(VAOFL);

    glGenBuffers(1, &VBOFL);
    glBindBuffer(GL_ARRAY_BUFFER, VBOFL);
    /*  Copy vertex attributes to GPU */
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(numVerticesFaceLine) * static_cast <GLsizeiptr>(vertexSize), &vertexBufferForFaceNrm[0], GL_DYNAMIC_DRAW);


    /*  Send vertex attributes to shaders */
    for (int i = 0; i < numAttribs; ++i)
    {
        glEnableVertexAttribArray(vLayout[i].location);
        glVertexAttribPointer(vLayout[i].location, vLayout[i].size, vLayout[i].type, vLayout[i].normalized, vertexSize, (void*)(uintptr_t)vLayout[i].offset);
    }

}
void Mesh::SendVertexDataLamp()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    /*  Copy vertex attributes to GPU */
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(numVertices) * static_cast <GLsizeiptr>(vertexSize), &vertexBuffer[0], GL_DYNAMIC_DRAW);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    /*  Copy vertex indices to GPU */
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(numIndices) * static_cast <GLsizeiptr>(indexSize), &indexBuffer[0], GL_DYNAMIC_DRAW);


    /*  Send vertex attributes to shaders */
    for (int i = 0; i < numAttribs; ++i)
    {
        glEnableVertexAttribArray(vLayout[i].location);
        glVertexAttribPointer(vLayout[i].location, vLayout[i].size, vLayout[i].type, vLayout[i].normalized, vertexSize, (void*)(uintptr_t)vLayout[i].offset);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

Mesh LoadOBJ(const char* path)
{
    Mesh mesh;
    MinMax m;
    LengthMinMax lm;


    ReadOBJ(path, mesh, m, lm);
    mesh = CalculateMesh(mesh, m, lm);

    return mesh;
}
Mesh ReverseLoadOBJ(const char* path)
{
    Mesh mesh;
    MinMax m;
    LengthMinMax lm;


    ReverseReadOBJ(path, mesh, m, lm);
    mesh = CalculateMesh(mesh, m, lm);

    return mesh;
}

void ReadOBJ(const char* path, Mesh& mesh, MinMax& m, LengthMinMax& lm)
{
    Vertex v;
    std::ifstream file{ path };
    if (!file)
    {
        throw std::runtime_error(std::string("ERROR: Unable to open scene file: ") + path);
    }

    std::string line;
    char mode;

    while (file) {

        getline(file, line);
        if (line.empty())
        {
            continue;
        }

        std::istringstream sstr{ line };
        sstr >> mode;
        if (mode == 'v')
        {
            sstr >> v.pos.x >> v.pos.y >> v.pos.z;

            addVertex(mesh, v);
            CalculateMinMax(v, m);
        }
        else if (mode == 'f') {
            unsigned int vertexIndex[3];
            sstr >> vertexIndex[0] >> vertexIndex[1] >> vertexIndex[2];


            //if (matches != 3) {
            //    printf("File can't be read by our simple parser : ( Try exporting with other options\n");
            //    //return false;
            //}
            addIndex(mesh, vertexIndex[0] - 1);
            addIndex(mesh, vertexIndex[1] - 1);
            addIndex(mesh, vertexIndex[2] - 1);

            // For each vertex of each triangle
        }
    }
    lm.lenX = m.max.x - m.min.x;
    lm.lenY = m.max.y - m.min.y;
    lm.lenZ = m.max.z - m.min.z;
}

void ReverseReadOBJ(const char* path, Mesh& mesh, MinMax& m, LengthMinMax& lm)
{
    Vertex v;
    std::ifstream file{ path };
    if (!file)
    {
        throw std::runtime_error(std::string("ERROR: Unable to open scene file: ") + path);
    }

    std::string line;
    char mode;

    while (file) {

        getline(file, line);
        if (line.empty())
        {
            continue;
        }

        std::istringstream sstr{ line };
        sstr >> mode;
        if (mode == 'v')
        {
            sstr >> v.pos.x >> v.pos.y >> v.pos.z;

            addVertex(mesh, v);
            CalculateMinMax(v, m);
        }
        else if (mode == 'f') {
            unsigned int vertexIndex[3];
            sstr >> vertexIndex[2] >> vertexIndex[1] >> vertexIndex[0];


            //if (matches != 3) {
            //    printf("File can't be read by our simple parser : ( Try exporting with other options\n");
            //    //return false;
            //}
            addIndex(mesh, vertexIndex[0] - 1);
            addIndex(mesh, vertexIndex[1] - 1);
            addIndex(mesh, vertexIndex[2] - 1);

            // For each vertex of each triangle
        }
    }
    lm.lenX = m.max.x - m.min.x;
    lm.lenY = m.max.y - m.min.y;
    lm.lenZ = m.max.z - m.min.z;
}

Mesh CalculateMesh(Mesh& mesh, MinMax& m, LengthMinMax& lm)
{
    int ia, ib, ic;
    size_t sizeOfIndicies = mesh.numIndices;
    std::vector<int> nb_seen;
    nb_seen.resize(mesh.numVertices, 0);
    std::vector<std::vector<glm::vec3>> prevNormal;
    int doubleNumVertices = mesh.numVertices * 2;
    int doubleNumFaceVertices = mesh.numTris * 2;
    int NumVertice = mesh.numVertices;
    mesh.faceBuffer.resize(mesh.numTris, Vertex());
    prevNormal.resize(NumVertice, std::vector<glm::vec3>(0));

    mesh.vertexBufferForVertexNrm.resize(doubleNumVertices, Vertex());
    mesh.vertexBufferForFaceNrm.resize(doubleNumFaceVertices, Vertex());


    //Move to origin
    glm::vec3 origin = { 0,0,0 };
    MoveToOrigin(mesh, origin, m);

    for (int i = 0; i < NumVertice; i++)
    {

        mesh.vertexBuffer[i].pos.x = 2 * ((mesh.vertexBuffer[i].pos.x - m.min.x) / lm.lenX) - 1.f;
        mesh.vertexBuffer[i].pos.y = 2 * ((mesh.vertexBuffer[i].pos.y - m.min.y) / lm.lenY) - 1.f;
        mesh.vertexBuffer[i].pos.z = 2 * ((mesh.vertexBuffer[i].pos.z - m.min.z) / lm.lenZ) - 1.f;
        if (lm.lenZ == 0)
            mesh.vertexBuffer[i].pos.z = 0;
    }
    int ione = 0;
    int itwo = 0;
    for (unsigned int i = 0; i < sizeOfIndicies; i += 3) {

        ia = mesh.indexBuffer[i];
        ione = i + 1;
        ib = mesh.indexBuffer[ione];
        itwo = i + 2;
        ic = mesh.indexBuffer[itwo];

        glm::vec3 normal = Normalize(glm::cross(
            mesh.vertexBuffer[ib].pos - mesh.vertexBuffer[ia].pos,
            mesh.vertexBuffer[ic].pos - mesh.vertexBuffer[ia].pos));

        mesh.faceBuffer[i / 3].nrm = normal;
        mesh.faceBuffer[i / 3].pos = (mesh.vertexBuffer[ia].pos + mesh.vertexBuffer[ib].pos + mesh.vertexBuffer[ic].pos) / 3.f;

        int v[3];
        v[0] = ia;
        v[1] = ib;
        v[2] = ic;
        //Averaging normals
        for (int j = 0; j < 3; j++)
        {
            bool IgnoreParrel = false;
            int cur_v = v[j];
            nb_seen[cur_v]++;
            if (nb_seen[cur_v] == 1)
            {
                mesh.vertexBuffer[cur_v].nrm = normal;
                prevNormal[cur_v].push_back(normal);
            }
            else
            {
                for (int k = 0; k < prevNormal[cur_v].size(); k++)
                {
                    if (prevNormal[cur_v][k] == normal)
                        IgnoreParrel = true;
                }
                // average
                if (IgnoreParrel != true)
                {
                    mesh.vertexBuffer[cur_v].nrm.x = mesh.vertexBuffer[cur_v].nrm.x * (1.f - 1.f / nb_seen[cur_v]) + normal.x * 1.f / nb_seen[cur_v];
                    mesh.vertexBuffer[cur_v].nrm.y = mesh.vertexBuffer[cur_v].nrm.y * (1.f - 1.f / nb_seen[cur_v]) + normal.y * 1.f / nb_seen[cur_v];
                    mesh.vertexBuffer[cur_v].nrm.z = mesh.vertexBuffer[cur_v].nrm.z * (1.f - 1.f / nb_seen[cur_v]) + normal.z * 1.f / nb_seen[cur_v];
                    mesh.vertexBuffer[cur_v].nrm = glm::normalize(mesh.vertexBuffer[cur_v].nrm);
                }
            }
        }
    }
    //face normal
    int f = 0;
    mesh.numVerticesFaceLine = 0;
    for (int i = 0; i < doubleNumFaceVertices; i += 2)
    {
        mesh.vertexBufferForFaceNrm[i].pos = mesh.faceBuffer[f].pos;
        ione = i + 1;
        mesh.vertexBufferForFaceNrm[ione].pos = mesh.faceBuffer[f].pos + (mesh.faceBuffer[f].nrm / 7.f);

        mesh.numVerticesFaceLine += 2;

        f++;
    }

    //vertex normal
    mesh.numVerticesLine = 0;
    int j = 0;
    for (int i = 0; i < doubleNumVertices; i += 2)
    {
        mesh.vertexBufferForVertexNrm[i].pos = mesh.vertexBuffer[j].pos;
        ione = i + 1;
        mesh.vertexBufferForVertexNrm[ione].pos = mesh.vertexBuffer[j].pos + (mesh.vertexBuffer[j].nrm / 7.f);

        mesh.numVerticesLine += 2;

        j++;
    }

    return mesh;
}

std::vector<GLuint> Mesh::loadCubemap(std::vector<std::string> faces)
{
    std::vector<GLuint> textureID;
    //glGenFramebuffers(1, &FBO);
    //glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    textureID.resize(faces.size());

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        glGenTextures(1, &textureID[i]);
        glBindTexture(GL_TEXTURE_2D, textureID[i]);
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);//window width,height 1600, 1000
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID[i], 0);
    }


    return textureID;
}
void Mesh::setTexture()
{
    cubemapTextureEnvironment = loadCubemap(faces);
    

}

void CalculateMinMax(Vertex v, MinMax& m)
{
    if (v.pos.x < m.min.x)
        m.min.x = v.pos.x;
    if (v.pos.y < m.min.y)
        m.min.y = v.pos.y;
    if (v.pos.z < m.min.z)
        m.min.z = v.pos.z;
    if (v.pos.x > m.max.x)
        m.max.x = v.pos.x;
    if (v.pos.y > m.max.y)
        m.max.y = v.pos.y;
    if (v.pos.z > m.max.z)
        m.max.z = v.pos.z;

}

void MoveToOrigin(Mesh& mesh, glm::vec3& origin, MinMax& m)
{
    for (int i = 0; i < mesh.numVertices; i++)
    {
        origin.x += mesh.vertexBuffer[i].pos.x;
        origin.y += mesh.vertexBuffer[i].pos.y;
        origin.z += mesh.vertexBuffer[i].pos.z;
    }
    origin = origin / static_cast<float>(mesh.numVertices);

    for (int i = 0; i < mesh.numVertices; i++)
    {
        mesh.vertexBuffer[i].pos.x -= origin.x;
        mesh.vertexBuffer[i].pos.y -= origin.y;
        mesh.vertexBuffer[i].pos.z -= origin.z;
    }

    m.min -= origin;
    m.max -= origin;
}

GLfloat* Mesh::readPPM(const char* vertex_file_path, int& width, int& height)
{
    float* RGB;
    float tmp = 0;
    float max;
    std::string mMagic;
    std::ifstream file(vertex_file_path);
    if (!file.is_open())
    {
        throw std::runtime_error(std::string("ERROR: Unable to open scene file: ") + vertex_file_path);
    }
    file >> mMagic;
    file.seekg(1, file.cur);
    char c;
    file.get(c);
    if (c == '#')
    {
        // We got comments in the PPM image and skip the comments
        while (c != '\n')
        {
            file.get(c);
        }
    }
    else
    {
        file.seekg(-1, file.cur);
    }

    file >> width >> height >> max;

    int memSize = width * height * 3;
    RGB = new float[memSize];


    std::string pixel_str;
    for (int i = 0; i < memSize; i++)
    {
        file >> tmp;
        tmp /= max;
        RGB[i] = tmp;
    }

    return RGB;

}

void Mesh::setTextureGrid()
{

    glGenTextures(1, &diffuseMap);
    glGenTextures(1, &specularMap);
    //glGenTextures(1, &emissionMap);

    int imageWidth = 0, imageHeight = 0;


    GLfloat* image;
    // Diffuse map
    image = readPPM("../textures/grid.ppm", imageWidth, imageHeight);
    //image = stbi_load("../textures/metal_roof_diff_512x512.png", &imageWidth, &imageHeight, &channel, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_FLOAT, image);
    //glGenerateMipmap(GL_TEXTURE_2D);
    //stbi_image_free(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Specular map
    image = readPPM("../textures/grid.ppm", imageWidth, imageHeight);
    glBindTexture(GL_TEXTURE_2D, specularMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_FLOAT, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

}
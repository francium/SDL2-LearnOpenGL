#pragma once


#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "platform.hpp"
#include "shader.hpp"
#include "texture.hpp"


struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};


class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Texture> textures;
    GLuint vao;

    Mesh(
        std::vector<Vertex> vertices,
        std::vector<u32> indices,
        std::vector<Texture> textures
    )
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        this->setup_mesh();
    }

    void cleanup()
    {
        for (const Texture t: this->textures)
        {
            Texture_cleanup(&t);
        }
    }

    void draw(Shader *shader)
    {
        u32 diffuse_nr = 1;
        u32 specular_nr = 1;
        u32 normal_nr = 1;
        u32 height_nr = 1;
        for (u32 i = 0; i < this->textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            char number[4];
            const char *name = textures[i].type;
            if (strcmp(name, "texture_diffuse") == 0)
                sprintf(number, "%u", diffuse_nr++);
            else if (strcmp(name, "texture_specular"))
                sprintf(number, "%u", specular_nr++);
            else if (strcmp(name, "texture_normal"))
                sprintf(number, "%u", normal_nr++);
            else if (strcmp(name, "texture_height"))
                sprintf(number, "%u", height_nr++);

            char name_number[20];
            sprintf(name_number, "%s%s", name, number);
            glUniform1i(glGetUniformLocation(shader->id, name_number), i);
            glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
        }

        glBindVertexArray(this->vao);
        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }

private:
    GLuint vbo;
    GLuint ebo;

    void setup_mesh()
    {
        glGenVertexArrays(1, &this->vao);
        glGenBuffers(1, &this->vbo);
        glGenBuffers(1, &this->ebo);

        glBindVertexArray(this->vao);
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            this->vertices.size() * sizeof(Vertex),
            &this->vertices[0],
            GL_STATIC_DRAW
        );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            this->indices.size() * sizeof(unsigned int),
            &this->indices[0],
            GL_STATIC_DRAW
        );

        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (void*)offsetof(Vertex, normal)
        );

        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (void*)offsetof(Vertex, tex_coords)
        );

        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(
            3,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (void*)offsetof(Vertex, tangent)
        );

        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(
            4,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (void*)offsetof(Vertex, bitangent)
        );

        glBindVertexArray(0);
    }
};

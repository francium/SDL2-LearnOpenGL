#pragma once


const int stride = 5;
const float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};


u32 num_objects = 45;
glm::vec3 obj_positions[] = {
    glm::vec3(-2.0f, 0.5f, 0.0f),
    glm::vec3(-2.0f, 1.5f, 0.0f),

    glm::vec3(-1.0f, 0.5f, 0.0f),

    glm::vec3(0.0f, 0.5f, 0.0f),
    glm::vec3(0.0f, 1.5f, 0.0f),

    glm::vec3(1.0f, 0.5f, 0.0f),
    glm::vec3(1.0f, 1.5f, 0.0f),

    glm::vec3(2.0f, 0.5f, 0.0f),
    glm::vec3(2.0f, 1.5f, 0.0f),
    glm::vec3(2.0f, 2.5f, 0.0f),

    glm::vec3(3.0f, 1.5f, 0.0f),
    glm::vec3(3.0f, 2.5f, 0.0f),

    glm::vec3(4.0f, 0.5f, 0.0f),
    glm::vec3(4.0f, 1.5f, 0.0f),
    glm::vec3(4.0f, 2.5f, 0.0f),

    //
    glm::vec3(-2.0f, 0.5f, 1.0f),
    glm::vec3(-2.0f, 1.5f, 1.0f),

    glm::vec3(-2.0f, 0.5f, 2.0f),
    glm::vec3(-2.0f, 1.5f, 2.0f),

    glm::vec3(-2.0f, 0.5f, 3.0f),
    glm::vec3(-2.0f, 1.5f, 3.0f),

    glm::vec3(-2.0f, 0.5f, 4.0f),
    glm::vec3(-2.0f, 1.5f, 4.0f),

    glm::vec3(-2.0f, 0.5f, 5.0f),
    glm::vec3(-2.0f, 1.5f, 5.0f),

    glm::vec3(-2.0f, 1.5f, 6.0f),
    glm::vec3(-2.0f, 2.5f, 6.0f),

    glm::vec3(-2.0f, 0.5f, 7.0f),
    glm::vec3(-2.0f, 1.5f, 7.0f),

    //
    glm::vec3(5.0f, 0.5f, 1.0f),
    glm::vec3(5.0f, 1.5f, 1.0f),

    glm::vec3(5.0f, 0.5f, 2.0f),
    glm::vec3(5.0f, 1.5f, 2.0f),

    glm::vec3(5.0f, 0.5f, 3.0f),
    glm::vec3(5.0f, 1.5f, 3.0f),
    glm::vec3(5.0f, 2.5f, 3.0f),

    glm::vec3(5.0f, 2.5f, 4.0f),

    glm::vec3(5.0f, 0.5f, 5.0f),
    glm::vec3(5.0f, 1.5f, 5.0f),

    glm::vec3(5.0f, 1.5f, 6.0f),
    glm::vec3(5.0f, 2.5f, 6.0f),
    glm::vec3(5.0f, 2.5f, 6.0f),

    glm::vec3(5.0f, 0.5f, 7.0f),
    glm::vec3(5.0f, 1.5f, 7.0f),
    glm::vec3(5.0f, 2.5f, 7.0f),
};


internal void
init_rendering_data(GLuint *vao, GLuint *vbo)
{
    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);

    glBindVertexArray(*vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, *vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(f32), nullptr);
        glEnableVertexAttribArray(0);

        // Tex coord
        glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride * sizeof(f32),
            (void *)(3 * sizeof(f32))
        );
        glEnableVertexAttribArray(1);
    }
}

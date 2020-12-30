#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model.hpp"
#include "transforms.hpp"


struct Entity
{
    glm::vec3 _position;
    glm::vec3 _direction;
    Model _model;
    Shader *_shader;

    Entity(const char *model_dir, const char *model_file, Shader *shader)
        : Entity(model_dir, model_file, shader, glm::vec3(0.0))
    {}

    Entity(const char *model_dir, const char *model_file, Shader *shader, glm::vec3 position)
        : Entity(model_dir, model_file, shader, position, glm::vec3(0.0))
    {}

    Entity(
        const char *model_dir,
        const char *model_file,
        Shader *shader,
        glm::vec3 position,
        glm::vec3 direction
    )
        : _position(position)
        , _direction(direction)
        , _model(model_dir, model_file, false)
        , _shader(shader)
    {}

    void move_to(glm::vec3 position)
    {
        this->_position = position;
    }

    void face_to(glm::vec3 direction)
    {
        this->_direction = direction;
    }

    void cleanup()
    {
        this->_model.cleanup();
    }

    void draw(Transforms *transforms)
    {
        Shader_use(this->_shader);

        Shader_set_matrix4fv(
            this->_shader,
            "view_matrix",
            glm::value_ptr(transforms->view)
        );
        Shader_set_matrix4fv(
            this->_shader,
            "projection_matrix",
            glm::value_ptr(transforms->projection)
        );
        Shader_set_matrix4fv(
            this->_shader,
            "model_matrix",
            glm::value_ptr(glm::mat4(1.0))
        );

        this->_model.draw(this->_shader);
    }
};

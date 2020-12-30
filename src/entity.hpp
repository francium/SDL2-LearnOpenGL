#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model.hpp"
#include "transforms.hpp"


struct EntityTransforms
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct Entity
{
    EntityTransforms _transforms;
    Model _model;
    Shader *_shader;

    Entity(const char *model_dir, const char *model_file, Shader *shader)
        : _model(model_dir, model_file, false)
        , _shader(shader)
    {
        this->_transforms.position = glm::vec3(0.0f);
        this->_transforms.direction = glm::vec3(0.0f, 0.0f, 1.0f);
        this->_transforms.rotation = glm::vec3(0.0f);
        this->_transforms.scale = glm::vec3(1.0f);
    }

    void cleanup()
    {
        this->_model.cleanup();
    }

    void scale(glm::vec3 scale)
    {
        this->_transforms.scale = scale;
    }

    void rotate(glm::vec3 rotation)
    {
        this->_transforms.rotation = rotation;
    }

    void translate(glm::vec3 position)
    {
        this->_transforms.position = position;
    }

    void face_to(glm::vec3 direction)
    {
        this->_transforms.direction = direction;
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

        glm::mat4 model_matrix = glm::mat4(1.0f);
        model_matrix = glm::rotate(model_matrix, this->_transforms.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model_matrix = glm::rotate(model_matrix, this->_transforms.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model_matrix = glm::rotate(model_matrix, this->_transforms.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model_matrix = glm::scale(model_matrix, this->_transforms.scale);
        model_matrix = glm::translate(model_matrix, this->_transforms.position);
        Shader_set_matrix4fv(
            this->_shader,
            "model_matrix",
            glm::value_ptr(model_matrix)
        );

        this->_model.draw(this->_shader);
    }
};

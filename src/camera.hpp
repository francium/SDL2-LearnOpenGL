#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "platform.hpp"
#include "util.hpp"


enum CameraMovement
{
    CameraForward,
    CameraBackward,
    CameraLeft,
    CameraRight,
};


struct Camera
{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    f32 yaw;
    f32 pitch;
    f32 fov;
};


internal void
Camera_init(Camera *c, f32 height, f32 fov)
{
    c->position = glm::vec3(0.0f, height, 8.0f);
    c->front = glm::vec3(-0.0f, 0.0f, -1.0f);
    c->up = glm::vec3(0.0f, 1.0f, 0.0f);
    c->yaw = 0.0f;
    c->pitch = 0.0f;
    c->fov = fov;
}


internal glm::mat4
Camera_toViewMatrix(Camera *c)
{
    return glm::lookAt(c->position, c->position + c->front, c->up);
}


internal void
Camera_process_keyboard(Camera *c, CameraMovement direction, f32 dt)
{
    f32 camera_speed = dt * 0.005f;

    switch (direction)
    {
        case CameraForward:
        {
            c->position += camera_speed * c->front;
        } break;

        case CameraBackward:
        {
            c->position -= camera_speed * c->front;
        } break;

        case CameraLeft:
        {
            c->position -= glm::normalize(glm::cross(c->front, c->up))
                         * camera_speed;
        } break;

        case CameraRight:
        {
            c->position += glm::normalize(glm::cross(c->front, c->up))
                         * camera_speed;
        } break;
    }

    // FPS style, stay on ground, no floating
    c->position.y = 1.5f;
}


internal void
Camera_process_mouse_motion(Camera *c, f32 dx, f32 dy)
{
    float sensitivity = 0.1;
    dx *= sensitivity;
    dy *= sensitivity;

    c->yaw += dx;
    c->pitch += dy;
    c->pitch = clamp(-70.0f, c->pitch, 80.0f);

    glm::vec3 front;
    front.x = cos(glm::radians(c->yaw)) * cos(glm::radians(c->pitch));
    front.y = sin(glm::radians(c->pitch));
    front.z = sin(glm::radians(c->yaw)) * cos(glm::radians(c->pitch));
    c->front = glm::normalize(front);
}


internal void
Camera_process_mouse_scroll(Camera *c, f32 dy)
{
    c->fov += dy;
    c->fov = clamp(10.0f, c->fov, 45.0f);
}

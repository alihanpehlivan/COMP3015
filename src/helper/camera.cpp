#include "../pch.h"
#include "camera.h"

void Camera::ProcessKeyboard(ECameraDir dir, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (dir == FORWARD)
        Position += Front * velocity;
    if (dir == BACKWARD)
        Position -= Front * velocity;
    if (dir == LEFT)
        Position -= Right * velocity;
    if (dir == RIGHT)
        Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float x, float y, bool constrainPitch)
{
    x *= MouseSensitivity;
    y *= MouseSensitivity;

    Yaw += x;
    Pitch += y;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    UpdateCameraPosition();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;

    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 80.0f)
        Zoom = 80.0f;
}

void Camera::UpdateCameraPosition()
{
    // calculate the new Front vector
    glm::vec3 front{};
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    // also re-calculate the Right and Up vector
    // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

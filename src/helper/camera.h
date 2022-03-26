#pragma once

// Default camera pos
constexpr auto DEF_CAMERA_YAW = -90.0f;
constexpr auto DEF_CAMERA_PITCH = 0.0f;
constexpr auto DEF_CAMERA_SPEED = 5.0f;
constexpr auto DEF_CAMERA_SENSITIVITY = 0.5f;
constexpr auto DEF_CAMERA_ZOOM = 70.0f;

class Camera
{
public:
    enum ECameraDir
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
    };

    // With vectors
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = DEF_CAMERA_YAW,
        float pitch = DEF_CAMERA_PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
        MovementSpeed(DEF_CAMERA_SPEED),
        MouseSensitivity(DEF_CAMERA_SENSITIVITY),
        Zoom(DEF_CAMERA_ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        UpdateCameraPosition();
    }

    // With scalar values
    Camera(
        float posX,
        float posY,
        float posZ,
        float upX,
        float upY,
        float upZ,
        float yaw,
        float pitch) : 
        Front(glm::vec3(0.0f, 0.0f, -1.0f)),
        MovementSpeed(DEF_CAMERA_SPEED),
        MouseSensitivity(DEF_CAMERA_SENSITIVITY),
        Zoom(DEF_CAMERA_ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        UpdateCameraPosition();
    }

    ~Camera() = default;

    void ProcessKeyboard(ECameraDir dir, float deltaTime);
    void ProcessMouseMovement(float x, float y, bool constrainPitch = true);
    void ProcessMouseScroll(float y);

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix();

    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // euler Angles
    float Yaw;
    float Pitch;

    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

private:
    void UpdateCameraPosition();
};

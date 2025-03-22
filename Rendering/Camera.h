#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    static constexpr float YAW = -90.0f;
    static constexpr float PITCH = 0.0f;
    static constexpr float SPEED = 2.5f;
    static constexpr float SENSITIVITY = 0.1f;
    static constexpr float ZOOM = 45.0f;

    enum CameraMovement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    void updateCameraVectors();
public:
    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    glm::mat4 getViewMatrix();

    void processKeyboard(CameraMovement direction, float deltaTime);

    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    void processMouseScroll(float yoffset);
};

#endif // CAMERA_H
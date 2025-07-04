#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera
{
public:
    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

    void processKeyboard(CameraMovement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspect) const;
    float getZoom() const;
    glm::vec3 getPosition() const;

protected:
    void updateCameraVectors();

private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    // Углы Эйлера
    float m_yaw;
    float m_pitch;

    float m_movementSpeed = 2.5f;
    float m_mouseSensitivity = 0.1f;
    float m_zoom = 45.0f;
};

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_position(position),
      m_worldUp(up),
      m_yaw(yaw),
      m_pitch(pitch),
      m_front(glm::vec3(0.0f, 0.0f, -1.0f))
{
    updateCameraVectors();
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = m_movementSpeed * deltaTime;
    if (direction == CameraMovement::FORWARD)
        m_position += m_front * velocity;
    if (direction == CameraMovement::BACKWARD)
        m_position -= m_front * velocity;
    if (direction == CameraMovement::LEFT)
        m_position -= m_right * velocity;
    if (direction == CameraMovement::RIGHT)
        m_position += m_right * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= m_mouseSensitivity;
    yoffset *= m_mouseSensitivity;

    m_yaw += xoffset;
    m_pitch += yoffset;

    // Ограничение угла, чтобы не было переворота
    if (constrainPitch)
    {
        m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
    }

    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspect) const
{
    return glm::perspective(glm::radians(m_zoom), aspect, 0.1f, 100.0f);
}

void Camera::processMouseScroll(float yoffset)
{
    m_zoom = glm::clamp(m_zoom - yoffset, 1.0f, 45.0f);
}

float Camera::getZoom() const
{
    return m_zoom;
}

glm::vec3 Camera::getPosition() const
{
    return m_position;
}

void Camera::updateCameraVectors()
{
    // Вычисляем новый фронтальный вектор сферическими координатами
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}
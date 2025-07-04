#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include "Ball.hpp"

class Physics
{
public:
    Physics(float tableWidth, float tableHeight, float friction);

    void Update(std::vector<Ball> &balls, float dt);

    bool CheckPocketCollision(Ball &ball, const glm::vec3 &pocketPos, float pocketRadius);

private:
    float tableWidth;
    float tableHeight;
    float friction; // коэффициент трения, замедляющий шары

    void ApplyFriction(Ball &ball, float dt);
    void HandleWallCollisions(Ball &ball);
    void HandleBallCollisions(Ball &ballA, Ball &ballB);
};

Physics::Physics(float tableWidth, float tableHeight, float friction)
    : tableWidth(tableWidth), tableHeight(tableHeight), friction(friction) {}

void Physics::Update(std::vector<Ball> &balls, float dt)
{
    for (auto &ball : balls)
    {
        ball.update(dt);

        HandleWallCollisions(ball);

        ball.applyFriction(friction, dt);
    }

    for (size_t i = 0; i < balls.size(); ++i)
    {
        for (size_t j = i + 1; j < balls.size(); ++j)
        {
            HandleBallCollisions(balls[i], balls[j]);
        }
    }
}

bool Physics::CheckPocketCollision(Ball &ball, const glm::vec3 &pocketPos, float pocketRadius)
{
    // Пропускаем шары, которые уже падают или в лузе
    if (ball.isFalling || ball.isPocketed) 
        return false;
    
    float dist = glm::distance(ball.getPosition(), pocketPos);
    if (dist < pocketRadius)
    {
        // Рассчитываем направление от шара к центру лузы
        glm::vec3 pocketDirection = pocketPos - ball.getPosition();
        ball.startFalling(pocketDirection);
        return true;
    }
    return false;
}

void Physics::ApplyFriction(Ball &ball, float dt)
{
    ball.applyFriction(friction, dt);
}

void Physics::HandleWallCollisions(Ball &ball)
{

    // Пропускаем падающие и запотенченные шары
    if (ball.isFalling || ball.isPocketed) return;

    glm::vec3 position = ball.getPosition();
    glm::vec3 velocity = ball.getVelocity();
    float radius = ball.getRadius();

    float left = -tableWidth / 2.0f + radius;
    float right = tableWidth / 2.0f - radius;
    float top = tableHeight / 2.0f - radius;
    float bottom = -tableHeight / 2.0f + radius;

    bool positionChanged = false;
    bool velocityChanged = false;

    if (position.x < left)
    {
        position.x = left;
        velocity.x = -velocity.x;
        positionChanged = true;
        velocityChanged = true;
    }
    else if (position.x > right)
    {
        position.x = right;
        velocity.x = -velocity.x;
        positionChanged = true;
        velocityChanged = true;
    }

    if (position.z < bottom)
    {
        position.z = bottom;
        velocity.z = -velocity.z;
        positionChanged = true;
        velocityChanged = true;
    }
    else if (position.z > top)
    {
        position.z = top;
        velocity.z = -velocity.z;
        positionChanged = true;
        velocityChanged = true;
    }

    if (positionChanged)
    {
        ball.setPosition(position);
    }
    if (velocityChanged)
    {
        ball.setVelocity(velocity);
    }
}

void Physics::HandleBallCollisions(Ball &ballA, Ball &ballB)
{

    if (ballA.isFalling || ballA.isPocketed || 
        ballB.isFalling || ballB.isPocketed) 
    {
        return;
    }

    glm::vec3 posA = ballA.getPosition();
    glm::vec3 posB = ballB.getPosition();
    glm::vec3 velA = ballA.getVelocity();
    glm::vec3 velB = ballB.getVelocity();

    glm::vec3 delta = posB - posA;
    float dist = glm::length(delta);
    float penetration = ballA.getRadius() + ballB.getRadius() - dist;

    if (penetration > 0.0f && dist > 0.0f)
    {
        // Раздвигаем шары, чтобы не пересекались
        glm::vec3 collisionNormal = delta / dist;

        float totalMass = ballA.getMass() + ballB.getMass();
        float correction = penetration / 2.0f;

        posA -= collisionNormal * correction * (ballB.getMass() / totalMass);
        posB += collisionNormal * correction * (ballA.getMass() / totalMass);

        // Обновляем позиции
        ballA.setPosition(posA);
        ballB.setPosition(posB);

        // Скорости по формулам упругого столкновения
        glm::vec3 relativeVelocity = velB - velA;
        float velocityAlongNormal = glm::dot(relativeVelocity, collisionNormal);

        if (velocityAlongNormal > 0)
            return; // уже разлетаются

        // Коэффициент восстановления (1 - полностью упругое)
        const float restitution = 0.9f;

        float impulseMagnitude = -(1.0f + restitution) * velocityAlongNormal;
        impulseMagnitude /= (1.0f / ballA.getMass() + 1.0f / ballB.getMass());

        glm::vec3 impulse = impulseMagnitude * collisionNormal;

        // Применяем импульсы через методы класса Ball
        ballA.applyImpulse(-impulse);
        ballB.applyImpulse(impulse);

        // Добавляем угловой импульс в точке столкновения
        ballA.applyAngularImpulse(ballA.getPosition() + collisionNormal * ballA.getRadius(), -impulse);
        ballB.applyAngularImpulse(ballB.getPosition() - collisionNormal * ballB.getRadius(), impulse);
    }
}
#pragma once
#include "Frost.h"
#include "Components/GameComponents.h"
#include "Events/GameEvents.h"
#include "SpriteSheet.h"

class PlayerScript : public Frost::ScriptBase
{
public:
    void OnStart() override
    {
        FROST_LOG("PlayerScript started on entity %u", m_Entity);
    }

    void OnUpdate(float dt) override
    {
        auto& transform = GetComponent<Transform2D>();
        auto& velocity = GetComponent<Velocity2D>();
        auto& state = GetComponent<PlayerState>();

        // Read active effects
        float maxSpeed = 300.0f;
        float thrust = 200.0f;

        if (HasComponent<SpeedBoostEffect>())
        {
            auto& sb = GetComponent<SpeedBoostEffect>();
            maxSpeed = sb.maxSpeed;
            thrust = sb.thrust;
        }

        constexpr float rotateSpeed = 3.0f;
        constexpr float drag = 0.98f;

        if (Frost::Input::IsKeyPressed(GLFW_KEY_A))
            transform.rotation -= rotateSpeed * dt;
        if (Frost::Input::IsKeyPressed(GLFW_KEY_D))
            transform.rotation += rotateSpeed * dt;

        state.isThrusting = Frost::Input::IsKeyPressed(GLFW_KEY_W);
        if (state.isThrusting)
        {
            Frost::vec2 forward{
                std::cos(transform.rotation - Frost::HALF_PI),
                std::sin(transform.rotation - Frost::HALF_PI)
            };
            velocity.velocity.x += forward.x * thrust * dt;
            velocity.velocity.y += forward.y * thrust * dt;
        }

        state.isShooting = Frost::Input::IsKeyJustPressed(GLFW_KEY_SPACE);

        // Frame-rate independent drag
        float dragFactor = std::pow(drag, dt * 60.0f);
        velocity.velocity.x *= dragFactor;
        velocity.velocity.y *= dragFactor;

        // Speed cap — uses effect value if active
        float speedSq = velocity.velocity.x * velocity.velocity.x
            + velocity.velocity.y * velocity.velocity.y;
        if (speedSq > maxSpeed * maxSpeed)
        {
            float speed = std::sqrt(speedSq);
            velocity.velocity.x = (velocity.velocity.x / speed) * maxSpeed;
            velocity.velocity.y = (velocity.velocity.y / speed) * maxSpeed;
        }

        SyncThruster(transform, velocity);
    }

    void SetThrusterEntity(Frost::Entity e) { m_ThrusterEntity = e; }

private:
    void SyncThruster(const Transform2D& playerT, const Velocity2D& vel)
    {
        if (m_ThrusterEntity == Frost::NULL_ENTITY) return;
        if (!m_Registry->IsAlive(m_ThrusterEntity))  return;

        auto& thrusterT = m_Registry->Get<Transform2D>(m_ThrusterEntity);
        auto& flicker = m_Registry->Get<Flicker>(m_ThrusterEntity);
        auto& playerState = GetComponent<PlayerState>();

        // position thruster behind the ship
        float angle = playerT.rotation - Frost::HALF_PI;
        float offsetDist = 35.0f;

        thrusterT.position = {
            playerT.position.x - std::cos(angle) * offsetDist,
            playerT.position.y - std::sin(angle) * offsetDist
        };
        thrusterT.rotation = playerT.rotation;

        // control flicker
        if (!playerState.isThrusting)
        {
            flicker.visible = false;
            flicker.timer = 0.0f;
        }
    }

    Frost::Entity m_ThrusterEntity = Frost::NULL_ENTITY;
};
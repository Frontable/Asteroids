#pragma once
#include "Frost.h"
#include "../Components/GameComponents.h"
#include "../SpriteSheet.h"

class ShootingScript : public Frost::ScriptBase
{
public:

    void OnStart() override
    {
        m_ShootCooldown = 0.5f; 
    }

    void OnUpdate(float dt) override
    {
        auto& state = GetComponent<PlayerState>();
        auto& transform = GetComponent<Transform2D>();

        // Tick shoot cooldown
        m_ShootCooldown = std::max(0.0f, m_ShootCooldown - dt);

        // Determine fire rate — check rapid fire effect
        float cooldownMax = m_DefaultCooldown;
        if (HasComponent<RapidFireEffect>())
            cooldownMax = GetComponent<RapidFireEffect>().cooldownMax;

        // Only shoot if cooldown expired
        bool wantsToShoot = state.isShooting ||
            Frost::Input::IsKeyPressed(GLFW_KEY_SPACE);

        if (!wantsToShoot || m_ShootCooldown > 0.0f) return;

        m_ShootCooldown = cooldownMax;

        Frost::vec2 forward{
            std::cos(transform.rotation - Frost::HALF_PI),
            std::sin(transform.rotation - Frost::HALF_PI)
        };

        Frost::Entity bullet = m_Registry->Create();

        m_Registry->Add<Transform2D>(bullet, {
            { transform.position.x + forward.x * 30.0f,
              transform.position.y + forward.y * 30.0f },
            transform.rotation, { 8.0f, 8.0f }
            });
        m_Registry->Add<Velocity2D>(bullet,
            { { forward.x * 600.0f, forward.y * 600.0f }, 0.0f });
        m_Registry->Add<Sprite>(bullet, GetSprite(SpriteID::PLAYER_BULLET));
        m_Registry->Add<CircleCollider>(bullet, { 4.0f });
        m_Registry->Add<Lifetime>(bullet, { 1.5f });
        m_Registry->Add<SpawnImmunity>(bullet, { 0.1f });
        m_Registry->Add<BulletTag>(bullet, {});
    }

private:
    float m_DefaultCooldown = 0.3f;
    float m_ShootCooldown = 0.0f;
};
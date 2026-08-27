#pragma once
#include "Frost.h"
#include "../Components/GameComponents.h"
#include "../Events/GameEvents.h"
#include "../Systems/CollisionSystem.h"
#include <functional>

class DamageSystem
{
public:
    DamageSystem(Frost::Registry& registry, CollisionSystem& collision)
        : m_Registry(registry), m_Collision(collision) {
    }

    void Update(float dt)
    {
        auto& pairs = m_Collision.GetPairs();

        std::vector<Frost::Entity> toDestroy;

        auto alreadyQueued = [&](Frost::Entity e)
            {
                for (auto x : toDestroy) if (x == e) return true;
                return false;
            };

        for (auto& [a, b] : pairs)
        {
            // make sure entities are still alive
            if (!m_Registry.IsAlive(a) || !m_Registry.IsAlive(b))
                continue;

            bool aIsBullet = m_Registry.Has<BulletTag>(a);
            bool bIsBullet = m_Registry.Has<BulletTag>(b);
            bool aIsAsteroid = m_Registry.Has<AsteroidTag>(a);
            bool bIsAsteroid = m_Registry.Has<AsteroidTag>(b);
            bool aIsPlayer = m_Registry.Has<PlayerTag>(a);
            bool bIsPlayer = m_Registry.Has<PlayerTag>(b);

            // bullet <-> Asteroid
            if ((aIsBullet && bIsAsteroid) || (bIsBullet && aIsAsteroid))
            {
                Frost::Entity asteroid = aIsAsteroid ? a : b;
                Frost::Entity bullet = aIsBullet ? a : b;

                if (!alreadyQueued(asteroid) && !alreadyQueued(bullet))
                {
                    auto& transform = m_Registry.Get<Transform2D>(asteroid);
                    auto& tag = m_Registry.Get<AsteroidTag>(asteroid);
                    auto& vel = m_Registry.Get<Velocity2D>(asteroid);

                    // Fire event — score + split handled by listeners
                    Frost::EventBus::Emit(AsteroidDestroyedEvent{
                        transform.position,
                        tag.size,
                        vel.velocity
                        });

                    Frost::EventBus::Emit(BulletImpactEvent{
                        m_Registry.Get<Transform2D>(bullet).position
                        });

                    toDestroy.push_back(asteroid);
                    toDestroy.push_back(bullet);
                }
            }

            // Player <-> Asteroid
            if ((aIsPlayer && bIsAsteroid) || (bIsPlayer && aIsAsteroid))
            {

                Frost::Entity player = aIsPlayer ? a : b;
                Frost::Entity asteroid = aIsAsteroid ? a : b;

                // Check shield first
                if (m_Registry.Has<ShieldEffect>(player))
                {
                    auto& shield = m_Registry.Get<ShieldEffect>(player);
                    if (shield.active)
                    {
                        shield.active = false; // shield breaks
                        Frost::EventBus::Emit(ShieldBrokenEvent{ player });
                        FROST_LOG("Shield absorbed a hit!");
                        continue; // no health damage
                    }
                }

                // No shield — apply health damage
                auto& health = m_Registry.Get<Health>(player);
                if (health.iFrames > 0.0f) continue;
                // ... rest unchanged

                // Skip if player has invincibility frames
                

                // Deal damage based on asteroid size
                float damage = 20.0f;
                if (m_Registry.Has<AsteroidTag>(asteroid))
                {
                    auto& tag = m_Registry.Get<AsteroidTag>(asteroid);
                    switch (tag.size)
                    {
                    case AsteroidTag::Size::Large:  damage = 34.0f; break;
                    case AsteroidTag::Size::Medium: damage = 20.0f; break;
                    case AsteroidTag::Size::Small:  damage = 10.0f; break;
                    }
                }

                health.current -= damage;
                health.iFrames = health.iFrameMax;

                Frost::EventBus::Emit(PlayerDamagedEvent{
                    damage,
                    health.current
                    });

                
                if (health.current <= 0.0f)
                {
                    health.isDead = true;
                    Frost::EventBus::Emit(PlayerDiedEvent{ m_Score });
                }
            }
        }

        for (Frost::Entity e : toDestroy)
            m_Registry.DestroyDeferred(e);

        m_Collision.ClearPairs();
    }

    void SetScore(int score) { m_Score = score; }

private:
    Frost::Registry& m_Registry;
    CollisionSystem& m_Collision;
    int m_Score = 0;
};
#pragma once
#include "Frost.h"
#include "Components/GameComponents.h"
#include "Events/GameEvents.h"
#include <cmath>

class PowerUpSystem
{
public:
    PowerUpSystem(Frost::Registry& registry, Frost::Entity playerEntity)
        : m_Registry(registry)
        , m_PlayerEntity(playerEntity) {
    }

    void Update(float dt)
    {
        TickWorldPickups(dt);
        TickActiveEffects(dt);
        CheckPlayerPickup();
    }

private:
    
    void TickWorldPickups(float dt)
    {
        auto view = m_Registry.GetView<Transform2D, PowerUpTag>();
        for (auto entry : view)
        {
            Transform2D& t = std::get<0>(entry.components);
            t.rotation += 2.0f * dt; // slowly spin
        }
    }

    
    void TickActiveEffects(float dt)
    {
        if (!m_Registry.IsAlive(m_PlayerEntity)) return;

        // Shield — no tick needed, absorbed on hit

        // Rapid fire
        if (m_Registry.Has<RapidFireEffect>(m_PlayerEntity))
        {
            auto& rf = m_Registry.Get<RapidFireEffect>(m_PlayerEntity);
            rf.timeLeft -= dt;
            rf.cooldown = std::max(0.0f, rf.cooldown - dt);
            if (rf.timeLeft <= 0.0f)
            {
                m_Registry.Remove<RapidFireEffect>(m_PlayerEntity);
                Frost::EventBus::Emit(PowerUpExpiredEvent{
                    PowerUpType::RapidFire, m_PlayerEntity });
                FROST_LOG("RapidFire expired");
            }
        }

        // Speed boost
        if (m_Registry.Has<SpeedBoostEffect>(m_PlayerEntity))
        {
            auto& sb = m_Registry.Get<SpeedBoostEffect>(m_PlayerEntity);
            sb.timeLeft -= dt;
            if (sb.timeLeft <= 0.0f)
            {
                m_Registry.Remove<SpeedBoostEffect>(m_PlayerEntity);
                Frost::EventBus::Emit(PowerUpExpiredEvent{
                    PowerUpType::SpeedBoost, m_PlayerEntity });
                FROST_LOG("SpeedBoost expired");
            }
        }

        // Shield effect tick
        if (m_Registry.Has<ShieldEffect>(m_PlayerEntity))
        {
            auto& shield = m_Registry.Get<ShieldEffect>(m_PlayerEntity);
            shield.timeLeft -= dt;
            if (shield.timeLeft <= 0.0f || !shield.active)
            {
                m_Registry.Remove<ShieldEffect>(m_PlayerEntity);
                Frost::EventBus::Emit(PowerUpExpiredEvent{
                    PowerUpType::Shield, m_PlayerEntity });
                FROST_LOG("Shield expired");
            }
        }
    }

    
    void CheckPlayerPickup()
    {
        if (!m_Registry.IsAlive(m_PlayerEntity)) return;
        if (!m_Registry.Has<Transform2D>(m_PlayerEntity)) return;
        if (!m_Registry.Has<CircleCollider>(m_PlayerEntity)) return;

        auto& playerT = m_Registry.Get<Transform2D>(m_PlayerEntity);
        auto& playerC = m_Registry.Get<CircleCollider>(m_PlayerEntity);

        auto view = m_Registry.GetView<Transform2D, CircleCollider, PowerUpTag>();
        for (auto entry : view)
        {
            Frost::Entity e = entry.entity;
            Transform2D& t = std::get<0>(entry.components);
            CircleCollider& c = std::get<1>(entry.components);

            float dx = playerT.position.x - t.position.x;
            float dy = playerT.position.y - t.position.y;
            float distSq = dx * dx + dy * dy;
            float radSum = playerC.radius + c.radius;

            if (distSq < radSum * radSum)
            {
                // Collect
                auto& pickup = m_Registry.Get<PowerUp>(e);
                ApplyEffect(pickup.type, pickup.duration);
                m_Registry.DestroyDeferred(e);

                Frost::EventBus::Emit(PowerUpCollectedEvent{
                    pickup.type, m_PlayerEntity });
            }
        }
    }

    void ApplyEffect(PowerUpType type, float duration)
    {
        switch (type)
        {
        case PowerUpType::Shield:
            if (!m_Registry.Has<ShieldEffect>(m_PlayerEntity))
                m_Registry.Add<ShieldEffect>(m_PlayerEntity,
                    { true, duration });
            else
                m_Registry.Get<ShieldEffect>(m_PlayerEntity).timeLeft = duration;
            FROST_LOG("Shield activated!");
            break;

        case PowerUpType::RapidFire:
            if (!m_Registry.Has<RapidFireEffect>(m_PlayerEntity))
                m_Registry.Add<RapidFireEffect>(m_PlayerEntity,
                    { duration, 0.0f, 0.15f });
            else
                m_Registry.Get<RapidFireEffect>(m_PlayerEntity).timeLeft = duration;
            FROST_LOG("RapidFire activated!");
            break;

        case PowerUpType::SpeedBoost:
            if (!m_Registry.Has<SpeedBoostEffect>(m_PlayerEntity))
                m_Registry.Add<SpeedBoostEffect>(m_PlayerEntity,
                    { duration, 600.0f, 400.0f });
            else
                m_Registry.Get<SpeedBoostEffect>(m_PlayerEntity).timeLeft = duration;
            FROST_LOG("SpeedBoost activated!");
            break;
        }
    }

    Frost::Registry& m_Registry;
    Frost::Entity m_PlayerEntity;
};
#pragma once
#include "Frost.h"
#include "../Components/GameComponents.h"
#include "../Events/GameEvents.h"
#include "../SpriteSheet.h"
#include <cstdlib>
#include <cmath>

class AsteroidSpawner
{
public:
    AsteroidSpawner(Frost::Registry& registry, int screenW, int screenH)
        : m_Registry(registry)
        , m_ScreenW(screenW)
        , m_ScreenH(screenH) {
    }

    void Update(float dt)
    {
        m_SpawnTimer += dt;
        if (m_SpawnTimer >= m_SpawnInterval)
        {
            m_SpawnTimer = 0.0f;
            SpawnAtEdge();
            if (m_SpawnInterval > 0.5f)
                m_SpawnInterval -= 0.05f;
        }
    }

    void SpawnAt(Frost::vec2 position, AsteroidTag::Size size,
        Frost::vec2 direction = { 0.0f, 0.0f })
    {
        float scale, radius, speed;
        switch (size)
        {
        case AsteroidTag::Size::Large:
            scale = 60.0f; radius = 28.0f;
            speed = 80.0f + rand() % 80;  break;
        case AsteroidTag::Size::Medium:
            scale = 35.0f; radius = 16.0f;
            speed = 120.0f + rand() % 80; break;
        case AsteroidTag::Size::Small:
            scale = 20.0f; radius = 9.0f;
            speed = 160.0f + rand() % 80; break;
        }

        // random direction if not provided
        if (direction.x == 0.0f && direction.y == 0.0f)
        {
            float angle = ((float)(rand() % 360)) * Frost::DEG2RAD;
            direction = { std::cos(angle), std::sin(angle) };
        }

        // normalize
        float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (len > 0.0f) { direction.x /= len; direction.y /= len; }

        float angularVel = ((rand() % 200) - 100) / 100.0f;

        Sprite sprite;
        switch (size)
        {
        case AsteroidTag::Size::Large:
            sprite = GetSprite(SpriteID::ASTEROID_LARGE); break;
        case AsteroidTag::Size::Medium:
            sprite = GetRandomMediumAsteroid();            break;
        case AsteroidTag::Size::Small:
            sprite = GetRandomSmallAsteroid();             break;
        }

        Frost::Entity e = m_Registry.Create();

        m_Registry.Add<Transform2D>(e, {
            position, 0.0f, { scale, scale }
            });
        m_Registry.Add<Velocity2D>(e, {
            { direction.x * speed, direction.y * speed },
            angularVel
            });
        m_Registry.Add<Sprite>(e, sprite);
        m_Registry.Add<CircleCollider>(e, { radius });
        m_Registry.Add<AsteroidTag>(e, { size });
        m_Registry.Add<SpawnImmunity>(e, { 0.3f });
    }

    void Reset()
    {
        m_SpawnTimer = 0.0f;
        m_SpawnInterval = 2.0f;
    }

private:
    void SpawnAtEdge()
    {
        int edge = rand() % 4;
        Frost::vec2 pos{};

        switch (edge)
        {
        case 0: pos = { (float)(rand() % m_ScreenW), -80.0f }; break;
        case 1: pos = { (float)(rand() % m_ScreenW), m_ScreenH + 80.0f }; break;
        case 2: pos = { -80.0f,                    (float)(rand() % m_ScreenH) }; break;
        case 3: pos = { m_ScreenW + 80.0f,           (float)(rand() % m_ScreenH) }; break;
        }

        SpawnAt(pos, AsteroidTag::Size::Large);
    }

    Frost::Registry& m_Registry;
    int   m_ScreenW;
    int   m_ScreenH;
    float m_SpawnTimer = 0.0f;
    float m_SpawnInterval = 2.0f;
};
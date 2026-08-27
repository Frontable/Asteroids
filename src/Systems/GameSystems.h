#pragma once
#include "Frost.h"
#include "../Components/GameComponents.h"


class MovementSystem
{
public:
    MovementSystem(Frost::Registry& registry) : m_Registry(registry) {}

    void Update(float dt)
    {
        auto view = m_Registry.GetView<Transform2D, Velocity2D>();
        for (auto entry : view)
        {
            Transform2D& transform = std::get<0>(entry.components);
            Velocity2D& velocity = std::get<1>(entry.components);

            transform.position.x += velocity.velocity.x * dt;
            transform.position.y += velocity.velocity.y * dt;
            transform.rotation += velocity.angularVelocity * dt;
        }
    }

private:
    Frost::Registry& m_Registry;
};


class ScreenWrapSystem
{
public:
    ScreenWrapSystem(Frost::Registry& registry, int w, int h)
        : m_Registry(registry), m_W(w), m_H(h) {
    }

    void Update(float dt)
    {
        auto view = m_Registry.GetView<Transform2D>();
        for (auto entry : view)
        {
            
            Transform2D& t = std::get<0>(entry.components);

            float hw = t.scale.x * 0.5f;
            float hh = t.scale.y * 0.5f;

            if (t.position.x + hw < 0)         t.position.x = m_W + hw;
            else if (t.position.x - hw > m_W)  t.position.x = -hw;
            if (t.position.y + hh < 0)         t.position.y = m_H + hh;
            else if (t.position.y - hh > m_H)  t.position.y = -hh;
        }
    }

private:
    Frost::Registry& m_Registry;
    int m_W, m_H;
};


class LifetimeSystem
{
public:
    LifetimeSystem(Frost::Registry& registry) : m_Registry(registry) {}

    void Update(float dt)
    {
        auto view = m_Registry.GetView<Lifetime>();
        for (auto entry : view)
        {
            Frost::Entity e = entry.entity;
            Lifetime& life = std::get<0>(entry.components);

            life.timeLeft -= dt;
            if (life.timeLeft <= 0.0f)
                m_Registry.DestroyDeferred(e);
        }
    }

private:
    Frost::Registry& m_Registry;
};


class SpawnImmunitySystem
{
public:
    SpawnImmunitySystem(Frost::Registry& registry) : m_Registry(registry) {}

    void Update(float dt)
    {
        std::vector<Frost::Entity> toRemove;

        auto view = m_Registry.GetView<SpawnImmunity>();
        for (auto entry : view)
        {
            Frost::Entity  e = entry.entity;
            SpawnImmunity& immunity = std::get<0>(entry.components);

            immunity.timeLeft -= dt;
            if (immunity.timeLeft <= 0.0f)
                toRemove.push_back(e);
        }

        for (Frost::Entity e : toRemove)
            m_Registry.Remove<SpawnImmunity>(e);
    }

private:
    Frost::Registry& m_Registry;
};


class FlickerSystem
{
public:
    FlickerSystem(Frost::Registry& registry) : m_Registry(registry) {}

    void Update(float dt)
    {
        auto view = m_Registry.GetView<Flicker>();
        for (auto entry : view)
        {
            Flicker& flicker = std::get<0>(entry.components);
            flicker.timer += dt;
            if (flicker.timer >= flicker.interval)
            {
                flicker.timer = 0.0f;
                flicker.visible = !flicker.visible;
            }
        }
    }

private:
    Frost::Registry& m_Registry;
};

class IFrameSystem
{
public:
    IFrameSystem(Frost::Registry& registry) : m_Registry(registry) {}

    void Update(float dt)
    {
        auto view = m_Registry.GetView<Health>();
        for (auto entry : view)
        {
            Health& health = std::get<0>(entry.components);
            if (health.iFrames > 0.0f)
                health.iFrames -= dt;
        }
    }

private:
    Frost::Registry& m_Registry;
};
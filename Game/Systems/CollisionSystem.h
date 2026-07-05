#pragma once
#include "Frost.h"
#include "../Components/GameComponents.h"
#include "../Events/GameEvents.h"

class CollisionSystem
{
public:
    CollisionSystem(Frost::Registry& registry) : m_Registry(registry) {}

    void Update(float dt)
    {
        std::vector<Frost::Entity> entities;
        auto view = m_Registry.GetView<Transform2D, CircleCollider>();
        for (auto entry : view)
            entities.push_back(entry.entity);

        for (size_t i = 0; i < entities.size(); i++)
        {
            for (size_t j = i + 1; j < entities.size(); j++)
            {
                Frost::Entity a = entities[i];
                Frost::Entity b = entities[j];

                // skip immune entities
                if (m_Registry.Has<SpawnImmunity>(a)) continue;
                if (m_Registry.Has<SpawnImmunity>(b)) continue;

                auto& tA = m_Registry.Get<Transform2D>(a);
                auto& cA = m_Registry.Get<CircleCollider>(a);
                auto& tB = m_Registry.Get<Transform2D>(b);
                auto& cB = m_Registry.Get<CircleCollider>(b);

                if (Overlaps(tA.position, cA.radius,
                    tB.position, cB.radius))
                {
                    m_CollisionPairs.push_back({ a, b });
                }
            }
        }
    }

    std::vector<std::pair<Frost::Entity, Frost::Entity>>& GetPairs()
    {
        return m_CollisionPairs;
    }

    void ClearPairs() { m_CollisionPairs.clear(); }

private:
    bool Overlaps(const Frost::vec2& posA, float rA,
        const Frost::vec2& posB, float rB)
    {
        float dx = posA.x - posB.x;
        float dy = posA.y - posB.y;
        float distSq = dx * dx + dy * dy;
        float radSum = rA + rB;
        return distSq < radSum * radSum;
    }

    Frost::Registry& m_Registry;
    std::vector<std::pair<Frost::Entity, Frost::Entity>> m_CollisionPairs;
};
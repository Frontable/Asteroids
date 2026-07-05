#pragma once
#include "Frost.h"
#include "../Components/GameComponents.h"
#include "../SpriteSheet.h"

class RenderSystem
{
public:
    RenderSystem(Frost::Registry& registry,
        Frost::BatchRenderer& renderer,
        Frost::Texture* atlas,
        Frost::Texture* white)
        : m_Registry(registry)
        , m_Renderer(renderer)
        , m_Atlas(atlas)
        , m_White(white) {
    }

    void Update(float dt)
    {
        // collect entities with sort before particles
        std::vector<Frost::Entity> entities;
        auto view = m_Registry.GetView<Transform2D, Sprite>();
        for (auto entry : view)
            entities.push_back(entry.entity);

        std::sort(entities.begin(), entities.end(),
            [&](Frost::Entity a, Frost::Entity b) {
                // non-flickering entities first
                bool aFlicker = m_Registry.Has<Flicker>(a);
                bool bFlicker = m_Registry.Has<Flicker>(b);
                return !aFlicker && bFlicker;
            });

        for (Frost::Entity e : entities)
        {
            auto& transform = m_Registry.Get<Transform2D>(e);
            auto& sprite = m_Registry.Get<Sprite>(e);

            // skip invisible flicker entities
            if (m_Registry.Has<Flicker>(e))
            {
                auto& flicker = m_Registry.Get<Flicker>(e);
                if (!flicker.visible) continue;
            }

            vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

            m_Renderer.DrawSprite(
                { transform.position.x, transform.position.y, 0.0f },
                transform.scale,
                transform.rotation,
                { sprite.uv, sprite.size },
                m_Atlas,
                ATLAS_WIDTH, ATLAS_HEIGHT,
                color
            );
        }
    }

private:
    using vec4 = Frost::vec4;

    Frost::Registry& m_Registry;
    Frost::BatchRenderer& m_Renderer;
    Frost::Texture* m_Atlas;
    Frost::Texture* m_White;
};
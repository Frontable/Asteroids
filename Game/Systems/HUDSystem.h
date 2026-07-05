#pragma once
#include "Frost.h"
#include "Components/GameComponents.h"

class HUDSystem
{
public:
    HUDSystem(Frost::Registry& registry, int screenW, int screenH)
        : m_Registry(registry)
        , m_ScreenW(screenW)
        , m_ScreenH(screenH) {
    }

    // call after main render pass with UI projection
    void Render(Frost::BatchRenderer& renderer,
        Frost::Texture* whiteTex,
        Frost::Shader* shader,
        const Frost::mat4& uiProjection,
        Frost::Entity         playerEntity,
        int                   score)
    {
        if (!m_Registry.IsAlive(playerEntity)) return;
        if (!m_Registry.Has<Health>(playerEntity))     return;
        if (!m_Registry.Has<Experience>(playerEntity)) return;

        auto& health = m_Registry.Get<Health>(playerEntity);
        auto& xp = m_Registry.Get<Experience>(playerEntity);

        shader->Bind();
        shader->SetMat4("uProjection", uiProjection);
        shader->SetInt("uUseTexture", 0); // colored quads

        renderer.Begin(shader, uiProjection);

        // hp bar
        DrawQuad(renderer, whiteTex,
            { 20.0f, 20.0f },
            { BAR_W, BAR_H },
            { 0.2f, 0.2f, 0.2f, 0.8f });

        // filling
        float hp = health.Percent();
        Frost::vec4 hpColor = HealthColor(hp);
        DrawQuad(renderer, whiteTex,
            { 20.0f, 20.0f },
            { BAR_W * hp, BAR_H },
            hpColor);

        // xp bar
        float xpPercent = (float)xp.current / (float)xp.threshold;

        // Background
        DrawQuad(renderer, whiteTex,
            { 20.0f, 20.0f + BAR_H + BAR_GAP },
            { BAR_W, BAR_H * 0.6f },
            { 0.2f, 0.2f, 0.2f, 0.8f });

        // filling
        DrawQuad(renderer, whiteTex,
            { 20.0f, 20.0f + BAR_H + BAR_GAP },
            { BAR_W * xpPercent, BAR_H * 0.6f },
            { 0.4f, 0.6f, 1.0f, 0.9f });

        renderer.End();
    }

  
    void RenderText(Frost::TextRenderer& text,
        const Frost::mat4& uiProjection,
        Frost::Entity         playerEntity,
        int                   score)
    {
        if (!m_Registry.IsAlive(playerEntity)) return;

        auto& health = m_Registry.Get<Health>(playerEntity);
        auto& xp = m_Registry.Get<Experience>(playerEntity);

        // hp label
        std::string hpStr = "HP  " +
            std::to_string((int)health.current) + " / " +
            std::to_string((int)health.max);
        text.RenderText(hpStr, 20.0f, 20.0f, 0.7f,
            { 1.0f, 1.0f, 1.0f, 1.0f }, uiProjection);

        // xp n level label
        std::string xpStr = "LVL " + std::to_string(xp.level) +
            "   XP " + std::to_string(xp.current) +
            " / " + std::to_string(xp.threshold);
        text.RenderText(xpStr, 20.0f, 20.0f + BAR_H + BAR_GAP, 0.55f,
            { 0.7f, 0.8f, 1.0f, 1.0f }, uiProjection);

        // score
        std::string scoreStr = "SCORE  " + std::to_string(score);
        text.RenderText(scoreStr,
            m_ScreenW - 220.0f, 20.0f, 0.7f,
            { 1.0f, 1.0f, 1.0f, 1.0f }, uiProjection);
    }

private:
    void DrawQuad(Frost::BatchRenderer& renderer,
        Frost::Texture* tex,
        Frost::vec2           pos,
        Frost::vec2           size,
        Frost::vec4           color)
    {
        renderer.DrawSprite(
            { pos.x + size.x * 0.5f, pos.y + size.y * 0.5f, 0.0f },
            size,
            0.0f,
            { {0,0}, {1,1} },  // white pixel UVs
            tex,
            1.0f, 1.0f,
            color
        );
    }

    Frost::vec4 HealthColor(float percent)
    {
        if (percent > 0.6f) return { 0.2f, 0.85f, 0.2f, 0.9f }; // green
        if (percent > 0.3f) return { 1.0f, 0.75f, 0.0f, 0.9f }; // yellow
        return               { 0.9f, 0.15f, 0.15f, 0.9f };       // red
    }

    Frost::Registry& m_Registry;
    int m_ScreenW, m_ScreenH;

    static constexpr float BAR_W = 200.0f;
    static constexpr float BAR_H = 16.0f;
    static constexpr float BAR_GAP = 6.0f;
};
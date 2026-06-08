#pragma once
#include "Application.h"
#include "Math/FrostMath.h"
#include "Rendering/Core/SpriteBatchRenderer.h"
#include "Rendering/Essentials/ShaderLoader.h"
#include "Rendering/Essentials/TextureLoader.h"
#include "Rendering/Core/TextRenderer.h"
#include "Rendering/Core/DebugRenderer.h"
#include "SpriteSheet.h"
#include <memory>
#include "ECS/ECS.h"

enum class GameState { Playing, GameOver };

class Editor : public Application
{
public:
    Editor();
    Editor(const char* title, int width, int height);
    ~Editor() = default;

private:
    void Init()           override;
    void Input(float dt)  override;
    void Update(float dt) override;
    void Render(float dt) override;
    void Clean()          override;

    void loadSystems();
    void SpawnPlayer();
    void RestartGame();
    void wireDamageCallbacks();

    // Rendering
    std::unique_ptr<SpriteBatchRenderer>    m_Renderer;
    std::shared_ptr<FrostEngine::Shader>    m_Shader;
    std::shared_ptr<FrostEngine::Texture>   m_Texture;
    FrostEngine::TextRenderer               m_TextRenderer;
    FrostEngine::DebugRenderer              m_DebugRenderer;
    mat4                                    m_UIProjection;

    // Game state
    GameState m_GameState = GameState::Playing;
    Entity    m_PlayerEntity = 0;
    Entity    m_ThrusterEntity = 0;
    int       m_Score = 0;
    bool      m_DebugDraw = false;
    bool      m_Thrusting = false;
};
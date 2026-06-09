#include "Editor.h"
#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "Rendering/Core/Camera2D.h"
#include "Systems/PhysicsSystems/MovementSystem.h"
#include "Systems/PlayerSystems/PlayerSystem.h"
#include "Systems/UtilitySystems/LifetimeSystem.h"
#include "Systems/PlayerSystems/ShootingSystem.h"
#include "Systems/PhysicsSystems/CollisionSystem.h"
#include "Systems/AsteroidsSystems/DamageSystem.h"
#include "Systems/AsteroidsSystems/AsteroidSpawnerSystem.h"
#include "Systems/UtilitySystems/SpawnImmunitySystem.h"
#include "Systems/RenderSystem.h"
#include "Systems/PhysicsSystems/ScreenWrapSystem.h"
#include "Systems/UtilitySystems/ScoreSystem.h"
#include "Systems/UtilitySystems/FlickerSystem.h"
#include "SpriteSheet.h"
#include "Platform/Input.h"
#include <iostream>
#include <cmath>

Editor::Editor()
    : Editor("Asteroids", 1280, 720)
{
    //Init();
}

Editor::Editor(const char* _title, int _width, int _height)
    : Application{ _title, _width, _height }
{
}

void Editor::Init()
{
    
    m_MainContext.Add<FrostEngine::Camera2D>(1280, 720);

    
    m_MainContext.Add<FrostEngine::ECS>();
    auto& ecs = m_MainContext.Get<FrostEngine::ECS>();
    ecs.RegisterComponent<Transform2D>();
    ecs.RegisterComponent<Velocity2D>();
    ecs.RegisterComponent<Sprite>();
    ecs.RegisterComponent<CircleCollider>();
    ecs.RegisterComponent<Lifetime>();
    ecs.RegisterComponent<PlayerTag>();
    ecs.RegisterComponent<AsteroidTag>();
    ecs.RegisterComponent<BulletTag>();
    ecs.RegisterComponent<SpawnImmunity>();
    ecs.RegisterComponent<Flicker>();

    
    m_Renderer = std::make_unique<SpriteBatchRenderer>();

    
    m_Shader = FrostEngine::ShaderLoader::Create(
        "Assets/Shaders/basicV.glsl",
        "Assets/Shaders/basicF.glsl"
    );

    
    m_Texture = FrostEngine::TextureLoader::Create(
        FrostEngine::Texture::TextureType::BLENDED,
        "Assets/images/asteroids.png"
    );

    
    m_UIProjection = ortho(0.0f, 1280.0f, 720.0f, 0.0f);

    m_DebugRenderer.Init("Assets/Shaders/debugV.glsl", "Assets/Shaders/debugF.glsl");

    m_TextRenderer.Init("Assets/fonts/Cousine/Cousine-Regular.ttf", "Assets/Shaders/textV.glsl", "Assets/Shaders/textF.glsl", 32);

    
    loadSystems();
    SpawnPlayer();
}

void Editor::SpawnPlayer()
{
    auto& ecs = m_MainContext.Get<FrostEngine::ECS>();
    m_PlayerEntity = ecs.CreateEntity();

    ecs.AddComponent<Transform2D>(m_PlayerEntity, {
        { 640.0f, 360.0f }, 0.0f, { 60.0f, 60.0f }
        });
    ecs.AddComponent<Velocity2D>(m_PlayerEntity, { {0.0f, 0.0f}, 0.0f });
    ecs.AddComponent<Sprite>(m_PlayerEntity, GetSprite(SpriteID::PLAYER));
    ecs.AddComponent<CircleCollider>(m_PlayerEntity, { 25.0f });
    ecs.AddComponent<PlayerTag>(m_PlayerEntity, {});
    ecs.AddComponent<SpawnImmunity>(m_PlayerEntity, { 2.0f });

    
    m_ThrusterEntity = ecs.CreateEntity();
    ecs.AddComponent<Transform2D>(m_ThrusterEntity, {
        { 640.0f, 360.0f }, 0.0f, { 18.0f, 14.0f }
        });
    ecs.AddComponent<Sprite>(m_ThrusterEntity, GetSprite(SpriteID::PLAYER_THRUSTER));
    ecs.AddComponent<Flicker>(m_ThrusterEntity, {});
}


void Editor::Input(float dt)
{
    if (m_GameState == GameState::GameOver)
    {
        if (Input::IsKeyJustPressed(GLFW_KEY_R))
            RestartGame();
        return;
    }

    // Toggle debug draw with F1
    if (Input::IsKeyJustPressed(GLFW_KEY_F1))
        m_DebugDraw = !m_DebugDraw;
}

void Editor::Update(float dt)
{
    if (m_GameState == GameState::GameOver)
        return;

    auto& ecs = m_MainContext.Get<FrostEngine::ECS>();

    ecs.getSystem<PlayerSystem>().Update(dt);
    ecs.getSystem<ShootingSystem>().Update(dt);
    ecs.getSystem<MovementSystem>().Update(dt);
    ecs.getSystem<ScreenWrapSystem>().Update(dt);
    ecs.getSystem<AsteroidSpawnerSystem>().Update(dt);
    ecs.getSystem<SpawnImmunitySystem>().Update(dt);
    ecs.getSystem<CollisionSystem>().Update(dt);
    ecs.getSystem<DamageSystem>().Update(dt);
    ecs.getSystem<LifetimeSystem>().Update(dt);

    
    bool thrusting = ecs.getSystem<PlayerSystem>().IsThrusting;
    auto& playerT = ecs.GetComponent<Transform2D>(m_PlayerEntity);
    auto& thrusterT = ecs.GetComponent<Transform2D>(m_ThrusterEntity);

    // Offset thruster behind the ship based on player rotation
    float offsetDist = 35.0f;
    float angle = playerT.rotation - PI / 2.0f;
    thrusterT.position = {
        playerT.position.x - cos(angle) * offsetDist,
        playerT.position.y - sin(angle) * offsetDist
    };
    thrusterT.rotation = playerT.rotation;

    // Only flicker when thrusting
    if (thrusting)
        ecs.getSystem<FlickerSystem>().Update(dt);
    else
    {
        
        auto& flicker = ecs.GetComponent<Flicker>(m_ThrusterEntity);
        flicker.visible = false;
        flicker.timer = 0.0f;
    }
}

void Editor::Render(float dt)
{
    m_Shader.get()->Bind();
    auto& camera = m_MainContext.Get<FrostEngine::Camera2D>();
    camera.Update();
    m_Shader.get()->SetMatrix4x4("uProjection", camera.GetCameraMatrix());
    m_Shader.get()->SetUniformInt("useTexture", true);

    glActiveTexture(GL_TEXTURE0);
    m_Shader.get()->SetUniformInt("tex", 0);
    m_Texture->Bind();

    auto& ecs = m_MainContext.Get<FrostEngine::ECS>();
    ecs.getSystem<RenderSystem>().Update(dt);
    m_Renderer.get()->Render();

    // Draw score (always visible during play)
    if (m_GameState == GameState::Playing)
    {
        m_TextRenderer.RenderText(
            "SCORE: " + std::to_string(m_Score),
            20.0f, 20.0f, 1.0f,
            { 1.0f, 1.0f, 1.0f, 1.0f },
            m_UIProjection
        );
    }

    // Game over overlay
    if (m_GameState == GameState::GameOver)
    {
        m_TextRenderer.RenderText(
            "GAME OVER",
            440.0f, 300.0f, 1.5f,
            { 1.0f, 0.2f, 0.2f, 1.0f },
            m_UIProjection
        );
        m_TextRenderer.RenderText(
            "SCORE: " + std::to_string(m_Score),
            490.0f, 370.0f, 1.0f,
            { 1.0f, 1.0f, 1.0f, 1.0f },
            m_UIProjection
        );
        m_TextRenderer.RenderText(
            "PRESS R TO RESTART",
            390.0f, 430.0f, 1.0f,
            { 0.8f, 0.8f, 0.8f, 1.0f },
            m_UIProjection
        );
    }
    // Debug hitboxes
    if (m_DebugDraw)
    {
        auto& ecs = m_MainContext.Get<FrostEngine::ECS>();
        auto& camera = m_MainContext.Get<FrostEngine::Camera2D>();

        
        for (auto& sys : { &ecs.getSystem<CollisionSystem>() })
        {
            for (Entity e : sys->m_entities)
            {
                auto& transform = ecs.GetComponent<Transform2D>(e);
                auto& collider = ecs.GetComponent<CircleCollider>(e);

                // Color code by type
                vec4 color = { 0.0f, 1.0f, 0.0f, 1.0f }; // green = default
                if (ecs.HasComponent<PlayerTag>(e))
                    color = { 0.0f, 0.5f, 1.0f, 1.0f };  // blue = player
                if (ecs.HasComponent<BulletTag>(e))
                    color = { 1.0f, 1.0f, 0.0f, 1.0f };  // yellow = bullet
                if (ecs.HasComponent<AsteroidTag>(e))
                    color = { 1.0f, 0.3f, 0.0f, 1.0f };  // orange = asteroid

                m_DebugRenderer.DrawCircle(transform.position, collider.radius, color);
            }
        }

        m_DebugRenderer.Flush(camera.GetCameraMatrix());
    }
}

void Editor::Clean() {}

void Editor::loadSystems()
{
    auto& ecs = m_MainContext.Get<FrostEngine::ECS>();

    // Movement
    ecs.RegisterSystem<MovementSystem>(m_MainContext);
    Signature moveSignature;
    moveSignature.set(ecs.GetComponentID<Transform2D>());
    moveSignature.set(ecs.GetComponentID<Velocity2D>());
    ecs.SetSystemSignature<MovementSystem>(moveSignature);

    // Player
    ecs.RegisterSystem<PlayerSystem>(m_MainContext);
    Signature playerSignature;
    playerSignature.set(ecs.GetComponentID<Transform2D>());
    playerSignature.set(ecs.GetComponentID<Velocity2D>());
    playerSignature.set(ecs.GetComponentID<PlayerTag>());
    ecs.SetSystemSignature<PlayerSystem>(playerSignature);

    // Shooting
    ecs.RegisterSystem<ShootingSystem>(m_MainContext);
    Signature shootSignature;
    shootSignature.set(ecs.GetComponentID<Transform2D>());
    shootSignature.set(ecs.GetComponentID<Velocity2D>());
    shootSignature.set(ecs.GetComponentID<PlayerTag>());
    ecs.SetSystemSignature<ShootingSystem>(shootSignature);

    // Lifetime
    ecs.RegisterSystem<LifetimeSystem>(m_MainContext);
    Signature lifeSignature;
    lifeSignature.set(ecs.GetComponentID<Lifetime>());
    ecs.SetSystemSignature<LifetimeSystem>(lifeSignature);

    // Collision
    ecs.RegisterSystem<CollisionSystem>(m_MainContext);
    Signature collisionSignature;
    collisionSignature.set(ecs.GetComponentID<Transform2D>());
    collisionSignature.set(ecs.GetComponentID<CircleCollider>());
    ecs.SetSystemSignature<CollisionSystem>(collisionSignature);

    // Damage
    ecs.RegisterSystem<DamageSystem>(m_MainContext);

    // Screen wrap
    ecs.RegisterSystem<ScreenWrapSystem>(m_MainContext, 1280, 720);
    Signature wrapSignature;
    wrapSignature.set(ecs.GetComponentID<Transform2D>());
    ecs.SetSystemSignature<ScreenWrapSystem>(wrapSignature);

    // Spawn immunity
    ecs.RegisterSystem<SpawnImmunitySystem>(m_MainContext);
    Signature immunitySignature;
    immunitySignature.set(ecs.GetComponentID<SpawnImmunity>());
    ecs.SetSystemSignature<SpawnImmunitySystem>(immunitySignature);

    // Render
    ecs.RegisterSystem<RenderSystem>(m_MainContext, m_Renderer.get(), ATLAS_WIDTH, ATLAS_HEIGHT);
    Signature renderSignature;
    renderSignature.set(ecs.GetComponentID<Transform2D>());
    renderSignature.set(ecs.GetComponentID<Sprite>());
    ecs.SetSystemSignature<RenderSystem>(renderSignature);

    // Score
    ecs.RegisterSystem<ScoreSystem>(m_MainContext);

    // Asteroid spawner
    ecs.RegisterSystem<AsteroidSpawnerSystem>(m_MainContext, 1280, 720);

    ecs.RegisterSystem<FlickerSystem>(m_MainContext);
    Signature flickerSignature;
    flickerSignature.set(ecs.GetComponentID<Flicker>());
    ecs.SetSystemSignature<FlickerSystem>(flickerSignature);

    // Wire up damage callbacks
    auto& damage = ecs.getSystem<DamageSystem>();
    auto& spawner = ecs.getSystem<AsteroidSpawnerSystem>();

    wireDamageCallbacks();
}

void Editor::wireDamageCallbacks()
{
    auto& ecs = m_MainContext.Get<FrostEngine::ECS>();
    auto& damage = ecs.getSystem<DamageSystem>();
    auto& spawner = ecs.getSystem<AsteroidSpawnerSystem>();

    damage.OnGameOver = [this]() {
        m_GameState = GameState::GameOver;
        std::cout << "Game Over!\n";
        };

    damage.OnAsteroidDestroyed = [this, &spawner](vec2 position, AsteroidTag::Size size, vec2 velocity)
        {
            switch (size)
            {
            case AsteroidTag::Size::Large:  m_Score += 100; break;
            case AsteroidTag::Size::Medium: m_Score += 50;  break;
            case AsteroidTag::Size::Small:  m_Score += 25;  break;
            }
            float len = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            vec2 dir = (len > 0.0f)
                ? vec2{ velocity.x / len, velocity.y / len }
            : vec2{ 1.0f, 0.0f };

            vec2 perp = { -dir.y, dir.x };

            if (size == AsteroidTag::Size::Large)
            {
                spawner.SpawnAt(position, AsteroidTag::Size::Medium, { dir.x + perp.x, dir.y + perp.y });
                spawner.SpawnAt(position, AsteroidTag::Size::Medium, { dir.x - perp.x, dir.y - perp.y });
            }
            else if (size == AsteroidTag::Size::Medium)
            {
                spawner.SpawnAt(position, AsteroidTag::Size::Small, { dir.x,   dir.y });
                spawner.SpawnAt(position, AsteroidTag::Size::Small, { -dir.x,  -dir.y });
                spawner.SpawnAt(position, AsteroidTag::Size::Small, { perp.x,  perp.y });
                spawner.SpawnAt(position, AsteroidTag::Size::Small, { -perp.x, -perp.y });
            }
        };
}

void Editor::RestartGame()
{
    m_Score = 0;
    auto& ecs = m_MainContext.Get<FrostEngine::ECS>();
    ecs.Reset();
    ecs.getSystem<AsteroidSpawnerSystem>().Reset();
    wireDamageCallbacks();
    SpawnPlayer();
    m_GameState = GameState::Playing;
}
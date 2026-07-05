#pragma once
#include "Frost.h"
#include "../Components/GameComponents.h"
#include "../Events/GameEvents.h"
#include "../SpriteSheet.h"
#include "../Scripts/PlayerScript.h"
#include "../Scripts/ShootingScript.h"
#include "../Systems/GameSystems.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/AsteroidSpawner.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/HUDSystem.h"
#include "../Systems/ExperienceSystem.h"
#include "../src/Assets/AssetLoader.h"

class AsteroidsScene : public Frost::Scene
{
public:
    AsteroidsScene()
        : Frost::Scene("Asteroids")
    {
    }

    void OnInit() override
    {
        m_Movement = std::make_unique<MovementSystem>(*m_Registry);
        m_ScreenWrap = std::make_unique<ScreenWrapSystem>(*m_Registry, 1280, 720);
        m_Lifetime = std::make_unique<LifetimeSystem>(*m_Registry);
        m_SpawnImmunity = std::make_unique<SpawnImmunitySystem>(*m_Registry);
        m_Flicker = std::make_unique<FlickerSystem>(*m_Registry);
        m_Collision = std::make_unique<CollisionSystem>(*m_Registry);
        m_Damage = std::make_unique<DamageSystem>(*m_Registry, *m_Collision);
        m_Spawner = std::make_unique<AsteroidSpawner>(*m_Registry, 1280, 720);
        m_IFrames = std::make_unique<IFrameSystem>(*m_Registry);
        m_HUD = std::make_unique<HUDSystem>(*m_Registry, 1280, 720);
        m_XPSystem = std::make_unique<ExperienceSystem>(*m_Registry, m_PlayerEntity);

        // text renderer for HUD labels
        m_TextRenderer.Init("Assets/fonts/Cousine/Cousine-Regular.ttf", "Assets/Shaders/textV.glsl", "Assets/Shaders/textF.glsl", 24);

        // fixed UI projection — not affected by camera
        m_UIProjection = Frost::ortho(0.0f, 1280.0f, 720.0f, 0.0f);

        FROST_LOG("AsteroidsScene initializing");

        // load assets
        auto& app = Frost::Application::Get();
        m_Atlas = Frost::AssetLoader::LoadTexture("atlas", "Assets/Images/asteroids.png");
        m_Shader = Frost::AssetLoader::LoadShader("gae", "Assets/Shaders/V.glsl", "Assets/Shaders/F.glsl");

        // render system
       m_RenderSystem = std::make_unique<RenderSystem>(
            *m_Registry,
            app.GetRenderer(),
            m_Atlas,
            app.GetWhiteTex()
        );

        m_DebugRenderer.Init("Assets/Shaders/debugV.glsl", "Assets/Shaders/debugF.glsl");

        // spawn player
        SpawnPlayer();

        // wire events
        WireEvents();

        FROST_LOG("AsteroidsScene ready");
    }


    void OnUpdate(float dt) override
    {
        if (Frost::Input::IsKeyJustPressed(GLFW_KEY_F1))
            m_DebugDraw = !m_DebugDraw;

        m_SpawnImmunity->Update(dt);
        m_Spawner->Update(dt);
        m_Movement->Update(dt);
        m_ScreenWrap->Update(dt);
        m_Flicker->Update(dt);
        m_Collision->Update(dt);
        m_Damage->Update(dt);
        m_Lifetime->Update(dt);
        m_IFrames->Update(dt);
    }

    void OnRender() override
    {
        auto& app = Frost::Application::Get();
        auto& renderer = app.GetRenderer();
        auto& camera = app.GetCamera();

        m_Shader->Bind();
        m_Shader->SetInt("uUseTexture", 1);

        renderer.Begin(m_Shader, camera.GetViewProjection());
        m_RenderSystem->Update(0.0f);
        renderer.End();

        // HUD bars
        m_HUD->Render(
            app.GetRenderer(),
            app.GetWhiteTex(),
            m_Shader,
            m_UIProjection,
            m_PlayerEntity,
            m_Score
        );

        // HUD text
        m_HUD->RenderText(
            m_TextRenderer,
            m_UIProjection,
            m_PlayerEntity,
            m_Score
        );

        if (m_DebugDraw)
        {
            auto view = m_Registry->GetView<Transform2D, CircleCollider>();
            for (auto entry : view)
            {
                Frost::Entity    e = entry.entity;
                Transform2D& transform = std::get<0>(entry.components);
                CircleCollider& collider = std::get<1>(entry.components);

                // Color by tag 4 now
                Frost::vec4 color = { 0.0f, 1.0f, 0.0f, 1.0f }; // green default

                if (m_Registry->Has<PlayerTag>(e))
                    color = { 0.0f, 0.5f, 1.0f, 1.0f };  // blue 4 plauer
                else if (m_Registry->Has<BulletTag>(e))
                    color = { 1.0f, 1.0f, 0.0f, 1.0f };  // yellow bullet
                else if (m_Registry->Has<AsteroidTag>(e))
                    color = { 1.0f, 0.3f, 0.0f, 1.0f };  // orange — assteroid

                // dim immune entities
                if (m_Registry->Has<SpawnImmunity>(e))
                    color.a = 0.3f;

                m_DebugRenderer.DrawCircle(transform.position, collider.radius, color);
            }

            m_DebugRenderer.Flush(
                Frost::Application::Get().GetCamera().GetViewProjection()
            );
        }

    }

    void OnShutdown() override
    {
        FROST_LOG("AsteroidsScene shutting down");
    }

private:
    void SpawnPlayer()
    {
        m_PlayerEntity = CreateEntity();

        AddComponent<Transform2D>(m_PlayerEntity, {
            { 640.0f, 360.0f }, 0.0f, { 60.0f, 60.0f }
            });
        AddComponent<Velocity2D>(m_PlayerEntity, {});
        AddComponent<Sprite>(m_PlayerEntity, GetSprite(SpriteID::PLAYER));
        AddComponent<CircleCollider>(m_PlayerEntity, { 25.0f });
        AddComponent<PlayerTag>(m_PlayerEntity, {});
        AddComponent<PlayerState>(m_PlayerEntity, PlayerState{
            false,  // isThrusting
            false   // isShooting
            });
        AddComponent<SpawnImmunity>(m_PlayerEntity, { 2.0f });

        // player thruster 
        m_ThrusterEntity = CreateEntity();
        AddComponent<Transform2D>(m_ThrusterEntity, {
            { 640.0f, 360.0f }, 0.0f, { 18.0f, 14.0f }
            });
        AddComponent<Sprite>(m_ThrusterEntity,
            GetSprite(SpriteID::PLAYER_THRUSTER));
        AddComponent<Flicker>(m_ThrusterEntity, {});

        
        PlayerScript* playerScript = AttachScript<PlayerScript>(m_PlayerEntity);
        AttachScript<ShootingScript>(m_PlayerEntity);

        
        playerScript->SetThrusterEntity(m_ThrusterEntity);

        AddComponent<Health>(m_PlayerEntity, { 100.0f, 100.0f });
        AddComponent<Experience>(m_PlayerEntity, {});
    }

    void WireEvents()
    {
        Frost::EventBus::On<AsteroidDestroyedEvent>(
            [this](const AsteroidDestroyedEvent& e)
            {
                // Score
                switch (e.size)
                {
                case AsteroidTag::Size::Large:  m_Score += 100; break;
                case AsteroidTag::Size::Medium: m_Score += 50;  break;
                case AsteroidTag::Size::Small:  m_Score += 25;  break;
                }

                // XP
                m_XPSystem->AwardXP(e.size);

                float len = std::sqrt(e.velocity.x * e.velocity.x
                    + e.velocity.y * e.velocity.y);
                Frost::vec2 dir = (len > 0.0f)
                    ? Frost::vec2{ e.velocity.x / len, e.velocity.y / len }
                : Frost::vec2{ 1.0f, 0.0f };

                Frost::vec2 perp = { -dir.y, dir.x };

                if (e.size == AsteroidTag::Size::Large)
                {
                    m_Spawner->SpawnAt(e.position,
                        AsteroidTag::Size::Medium,
                        { dir.x + perp.x, dir.y + perp.y });
                    m_Spawner->SpawnAt(e.position,
                        AsteroidTag::Size::Medium,
                        { dir.x - perp.x, dir.y - perp.y });
                }
                else if (e.size == AsteroidTag::Size::Medium)
                {
                    m_Spawner->SpawnAt(e.position,
                        AsteroidTag::Size::Small,
                        { dir.x,   dir.y });
                    m_Spawner->SpawnAt(e.position,
                        AsteroidTag::Size::Small,
                        { -dir.x,  -dir.y });
                    m_Spawner->SpawnAt(e.position,
                        AsteroidTag::Size::Small,
                        { perp.x,  perp.y });
                    m_Spawner->SpawnAt(e.position,
                        AsteroidTag::Size::Small,
                        { -perp.x, -perp.y });
                }
                // smol → nothing
            }
        );

        // Level up notification
        Frost::EventBus::On<PlayerLevelUpEvent>(
            [](const PlayerLevelUpEvent& e)
            {
                FROST_LOG("LEVEL UP → %d", e.newLevel);
                // future: speed boost, fire rate increase, etc
            }
        );

        // player on ded — go to game over
        Frost::EventBus::On<PlayerDiedEvent>(
            [](const PlayerDiedEvent& e)
            {
                FROST_LOG("Player died with score %d — reloading", e.finalScore);
                Frost::SceneManager::Get().Load<AsteroidsScene>();
            }
        );
    }

    // systems
    std::unique_ptr<MovementSystem>      m_Movement;
    std::unique_ptr<ScreenWrapSystem>    m_ScreenWrap;
    std::unique_ptr<LifetimeSystem>      m_Lifetime;
    std::unique_ptr<SpawnImmunitySystem> m_SpawnImmunity;
    std::unique_ptr<FlickerSystem>       m_Flicker;
    std::unique_ptr<CollisionSystem>     m_Collision;
    std::unique_ptr<DamageSystem>        m_Damage;
    std::unique_ptr<AsteroidSpawner>     m_Spawner;
    std::unique_ptr<IFrameSystem>        m_IFrames;
    std::unique_ptr<HUDSystem>           m_HUD;
    std::unique_ptr<ExperienceSystem>    m_XPSystem;
    Frost::TextRenderer                  m_TextRenderer;
    Frost::mat4                          m_UIProjection;


    std::unique_ptr<RenderSystem> m_RenderSystem;
    Frost::DebugRenderer m_DebugRenderer;
    bool                 m_DebugDraw = false;

    // assets
    Frost::Texture* m_Atlas;
    Frost::Shader* m_Shader;

    // entities
    Frost::Entity m_PlayerEntity = Frost::NULL_ENTITY;
    Frost::Entity m_ThrusterEntity = Frost::NULL_ENTITY;

    // score
    int m_Score = 0;
};
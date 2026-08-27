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
#include "../Systems/PowerUpSystem.h"
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


       
        m_TextRenderer.Init("Assets/fonts/Cousine/Cousine-Regular.ttf", "Assets/Shaders/textV.glsl", "Assets/Shaders/textF.glsl", 24);

        
        m_UIProjection = Frost::ortho(0.0f, 1280.0f, 720.0f, 0.0f);

        FROST_LOG("AsteroidsScene initializing");

        // load assets
        auto& app = Frost::Application::Get();
        m_Atlas = Frost::AssetLoader::LoadTexture("atlas", "Assets/Images/asteroids.png");
        m_Shader = Frost::AssetLoader::LoadShader("gae", "Assets/Shaders/V.glsl", "Assets/Shaders/F.glsl");

        Frost::AudioSystem::Preload("Assets/Sounds/shooting.wav");

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
        // after player
        m_PowerUpSystem = std::make_unique<PowerUpSystem>(*m_Registry, m_PlayerEntity);
        // wire events
        WireEvents();

        FROST_LOG("AsteroidsScene ready");
    }


    void OnUpdate(float dt) override
    {
        if (Frost::Input::IsKeyJustPressed(GLFW_KEY_F1))
            m_DebugDraw = !m_DebugDraw;

        auto& threadPool = Frost::Application::Get().GetThreadPool();

        //systems using different components, safe to do parallel
        auto f1 = threadPool.Submit([this, dt]() { m_Movement->Update(dt);      });
        auto f2 = threadPool.Submit([this, dt]() { m_SpawnImmunity->Update(dt); });
        auto f3 = threadPool.Submit([this, dt]() { m_Lifetime->Update(dt);      });
        auto f4 = threadPool.Submit([this, dt]() { m_Flicker->Update(dt);       });

        // jobs
        f1.get(); f2.get(); f3.get(); f4.get();

        //must wait on movement
        m_ScreenWrap->Update(dt);

        m_Spawner->Update(dt);

        
        m_Collision->Update(dt);
        m_Damage->Update(dt);
        m_IFrames->Update(dt);
        m_PowerUpSystem->Update(dt);
        m_XPSystem->Update(dt);
    }

    void OnRender() override
    {
        auto& app = Frost::Application::Get();
        auto& renderer = app.GetRenderer();
        auto& camera = app.GetCamera();

        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_Shader->Bind();
        m_Shader->SetInt("uUseTexture", 1);
        m_Shader->SetMat4("uProjection", camera.GetViewProjection());
        m_Atlas->Bind(0);
        m_Shader->SetInt("uTexture", 0);

        renderer.Begin(m_Shader, camera.GetViewProjection());
        m_RenderSystem->Update(0.0f);
        renderer.End();

        if (m_DebugDraw)
        {
            auto view = m_Registry->GetView<Transform2D, CircleCollider>();
            for (auto entry : view)
            {
                Frost::Entity   e = entry.entity;
                Transform2D& transform = std::get<0>(entry.components);
                CircleCollider& collider = std::get<1>(entry.components);

                Frost::vec4 color = { 0.0f, 1.0f, 0.0f, 1.0f };
                if (m_Registry->Has<PlayerTag>(e))
                    color = { 0.0f, 0.5f, 1.0f, 1.0f };
                else if (m_Registry->Has<BulletTag>(e))
                    color = { 1.0f, 1.0f, 0.0f, 1.0f };
                else if (m_Registry->Has<AsteroidTag>(e))
                    color = { 1.0f, 0.3f, 0.0f, 1.0f };
                if (m_Registry->Has<SpawnImmunity>(e))
                    color.a = 0.3f;

                m_DebugRenderer.DrawCircle(transform.position, collider.radius, color);
            }
            m_DebugRenderer.Flush(camera.GetViewProjection());
        }

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_Shader->Bind();
        m_Shader->SetInt("uUseTexture", 0);
        m_Shader->SetMat4("uProjection", m_UIProjection);
        app.GetWhiteTex()->Bind(0);
        m_Shader->SetInt("uTexture", 0);

        renderer.Begin(m_Shader, m_UIProjection);
        DrawHUDBars(renderer, app.GetWhiteTex());
        renderer.End();
        
        m_HUD->RenderText(
            m_TextRenderer,
            m_UIProjection,
            m_PlayerEntity,
            m_Score
        );

        glEnable(GL_DEPTH_TEST);
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
                
                
                // 25% chance to drop power-up on any asteroid
                if (rand() % 4 == 0)
                    SpawnPowerUp(e.position);
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
            [this](const PlayerDiedEvent& e)
            {
                FROST_LOG("Player died with score %d — reloading", e.finalScore);
                //currently resets the ecs registry and creates a new player
                ResetRegistry();
                SpawnPlayer();

            }
        );

        Frost::EventBus::On<ShieldBrokenEvent>(
            [](const ShieldBrokenEvent& e)
            {
                FROST_LOG("Shield broken!");
                
            }
        );

        Frost::EventBus::On<PowerUpCollectedEvent>(
            [](const PowerUpCollectedEvent& e)
            {
                switch (e.type)
                {
                case PowerUpType::Shield:    FROST_LOG("Collected: Shield");     break;
                case PowerUpType::RapidFire: FROST_LOG("Collected: RapidFire"); break;
                case PowerUpType::SpeedBoost:FROST_LOG("Collected: SpeedBoost"); break;
                }
            }
        );
    }

    void SpawnPowerUp(Frost::vec2 position)
    {
        auto& ecs = m_Registry;

        // Random type
        PowerUpType type = static_cast<PowerUpType>(rand() % 3);

        Frost::Entity e = CreateEntity();
        

        // Offset slightly from asteroid center
        Frost::vec2 spawnPos = {
            position.x + ((rand() % 40) - 20),
            position.y + ((rand() % 40) - 20)
        };

        AddComponent<Transform2D>(e, { spawnPos, 0.0f, { 20.0f, 20.0f } });
        AddComponent<CircleCollider>(e, { 15.0f });
        AddComponent<Lifetime>(e, { 10.0f }); // disappears after 10 seconds
        AddComponent<PowerUpTag>(e, {});
        AddComponent<PowerUp>(e, { type, 8.0f, 8.0f });

        // dont have sprites for powerups yet so they just take garbage parts from atlas
        switch (type)
        {
        case PowerUpType::Shield:
            AddComponent<Sprite>(e, GetSprite(SpriteID::POWERUP_SHIELD));
            break;
        case PowerUpType::RapidFire:
            AddComponent<Sprite>(e, GetSprite(SpriteID::POWERUP_RAPIDFIRE));
            break;
        case PowerUpType::SpeedBoost:
            AddComponent<Sprite>(e, GetSprite(SpriteID::POWERUP_SPEED));
            break;
        }
    }

    void DrawHUDBars(Frost::BatchRenderer& renderer, Frost::Texture* whiteTex)
    {
        if (!m_Registry->IsAlive(m_PlayerEntity)) return;
        if (!m_Registry->Has<Health>(m_PlayerEntity))     return;
        if (!m_Registry->Has<Experience>(m_PlayerEntity)) return;

        auto& health = m_Registry->Get<Health>(m_PlayerEntity);
        auto& xp = m_Registry->Get<Experience>(m_PlayerEntity);

        constexpr float BAR_W = 200.0f;
        constexpr float BAR_H = 16.0f;
        constexpr float XP_H = 10.0f;
        constexpr float GAP = 6.0f;
        constexpr float X = 20.0f;
        constexpr float Y = 20.0f;

        // ── Health bar background ──
        DrawQuad(renderer, whiteTex,
            { X, Y }, { BAR_W, BAR_H },
            { 0.15f, 0.15f, 0.15f, 0.85f });

        // ── Health bar fill ──
        float hp = health.Percent();
        Frost::vec4 hpColor;
        if (hp > 0.6f) hpColor = { 0.2f,  0.85f, 0.2f,  0.9f };
        else if (hp > 0.3f) hpColor = { 1.0f,  0.75f, 0.0f,  0.9f };
        else                hpColor = { 0.9f,  0.15f, 0.15f, 0.9f };

        DrawQuad(renderer, whiteTex,
            { X, Y }, { BAR_W * hp, BAR_H },
            hpColor);

        // ── XP bar background ──
        DrawQuad(renderer, whiteTex,
            { X, Y + BAR_H + GAP }, { BAR_W, XP_H },
            { 0.15f, 0.15f, 0.15f, 0.85f });

        // ── XP bar fill ──
        float xpPct = (float)xp.current / (float)xp.threshold;
        DrawQuad(renderer, whiteTex,
            { X, Y + BAR_H + GAP }, { BAR_W * xpPct, XP_H },
            { 0.3f, 0.5f, 1.0f, 0.9f });

        // ── Active power-up indicators ──
        float indicatorX = X;
        float indicatorY = Y + BAR_H + GAP + XP_H + GAP;

        if (m_Registry->Has<ShieldEffect>(m_PlayerEntity))
        {
            DrawQuad(renderer, whiteTex,
                { indicatorX, indicatorY }, { 20.0f, 20.0f },
                { 0.2f, 0.6f, 1.0f, 0.9f }); // blue — shield
            indicatorX += 26.0f;
        }
        if (m_Registry->Has<RapidFireEffect>(m_PlayerEntity))
        {
            DrawQuad(renderer, whiteTex,
                { indicatorX, indicatorY }, { 20.0f, 20.0f },
                { 1.0f, 0.8f, 0.0f, 0.9f }); // yellow — rapid fire
            indicatorX += 26.0f;
        }
        if (m_Registry->Has<SpeedBoostEffect>(m_PlayerEntity))
        {
            DrawQuad(renderer, whiteTex,
                { indicatorX, indicatorY }, { 20.0f, 20.0f },
                { 0.2f, 1.0f, 0.4f, 0.9f }); // green — speed
        }
    }

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
            { {0,0}, {1,1} },
            tex,
            1.0f, 1.0f,
            color
        );
    }

    // systems
    std::unique_ptr<MovementSystem> m_Movement;
    std::unique_ptr<ScreenWrapSystem> m_ScreenWrap;
    std::unique_ptr<LifetimeSystem> m_Lifetime;
    std::unique_ptr<SpawnImmunitySystem> m_SpawnImmunity;
    std::unique_ptr<FlickerSystem> m_Flicker;
    std::unique_ptr<CollisionSystem> m_Collision;
    std::unique_ptr<DamageSystem> m_Damage;
    std::unique_ptr<AsteroidSpawner> m_Spawner;
    std::unique_ptr<IFrameSystem> m_IFrames;
    std::unique_ptr<HUDSystem> m_HUD;
    std::unique_ptr<ExperienceSystem> m_XPSystem;
    std::unique_ptr<PowerUpSystem> m_PowerUpSystem;
    Frost::TextRenderer m_TextRenderer;
    Frost::mat4 m_UIProjection;


    std::unique_ptr<RenderSystem> m_RenderSystem;
    Frost::DebugRenderer m_DebugRenderer;
    bool m_DebugDraw = false;

    // assets
    Frost::Texture* m_Atlas;
    Frost::Shader* m_Shader;

    // entities
    Frost::Entity m_PlayerEntity = Frost::NULL_ENTITY;
    Frost::Entity m_ThrusterEntity = Frost::NULL_ENTITY;

    // score
    int m_Score = 0;
};
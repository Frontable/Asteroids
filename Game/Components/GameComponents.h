#pragma once
#include "Core/FrostMath.h"


struct Transform2D
{
    Frost::vec2 position = { 0.0f, 0.0f };
    float       rotation = 0.0f;
    Frost::vec2 scale = { 1.0f, 1.0f };
};

struct Velocity2D
{
    Frost::vec2 velocity = { 0.0f, 0.0f };
    float       angularVelocity = 0.0f;
};

struct Sprite
{
    Frost::vec2 uv;
    Frost::vec2 size;
};

struct CircleCollider
{
    float radius = 10.0f;
};

struct SpawnImmunity
{
    float timeLeft = 1.0f;
};

struct Lifetime
{
    float timeLeft = 1.0f;
};

struct PlayerState
{
    bool isThrusting = false;
    bool isShooting = false;
};

struct Flicker
{
    float timer = 0.0f;
    float interval = 0.05f;
    bool  visible = false;
};

struct Health
{
    float current = 100.0f;
    float max = 100.0f;
    float iFrames = 0.0f;   
    float iFrameMax = 1.5f;  
    bool  isDead = false;

    float Percent() const { return current / max; }
};

struct Experience
{
    int   current = 0;
    int   level = 1;
    int   threshold = 500;   

    // XP per asteroid size
    static constexpr int XP_LARGE = 150;
    static constexpr int XP_MEDIUM = 75;
    static constexpr int XP_SMALL = 30;
};

struct PlayerTag {};
struct BulletTag {};

struct AsteroidTag
{
    enum class Size { Large, Medium, Small };
    Size size = Size::Large;
};
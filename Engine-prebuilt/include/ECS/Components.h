#pragma once
#include "Math/FrostMath.h"
#include "Rendering/Essentials/Texture.h"

struct Transform2D
{
    vec2 position;
    float rotation = 0.0f;
    vec2 scale = { 1.0f, 1.0f };
};

struct Velocity2D
{
    vec2 velocity;
    float angularVelocity = 0.0f;
};

struct Sprite
{
    vec2 uv;
    vec2 size;
};

struct CircleCollider
{
    float radius = 10.0f;
};

struct Lifetime
{
    float timeLeft = 1.0f;
};

struct SpawnImmunity
{
    float timeLeft = 1.0f; // immune to collision for 1 second after spawning
};

struct Flicker
{
    float timer = 0.0f;
    float interval = 0.05f; // how fast it flickers (seconds)
    bool  visible = false;
};

struct PlayerTag {};
struct AsteroidTag
{
    enum class Size { Large, Medium, Small };
    Size size = Size::Large;
};
struct BulletTag {};








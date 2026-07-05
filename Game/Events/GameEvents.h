#pragma once
#include "Core/FrostMath.h"
#include "../Components/GameComponents.h"


struct AsteroidDestroyedEvent
{
    Frost::vec2      position;
    AsteroidTag::Size size;
    Frost::vec2      velocity;
};

struct BulletImpactEvent
{
    Frost::vec2 position;
};

struct PlayerDiedEvent
{
    int finalScore;
};

struct ScoreChangedEvent
{
    int newScore;
};

struct PlayerDamagedEvent
{
    float damage;
    float healthRemaining;
};

struct PlayerLevelUpEvent
{
    int newLevel;
    int newThreshold;
};

struct GameRestartEvent {};
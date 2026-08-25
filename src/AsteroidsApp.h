#pragma once
#include "Frost.h"
#include "Scenes/AsteroidsScene.h"

class AsteroidsApp : public Frost::Application
{
public:
    AsteroidsApp()
        : Application({
            "Asteroids",
            1280, 720,
            true,
            "Assets/"
            }) {
    }

    void OnInit() override
    {
        Frost::SceneManager::Get().Load<AsteroidsScene>();
    }
};
#pragma once
#include "Platform/Window.h"
#include "Platform/Context.h"

class Application
{
public:
    Application();
    Application(const char* title, int width, int height);
    virtual ~Application() = default;

    void Run();

    virtual void Init() = 0;
    virtual void Input(float dt) = 0;
    virtual void Update(float dt) = 0;
    virtual void Render(float dt) = 0;
    virtual void Clean() = 0;

protected:
    bool        m_IsRunning = true;
    const char* m_Title;
    int         m_Width, m_Height;
    Context     m_MainContext{};
};
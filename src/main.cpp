#define WIN32_LEAN_AND_MEAN
#include "Core/EntryPoint.h"
#include "AsteroidsApp.h"


Frost::Application* CreateApplication()
{
    return new AsteroidsApp();
}
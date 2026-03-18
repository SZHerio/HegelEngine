#include "engine/core/Application.h"
#include "engine/core/Log.h"

int main()
{
    HegelEngine::core::Log::init();
    HE_CORE_INFO("Core logger initialized!");
    HE_APP_INFO("Sandbox logger initialized!");


    HegelEngine::core::Application app;
    app.run();
}
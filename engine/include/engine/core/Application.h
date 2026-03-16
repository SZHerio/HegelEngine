#pragma once

struct GLFWwindow;

namespace HegelEngine::core
{
    class Application
    {
    public:
        Application();
        ~Application();

        void run();
        
    private:
        bool init();
        void shutdown();
        void processInput();
    
    private:
        GLFWwindow* m_window = nullptr;
        int m_width = 1280;
        int m_height = 720;
    };
}

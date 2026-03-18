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
        void renderFrame();

        bool initGeometry();
        void destroyGeometry();
    
    private:
        GLFWwindow* m_window = nullptr;
        int m_width = 1280;
        int m_height = 720;

        unsigned int m_shaderProgram = 0;
        unsigned int m_vao = 0;
        unsigned int m_vbo = 0;
        unsigned int m_ebo = 0;
    };
}

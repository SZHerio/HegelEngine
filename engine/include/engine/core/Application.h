#pragma once

#include <glm/vec3.hpp>

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

        void updateCameraVectors();
        void onMouseMoved(double xpos, double ypos);
        void onFrameBufferResized(int width, int height);
    
    private:
        GLFWwindow* m_window = nullptr;
        int m_width = 1280;
        int m_height = 720;

        unsigned int m_shaderProgram = 0;
        unsigned int m_vao = 0;
        unsigned int m_vbo = 0;
        unsigned int m_texture = 0;

        float m_deltaTime = 0.0f;
        float m_lastFrame = 0.0f;

        glm::vec3 m_cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

        float m_yaw = -90.0f;
        float m_pitch = 0.0f;

        bool m_firstMouse = true;
        float m_lastMouseX = 640.0f;
        float m_lastMouseY = 360.0f;
    };
}

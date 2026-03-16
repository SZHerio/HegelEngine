#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "engine/core/Application.h"

namespace HegelEngine::core
{
    namespace
    {
        void framebufferSizeCallback(GLFWwindow* window, int width, int height)
        {
            glViewport(0,0, width, height);
            glClearColor(0.1f, 0.7f, 1.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(window);
        }
    }

    void Application::run()
    {
        if (!init()) return;

        while (!glfwWindowShouldClose(m_window))
        {
            processInput();

            glClearColor(0.1f, 0.7f, 1.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }
    }

    Application::Application()
    {
        
    }

    Application::~Application()
    {
        shutdown();
    }

    bool Application::init()
    {
        if (!glfwInit())
        {
            std::cerr << "ERROR: Failed to initialize GFLW!\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif 
        m_window = glfwCreateWindow(m_width, m_height, "Hegel Engine", nullptr, nullptr);
        
        if (!m_window)
        {
            std::cerr << "ERROR: Failed to create GFLW window!\n";
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_window);
        glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            std::cerr << "ERROR: Failed to initialize glad!\n";
            return false;
        }

        glViewport(0,0,m_width, m_height);
        
        std::cout << "OpenGL was successflly initialized!\n";

        std::cout << "Version: " << glGetString(GL_RENDERER) << "\n";
        std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

        return true;
    }

    void Application::shutdown()
    {
        if (m_window)
        {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }

        glfwTerminate();
    }

    void Application::processInput()
    {
        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(m_window, true);
        }
    }
}
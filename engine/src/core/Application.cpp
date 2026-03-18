#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "engine/core/Application.h"
#include <string>
#include <fstream>
#include <sstream>
#include "engine/core/Log.h"
#include <filesystem>
#include <windows.h>

namespace HegelEngine::core
{
    namespace
    {
        std::filesystem::path getExecutableDirectory()
        {
            char buffer[MAX_PATH];
            const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);

            if (length == 0 || length == MAX_PATH)
            {
                HE_CORE_CRITICAL("Failed to execute a path!");
                return {};
            }

            return std::filesystem::path(buffer).parent_path();
        }

        std::filesystem::path getAssetPath(const std::filesystem::path& relativePath)
        {
            return getExecutableDirectory()/"assets"/relativePath;
        }

        void framebufferSizeCallback(GLFWwindow* window, int width, int height)
        {
            glViewport(0,0, width, height);
        }

        unsigned int compileShader(unsigned int type, const std::string& source)
        {
            const char* sourceDir = source.c_str();

            unsigned int shader = glCreateShader(type);
            glShaderSource(shader, 1, &sourceDir, nullptr);
            glCompileShader(shader);

            int success = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            
            if (!success)
            {
                char infoLog[512] = {};
                glGetShaderInfoLog(shader, 512, nullptr, infoLog);
                HE_CORE_CRITICAL("Shader compilation failed {}", infoLog);
                glDeleteShader(shader);
                return 0;
            }

            return shader;
        }

        std::string readTextFile(const std::filesystem::path& path)
        {
            std::ifstream file(path.string());

            if (!file.is_open())
            {
                HE_CORE_CRITICAL("Failed to open file {}", path.string());
                return {};
            }

            std::stringstream buffer;
            buffer << file.rdbuf();

            return buffer.str();
        }

        unsigned int createShaderProgramFromFiles(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
        {
            const std::string vertexSource = readTextFile(vertexPath);
            
            if (vertexSource.empty())
            {
                HE_CORE_CRITICAL("Vertex source is empty");
                return 0;
            }

            const std::string fragmentSource = readTextFile(fragmentPath);

            if (fragmentSource.empty())
            {
                HE_CORE_CRITICAL("Fragment source is empty");
                return 0;
            }

            const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);

            if (vertexShader == 0)
            {
                HE_CORE_CRITICAL("Vertex shader was not created");
                return 0;
            }

            const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

            if (fragmentShader == 0)
            {
                glDeleteShader(vertexShader);
                HE_CORE_CRITICAL("Fragment shader was not created");
                return 0;
            }

            const unsigned int program = glCreateProgram();
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);

            int success = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &success);

            if (!success)
            {
                char infoLog[512] = {};
                glGetProgramInfoLog(program, 512, nullptr, infoLog);
                HE_CORE_CRITICAL("Program compilation failed: {}", infoLog);
                glDeleteShader(vertexShader);
                glDeleteShader(fragmentShader);
                glDeleteProgram(program);

                return 0;
            }

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            return program;
        }
    }

    void Application::run()
    {
        if (!init()) return;

        while (!glfwWindowShouldClose(m_window))
        {
            processInput();
            renderFrame();
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
            HE_CORE_CRITICAL("Failed to initialize GFLW!");
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(m_width, m_height, "Hegel Engine", nullptr, nullptr);
        
        if (!m_window)
        {
            HE_CORE_CRITICAL("Failed to create GFLW window!");
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_window);
        glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            HE_CORE_CRITICAL("Failed to initialize glad!");
            return false;
        }

        glViewport(0,0,m_width, m_height);
        
        HE_CORE_INFO("OpenGL was successflly initialized!");
        HE_CORE_INFO("Version: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        HE_CORE_INFO("OpenGL version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

        if(!initGeometry())
        {
            HE_CORE_CRITICAL("Failed to initialize a geometry!");
            return false;
        }

        return true;
    }

    void Application::shutdown()
    {
        destroyGeometry();

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

    void Application::renderFrame()
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(m_shaderProgram);
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glfwSwapBuffers(m_window);
    }

    bool Application::initGeometry()
    {
        const float verticies[] =
        {
            0.5f, 0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f
        };

        const unsigned int indices[] = 
        {
            0,1,3,
            1,2,3
        };

        const auto vertexShaderPath = getAssetPath(std::filesystem::path("shaders")/"basic.vert");
        const auto fragmentShaderPath = getAssetPath(std::filesystem::path("shaders") / "basic.frag");

        m_shaderProgram = createShaderProgramFromFiles(vertexShaderPath, fragmentShaderPath);
        
        if (m_shaderProgram == 0)
        {
            HE_CORE_CRITICAL("Failed to initialize a triangle!");
            return false;
        }

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 3*sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return true;
    }

    void Application::destroyGeometry()
    {
        if (m_ebo)
        {
            glDeleteBuffers(1, &m_ebo);
            m_ebo = 0;
        }

        if (m_vbo)
        {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }

        if (m_vao)
        {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }

        if (m_shaderProgram)
        {
            glDeleteProgram(m_shaderProgram);
            m_shaderProgram = 0;
        }
    }
}
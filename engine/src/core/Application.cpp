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
        }

        unsigned int compileShader(unsigned int type, const char* source)
        {
            unsigned int shader = glCreateShader(type);
            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);

            int success = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

            if (!success)
            {
                char infoLog[512] = {};
                glGetShaderInfoLog(shader, 512, nullptr, infoLog);
                std::cerr << "ERROR: Shader compilation failed:\n" << infoLog << "\n";
                glDeleteShader(shader);
                return 0;
            }

            return shader;
        }

        unsigned int createShaderProgram()
        {
            const char* vertexShaderSource = R"(
                #version 330 core
                layout (location = 0) in vec3 aPos;

                void main()
                {
                    gl_Position = vec4(aPos, 1.0);
                }
            )";

            const char* fragmentShaderSource = R"(
                #version 330 core
                out vec4 FragColor;

                void main()
                {
                    FragColor = vec4(0.3, 0.2, 0.7, 1.0);
                }
            )";

            unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
            
            if(vertexShader == 0)
            {
                std::cerr << "ERROR IN compileShader()\nVertex shader was not created!\n";
                return 0;
            }

            unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

            if (fragmentShader == 0)
            {
                std::cerr << "ERROR IN compileShader()\nFragment shader was not created!\n";
                return 0;
            }

            unsigned int program = glCreateProgram();
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);

            int success = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &success);

            if(!success)
            {
                char infoLog[512] = {};
                glGetProgramInfoLog(program, 512, nullptr, infoLog);
                std::cerr << "ERROR IN createShaderProgram(): Program compilation failed:\n" << infoLog << "\n";
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
            std::cerr << "ERROR IN init(): Failed to initialize GFLW!\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(m_width, m_height, "Hegel Engine", nullptr, nullptr);
        
        if (!m_window)
        {
            std::cerr << "ERROR IN init(): Failed to create GFLW window!\n";
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_window);
        glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            std::cerr << "ERROR IN init(): Failed to initialize glad!\n";
            return false;
        }

        glViewport(0,0,m_width, m_height);
        
        std::cout << "OpenGL was successflly initialized!\n";
        std::cout << "Version: " << glGetString(GL_RENDERER) << "\n";
        std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

        if(!initTriangle())
        {
            std::cerr << "ERROR IN init(): Failed to initialize a triangle!\n";
            return false;
        }

        return true;
    }

    void Application::shutdown()
    {
        destroyTriangle();

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
        glClearColor(0.3f, 0.5f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(m_shaderProgram);
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0,3);
        glfwSwapBuffers(m_window);
    }

    bool Application::initTriangle()
    {
        const float verticies[] =
        {
            0.0f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f
        };

        m_shaderProgram = createShaderProgram();
        
        if (m_shaderProgram == 0)
        {
            std::cerr << "ERROR IN initTriangle(): Failed to initialize a triangle!\n";
            return false;
        }

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);

        glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 3*sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return true;
    }

    void Application::destroyTriangle()
    {
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
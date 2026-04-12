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
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
                HE_CORE_CRITICAL("Failed to get executable path");
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
            HE_CORE_CRITICAL("Failed to initialize GLFW");
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(m_width, m_height, "Hegel Engine", nullptr, nullptr);
        
        if (!m_window)
        {
            HE_CORE_CRITICAL("Failed to create GFLW window");
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_window);
        glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            HE_CORE_CRITICAL("Failed to initialize glad");
            return false;
        }

        glViewport(0,0,m_width, m_height);
        
        HE_CORE_INFO("OpenGL was successfully initialized");
        HE_CORE_INFO("Version: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        HE_CORE_INFO("OpenGL version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

        glEnable(GL_DEPTH_TEST);

        if(!initGeometry())
        {
            HE_CORE_CRITICAL("Failed to initialize a geometry");
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(m_shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        const int textureLocation = glGetUniformLocation(m_shaderProgram, "uTexture");
        glUniform1i(textureLocation, 0);

        glm::mat4 model = glm::mat4(1.0f);

        model = glm::rotate(
            model,
            static_cast<float>(glfwGetTime())*glm::radians(30.0f),
            glm::vec3(0.5f, 1.0f, 0.0f)
        );

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(m_width) / static_cast<float>(m_height),
            0.1f,
            100.0f
        );

        const int modelLocation = glGetUniformLocation(m_shaderProgram, "uModel");
        const int viewLocation = glGetUniformLocation(m_shaderProgram, "uView");
        const int projectionLocation = glGetUniformLocation(m_shaderProgram, "uProjection");

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glfwSwapBuffers(m_window);
    }

    bool Application::initGeometry()
    {
        const float vertices[] =
{
        // positions            // tex coords
        // front face
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,

        // back face
        -0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    1.0f, 0.0f,

        // left face
        -0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    1.0f, 1.0f,

        // right face
        0.5f,  0.5f,  0.5f,    0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,    0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,    0.0f, 0.0f,

        // bottom face
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,    1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,

        // top face
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f
};

        const unsigned int indices[] = 
        {
            0,1,3,
            1,2,3
        };

        const auto vertexShaderPath = getAssetPath(std::filesystem::path("shaders")/"basic.vert");
        const auto fragmentShaderPath = getAssetPath(std::filesystem::path("shaders") / "basic.frag");
        const auto texturePath = getAssetPath(std::filesystem::path("textures") / "test.png");

        m_shaderProgram = createShaderProgramFromFiles(vertexShaderPath, fragmentShaderPath);
        
        if (m_shaderProgram == 0)
        {
            HE_CORE_CRITICAL("Failed to initialize a geometry");
            return false;
        }

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        //glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 5*sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        HE_CORE_INFO("Texture path: {}", texturePath.string());
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_set_flip_vertically_on_load(true);

        int width = 0;
        int height = 0;
        int channels = 0;
        auto data = stbi_load(texturePath.string().c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            HE_CORE_CRITICAL("Cannot load a texture data: {}", texturePath.string());
            return false;
        }

        GLenum format = GL_RGB;
        
        switch (channels)
        {
        case 1:
            format = GL_RED;
            break;

        case 3:
            format = GL_RGB;
            break;

        case 4:
            format = GL_RGBA;
            break;

        default:
            stbi_image_free(data);
            HE_CORE_CRITICAL("Unsupportex texture channel colors");
            return false;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }

    void Application::destroyGeometry()
    {
        /*if (m_ebo)
        {
            glDeleteBuffers(1, &m_ebo);
            m_ebo = 0;
        }*/

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

        if (m_texture)
        {
            glDeleteTextures(1, &m_texture);
            m_texture = 0;
        }
    }
}
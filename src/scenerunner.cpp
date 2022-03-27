#include "pch.h"
#include "scenerunner.h"
#include "helper/camera.h"
#include "helper/scene.h"
#include <GLFW/glfw3.h>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_stdlib.h"

SceneRunner::SceneRunner()
{
}

bool SceneRunner::init(const std::string& title, int width, int height, int samples)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        LOG_CRITICAL("cant initialize glfw");
        return false;
    }

#ifdef __APPLE__
    // Select OpenGL 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#else
    // Select OpenGL 4.6
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#endif
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    if (_debug)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    if (_samples > 0)
        glfwWindowHint(GLFW_SAMPLES, _samples);

    _width = width;
    _height = height;

    // Create the window
    _window = glfwCreateWindow(_width, _height, title.c_str(), NULL, NULL);

    if (!_window)
    {
        LOG_CRITICAL("Unable to create OpenGL context.");
        return false;
    }

    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1); // Enable vsync. NOTE: This will also cap frames to 60.

    // Setup callbacks
    {
        glfwSetWindowUserPointer(_window, this);

        auto fKeyCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            static_cast<SceneRunner*>(glfwGetWindowUserPointer(window))->OnPressKey(key, scancode, action, mods);
        };
        glfwSetKeyCallback(_window, fKeyCallback);

        auto fResizeFrameBufferCallback = [](GLFWwindow* window, int width, int height)
        {
            static_cast<SceneRunner*>(glfwGetWindowUserPointer(window))->OnResizeFramebuffer(width, height);
        };
        glfwSetFramebufferSizeCallback(_window, fResizeFrameBufferCallback);

        auto fMouseMoveCallback = [](GLFWwindow* window, double x, double y)
        {
            static_cast<SceneRunner*>(glfwGetWindowUserPointer(window))->OnMouseMove(x, y);
        };
        glfwSetCursorPosCallback(_window, fMouseMoveCallback);

        auto fMouseScrollCallback = [](GLFWwindow* window, double xoffset, double yoffset)
        {
            static_cast<SceneRunner*>(glfwGetWindowUserPointer(window))->OnMouseScroll(xoffset, yoffset);
        };
        glfwSetScrollCallback(_window, fMouseScrollCallback);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr; // Don't save .ini file

    if (!ImGui_ImplGlfw_InitForOpenGL(_window, true))
    {
        LOG_CRITICAL("failed to initialize glfw OpenGL for ImGui");
        return false;
    }

    const char* glsl_version = "#version 330 core";
    if (!ImGui_ImplOpenGL3_Init(glsl_version))
    {
        LOG_CRITICAL("failed to initialize OpenGL for ImGui");
        return false;
    }

    // Get framebuffer size
    glfwGetFramebufferSize(_window, &_fbw, &_fbh);

    // Load the OpenGL functions.
    if (!gladLoadGL())
    {
        LOG_CRITICAL("failed to load OpenGL functions");
        return false;
    }

    GLUtils::dumpGLInfo();

    // Initialization
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

#ifndef __APPLE__
    if (_debug) {
        glDebugMessageCallback(GLUtils::debugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
            GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Start debugging");
    }
#endif

	return true;
}

int SceneRunner::run(Camera* camera, Scene* scene)
{
    _camera = camera;
    _scene = scene;

	assert(_window != nullptr);
	assert(_scene != nullptr);

    _scene->setDimensions(_fbw, _fbh);
    bool ret = _scene->initScene();
    _scene->resize(_fbw, _fbh);

    // If can't initialize the scene, gracefully shut down the app
    if (!ret)
        glfwSetWindowShouldClose(_window, 1);

    // Enter the main loop
    loop();

#ifndef __APPLE__
    if (_debug)
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 1,
            GL_DEBUG_SEVERITY_NOTIFICATION, -1, "End debug");
#endif

    // Close window and terminate GLFW
    glfwTerminate();

    // Exit program
    return EXIT_SUCCESS;
}

void SceneRunner::loop()
{
	assert(_window != nullptr);
    assert(_scene != nullptr);

    while (!glfwWindowShouldClose(_window))
    {
        GLUtils::checkForOpenGLError(__FILE__, __LINE__);

        // per-frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        _deltaTime = currentFrame - _lastFrame;
        _lastFrame = currentFrame;

        _scene->update(currentFrame);

        _scene->UpdateViewMatrix(_camera->GetViewMatrix());
        _scene->UpdateProjMatrix(glm::perspective(glm::radians(_camera->Zoom), (float)_width / (float)_height, 0.1f, 100.0f));

        _scene->render();

        glfwSwapBuffers(_window);
        glfwPollEvents();
    }
}

void SceneRunner::OnPressKey(int key, int scancode, int action, int mods)
{
    assert(_window != nullptr);
    assert(_camera != nullptr);
    //LOG_INFO("EVENT: press key {} => {} {} {}", key, action, scancode, mods);

    switch (key)
    {
    case GLFW_KEY_W:
        //if (action == GLFW_PRESS)
            _camera->ProcessKeyboard(Camera::FORWARD, _deltaTime);
        break;
    case GLFW_KEY_S:
        //if (action == GLFW_PRESS)
            _camera->ProcessKeyboard(Camera::BACKWARD, _deltaTime);
        break;
    case GLFW_KEY_A:
        //if (action == GLFW_PRESS)
            _camera->ProcessKeyboard(Camera::LEFT, _deltaTime);
        break;
    case GLFW_KEY_D:
        //if (action == GLFW_PRESS)
            _camera->ProcessKeyboard(Camera::RIGHT, _deltaTime);
        break;
    case GLFW_KEY_SPACE:
        if (action == GLFW_PRESS) _scene->ToggleBlinnPhong();
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(_window, 1);
        break;
    default:
        break;
    }
}

float lastX = 1024 / 2.0f;
float lastY = 768 / 2.0f;

void SceneRunner::OnMouseMove(double x, double y)
{
    assert(_camera != nullptr);
    //LOG_INFO("EVENT: mouse move {}, {}", x, y);

    float xpos = static_cast<float>(x);
    float ypos = static_cast<float>(y);

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    _camera->ProcessMouseMovement(xoffset, yoffset);
}

void SceneRunner::OnMouseScroll(double xoffset, double yoffset)
{
    assert(_camera != nullptr);
    //LOG_INFO("EVENT: mouse scroll {}, {}", xoffset, yoffset);
    _camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

void SceneRunner::OnResizeFramebuffer(int width, int height)
{
    assert(_scene != nullptr);
    LOG_INFO("EVENT: resize frame buffer to {}x{}", width, height);
    _scene->resize(width, height);
}

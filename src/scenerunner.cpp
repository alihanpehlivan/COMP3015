#include "pch.h"
#include "scenerunner.h"
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

    // Create the window
    _window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title.c_str(), NULL, NULL);

    if (!_window)
    {
        LOG_CRITICAL("Unable to create OpenGL context.");
        return false;
    }

    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1); // Enable vsync

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

int SceneRunner::run(Scene* scene)
{
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

        _scene->update(float(glfwGetTime()));
        _scene->render();

        glfwSwapBuffers(_window);
        glfwPollEvents();
    }
}

void SceneRunner::OnPressKey(int key, int scancode, int action, int mods)
{
    assert(_window != nullptr);
    //LOG_INFO("EVENT: press key {} => {} {} {}", key, action, scancode, mods);

    switch (key)
    {
    case GLFW_KEY_W:
        if (action == GLFW_PRESS) LOG_INFO("move forward");
        break;
    case GLFW_KEY_S:
        if (action == GLFW_PRESS) LOG_INFO("move backward");
        break;
    case GLFW_KEY_A:
        if (action == GLFW_PRESS) LOG_INFO("move left");
        break;
    case GLFW_KEY_D:
        if (action == GLFW_PRESS) LOG_INFO("move right");
        break;
    case GLFW_KEY_SPACE:
        if (action == GLFW_PRESS) _scene->animate(!_scene->animating());
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(_window, 1);
        break;
    default:
        break;
    }
}

void SceneRunner::OnMouseMove(double x, double y)
{
    //LOG_INFO("EVENT: mouse move {}, {}", x, y);

    //TODO: camera change position
}

void SceneRunner::OnMouseScroll(double xoffset, double yoffset)
{
    //LOG_INFO("EVENT: mouse scroll {}, {}", xoffset, yoffset);

    //TODO: camera zoom in/out
}

void SceneRunner::OnResizeFramebuffer(int width, int height)
{
    assert(_scene != nullptr);
    _scene->resize(width, height);
}
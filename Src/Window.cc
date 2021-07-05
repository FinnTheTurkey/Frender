#include "Frender/Frender.hh"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <unordered_map>
#include <functional>
#include <sstream>

// A stupid solution to a stupid problem
static std::unordered_map<GLFWwindow*, Frender::Window*> window_windows;

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    window_windows[window]->_sizeCallback(width, height);
}

Frender::Window::Window(Frender::WindowSettings settings)
{
    renderer = nullptr;

    // Create basic window for opengl
    if (!glfwInit())
    {
        fprintf(stderr, "Error: %s\n", "GLFW failed to initialize");
        return;
    }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(), NULL, NULL);

    if (!window)
    {
        fprintf(stderr, "Error: %s\n", "GLFW failed to create window; make sure your computer supports OpenGL 3.3");
        return;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress))
    {
        fprintf(stderr, "Error: %s\n", "Could not create GL context; make sure your computer supports OpenGL 3.3");
    }

    // Because apparently being able to pass custom arguments into
    // callbacks is too difficult for a library like glfw to implement
    window_windows[window] = this;

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &settings.width, &settings.height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glViewport(0, 0, settings.width, settings.height);
}

void Frender::Window::_sizeCallback(int width, int height)
{
    if (renderer != nullptr)
    {
        // TODO: Allow this to be manually overridden
        renderer->setRenderResolution(width, height);
    }
}

void showFPS(GLFWwindow* window)
{
    static double previousSeconds = 0.0;
    static int frameCount = 0;
    double elapsedSeconds;
    double currentSeconds = glfwGetTime(); // Time since start (s)

    elapsedSeconds = currentSeconds - previousSeconds;

    // Limit text update to 4/second
    if (elapsedSeconds > 0.25) {
        previousSeconds = currentSeconds;
        double fps = (double)frameCount / elapsedSeconds;
        double msPerFrame = 1000.0 / fps;

        std::ostringstream outs;
        outs.precision(3); // Set precision of numbers

        outs << std::fixed << "FluxTest" << " - " << "FPS: " << fps << " Frame time: " << msPerFrame << "ms";

        glfwSetWindowTitle(window, outs.str().c_str());

        // Reset frame count
        frameCount = 0;
    }

    frameCount ++;
}

void Frender::Window::mainloop(Renderer* render, std::function<void(float)> fn)
{
    renderer = render;

    double time = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        glfwMakeContextCurrent(window);
        glfwPollEvents();

        float delta = glfwGetTime() - time;
        time = glfwGetTime();

        double stime = glfwGetTime();
        fn(delta);

        renderer->render(delta);
        time_time = glfwGetTime() - time;

        showFPS(window);
        // Really bad artificial delay
        // while (glfwGetTime() - time < 0.000001)
        // {
        //     // Delay
        // }

        glfwSwapBuffers(window);
    }
}

void Frender::Window::setWindowTitle(const std::string &title)
{
    const char* x = title.c_str();
    glfwSetWindowTitle(window, x);
}

void Frender::Window::setVsync(bool value)
{
    glfwSwapInterval(value);
}

Frender::Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

// glfwDestroyWindow(window) at end
// glfwTerminate() at end
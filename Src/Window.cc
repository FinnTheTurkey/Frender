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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        window_windows[window]->_keyPressCallback(key, true);
    }
    else if (action == GLFW_RELEASE)
    {
        window_windows[window]->_keyPressCallback(key, false);
    }
}

void button_callback(GLFWwindow* window, int key, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        window_windows[window]->_buttonPressCallback(key, true);
    }
    else if (action == GLFW_RELEASE)
    {
        window_windows[window]->_buttonPressCallback(key, false);
    }
}

void cursor_enter_callback(GLFWwindow* window, int entered)
{
    // if (entered)
    // {
    //     // The cursor entered the content area of the window
    //     // Set the cursor mode back to what it was before
    //     window_windows[window]->_resetCursorMode();
    // }
    // else
    // {
    //     // The cursor left the content area of the window
    //     // ALWAYS set the cursor back to regular
    //     glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    // }
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

    // Input stuff
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, button_callback);
    glfwSetCursorEnterCallback(window, cursor_enter_callback);
}

void Frender::Window::_sizeCallback(int width, int height)
{
    if (renderer != nullptr)
    {
        // TODO: Allow this to be manually overridden
        renderer->setRenderResolution(width, height);
    }
}

void Frender::Window::_keyPressCallback(int key, bool state)
{
    if (state)
    {
        // Key pressed
        keys_down[key] = true;
        just_pressed[key] = true;

        std::cout << "Key pressed\n";
    }
    else
    {
        // Key released
        keys_down[key] = false;
        just_released[key] = true;
        std::cout << "Key released\n";
    }
}

bool Frender::Window::isKeyDown(int key)
{
    if (keys_down.find(key) == keys_down.end())
    {
        // Never been pressed
        return false;
    }

    return keys_down[key];
}

bool Frender::Window::isKeyJustPressed(int key)
{
    return just_pressed.find(key) != just_pressed.end();
}

bool Frender::Window::isKeyJustReleased(int key)
{
    return just_released.find(key) != just_released.end();
}

void Frender::Window::_buttonPressCallback(int key, bool state)
{
    if (state)
    {
        // Key pressed
        keys_down[key] = true;
        just_pressed[key] = true;
    }
    else
    {
        // Key released
        keys_down[key] = false;
        just_released[key] = true;
    }
}

bool Frender::Window::isMouseButtonDown(int key)
{
    if (button_down.find(key) == button_down.end())
    {
        // Never been pressed
        return false;
    }

    return button_down[key];
}

bool Frender::Window::isMouseButtonJustPressed(int key)
{
    return button_just_pressed.find(key) != button_just_pressed.end();
}

bool Frender::Window::isMouseButtonJustReleased(int key)
{
    return button_just_released.find(key) != button_just_released.end();
}

glm::vec2 Frender::Window::getMousePosition()
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    return {xpos, ypos};
}

glm::vec2 Frender::Window::getMouseOffset()
{
    return mouse_offset;
}

void Frender::Window::setMouseMode(MouseMode mode)
{
    switch (mode)
    {
    case (Regular):
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    }
    case (Captured):
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
    }
    }

    mmode = mode;
}

void Frender::Window::_resetCursorMode()
{
    setMouseMode(mmode);
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

        // Reset just_pressed and just_released
        just_pressed = {};
        just_released = {};

        mouse_offset = last_mouse_pos - getMousePosition();
        last_mouse_pos = getMousePosition();
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
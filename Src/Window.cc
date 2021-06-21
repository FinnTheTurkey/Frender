#include "Frender/Frender.hh"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

Frender::Window::Window(Frender::WindowSettings settings)
{
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

    glfwGetFramebufferSize(window, &settings.width, &settings.height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  
    glViewport(0, 0, settings.width, settings.height);
}

void Frender::Window::mainloop(Renderer* renderer)
{
    double time = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        glfwMakeContextCurrent(window);
        glfwPollEvents();

        float delta = glfwGetTime() - time;
        time = glfwGetTime();

        // TODO: Run callback here
        renderer->render(delta);

        glfwSwapBuffers(window);
    }
}

Frender::Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

// glfwDestroyWindow(window) at end
// glfwTerminate() at end
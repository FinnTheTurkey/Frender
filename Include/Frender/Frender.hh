#ifndef FRENDER_HH
#define FRENDER_HH

#include <string>
class GLFWwindow;

namespace Frender
{
    class Renderer;


    struct WindowSettings
    {
        int width;
        int height;

        std::string title;
    };

    class Window
    {
    public:
        Window(WindowSettings settings);

        void mainloop(Renderer* renderer);
    private:
        GLFWwindow* window;
    };

    class Renderer
    {
    public:
        void render(float delta);
    };
}

#endif
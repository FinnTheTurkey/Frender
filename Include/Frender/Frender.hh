#ifndef FRENDER_HH
#define FRENDER_HH

#include <string>
#include <map>
#include <variant>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "Frender/GLTools.hh"

class GLFWwindow;

namespace Frender
{
    class Renderer;

    // Typedefs
    typedef GLTools::UniformType UniformType;
    enum RenderType
    {
        Bulk,
        Detail
    };
    typedef uint32_t MeshRef;
    typedef GLTools::Vertex Vertex;

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
        ~Window();

        void mainloop(Renderer* renderer);

        void _sizeCallback(int width, int height);
    private:
        GLFWwindow* window;
        Renderer* renderer;
    };

    struct Material
    {
        GLTools::UniformBuffer uniforms;
        GLTools::Shader shader;
        RenderType type;
    };

    /*
    Seperate MaterialRef class so we don't need to have materials as pointers
    MaterialRef has all the info OpenGL would need to render the
    given material, but doesn't have any of the actual data
    */
    struct MaterialRef
    {
        Material* mat_ref;
        GLTools::UniformRef uniforms;
        GLTools::Shader shader;
    };

    /**
    Struct with all the nessesary data to render a materialed mesh
    */
    struct RenderObject
    {
        uint32_t* pos;
        glm::mat4 transform;
        MaterialRef mat;
        GLTools::MeshBuffer mesh;
    };

    class RenderObjectRef
    {
    public:
        RenderObjectRef(uint32_t* index, Renderer* renderer):index(index), renderer(renderer) {}
        // TODO: Add functions for stuff

    private:
        uint32_t* index;
        Renderer* renderer;
    };

    class Renderer
    {
    public:
        Renderer(int width, int height);

        void render(float delta);

        /**
        Creates an empty material.
        The material can be customised by changing uniforms.
        If no shaders are specified, it will use the bulk shader.
        Only Materials based on the bulk shader can be used by the bulk renderer
        */
        Material* createMaterial();
        Material* createMaterial(GLTools::Shader shader);

        /**
        Uploads a mesh to the GPU. No copy of the mesh is stored on the CPU
        */
        MeshRef createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        /**
        Creates a render object - aka a thing with a mesh and a material
        This thing will appear when the scene is rendered
        */
        RenderObjectRef createRenderObject(MeshRef mesh, Material* mat, glm::mat4 transform);

        void setCamera(const glm::mat4& matrix);

        /**
        Sets the resolution the scene should be rendered at
        */
        void setRenderResolution(int new_width, int new_height);

        // Settings
        float fov_rad = 1.57;
        float near_distance = 0.01;
        float far_distance = 100;

    private:
        /** Shaders used for Stage1 of the Bulk rendering process */
        GLTools::Shader stage1_bulk_shader;

        // Pools
        std::vector<Material> materials;
        std::vector<RenderObject> render_objects;
        std::vector<GLTools::MeshBuffer> meshes;

        // Useful info
        glm::mat4 camera;
        glm::mat4 inv_camera;
        glm::mat4 projection;

        // Less easily changed settings
        int width;
        int height;

        // Functions
        void bulkRender();
    };
}

#endif
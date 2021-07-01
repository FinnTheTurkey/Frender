#ifndef FRENDER_HH
#define FRENDER_HH

#include <string>
#include <map>
#include <variant>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <functional>

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
    typedef GLTools::Texture Texture;

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

        void mainloop(Renderer* renderer, std::function<void(float)> fn);

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
        GLTools::TextureManager textures;
    };

    /*
    Seperate MaterialRef class so we don't need to have materials as pointers
    MaterialRef has all the info OpenGL would need to render the
    given material, but doesn't have any of the actual data
    */
    struct MaterialRef
    {
        uint32_t mat_ref;
        // GLTools::TextureManager* textures; // Unfortunately, ptr is needed
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
        RenderObjectRef():renderer(nullptr) {};
        RenderObjectRef(uint32_t* index, Renderer* renderer):index(index), renderer(renderer) {}
        
        glm::mat4 getTransform();
        void setTransform(glm::mat4 t);

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
        uint32_t createMaterial();
        uint32_t createMaterial(GLTools::Shader shader);

        /**
        Gets the material as a pointer
        NOTE: This pointer is not guarenteed to always point
        at the material, and therefore shouldn't be stored anywhere
        */
        Material* getMaterial(uint32_t material);

        /**
        Creates a texture
        */
        Texture createTexture(int width, int height, const unsigned char* data);

        /**
        Uploads a mesh to the GPU. No copy of the mesh is stored on the CPU
        */
        MeshRef createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        /**
        Creates a render object - aka a thing with a mesh and a material
        This thing will appear when the scene is rendered
        */
        RenderObjectRef createRenderObject(MeshRef mesh, uint32_t mat, glm::mat4 transform);

        void setCamera(const glm::mat4& matrix);

        /**
        Sets the resolution the scene should be rendered at
        */
        void setRenderResolution(int new_width, int new_height);

        // Settings
        float fov_rad = 1.57;
        float near_distance = 0.01;
        float far_distance = 100;

        RenderObject* _getRenderObject(uint32_t* index)
        {
            return &render_objects[(*index)];
        }

    private:
        /** Shaders used for Stage1 of the Bulk rendering process */
        GLTools::Shader stage1_bulk_shader;

        // Pools
        std::vector<Material> materials;
        std::vector<RenderObject> render_objects;
        std::vector<GLTools::MeshBuffer> meshes;
        std::vector<Texture> textures;

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
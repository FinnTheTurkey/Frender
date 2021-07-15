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

        void setWindowTitle(const std::string& title);

        void setVsync(bool value);

        // Stats
        // The time_time is the time it took the engine to process and render the frame,
        double time_time;

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
        uint32_t mesh_index;
    };

    class RenderObjectRef
    {
    public:
        RenderObjectRef():renderer(nullptr) {};
        RenderObjectRef(uint32_t* index, Renderer* renderer):index(index), renderer(renderer) {}
        
        glm::mat4 getTransform();
        void setTransform(glm::mat4 t);

        RenderObjectRef duplicate();

    private:
        uint32_t* index;
        Renderer* renderer;
    };

    struct PointLight
    {
        glm::vec3 color;
        glm::vec3 position;
        float radius;
    };

    struct DirectionLight
    {
        glm::vec3 color;
        glm::vec3 direction;
    };

    struct _LightUniforms
    {
        uint32_t width;
        uint32_t height;
        uint32_t color;
        uint32_t cam_pos;
        uint32_t light_pos;
        uint32_t radius;
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

        /**
        Creates a point light, the most basic type of light
        */
        uint32_t createPointLight(glm::vec3 position, glm::vec3 color, float radius);

        /**
        Creates a direction light. Directional lights always apply to all elements of a scene
        */
        uint32_t createDirectionalLight(glm::vec3 color, glm::vec3 direction);

        // Settings
        float fov_rad = 1.57;
        float near_distance = 0.01;
        float far_distance = 100;

        /// How to scale the resolutions of the bloom passes
        /// Set to 0 for no bloom
        float bloom_res_scale = 0;
        /// How many passes of blur should be applied to the bloom
        float bloom_blur_amount = 5;
        /// How bright the bloom is
        float bloom_exposure = 1;

        // Stats
        uint64_t frame_count;
        double frame_rate;
        double frame_time;

        RenderObject* _getRenderObject(uint32_t* index)
        {
            return &render_objects[(*index)];
        }

    private:
        /** Shaders used for Stage1 of the Bulk rendering process */
        GLTools::Shader stage1_bulk_shader;

        // Stage 2 shaders aka lighting pass
        GLTools::Shader stage2_light_shader;
        _LightUniforms light_uniforms;
        GLTools::Shader stage2_dlight_shader;
        _LightUniforms dlight_uniforms;

        // Shaders used to show the final result - post processing goes here
        GLTools::Shader stage3_shader;
        uint32_t bloom_exposure_loc;
        GLTools::Shader bloom_shader;
        uint32_t bloom_horizontal_loc;

        // The plane which the final image is rendered on
        GLTools::MeshBuffer plane;

        // Mesh which represents a light
        GLTools::MeshBuffer light_sphere;

        // Stage 2 framebuffer and materials
        GLTools::Framebuffer stage2_fbo;
        GLTools::TextureManager stage2_tex;
        GLTools::TextureManager stage2_texd;
        bool has_stage2;

        // Stage 3 framebuffer and materials
        GLTools::Framebuffer stage3_fbo;
        GLTools::TextureManager stage3_tex;
        bool has_stage3;

        // FBOs and textures for bloom
        GLTools::Framebuffer bloom_fbo1;
        GLTools::TextureManager bloom_tex1;
        GLTools::Framebuffer bloom_fbo2;
        GLTools::TextureManager bloom_tex2;

        // Pools
        std::vector<Material> materials;
        std::vector<RenderObject> render_objects;
        std::vector<GLTools::MeshBuffer> meshes;
        std::vector<Texture> textures;

        // Pools of lights
        std::vector<PointLight> point_lights;
        std::vector<DirectionLight> directional_lights;

        // Useful info
        glm::mat4 camera;
        glm::mat4 inv_camera;
        glm::mat4 projection;

        // Less easily changed settings
        int width;
        int height;
        
        // Internal stats
        double elapsed_time;
        double elapsed_frames;

        // Functions
        void bulkRender();
        void processBloom();
    };
}

#endif
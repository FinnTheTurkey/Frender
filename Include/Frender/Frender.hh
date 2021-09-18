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

    enum MouseMode
    {
        Regular, Captured
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

        // Input handling
        void _keyPressCallback(int key, bool state);
        void _buttonPressCallback(int button, bool state);
        void _resetCursorMode();

        bool isKeyDown(int key);

        bool isKeyJustPressed(int key);

        bool isKeyJustReleased(int key);

        bool isMouseButtonDown(int button);

        bool isMouseButtonJustPressed(int button);

        bool isMouseButtonJustReleased(int button);

        glm::vec2 getMousePosition();

        glm::vec2 getMouseOffset();

        void setMouseMode(MouseMode mode);

        // TODO: Scrolling and joystick

        // Stats
        // The time_time is the time it took the engine to process and render the frame,
        double time_time;

    private:
        GLFWwindow* window;
        Renderer* renderer;

        std::unordered_map<int, bool> keys_down;
        std::unordered_map<int, bool> just_pressed;
        std::unordered_map<int, bool> just_released;

        std::unordered_map<int, bool> button_down;
        std::unordered_map<int, bool> button_just_pressed;
        std::unordered_map<int, bool> button_just_released;

        glm::vec2 last_mouse_pos = glm::vec2(0, 0);
        glm::vec2 mouse_offset = glm::vec2(0, 0);
        MouseMode mmode = Regular;
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

    enum RenderObjectTypes
    {
        Lit,
        Unlit,
        ForwardLit,
        Transparent
    };

    /**
    Simple struct with all the info required to re-create a renderobject
    */
    struct RenderObjectTraits
    {
        RenderObjectTypes type;
        GLTools::Shader shader;
        uint32_t material;
        MeshRef mesh;
        glm::mat4 transform;
    };

    struct RenderObjectLocator
    {
        RenderObjectLocator() {}
        RenderObjectLocator(RenderObjectTypes type, int* shader_section, int* mat_section, int *mesh_section, int *index):
        type(type), shader_section(shader_section), mat_section(mat_section),
        mesh_section(mesh_section), index(index) {}

        RenderObjectTypes type;
        int* shader_section = nullptr;
        int* mat_section = nullptr;
        int* mesh_section = nullptr;
        int* index = nullptr;
    };

    class RenderObjectRef
    {
    public:
        RenderObjectRef():renderer(nullptr) {};
        RenderObjectRef(RenderObjectTypes type, int *shader_section, int *mat_section, int *mesh_section, int *index, Renderer* renderer)
        :loc(type, shader_section, mat_section, mesh_section, index), renderer(renderer) {}
        
        glm::mat4 getTransform();
        void setTransform(glm::mat4 t);

        RenderObjectRef duplicate();

        RenderObjectLocator loc;
        Renderer* renderer = nullptr;

    private:
        
    };

    struct PointLight
    {
        glm::vec3 color;
        glm::vec3 position;
        float radius;
        glm::mat4 transform;
        int light_id;
    };

    struct DirectionLight
    {
        glm::vec3 color;
        glm::vec3 direction;
        int light_id;
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

    struct BoundingBox
    {
        glm::vec3 og_min;
        glm::vec3 og_max;

        glm::vec3 min_pos;
        glm::vec3 max_pos;

        void transformBoundingBox(glm::mat4 transform)
        {
            glm::vec4 box_mesh[8];
            glm::vec4 global_min(transform * glm::vec4(og_min, 1)), global_max(transform * glm::vec4(og_max, 1));

            // Manually add the points
            // Front
            box_mesh[0] = glm::vec4(global_min.x, global_min.y, global_min.z, 1);
            box_mesh[1] = glm::vec4(global_max.x, global_min.y, global_min.z, 1);
            box_mesh[2] = glm::vec4(global_min.x, global_max.y, global_min.z, 1);
            box_mesh[3] = glm::vec4(global_max.x, global_max.y, global_min.z, 1);

            // Back
            box_mesh[4] = glm::vec4(global_min.x, global_min.y, global_max.z, 1);
            box_mesh[5] = glm::vec4(global_max.x, global_min.y, global_max.z, 1);
            box_mesh[6] = glm::vec4(global_min.x, global_max.y, global_max.z, 1);
            box_mesh[7] = glm::vec4(global_max.x, global_max.y, global_max.z, 1);

            // Now find the new biggest and smallest
            // Start with a position in the mesh for the min and max
            min_pos = glm::vec3(box_mesh[0].x, box_mesh[0].y, box_mesh[0].z);
            max_pos = glm::vec3(box_mesh[0].x, box_mesh[0].y, box_mesh[0].z);

            for (int i = 0; i < 8; i++)
            {
                auto point = box_mesh[i];

                // Do each X, Y, and Z individually to make sure we cover everything
                if (point.x < min_pos.x)
                {
                    min_pos.x = point.x;
                }
                if (point.y < min_pos.y)
                {
                    min_pos.y = point.y;
                }
                if (point.z < min_pos.z)
                {
                    min_pos.z = point.z;
                }

                if (point.x > max_pos.x)
                {
                    max_pos.x = point.x;
                }
                if (point.y > max_pos.y)
                {
                    max_pos.y = point.y;
                }
                if (point.z > max_pos.z)
                {
                    max_pos.z = point.z;
                }
            }
        }
    };

    struct ROInfo
    {
        int* index;
        glm::mat4 model;
        BoundingBox bounding_box;
    };

    struct ROInfoGPU
    {
        glm::mat4 mvp;
        glm::mat4 model;
    };

    struct ROInfoLit : public ROInfo
    {
        std::array<int32_t, 8> lights;
        std::array<int32_t, 8> complete_lights;
        glm::vec3 minima_indexes;
        glm::vec3 maxima_indexes;
    };

    struct ROInfoGPULit
    {
        glm::mat4 mvp;
        glm::mat4 model;
        glm::vec4 lights1;
        glm::vec4 lights2;
    };

    template <typename CpuT, typename GpuT>
    struct MeshSection
    {
        MeshRef mesh;
        GLTools::VertexArray* vao;

        std::vector<CpuT> cpu_info;
        GLTools::Buffer<GpuT>* gpu_buffer; // Buffer must be ptr because vector can re-allocate
        int* index;
        std::vector<int*> indicies;
    };

    template <typename MeshT>
    struct MatSection
    {
        MaterialRef mat;
        std::vector<MeshT> meshes;
        int* index;
    };

    template <typename MatT>
    struct ShaderSection
    {
        GLTools::Shader shader;
        std::vector<MatT> mats;
        int* index;
    };

    struct Extrema
    {
        enum ExtremaType
        {
            Minima, Maxima
        };
        enum ObjectType
        {
            Light, Object
        };

        ExtremaType et;
        ObjectType obj;

        glm::vec3 position;

        int light;

        RenderObjectLocator loc;
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
        Creates a material that is not effected by lighting.
        If bloom is enabled, supports emmisive materials
        */
        uint32_t createUnlitMaterial(float emmisive = 0);

        /**
        Creates an empty material for Forward rendered lit objects
        */
        uint32_t createLitMaterial()
        {
            return createMaterial(lit_shader);
        }

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

        /**
        Creates a forward rendered lit render object
        */
        RenderObjectRef createLitRenderObject(GLTools::Shader shader, MeshRef mesh, uint32_t mat, glm::mat4 transform);
        RenderObjectRef createLitRenderObject(MeshRef mesh, uint32_t mat, glm::mat4 transform)
        {
            return createLitRenderObject(lit_shader, mesh, mat, transform);
        };

        /**
        Creates a RenderObject that doesn't require lighting.
        This function requires a shader to render the unlit object with
        */
        RenderObjectRef createUnlitRenderObject(GLTools::Shader shader, MeshRef mesh, uint32_t mat, glm::mat4 transform);

        /**
        Duplicates a given render object
        */
        RenderObjectRef duplicateRenderObject(RenderObjectRef ro);

        /**
        Gets info about a renderobject
        */
        RenderObjectTraits getRenderObjectTraits(RenderObjectRef ro);

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

        /**
        Sets the skybox
        */
        void setSkybox(int width, int height, float* data);

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
        bool use_fxaa = false;

        // Stats
        uint64_t frame_count;
        double frame_rate;
        double frame_time;

        ROInfo* _getRenderObject(RenderObjectLocator loc)
        {
            switch (loc.type)
            {
                case (Lit):
                {
                    return &scene_tree[*loc.shader_section].mats[*loc.mat_section].meshes[*loc.mesh_section].cpu_info[*loc.index];
                }
                case (Unlit):
                {
                    return &funlit_scene_tree[*loc.shader_section].mats[*loc.mat_section].meshes[*loc.mesh_section].cpu_info[*loc.index];
                }
                case (ForwardLit):
                {
                    return &flit_scene_tree[*loc.shader_section].mats[*loc.mat_section].meshes[*loc.mesh_section].cpu_info[*loc.index];
                }
                default: break;
            }
            std::cout << "Trying to get inexistant render object\n";
            return nullptr;
        }

    private:
        /** Shaders used for Stage1 of the Bulk rendering process */
        GLTools::Shader stage1_bulk_shader;

        // Stage 2 shaders aka lighting pass
        GLTools::Shader stage2_light_shader;
        _LightUniforms light_uniforms;
        GLTools::Shader stage2_dlight_shader;
        _LightUniforms dlight_uniforms;
        GLTools::Shader stage2_alight_shader;
        _LightUniforms alight_uniforms;
        GLTools::TextureManager alight_tx;

        GLTools::Shader lit_shader;
        _LightUniforms lit_uniforms;

        // Forward rendered shaders
        GLTools::Shader unlit;

        // Shaders used to show the final result - post processing goes here
        GLTools::Shader stage3_shader;
        GLTools::Shader stage3fxaa_shader;
        uint32_t bloom_exposure_loc;
        uint32_t bloom_exposure_loc_fxaa;
        GLTools::Shader bloom_shader;
        uint32_t bloom_horizontal_loc;

        GLTools::Texture dummy_texture;

        // The plane which the final image is rendered on
        GLTools::MeshBuffer plane;

        // Mesh which represents a light
        GLTools::MeshBuffer light_sphere;
        GLTools::VertexArray light_sphere_vao;
        GLTools::Buffer<PointLight> point_light_buffer;

        // Stage 2 framebuffer and materials
        GLTools::Framebuffer stage2_fbo;
        GLTools::TextureManager stage2_tex;
        GLTools::TextureManager stage2_texd;
        bool has_stage2 = false;

        // Stage 3 framebuffer and materials
        GLTools::Framebuffer stage3_fbo;
        GLTools::TextureManager stage3_tex;
        bool has_stage3 = false;

        // FBOs and textures for bloom
        GLTools::Framebuffer bloom_fbo1;
        GLTools::TextureManager bloom_tex1;
        GLTools::Framebuffer bloom_fbo2;
        GLTools::TextureManager bloom_tex2;

        // Pools
        std::vector<Material> materials;
        // std::vector<RenderObject> render_objects;
        // std::vector<GLTools::MeshBuffer> meshes;
        std::vector<GLTools::Buffer<Vertex>*> meshes;
        std::vector<std::pair<uint32_t, GLTools::Buffer<uint32_t>*>> indices;
        std::vector<BoundingBox> bounding_boxes;
        std::vector<Texture> textures;

        // Scene tree for the bulk renderer
        /*
        Material 1
        ┌────────────────────────────────────────────────────────────┐
        │┼─────────────────────┼┼────────────────────┼┼──────────────┤
        ││Mesh 1               ││Mesh 2              ││Mesh 3        │
        │┼┬───────────────────┬┼│                    ││              │
        │┼│Buffer             │┼│                    ││              │
        │┼│                   │┼│                    ││              │
        │┼┼───────────────────┼┼│                    ││              │
        │┼│Info on CPU        │┼│                    ││              │
        │┼┼───────────────────┼┼│                    ││              │
        │┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼────────────────────┼┼──────────────┤
        └────────────────────────────────────────────────────────────┘
        */
        std::vector<ShaderSection<MatSection<MeshSection<ROInfo, ROInfoGPU>>>> scene_tree;

        std::vector<ShaderSection<MatSection<MeshSection<ROInfo, ROInfoGPU>>>> funlit_scene_tree;

        // Broad phase sweep and prune for forward lit objects
        std::array<std::vector<Extrema>, 3> broad_phase;

        // Scene tree for forward lit
        std::vector<ShaderSection<MatSection<MeshSection<ROInfoLit, ROInfoGPULit>>>> flit_scene_tree;

        glm::vec3 addExtrema(Extrema ex);

        // Pools of lights
        std::vector<PointLight> point_lights;
        std::vector<DirectionLight> directional_lights;
        GLTools::UniformBuffer light_buffer;
        int light_index; // TODO: id reuse queue

        // Skybox and ibl state
        bool has_skybox = false;
        Texture sky_cubemap;
        Texture irradiance_cubemap;
        Texture prefilter_cubemap;
        Texture brdf_precomputed;
        GLTools::MeshBuffer cube;
        GLTools::Shader equiToCubemap_shader;
        GLTools::Shader equiToCubemap_convolution_shader;
        GLTools::Shader equiToCubemap_prefilter_shader;
        GLTools::Shader skybox_shader;
        GLTools::TextureManager skybox_textures;
        uint32_t skybox_vp_loc;

        // Useful info
        glm::mat4 camera;
        glm::mat4 inv_camera;
        glm::mat4 projection;

        // Planes for frustum culling
        glm::vec4 frustum_planes[6];

        // Less easily changed settings
        int width;
        int height;
        
        // Internal stats
        double elapsed_time;
        double elapsed_frames;

        // Functions
        void bulkRender();
        void geometryPass(glm::mat4 vp);
        void unlitRender(glm::mat4 vp);
        void litRender(glm::mat4 vp);
        void calculateLighting(glm::mat4 vp);
        bool frustumCull(glm::vec3 min, glm::vec3 max);
        void processBloom();

        template <typename ROCpu, typename ROGpu>
        Frender::RenderObjectRef _addRenderObject(std::vector<ShaderSection<MatSection<MeshSection<ROCpu, ROGpu>>>>& scene, ROCpu cpu, ROGpu gpu, GLTools::Shader shader, MeshRef mesh, uint32_t mat, glm::mat4 transform, int rows = 8);
    };
}

#endif
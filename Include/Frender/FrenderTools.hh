#ifndef FRENDERTOOLS_HH
#define FRENDERTOOLS_HH

#include "Frender/Frender.hh"

namespace FrenderTools
{
    template <typename T>
    struct _RenderThing
    {
        glm::mat4 local_transform;
        T thing;
    };

    // TODO: Figure out how this is actually going to work

    /**
    Helper class that combines any type of Frender object, and allows them to be transformed together
    */
    class RenderGroup
    {
    public:
        void addRenderObject(Frender::RenderObjectRef ro);
        void addRenderGroup(RenderGroup group);

        // TODO: Add light

        void setTransform(glm::mat4 new_transform);
        void setGlobalTransform(glm::mat4 new_transform);
        glm::mat4 getTransform() const {return transform;};

        void applyTransform();

    private:
        glm::mat4 transform;
        glm::mat4 global_transform; // Global transform does NOT include transform

        std::vector<_RenderThing<Frender::RenderObjectRef>> render_objects;
        std::vector<_RenderThing<RenderGroup>> render_groups;
    };
}

#endif
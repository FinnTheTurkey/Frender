#include "Frender/Frender.hh"
#include "Frender/FrenderTools.hh"
#include "glm/detail/type_mat.hpp"

// =================================================
// RenderGroup
// =================================================
void FrenderTools::RenderGroup::addRenderObject(Frender::RenderObjectRef ro)
{
    // We need to make sure the RenderObject stays in the same
    // position in 3d space, even though we've switched it
    // from a global transform to a local transform

    glm::mat4 ro_transform = ro.getTransform();

    // Take off our global transform
    glm::mat4 ro_local_transform = glm::inverse(global_transform * transform) * ro_transform;

    render_objects.push_back({ro_local_transform, ro});
}


void FrenderTools::RenderGroup::addRenderGroup(RenderGroup rg)
{
    // We need to make sure the RenderObject stays in the same
    // position in 3d space, even though we've switched it
    // from a global transform to a local transform

    glm::mat4 ro_transform = rg.getTransform();

    // Take off our global transform
    glm::mat4 ro_local_transform = glm::inverse(global_transform * transform) * ro_transform;
    rg.setGlobalTransform(global_transform * transform);
    rg.setTransform(ro_local_transform);

    render_groups.push_back({ro_local_transform, rg});
}

void FrenderTools::RenderGroup::setGlobalTransform(glm::mat4 new_transform)
{
    global_transform = new_transform;
}

void FrenderTools::RenderGroup::setTransform(glm::mat4 new_transform)
{
    transform = new_transform;
    applyTransform();
}

void FrenderTools::RenderGroup::applyTransform()
{
    // Calculate and apply global transforms for every object
    glm::mat4 translator = global_transform * transform;
    for (auto i : render_objects)
    {
        i.thing.setTransform(translator * i.local_transform);
    }

    for (auto i : render_groups)
    {
        i.thing.setGlobalTransform(translator);
        i.thing.applyTransform();
    }
}
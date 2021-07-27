Frender
=======

Frender is a barebones, work in progress 3D renderer which uses the OpenGL ES 3.0 API.

Features
--------
 - Deferred Renderer
 - Light Volumes (kinda broken)
 - PBR Materials (also kinda broken)
 - Automatic mesh instancing to reduce draw calls and CPU overhead
 - Frustum Culling
 - Very broken bloom support
 - HDR rendering pipeline (though it _cannot_ output in HDR for HDR moniters)

Planned Features
---------------
 - Forward renderer with transparency support
 - Anti aliasing and post processing
 - Cubemaps and environment maps
 - Make everything broken not broken

How to build
============
It uses CMake. It isn't difficult. Just remember to clone with `--recursive` to pull in all the third pary modules

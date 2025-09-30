#include "scene/conic.hpp"

namespace cg
{

ConicSurface::ConicSurface() {}

ConicSurface::ConicSurface(float    bottom_radius,
                           float    top_radius,
                           uint32_t num_sides,
                           uint32_t num_stacks,
                           int32_t  position_loc,
                           int32_t  normal_loc)
{
    // Fail if top and bottom radius are both 0
    if(bottom_radius <= 0.0f && top_radius <= 0.0f) return;

    // Student to define. Module 6. Used to create the curved surface of a cylinder, cone, or
    // truncated cone.

    create_vertex_buffers(position_loc, normal_loc);
}

} // namespace cg

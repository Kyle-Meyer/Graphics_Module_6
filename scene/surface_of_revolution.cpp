#include "scene/surface_of_revolution.hpp"

namespace cg
{

SurfaceOfRevolution::SurfaceOfRevolution() {}

SurfaceOfRevolution::SurfaceOfRevolution(std::vector<Point3> &v,
                                         uint32_t             n,
                                         int32_t              position_loc,
                                         int32_t              normal_loc)
{
    // Student to define. Module 6.

    // Create VBOs and VAO
    create_vertex_buffers(position_loc, normal_loc);
}

} // namespace cg

#include "scene/unit_square.hpp"

namespace cg
{

UnitSquareSurface::UnitSquareSurface() {}

UnitSquareSurface::UnitSquareSurface(uint32_t n, int32_t position_loc, int32_t normal_loc)
{
    // Student to define. Module 6. Create vertex list and face list for subdivided unit square

    // Create VBOs and VAO
    create_vertex_buffers(position_loc, normal_loc);
}

} // namespace cg

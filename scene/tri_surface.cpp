#include "scene/tri_surface.hpp"

namespace cg
{

TriSurface::TriSurface() : vao_{0}, vbo_{0}, facebuffer_{0}, GeometryNode() {}

TriSurface::~TriSurface()
{
    // Delete vertex buffer objects
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &facebuffer_);
    glDeleteVertexArrays(1, &vao_);
}

void TriSurface::draw(SceneState &scene_state)
{
    // Student to define: Module 6
}

void TriSurface::construct(const std::vector<VertexAndNormal> &v, const std::vector<uint16_t> &f)
{
    vertices_ = v;
    faces_ = f;
}

void TriSurface::add(const Point3 &v0, const Point3 &v1, const Point3 &v2)
{
    faces_.push_back(add_vertex(v0));
    faces_.push_back(add_vertex(v1));
    faces_.push_back(add_vertex(v2));
}

void TriSurface::end(int32_t position_loc, int32_t normal_loc)
{
    // Student to define - Module 6 (needed for Teapot)

    // Create the vertex and face buffers
    create_vertex_buffers(position_loc, normal_loc);
}

void TriSurface::create_vertex_buffers(int32_t position_loc, int32_t normal_loc)
{
    // Student to define - Module 6
}

uint16_t TriSurface::add_vertex(const Point3 &vtx)
{
    // Check if vertex is in the list. This is just a brute force method.
    // Efficiency can be improved but we only use this at startup
    uint16_t index = 0;
    for(const auto &v : vertices_)
    {
        if(vtx == v.vertex) { return index; }
        index++;
    }

    // Not in the list, add it. Make sure the vertex normal is initialized
    // to (0,0,0)
    VertexAndNormal vertex_and_normal(vtx);
    vertices_.push_back(vertex_and_normal);
    return static_cast<uint16_t>(vertices_.size() - 1);
}

} // namespace cg

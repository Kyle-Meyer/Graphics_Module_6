#include "Module5/unit_square_node.hpp"

#include "geometry/types.hpp"

#include <vector>

namespace cg
{

UnitSquare::UnitSquare(int32_t position_loc, int32_t normal_loc) : GeometryNode()
{
    std::vector<VertexAndNormal> vertex_list;
    VertexAndNormal              vtx;
    vtx.normal.x = 0.0f;
    vtx.normal.y = 0.0f;
    vtx.normal.z = 1.0f;
    vtx.vertex.x = -0.5f;
    vtx.vertex.y = 0.5f;
    vtx.vertex.z = 0.0f;
    vertex_list.push_back(vtx);
    vtx.vertex.x = -0.5f;
    vtx.vertex.y = -0.5f;
    vertex_list.push_back(vtx);
    vtx.vertex.x = 0.5f;
    vtx.vertex.y = 0.5f;
    vertex_list.push_back(vtx);
    vtx.vertex.x = 0.5f;
    vtx.vertex.y = -0.5f;
    vertex_list.push_back(vtx);
    vertex_count_ = static_cast<GLsizei>(vertex_list.size());

    // Create a buffer object and load the data
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_count_ * sizeof(VertexAndNormal),
                 (GLvoid *)&vertex_list[0].vertex.x,
                 GL_STATIC_DRAW);

    // Allocate a VAO, enable it and set the vertex attribute arrays and pointers
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Bind the VBO, set vertex attribute pointers for position and normal (using stride and
    // offset). Enable the arrays.
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAndNormal), (void *)0);
    glEnableVertexAttribArray(position_loc);
    glVertexAttribPointer(
        normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAndNormal), (void *)(sizeof(Point3)));
    glEnableVertexAttribArray(normal_loc);

    // Make sure changes to this VAO are local
    glBindVertexArray(0);
}

UnitSquare::~UnitSquare() {}

void UnitSquare::draw(SceneState &scene_state)
{
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count_);
    glBindVertexArray(0);
}

} // namespace cg

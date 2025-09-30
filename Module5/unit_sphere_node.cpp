#include "Module5/unit_sphere_node.hpp"

#include "geometry/geometry.hpp"

#include <cmath>

namespace cg
{

UnitSphere::UnitSphere(uint32_t num_lat,
                       uint32_t num_lon,
                       int32_t  position_loc,
                       int32_t  normal_loc) :
    GeometryNode()
{
    // Convert to radians
    float min_lat_r = degrees_to_radians(-90.0f);
    float max_lat_r = degrees_to_radians(90.0f);
    float min_lon_r = degrees_to_radians(-180.0f);
    float max_lon_r = degrees_to_radians(180.0f);

    // Compute delta lat,lng
    float d_lat = (max_lat_r - min_lat_r) / static_cast<float>(num_lat);
    float d_lng = (max_lon_r - min_lon_r) / static_cast<float>(num_lon);

    // Create vertex list. Since vertex position and normal are equal we
    // can use just Point3 in the VBO.
    std::vector<Point3> vertex_list;
    for(float lat1 = min_lat_r, lat2 = lat1 + d_lat; lat2 <= max_lat_r + EPSILON;
        lat1 = lat2, lat2 += d_lat)
    {
        float cos_lat1 = std::cos(lat1);
        float sin_lat1 = std::sin(lat1);
        float cos_lat2 = std::cos(lat2);
        float sin_lat2 = std::sin(lat2);

        // Iterate over longitude, connecting a vertex on the next latitude row with one
        // on the current latitude row. Since we wrap around to the same longitude we do
        // not need to repeat vertices to create degenerate triangles to connect each row.
        for(float lon = min_lon_r; lon <= max_lon_r + EPSILON; lon += d_lng)
        {
            float cos_lon = std::cos(lon);
            float sin_lon = std::sin(lon);

            // Add a vertex on the upper latitude
            vertex_list.emplace_back(cos_lon * cos_lat2, sin_lon * cos_lat2, sin_lat2);

            // Add a vertex on the lower latitude
            vertex_list.emplace_back(cos_lon * cos_lat1, sin_lon * cos_lat1, sin_lat1);
        }
    }
    vertex_count_ = static_cast<GLsizei>(vertex_list.size());

    // Create a buffer object and load the data
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_count_ * sizeof(Point3),
                 (GLvoid *)&vertex_list[0].x,
                 GL_STATIC_DRAW);

    // Allocate a VAO, enable it
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Bind the VBO, set vertex attribute pointers for position and normal.
    // Enable the arrays.
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(position_loc);
    glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(normal_loc);

    // Make sure changes to this VAO are local
    glBindVertexArray(0);
}

UnitSphere::~UnitSphere() {}

void UnitSphere::draw(SceneState &scene_state)
{
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count_);
    glBindVertexArray(0);
}

} // namespace cg

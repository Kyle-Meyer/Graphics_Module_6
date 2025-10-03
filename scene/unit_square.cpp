#include "scene/unit_square.hpp"
#include "geometry/types.hpp"

namespace cg
{

UnitSquareSurface::UnitSquareSurface() {}

UnitSquareSurface::UnitSquareSurface(uint32_t n, int32_t position_loc, int32_t normal_loc)
{
   float step = 1.0f / static_cast<float>(n);

   //create vertices (n+1) x (n+1) grid 
   for(uint32_t i = 0; i <= n; i++)
   {
      for(uint32_t j = 0; j <= n; j++)
      {
         //calculate the positions in range -0.5 and 0.5
         float x = -0.5f + static_cast<float>(j) * step;
         float y = -0.5f + static_cast<float>(i) * step;
         float z = 0.0f;

         //create vertex with position and normal 
         VertexAndNormal vertex;
         vertex.vertex = Point3(x, y, z);
         vertex.normal = Vector3(0.0f, 0.0f, 1.0f); //normal always points up 

         vertices_.push_back(vertex);
      }
   }

   //create faces with 2 triangles per square 
   for(uint32_t i = 0; i < n; i++)
   {
      for(uint32_t j = 0; j < n; j++)
      {
         //calculate the vertices 
         uint16_t v0 = static_cast<uint16_t>(i * (n + 1) + j); //bottom left 
         uint16_t v1 = static_cast<uint16_t>(i * (n + 1) + j + 1); //bottom right
         uint16_t v2 = static_cast<uint16_t>((i + 1) * (n + 1) + j); //top left 
         uint16_t v3 = static_cast<uint16_t>((i + 1) * (n + 1) + j + 1); //top right 

         //first triangle CCW
         faces_.push_back(v0);
         faces_.push_back(v1);
         faces_.push_back(v2);

         //second triangle CCW 
         faces_.push_back(v1);
         faces_.push_back(v3);
         faces_.push_back(v2);
      }
   }
    // Create VBOs and VAO
    create_vertex_buffers(position_loc, normal_loc);
}

} // namespace cg

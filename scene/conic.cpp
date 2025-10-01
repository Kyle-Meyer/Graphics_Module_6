#include "scene/conic.hpp"
#include <cmath>

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

   //create the side surface 
   //height is 1, centered at origin 

   //calculate the slant angle for normals 
   float height = 1.0f;
   float radius_diff = bottom_radius - top_radius;

   //calculate normal direction in the r-z plane 
   //the norma is always perpendicular to the slant line 
   float slant_length = std::sqrt(radius_diff * radius_diff + height * height);
   float normal_r = height / slant_length;  //radial compliment 
   float normal_z = radius_diff / slant_length; //z compliment
   
   //create vertices 
   float z_step = height / static_cast<float>(num_stacks);
   float angle_step = 2.0f * M_PI / static_cast<float>(num_sides);

   for (uint32_t stack = 0; stack <= num_stacks; ++stack)
   {
       // Calculate z position and radius at this height
       float t = static_cast<float>(stack) / static_cast<float>(num_stacks);
       float z = -0.5f + t * height;
       float radius = bottom_radius + t * (top_radius - bottom_radius);
       
       for (uint32_t side = 0; side <= num_sides; ++side)
       {
           float angle = static_cast<float>(side) * angle_step;
           
           // Calculate position
           float x = radius * std::cos(angle);
           float y = radius * std::sin(angle);
           
           // Calculate normal (rotated around z-axis)
           float nx = normal_r * std::cos(angle);
           float ny = normal_r * std::sin(angle);
           float nz = normal_z;
           
           VertexAndNormal vertex;
           vertex.vertex = Point3(x, y, z);
           vertex.normal = Vector3(nx, ny, nz);
           vertex.normal.normalize();
           
           vertices_.push_back(vertex);
       }
   }
   
   // Create faces connecting the rings of vertices
   for (uint32_t stack = 0; stack < num_stacks; ++stack)
   {
       for (uint32_t side = 0; side < num_sides; ++side)
       {
           // Calculate vertex indices for this quad
           uint16_t v0 = static_cast<uint16_t>(stack * (num_sides + 1) + side);
           uint16_t v1 = static_cast<uint16_t>(stack * (num_sides + 1) + side + 1);
           uint16_t v2 = static_cast<uint16_t>((stack + 1) * (num_sides + 1) + side);
           uint16_t v3 = static_cast<uint16_t>((stack + 1) * (num_sides + 1) + side + 1);
           
           // First triangle (counter-clockwise from outside)
           faces_.push_back(v0);
           faces_.push_back(v1);
           faces_.push_back(v2);
           
           // Second triangle (counter-clockwise from outside)
           faces_.push_back(v1);
           faces_.push_back(v3);
           faces_.push_back(v2);
       }
   }

   create_vertex_buffers(position_loc, normal_loc);
}

} // namespace cg

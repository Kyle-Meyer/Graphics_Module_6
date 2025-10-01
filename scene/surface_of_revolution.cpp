#include "scene/surface_of_revolution.hpp"
#include "geometry/vector3.hpp"
#include <cmath>

namespace cg
{

SurfaceOfRevolution::SurfaceOfRevolution() {}

SurfaceOfRevolution::SurfaceOfRevolution(std::vector<Point3> &v,
                                         uint32_t             n,
                                         int32_t              position_loc,
                                         int32_t              normal_loc)
{
   if(v.size() < 2) return;
   
   std::vector<Vector3> profile_normals;

   for(size_t i = 0; i < v.size(); i++)
   {
      Vector3 tangent;

      // First point: use forward difference
      if( i == 0 )
         tangent = Vector3(v[1].x - v[0].x, 0.0f, v[1].z - v[0].z);
      // Last point: use backward difference
      else if ( i == v.size() - 1)
         tangent = Vector3(v[i].x - v[i-1].x, 0.0f, v[i].z - v[i-1].z);
      //middle point by default 
      else 
         tangent = Vector3(v[i+1].x - v[i-1].x, 0.0f, v[i+1].z - v[i-1].z);
      
      tangent.normalize();
        
      // Normal is perpendicular to tangent in the x-z plane
      // If tangent is (tx, 0, tz), then normal is (tz, 0, -tx)
      // This points outward from the surface
      Vector3 normal(tangent.z, 0.0f, -tangent.x);
      normal.normalize();
      
      profile_normals.push_back(normal);
   }

   float angle_step = 2.0f * M_PI / static_cast<float>(n);
    
    for (uint32_t sweep = 0; sweep <= n; ++sweep)
    {
        float angle = static_cast<float>(sweep) * angle_step;
        float cos_angle = std::cos(angle);
        float sin_angle = std::sin(angle);
        
        for (size_t i = 0; i < v.size(); ++i)
        {
            // Rotate vertex around z-axis
            float x = v[i].x * cos_angle;
            float y = v[i].x * sin_angle;
            float z = v[i].z;
            
            // Rotate normal around z-axis
            float nx = profile_normals[i].x * cos_angle;
            float ny = profile_normals[i].x * sin_angle;
            float nz = profile_normals[i].z;
            
            VertexAndNormal vertex;
            vertex.vertex = Point3(x, y, z);
            vertex.normal = Vector3(nx, ny, nz);
            vertex.normal.normalize();
            
            vertices_.push_back(vertex);
        }
    }
    
    // Create faces connecting the swept profiles
    uint32_t profile_size = static_cast<uint32_t>(v.size());
    
    for (uint32_t sweep = 0; sweep < n; ++sweep)
    {
        for (uint32_t i = 0; i < profile_size - 1; ++i)
        {
            // Calculate vertex indices for this quad
            uint16_t v0 = static_cast<uint16_t>(sweep * profile_size + i);
            uint16_t v1 = static_cast<uint16_t>(sweep * profile_size + i + 1);
            uint16_t v2 = static_cast<uint16_t>((sweep + 1) * profile_size + i);
            uint16_t v3 = static_cast<uint16_t>((sweep + 1) * profile_size + i + 1);
            
            // First triangle (counter-clockwise from outside)
            faces_.push_back(v0);
            faces_.push_back(v2);
            faces_.push_back(v1);
            
            // Second triangle (counter-clockwise from outside)
            faces_.push_back(v1);
            faces_.push_back(v2);
            faces_.push_back(v3);
        }
    }
    // Create VBOs and VAO
    create_vertex_buffers(position_loc, normal_loc);
}

} // namespace cg

#include "scene/tri_surface.hpp"
#include "geometry/types.hpp"
#include <GL/glext.h>

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
   //bind the vao
   glBindVertexArray(vao_);
   
   //draw using indexed array 
   glDrawElements(GL_TRIANGLES,         //primitive type
                  faces_.size(),        //number of indices 
                  GL_UNSIGNED_SHORT,    //index type 
                  0);                   //offset into the EBO

   //unbind the vao 
   glBindVertexArray(0);
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
    //3 indices per triangle, skip by 3
   for(size_t i = 0; i < faces_.size(); i += 3)
   {
      //faces 
      uint16_t idx0 = faces_[i];
      uint16_t idx1 = faces_[i + 1];
      uint16_t idx2 = faces_[i + 2];

      //vertices
      Point3 v0 = vertices_[idx0].vertex;
      Point3 v1 = vertices_[idx1].vertex;
      Point3 v2 = vertices_[idx2].vertex;

      //calculate the edges 
      Vector3 edge1 = v1 - v0;
      Vector3 edge2 = v2 - v0;

      //calculate the normal 
      Vector3 face_normal = edge1.cross(edge2);

      //dont normalize yet because we want the weight by area 
      //hence largere triangles will contribute more to the "weight"

      //accumulate face normals with the three vertices of the triangle 
      vertices_[idx0].normal += face_normal;
      vertices_[idx1].normal += face_normal;
      vertices_[idx2].normal += face_normal;
   }
   //now we normalize
   for (auto &v : vertices_)
   {
      v.normal.normalize();
   }
   // Create the vertex and face buffers
   create_vertex_buffers(position_loc, normal_loc);
}

void TriSurface::create_vertex_buffers(int32_t position_loc, int32_t normal_loc)
{
   //generate and bind the VAO 
   glGenVertexArrays(1, &vao_);
   glBindVertexArray(vao_);

   // generate and bind the VBO
   glGenBuffers(1, &vbo_);
   glBindBuffer(GL_ARRAY_BUFFER, vbo_);
   glBufferData(GL_ARRAY_BUFFER,
                vertices_.size() * sizeof(VertexAndNormal),
                vertices_.data(),
                GL_STATIC_DRAW);

   //setup the vertex position attribute pointer
   glVertexAttribPointer(position_loc,
                         3, 
                         GL_FLOAT, 
                         GL_FALSE,
                         sizeof(VertexAndNormal), 
                         (void*)offsetof(VertexAndNormal, vertex));
   glEnableVertexAttribArray(position_loc);

   //setup the normal attribute pointer
   glVertexAttribPointer(normal_loc,
                         3, 
                         GL_FLOAT,
                         GL_FALSE,
                         sizeof(VertexAndNormal),
                         (void*)offsetof(VertexAndNormal, normal));
   glEnableVertexAttribArray(normal_loc);

   //generate and bind the element buffer object EBO 
   glGenBuffers(1, &facebuffer_);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facebuffer_);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                faces_.size() * sizeof(uint16_t),
                faces_.data(),
                GL_STATIC_DRAW);

   //unbind vao 
   glBindVertexArray(0);

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

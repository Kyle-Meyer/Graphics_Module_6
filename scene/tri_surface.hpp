//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.667 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	Brian Russin
//
//	Author:	  David W. Nesbitt
//	File:     tri_surface.hpp
//	Purpose:  Scene graph geometry node indicating a triangle mesh based
//            surface. Uses indexed vertex arrays and per vertex normals.
//
//============================================================================

#ifndef __SCENE_TRI_SURFACE_HPP__
#define __SCENE_TRI_SURFACE_HPP__

#include "scene/geometry_node.hpp"

namespace cg
{

/**
 * Triangle mesh surface. Uses indexed vertex arrays. Stores
 * vertices as VertexAndNormal.
 */
class TriSurface : public GeometryNode
{
  public:
    /**
     * Constructor.
     */
    TriSurface();

    /**
     * Destructor.
     */
    ~TriSurface();

    /**
     * Draw this geometry node.
     */
    void draw(SceneState &scene_state) override;

    /**
     * Construct triangle surface by passing in vertex list and face list
     * @param  v  List of vertices (position and normal)
     * @param  f    Index list for triangles
     */
    void construct(const std::vector<VertexAndNormal> &v, const std::vector<uint16_t> &f);

    /**
     * Adds the vertices of the triangle to the vertex list. Accounts for
     * shared vertices by checking if the vertex is already in the list.
     * Manages the face list as well - stores indexes into the vertex list
     * for this triangle. This method allows triangles to be added one at
     * a time to a curved surface. Call End() when done to get vertex normal
     * assigned. Vertices should be oriented counter-clockwise.
     * @param  v0  First vertex in the triangle
     * @param  v1  Second vertex in the triangle
     * @param  v2  Last vertex in the triangle
     */
    void add(const Point3 &v0, const Point3 &v1, const Point3 &v2);

    /**
     * Marks the end of a triangle mesh. Calculates the vertex normals.
     */
    void end(int32_t position_loc, int32_t normal_loc);

    /**
     * Creates vertex buffers for this object.
     */
    void create_vertex_buffers(int32_t position_loc, int32_t normal_loc);

  protected:
    // Vertex buffer support
    GLuint vao_;
    GLuint vbo_;
    GLuint facebuffer_;

    // Vertex and normal list
    std::vector<VertexAndNormal> vertices_;

    // Use uint16_t for face list indexes (OpenGL ES compatible)
    std::vector<uint16_t> faces_;

    /**
     * Adds a vertex to the surface vertex list.  Returns the index into the
     * vertex list.  If the vertex is already in the list it does not
     * replicate it.
     * @param  vtx  Vertex
     */
    uint16_t add_vertex(const Point3 &vtx);
};

} // namespace cg

#endif

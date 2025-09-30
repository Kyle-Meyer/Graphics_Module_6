//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.667 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	Brian Russin
//
//	Author:	David W. Nesbitt
//	File:    unit_square_node.hpp
//	Purpose:	Simple geometry node that defines a unit square.
//
//============================================================================

#ifndef __MODULE5_UNIT_SQUARE_NODE_HPP__
#define __MODULE5_UNIT_SQUARE_NODE_HPP__

#include "scene/geometry_node.hpp"

namespace cg
{

/**
 * Unit square geometry node.
 */
class UnitSquare : public GeometryNode
{
  public:
    /**
     * Constructor. Construct the vertex list for a triangle strip
     * representing a unit square in the x,y plane. Make sure the
     * vertices alternate top to bottom in y. All normals are (0,0,1).
     */
    UnitSquare(int32_t position_loc, int32_t normal_loc);

    /**
     * Destructor
     */
    ~UnitSquare();

    /**
     * Draw a unit square. Note that geometry nodes are at the
     * leaves of the tree, so no children exist.
     */
    void draw(SceneState &scene_state) override;

  protected:
    GLuint  vao_;          // Vertex Array Object
    GLuint  vbo_;          // Vertex Buffer Object
    GLsizei vertex_count_; // Number of vertices in the square
};
} // namespace cg

#endif

//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.667 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	Brian Russin
//
//	Author:	 David W. Nesbitt
//	File:    unit_sphere_node.hpp
//	Purpose: Simple geometry node that defines a unit sphere and draws
//           it with glutSolidSphere.
//
//============================================================================

#ifndef __MODULE5_UNIT_SPHERE_NODE_HPP__
#define __MODULE5_UNIT_SPHERE_NODE_HPP__

#include "scene/geometry_node.hpp"

namespace cg
{

/**
 * Unit sphere geometry node.
 */
class UnitSphere : public GeometryNode
{
  public:
    /**
     * Constructor.
     * @param  num_lat  Number of latitude stacks.
     * @param  num_lon  Number of longitude slices.
     * @param  position_loc  Location of position attribute in the vertex shader.
     * @param  normal_loc    Location of normal attribute in the vertex shader.
     */
    UnitSphere(uint32_t num_lat, uint32_t num_lon, int32_t position_loc, int32_t normal_loc);

    /**
     * Destructor
     */
    ~UnitSphere();

    /**
     * Draw the ball / sphere. Note that geometry nodes are at the
     * leaves of the tree, so no children exist.
     */
    void draw(SceneState &scene_state) override;

  protected:
    GLuint  vao_;          // Vertex Array Object
    GLuint  vbo_;          // Vertex Buffer Object
    GLsizei vertex_count_; // Number of vertices in the sphere
};

} // namespace cg

#endif

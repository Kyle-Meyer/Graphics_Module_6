//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.667 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	Brian Russin
//
//	Author:	David W. Nesbitt
//	File:    ball_transform_node.hpp
//	Purpose:	Transform node that controls the movement of a ball within
//          a confined space.
//
//============================================================================

#ifndef __MODULE5_BALL_TRANSFORM_NODE_HPP__
#define __MODULE5_BALL_TRANSFORM_NODE_HPP__

#include "scene/transform_node.hpp"

namespace cg
{

/**
 * Ball transform node.
 */
class BallTransform : public TransformNode
{
  public:
    /**
     * Constructor given the number of frames per second.
     * @param  fps  Frames per second.
     */
    BallTransform(float fps);

    /**
     * Update the scene node and its children
     */
    void update(SceneState &scene_state) override;

    // Use the base class Draw method

    /**
     * Get the current position
     * @return  Returns the current posiiton / center of the sphere.
     */
    const Point3 &get_position() const;

    /**
     * Gets the ball radius
     * @return   Returns the radius.
     */
    float get_radius() const;

    /**
     * Get the direction of travel (a unit vector).
     * @return  Returns the direction of travel.
     */
    const Vector3 &get_direction() const;

    /**
     * Gets the speed of travel.
     * @return   Returns the speed of travel
     */
    float get_speed() const;

    /**
     * Special method to set intersect time (for simple collision detection)
     * @param  t  Time until intersect (0.0 - no intersect)
     */
    void set_intersect_time(float t);

    /**
     * Get the time until nearest intersection
     * @return  Returns the time to nearest intersection (0.0 if none occurs)
     */
    float get_intersect_time() const;

    /**
     * Set the plane of intersection (use this in collision response)
     * @param  plane  Plane of intersection
     */
    void set_intersecting_plane(const Plane &plane);

    /**
     * Intersection the ray with the plane. Use a radius r to indicate a
     * sphere. The ray indicates the movement of the sphere. The return
     * value indicates the time along the ray where an intersect of a
     * sphere moving along the ray. A return value > 1.0 indicates no
     * intersection occurs.
     * @plane  plane  Plane to test intersection against
     */
    float intersect_with_plane(const Plane &plane);

    /**
     * Test if this moving ball intersect another moving ball
     * @param  ball  Other moving ball to test intersection against
     */
    bool intersect_ball(std::shared_ptr<BallTransform> ball);

  protected:
    float   radius_;    // Radius
    float   speed_;     // Speed - units per frame
    Point3  position_;  // Current position
    Vector3 direction_; // Direction vector (unit length)

    // Plane where an intersection occurs
    Plane intersecting_plane_;

    // Time of interesection (0.0 if no intersection occurs)
    float intersection_time_;

    // Set the default constructor to private to force use of the
    // one with arguments
    BallTransform();

    // Sets the transformation matrix
    void set_transform();

    // Create a random value between a specified minv and maxv.
    float get_random(float min, float max);

    float get_rand_0_1() const;
};

} // namespace cg

#endif

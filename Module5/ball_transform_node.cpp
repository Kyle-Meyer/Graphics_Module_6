#include "Module5/ball_transform_node.hpp"

namespace cg
{

BallTransform::BallTransform() {}

BallTransform::BallTransform(float fps)
{
    // Set a random initial position with x,y values between
    // -40 and 40 and z between 25 and 75
    position_.set(get_random(-40.0f, 40.0f), get_random(-40.0f, 40.0f), get_random(25.0f, 75.0f));

    // Set a random radius between 3 and 7
    radius_ = get_random(3.0f, 7.0f);

    // Set a random initial direction
    direction_.set(get_rand_0_1(), get_rand_0_1(), get_rand_0_1());
    direction_.normalize();

    // Set a random speed between 5 and 15 units per second. Convert it to
    // speed per frame.
    speed_ = get_random(5.0f, 15.0f) / fps;
    set_transform();
}

void BallTransform::update(SceneState &scene_state)
{
    // Check if intersection occured
    if(intersection_time_ != 0.0f && intersection_time_ < 1.0f)
    {
        // An intersect occured move along the current direction by parameter t,
        // reflect the direction about the normal to the intersected plane, then move
        // the position along the new direction by the remaining distance.
        position_ = position_ + direction_ * (speed_ * intersection_time_);
        direction_ = direction_.reflect(intersecting_plane_.get_normal());
        position_ = position_ + direction_ * (speed_ * (1.0f - intersection_time_));
    }
    else
    {
        // No intersection - move origin along direction vector
        position_ = position_ + (direction_ * speed_);
    }

    // Update this node's transformations
    set_transform();

    // Update all children
    SceneNode::update(scene_state);
}

const Point3 &BallTransform::get_position() const { return position_; }

float BallTransform::get_radius() const { return radius_; }

const Vector3 &BallTransform::get_direction() const { return direction_; }

float BallTransform::get_speed() const { return speed_; }

void BallTransform::set_intersect_time(float t) { intersection_time_ = t; }

float BallTransform::get_intersect_time() const { return intersection_time_; }

void BallTransform::set_intersecting_plane(const Plane &plane) { intersecting_plane_ = plane; }

float BallTransform::intersect_with_plane(const Plane &plane)
{
    // Find the signed distance of sphere at start and end of the
    // time interval. Note that speed indicates the distance moved
    // per frame
    float dc = plane.solve(position_);
    float de = plane.solve(position_ + direction_ * speed_);

    // No intersect if both dc and de are > r
    if(dc > radius_ && de > radius_) { return 100.0f; }

    // Intersect occurs when sphere first touches plane
    return (dc - radius_) / (dc - de);
}

bool BallTransform::intersect_ball(std::shared_ptr<BallTransform> ball)
{
    // Test if 2 balls already are intersecting. This can occur if an
    // intersection is missed in a prior frame due to not checking all
    // possible intersections. Since ray-sphere intersection does not return
    // a value < 0 it will not detect this case.
    Vector3 vb = ball->get_position() - get_position();
    float   vbn = vb.norm();
    if(vbn < (ball->get_radius() + radius_))
    {
        // Set the time of intersection at 0 and the plane of intersection
        // (only need the normal so a reflection can occur)
        vb *= (1.0f / vbn); // Normalize vb
        ball->set_intersect_time(0.0f);
        ball->set_intersecting_plane(Plane(ball->get_position(), vb));
        set_intersect_time(0.0f);
        set_intersecting_plane(Plane(get_position(), vb * -1.0f));
        return true;
    }

    // Create a ray at the position of this ball with direction
    // vector = difference of the 2 velocity vectors. Get the length of v
    // (note that the ray to sphere intersection requires a unit length ray
    // direction)
    Vector3 v = direction_ * speed_ - (ball->get_direction() * ball->get_speed());
    float   l = v.norm();
    Ray3    ray(position_, v * (1.0f / l));

    // Construct a bounding sphere at the center of the other ball with radius equal to
    // sum of the 2 balls
    BoundingSphere sphere(ball->get_position(), ball->get_radius() + radius_);

    auto int_result = ray.intersect(sphere);
    // float t = ray.intersect(sphere);
    if(int_result.intersects && int_result.distance < l + EPSILON)
    // if(t > EPSILON && t < l + EPSILON)
    {
        // Find the centers of both balls when intersection occurs - convert t into proper units
        int_result.distance *= 1.0f / l;
        Point3 c1 = position_ + (direction_ * (speed_ * int_result.distance));
        Point3 c2 = ball->get_position() +
                    (ball->get_direction() * (ball->get_speed() * int_result.distance));

        // Find the intersect point - it is along a vector between the 2 sphere centers
        Vector3 d = (c2 - c1).normalize();
        Point3  int_pt = c1 + d * radius_;

        // The plane of intersection is at the intersection point with normal along the vector
        // between the centers (normal will be opposite direction for each ball)

        // Set the time of intersection and the plane of intersection for both balls
        ball->set_intersect_time(int_result.distance);
        ball->set_intersecting_plane(Plane(int_pt, d));
        set_intersect_time(int_result.distance);
        set_intersecting_plane(Plane(int_pt, -1.0f * d));
        return true;
    }
    else { return false; }
}

void BallTransform::set_transform()
{
    model_matrix_.set_identity();
    model_matrix_.translate(position_.x, position_.y, position_.z);
    model_matrix_.scale(radius_, radius_, radius_);
}

float BallTransform::get_random(float min, float max)
{
    constexpr float RAND_MAX_F = static_cast<float>(RAND_MAX);
    auto            rand_f = []() { return static_cast<float>(rand()); };
    return min + ((max - min) * rand_f() / RAND_MAX_F);
}

float BallTransform::get_rand_0_1() const
{
    constexpr float RAND_MAX_F = static_cast<float>(RAND_MAX);
    auto            rand_f = []() { return static_cast<float>(rand()); };
    return rand_f() / RAND_MAX_F;
}

} // namespace cg

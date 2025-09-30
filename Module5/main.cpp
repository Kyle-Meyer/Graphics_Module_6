//============================================================================
//	Johns Hopkins University Whiting School of Engineering
//	605.667  Computer Graphics
//	Instructor:	Brian Russin
//
//	Author:  David W. Nesbitt
//	File:    Module5/main.cpp
//	Purpose: OpenGL and SDL program to draw a simple 3D animation with
//          collision detection. The scene contains moving spheres inside
//          a fixed, cube enclosure. A simple scene graph is used to define the
//          scene. Transform nodes, geometry nodes, and presentation nodes
//          are used.
//
//============================================================================

#include "filesystem_support/file_locator.hpp"
#include "geometry/geometry.hpp"
#include "scene/graphics.hpp"
#include "scene/scene.hpp"

#include "Module5/ball_transform_node.hpp"
#include "Module5/lighting_shader_node.hpp"
#include "Module5/shader_src.hpp"
#include "Module5/unit_sphere_node.hpp"
#include "Module5/unit_square_node.hpp"

#include <array>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

namespace cg
{

// Simple logging function, should be defined in the cg namespace
void logmsg(const char *message, ...)
{
    // Open file if not already opened
    static FILE *lfile = NULL;
    if(lfile == NULL) { lfile = fopen("Module5.log", "w"); }

    va_list arg;
    va_start(arg, message);
    vfprintf(lfile, message, arg);
    putc('\n', lfile);
    fflush(lfile);
    va_end(arg);
}

} // namespace cg

constexpr uint32_t NUMBER_OF_BALLS = 9;
constexpr uint32_t NUMBER_OF_RED_BALLS = 3;
constexpr uint32_t NUMBER_OF_GREEN_BALLS = 3;
constexpr uint32_t NUMBER_OF_BLUE_BALLS = 3;

// SDL Objects
SDL_Window       *g_sdl_window = nullptr;
SDL_GLContext     g_gl_context;
constexpr int32_t DRAWS_PER_SECOND = 72;
constexpr float   FRAMES_PER_SEC = 72.0f;
constexpr int32_t DRAW_INTERVAL_MILLIS =
    static_cast<int32_t>(1000.0 / static_cast<double>(DRAWS_PER_SECOND));

// Root of the scene graph
std::shared_ptr<cg::SceneNode> g_scene_root;

cg::SceneState g_scene_state;

// Bounding planes of the enclosure. Used for intersection testing.
std::vector<cg::Plane> g_bounding_planes;

// Ball transforms. We have this global so we can more easily do intersection testing
std::array<std::shared_ptr<cg::BallTransform>, NUMBER_OF_BALLS> g_ball_transforms;

// Sleep function to help run a reasonable timer
void sleep(int32_t milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

/**
 Display callback. Clears the prior scene and draws a new one.
 */
void display()
{
    // Clear the framebuffer and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Init scene state and draw the scene graph
    g_scene_state.init();
    g_scene_root->draw(g_scene_state);

    // Swap buffers
    SDL_GL_SwapWindow(g_sdl_window);
}

/**
 * Keyboard event handler.
 */
bool handle_key_event(const SDL_Event &event)
{
    bool cont_program = true;

    switch(event.key.key)
    {
        case SDLK_ESCAPE: cont_program = false; break;
        default: break;
    }

    return cont_program;
}

/**
 * Reshape function. Load a 2-D orthographic projection matrix using the
 * window width and height so we can directly take window coordinates and
 * send to OpenGL
 * @param  width  Window width
 * @param  height Window height
 */
void reshape(int32_t width, int32_t height)
{
    // Center the world window in the viewport so it does not
    // stretch world window
    float window_aspect = static_cast<float>(width) / static_cast<float>(height);
    if(window_aspect > 1.0f)
    {
        // Window is wider than world
        int32_t w = static_cast<int32_t>(height);
        int32_t start_x = static_cast<int32_t>((width - w) * 0.5f);
        glViewport(start_x, 0, w, height);
    }
    else
    {
        // Window is taller than world
        int32_t h = static_cast<int32_t>(width);
        int32_t start_y = static_cast<int32_t>((height - h) * 0.5f);
        glViewport(0, start_y, width, h);
    }
}

/**
 * Window event handler.
 */
bool handle_window_event(const SDL_Event &event)
{
    bool cont_program = true;

    switch(event.type)
    {
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            reshape(event.window.data1, event.window.data2);
            break;
        default: break;
    }

    return cont_program;
}

/**
 * Handle Events function.
 */
bool handle_events()
{
    SDL_Event e;
    bool      cont_program = true;
    while(SDL_PollEvent(&e))
    {
        switch(e.type)
        {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED: cont_program = false; break;

            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: cont_program = handle_window_event(e); break;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: cont_program = handle_key_event(e); break;
            default: break;
        }
    }
    return cont_program;
}

/**
 * Make sure this runs using a reasonable timing calculation.
 * Without using a timer, the speed of movement will depend on how fast
 * the program runs (fast movement on a fast PC and slow movement on a
 * slower PC)
 */
void update_scene()
{
    // Initialize all balls to have no intersection
    for(uint32_t i = 0; i < NUMBER_OF_BALLS; i++)
    {
        g_ball_transforms[i]->set_intersect_time(0.0f);
    }

    // Go through all balls and test for intersection with subsequent ball
    for(uint32_t i = 0; i < NUMBER_OF_BALLS; i++)
    {
        // If intersection with a prior ball is not found, test for intersection with successive
        // balls
        if(g_ball_transforms[i]->get_intersect_time() == 0.0f)
        {
            for(uint32_t j = i + 1; j < NUMBER_OF_BALLS; j++)
            {
                // If an intersection occurs, break out of loop. We will only worry about a ball
                // intersecting one other in a single frame and won't care much if it is the closest
                if(g_ball_transforms[i]->intersect_ball(g_ball_transforms[j])) { break; }
            }
        }
    }

    // Go through all ball and test for plane intersection on those that do not intersect with
    // another ball
    for(uint32_t i = 0; i < NUMBER_OF_BALLS; i++)
    {
        // Check for collision with any planes
        if(g_ball_transforms[i]->get_intersect_time() == 0.0f)
        {
            float     t = 0.0f;
            float     min_t = 1.0f;
            cg::Plane intersecting_plane;
            for(const auto &plane : g_bounding_planes)
            {
                t = g_ball_transforms[i]->intersect_with_plane(plane);
                if(t < min_t)
                {
                    // Copy the nearest intersection and the plane of intersection
                    min_t = t;
                    intersecting_plane = plane;
                }
            }
            if(min_t != 1.0f)
            {
                g_ball_transforms[i]->set_intersect_time(min_t);
                g_ball_transforms[i]->set_intersecting_plane(intersecting_plane);
            }
        }
    }

    // Update the scene graph
    cg::SceneState scene_state;
    g_scene_root->update(scene_state);
}

/**
 * Construct a unit box with outward facing normals.
 * @param  unit_square  Geometry node to use
 */
std::shared_ptr<cg::SceneNode> construct_unit_box(std::shared_ptr<cg::UnitSquare> unit_square)
{
    // Contruct transform nodes for the sides of the box.
    // Perform rotations so the sides face outwards

    // Bottom is rotated 180 degrees so it faces outwards
    auto bottom_transform = std::make_shared<cg::TransformNode>();
    bottom_transform->translate(0.0f, 0.0f, -0.5f);
    bottom_transform->rotate_x(180.0f);

    // Back is rotated -90 degrees about x: (z -> y)
    auto back_transform = std::make_shared<cg::TransformNode>();
    back_transform->translate(0.0f, 0.5f, 0.0f);
    back_transform->rotate_x(-90.0f);

    // Front wall is rotated 90 degrees about x: (y -> z)
    auto front_transform = std::make_shared<cg::TransformNode>();
    front_transform->translate(0.0f, -0.5f, 0.0f);
    front_transform->rotate_x(90.0f);

    // Left wall is rotated -90 about y: (z -> -x)
    auto left_transform = std::make_shared<cg::TransformNode>();
    left_transform->translate(-0.5f, 0.0f, 00.0f);
    left_transform->rotate_y(-90.0f);

    // Right wall is rotated 90 degrees about y: (z -> x)
    auto right_transform = std::make_shared<cg::TransformNode>();
    right_transform->translate(0.5f, 0.0f, 0.0f);
    right_transform->rotate_y(90.0f);

    // Top
    auto top_transform = std::make_shared<cg::TransformNode>();
    top_transform->translate(0.0f, 0.0f, 0.50f);

    // Create a SceneNode and add the 6 sides of the box.
    auto box = std::make_shared<cg::SceneNode>();
    box->add_child(back_transform);
    back_transform->add_child(unit_square);
    box->add_child(left_transform);
    left_transform->add_child(unit_square);
    box->add_child(right_transform);
    right_transform->add_child(unit_square);
    box->add_child(front_transform);
    front_transform->add_child(unit_square);
    box->add_child(bottom_transform);
    bottom_transform->add_child(unit_square);
    box->add_child(top_transform);
    top_transform->add_child(unit_square);

    return box;
}

/**
 * Construct the scene
 */
void construct_scene()
{
    // Shader node
    auto shader = std::make_shared<cg::LightingShaderNode>();
    if(!shader->create("Module5/simple_light.vert", "Module5/simple_light.frag") ||
       !shader->get_locations())
    // if(!shader->create_from_source(simple_light_vert, simple_light_frag) ||
    // !shader->get_locations())
    {
        exit(-1);
    }

    // Form bounding planes (for intersection testing)
    cg::Plane p0(cg::Point3(0.0f, 0.0f, 0.0f), cg::Vector3(0.0f, 0.0f, 1.0f));
    g_bounding_planes.push_back(p0);
    cg::Plane p1(cg::Point3(0.0f, 0.0f, 100.0f), cg::Vector3(0.0f, 0.0f, -1.0f));
    g_bounding_planes.push_back(p1);
    cg::Plane p2(cg::Point3(-50.0f, 0.0f, 50.0f), cg::Vector3(1.0f, 0.0f, 0.0f));
    g_bounding_planes.push_back(p2);
    cg::Plane p3(cg::Point3(50.0f, 0.0f, 50.0f), cg::Vector3(-1.0f, 0.0f, 0.0f));
    g_bounding_planes.push_back(p3);
    cg::Plane p4(cg::Point3(0.0f, 50.0f, 50.0f), cg::Vector3(0.0f, -1.0f, 0.0f));
    g_bounding_planes.push_back(p4);
    cg::Plane p5(cg::Point3(0.0f, -50.0f, 50.0f), cg::Vector3(0.0f, 1.0f, 0.0f));
    g_bounding_planes.push_back(p5);

    auto unit_square =
        std::make_shared<cg::UnitSquare>(shader->get_position_loc(), shader->get_normal_loc());

    auto unit_sphere = std::make_shared<cg::UnitSphere>(
        8, 36, shader->get_position_loc(), shader->get_normal_loc());

    // Contruct transform nodes for the walls. Perform rotations so the
    // walls face inwards
    auto floor_transform = std::make_shared<cg::TransformNode>();
    floor_transform->scale(100.0f, 100.0f, 100.0f);

    // Back wall is rotated +90 degrees about x: (y -> z)
    auto back_wall_transform = std::make_shared<cg::TransformNode>();
    back_wall_transform->translate(0.0f, 50.0f, 50.0f);
    back_wall_transform->rotate_x(90.0f);
    back_wall_transform->scale(100.0f, 100.0f, 1.0f);

    // Left wall is rotated 90 degrees about y: (z -> x)
    auto left_wall_transform = std::make_shared<cg::TransformNode>();
    left_wall_transform->translate(-50.0f, 0.0f, 50.0f);
    left_wall_transform->rotate_y(90.0f);
    left_wall_transform->scale(100.0f, 100.0f, 1.0f);

    // Right wall is rotated -90 about y: (z -> -x)
    auto right_wall_transform = std::make_shared<cg::TransformNode>();
    right_wall_transform->translate(50.0f, 0.0f, 50.0f);
    right_wall_transform->rotate_y(-90.0f);
    right_wall_transform->scale(100.0f, 100.0f, 1.0f);

    // Rotate ceiling by 180 so it faces downward
    auto ceiling_transform = std::make_shared<cg::TransformNode>();
    ceiling_transform->translate(0.0f, 0.0f, 100.0f);
    ceiling_transform->rotate_x(180.0f);
    ceiling_transform->scale(100.0f, 100.0f, 1.0f);

    // Construct color nodes for the walls, floor, and ceiling
    auto floor_color = std::make_shared<cg::ColorNode>(cg::Color4(0.6f, 0.5f, 0.2f));
    auto back_wall_color = std::make_shared<cg::ColorNode>(cg::Color4(0.9f, 0.7f, 0.5f));
    auto wall_color = std::make_shared<cg::ColorNode>(cg::Color4(1.0f, 1.0f, 1.0f));
    auto ceiling_color = std::make_shared<cg::ColorNode>(cg::Color4(0.1f, 0.4f, 1.0f));

    // Construct a unit box with outward normals as a scene graph node
    // using relative transforms
    // auto unit_box = construct_unit_box(unit_square);

    // Set a purple color for the box
    // auto box_color = std::make_shared<cg::ColorNode>(cg::Color4(0.5f, 0.0f, 0.5f));

    // Construct a transform node to position, size, and orient the box
    auto box_transform = std::make_shared<cg::TransformNode>();
    box_transform->translate(25.0f, 25.0f, 10.1f);
    box_transform->rotate_z(45.0f);
    box_transform->scale(40.0f, 20.0f, 20.0f);

    // Construct transform nodes for the balls. Each one has a
    // random initial position with x,y values between -40 and 40
    // and z values between 25 and 75. A random radius between 3 and
    // 7 units, a random initial direction vectors (unit length) and
    // random velocity between 5 and 15 units per second is used.
    for(uint32_t i = 0; i < NUMBER_OF_BALLS; i++)
    {
        g_ball_transforms[i] = std::make_shared<cg::BallTransform>(FRAMES_PER_SEC);
    }

    // Construct color nodes for the balls
    auto red_ball = std::make_shared<cg::ColorNode>(cg::Color4(1.0f, 0.0f, 0.0f));
    auto green_ball = std::make_shared<cg::ColorNode>(cg::Color4(0.0f, 1.0f, 0.0f));
    auto blue_ball = std::make_shared<cg::ColorNode>(cg::Color4(0.0f, 0.0f, 1.0f));

    // Construct the scene layout
    g_scene_root = std::make_shared<cg::SceneNode>();
    g_scene_root->add_child(shader);

    // Walls
    shader->add_child(back_wall_color);
    back_wall_color->add_child(back_wall_transform);
    back_wall_transform->add_child(unit_square);
    shader->add_child(wall_color);
    wall_color->add_child(left_wall_transform);
    left_wall_transform->add_child(unit_square);
    wall_color->add_child(right_wall_transform);
    right_wall_transform->add_child(unit_square);
    shader->add_child(floor_color);
    floor_color->add_child(floor_transform);
    floor_transform->add_child(unit_square);
    shader->add_child(ceiling_color);
    ceiling_color->add_child(ceiling_transform);
    ceiling_transform->add_child(unit_square);

    // Comment out the box for Module 5
    // shader->add_child(box_color);
    // box_color->add_child(box_transform);
    // box_transform->add_child(unit_box);

    // Add a unit sphere child node to each ball transform node
    for(uint32_t i = 0; i < NUMBER_OF_BALLS; i++) { g_ball_transforms[i]->add_child(unit_sphere); }

    // Add ball transforms under each ball presentation node
    uint32_t j = 0;
    shader->add_child(red_ball);
    for(uint32_t i = 0; i < NUMBER_OF_RED_BALLS; i++)
    {
        red_ball->add_child(g_ball_transforms[j++]);
    }
    shader->add_child(green_ball);
    for(uint32_t i = 0; i < NUMBER_OF_GREEN_BALLS; i++)
    {
        green_ball->add_child(g_ball_transforms[j++]);
    }
    shader->add_child(blue_ball);
    for(uint32_t i = 0; i < NUMBER_OF_BLUE_BALLS; i++)
    {
        blue_ball->add_child(g_ball_transforms[j++]);
    }
}

/**
 * Main
 */
int main(int argc, char **argv)
{
    cg::set_root_paths(argv[0]);

    // Initialize SDL
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cout << "Error initializing SDL: " << SDL_GetError() << '\n';
        exit(1);
    }

    // Initialize display mode and window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_PropertiesID props = SDL_CreateProperties();
    if(props == 0)
    {
        std::cout << "Error creating SDL Window Properties: " << SDL_GetError() << '\n';
        exit(1);
    }

    SDL_SetStringProperty(
        props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Simple Animation by Brian Russin");
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 800);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 800);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, 200);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 200);

    g_sdl_window = SDL_CreateWindowWithProperties(props);
    if(g_sdl_window == nullptr)
    {
        std::cout << "Error initializing SDL Window" << SDL_GetError() << '\n';
        exit(1);
    }

    // Initialize OpenGL
    g_gl_context = SDL_GL_CreateContext(g_sdl_window);

    std::cout << "OpenGL  " << glGetString(GL_VERSION) << ", GLSL "
              << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

#if BUILD_WINDOWS
    int32_t glew_init_result = glewInit();
    if(GLEW_OK != glew_init_result)
    {
        std::cout << "GLEW Error: " << glewGetErrorString(glew_init_result) << '\n';
        exit(EXIT_FAILURE);
    }
#endif

    // Set the clear color to black. Any part of the window outside the
    // viewport should appear black
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Construct scene.
    construct_scene();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Enable back face polygon removal
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // Set a fixed perspective projection. fov = 70, aspect = 1.0, near = 1.0 far = 200.
    // We are hard-coding viewing and projection matrices since they do not change in
    // this application.
    cg::Matrix4x4 projection;
    projection.m00() = 1.428f;
    projection.m11() = 1.428f;
    projection.m22() = -1.010f;
    projection.m23() = -2.010f;
    projection.m32() = -1.0f;
    projection.m33() = 0.0f;

    // Set a fixed camera outside the center of the front wall (imagine
    // it being a window) looking parallel to the floor
    cg::Matrix4x4 view;
    view.m00() = 1.0f;
    view.m11() = 0.0f;
    view.m12() = 1.0f;
    view.m13() = -50.0f;
    view.m21() = -1.0f;
    view.m22() = 0.0f;
    view.m23() = -90.0f;
    view.m33() = 1.0f;

    // Set the composite projection and viewing matrix
    // These remain fixed.
    g_scene_state.pv = projection * view;

    glViewport(0, 0, 800, 800);

    // Main loop
    while(handle_events())
    {
        update_scene();
        display();
        sleep(DRAW_INTERVAL_MILLIS);
    }

    // Destroy OpenGL Context, SDL Window and SDL
    SDL_GL_DestroyContext(g_gl_context);
    SDL_DestroyWindow(g_sdl_window);
    SDL_Quit();

    return 0;
}

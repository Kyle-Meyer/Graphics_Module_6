//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.667 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	Brian Russin
//
//	Author:  David W. Nesbitt
//	File:    Module6/main.cpp
//	Purpose: OpenGL and SDL program to draw a simple 3-D scene. It starts
//           with some simple object modeling and representation, adds camera
//           and projection controls, adds lighting and shading, then adds
//           texture mapping.
//
//============================================================================

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"
#include "filesystem_support/file_locator.hpp"
#include "geometry/geometry.hpp"
#include "scene/color_node.hpp"
#include "scene/graphics.hpp"
#include "scene/scene.hpp"

#include "Module6/lighting_shader_node.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "scene/conic.hpp"
#include "scene/surface_of_revolution.hpp"
#include "scene/mesh_teapot.hpp"
#include "scene/transform_node.hpp"
#include "scene/unit_square_node.hpp"

namespace cg
{

// Simple logging function, should be defined in the cg namespace
void logmsg(const char *message, ...)
{
    // Open file if not already opened
    static FILE *lfile = NULL;
    if(lfile == NULL) { lfile = fopen("Module6.log", "w"); }

    va_list arg;
    va_start(arg, message);
    vfprintf(lfile, message, arg);
    putc('\n', lfile);
    fflush(lfile);
    va_end(arg);
}

} // namespace cg

// SDL Objects
SDL_Window       *g_sdl_window = nullptr;
SDL_GLContext     g_gl_context;
constexpr int32_t DRAWS_PER_SECOND = 30;
constexpr int32_t DRAW_INTERVAL_MILLIS =
    static_cast<int32_t>(1000.0 / static_cast<double>(DRAWS_PER_SECOND));

// Root of the scene graph and scene state
std::shared_ptr<cg::SceneNode> g_scene_root;

cg::SceneState g_scene_state;

// Sleep function to help run a reasonable timer
void sleep(int32_t milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

/**
 * Display callback. Clears the prior scene and draws a new one.
 */
void display()
{
    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Initialize scene state
    g_scene_state.init();
    
    // Draw the scene (if root exists)
    if (g_scene_root)
    {
        g_scene_root->draw(g_scene_state);
    }
    cg::check_error("After draw");
    // Swap buffers to display
    SDL_GL_SwapWindow(g_sdl_window);
}

/**
 * Reshape callback. Update projection to reflect new aspect ratio.
 * @param  width  Window width
 * @param  height Window height
 */
void reshape(int32_t width, int32_t height)
{
    // TODO - Module 7 - update perspective projection
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
 * Keyboard event handler.
 */
bool handle_key_event(const SDL_Event &event)
{
    bool cont_program = true;

   if(event.type == SDL_EVENT_KEY_DOWN)
   {
      switch (event.key.key)
      {
         case SDLK_ESCAPE:
            cont_program = false; 
            break;
         default:
            break;
      }
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
 * Construct the scene
 */
void construct_scene()
{
   // Shader node
   auto shader = std::make_shared<cg::LightingShaderNode>();
   if(!shader->create("Module6/simple_light.vert", "Module6/simple_light.frag") ||
      !shader->get_locations())
   {
      std::cout << "shaders failed \n";
      exit(-1);
   }
   std::cout << "Shaders created successfully\n";

   // Get the position and normal locations to use when constructing VAOs
   int32_t position_loc = shader->get_position_loc();
   int32_t normal_loc = shader->get_normal_loc();
    
   std::cout << "Position loc: " << position_loc << ", Normal loc: " << normal_loc << " \n";

   //lets just make the scene root the shader node
   g_scene_root = shader;


   // ========================================================================
   // ROOM: Floor, Walls, Ceiling
   // ========================================================================
   
   // --- FLOOR ---
   auto floor_color = std::make_shared<cg::ColorNode>(cg::Color4(0.3f, 0.45f, 0.1f, 1.0f));
   shader->add_child(floor_color);
   
   auto floor_transform = std::make_shared<cg::TransformNode>();
   floor_transform->translate(0.0f, 0.0f, 0.0f);   // At origin (floor level)
   floor_transform->scale(200.0f, 200.0f, 1.0f);  // Scale to 200x200
   floor_color->add_child(floor_transform);
   
   auto floor_square = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   floor_transform->add_child(floor_square);

   // --- WALLS ---
   auto wall_color = std::make_shared<cg::ColorNode>(cg::Color4(0.7f, 0.55f, 0.55f, 1.0f));
   shader->add_child(wall_color);

   // Back wall (y = 100)
   auto back_wall_transform = std::make_shared<cg::TransformNode>();
   back_wall_transform->translate(0.0f, 100.0f, 40.0f);
   back_wall_transform->rotate_x(90.0f);
   back_wall_transform->scale(200.0f, 80.0f, 1.0f);
   wall_color->add_child(back_wall_transform);
   auto back_wall = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   back_wall_transform->add_child(back_wall);
   
   // Front wall (y = -100)
   auto front_wall_transform = std::make_shared<cg::TransformNode>();

   front_wall_transform->translate(0.0f, -100.0f, 40.0f);
   front_wall_transform->rotate_x(-90.0f);
   front_wall_transform->scale(200.0f, 80.0f, 1.0f);
   wall_color->add_child(front_wall_transform);
   auto front_wall = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   front_wall_transform->add_child(front_wall);

   // Left wall (x = -100)
   auto left_wall_transform = std::make_shared<cg::TransformNode>();
   left_wall_transform->translate(-100.0f, 0.0f, 40.0f);
   left_wall_transform->rotate_x(90.0f);
   left_wall_transform->rotate_y(90.0f);
   left_wall_transform->scale(200.0f, 80.0f, 1.0f);
   wall_color->add_child(left_wall_transform);
   auto left_wall = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   left_wall_transform->add_child(left_wall);

   // Right wall (x = 100)
   auto right_wall_transform = std::make_shared<cg::TransformNode>();
   right_wall_transform->translate(100.0f, 0.0f, 40.0f);
   right_wall_transform->rotate_x(90.0f);
   right_wall_transform->rotate_y(-90.0f);
   right_wall_transform->scale(200.0f, 80.0f, 1.0f);
   wall_color->add_child(right_wall_transform);
   auto right_wall = std::make_shared<cg::UnitSquareSurface>(10 ,position_loc, normal_loc);
   right_wall_transform->add_child(right_wall);

   // --- CEILING ---
   auto ceiling_color = std::make_shared<cg::ColorNode>(cg::Color4(1.0f, 1.0f, 1.0f, 1.0f));
   shader->add_child(ceiling_color);
   
   auto ceiling_transform = std::make_shared<cg::TransformNode>();
   ceiling_transform->translate(0.0f, 0.0f, 80.0f);
   ceiling_transform->rotate_x(180.0f);
   ceiling_transform->scale(200.0f, 200.0f, 1.0f);
   ceiling_color->add_child(ceiling_transform);
   
   auto ceiling_square = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   ceiling_transform->add_child(ceiling_square);

   // ========================================================================
   // TABLE with legs (hierarchical)
   // ========================================================================
   
   auto table_color = std::make_shared<cg::ColorNode>(cg::Color4(0.55f, 0.45f, 0.15f, 1.0f));
   shader->add_child(table_color);

   // Parent transform for entire table (position and rotation)
   auto table_parent_transform = std::make_shared<cg::TransformNode>();
   table_parent_transform->translate(-50.0f, 50.0f, 0.0f);
   table_parent_transform->rotate_z(30.0f);
   table_color->add_child(table_parent_transform);

   // Table top (60x30x6, bottom at z=20, so center at z=23)
   /*
   auto table_top_transform = std::make_shared<cg::TransformNode>();
   table_top_transform->translate(0.0f, 0.0f, 18.0f);
   table_top_transform->scale(60.0f, 30.0f, 1.0f);
   table_parent_transform->add_child(table_top_transform);
   auto table_top = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   table_top_transform->add_child(table_top);
*/

   // Top face (z=26)
   auto table_top_top_transform = std::make_shared<cg::TransformNode>();
   table_top_top_transform->translate(0.0f, 0.0f, 26.0f);
   table_top_top_transform->scale(60.0f, 30.0f, 1.0f);
   table_parent_transform->add_child(table_top_top_transform);
   auto table_top_top = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   table_top_top_transform->add_child(table_top_top);
   
   // Bottom face (z=20)
   auto table_top_bottom_transform = std::make_shared<cg::TransformNode>();
   table_top_bottom_transform->translate(0.0f, 0.0f, 20.0f);
   table_top_bottom_transform->rotate_x(180.0f);  // Flip to face down
   table_top_bottom_transform->scale(60.0f, 30.0f, 1.0f);
   table_parent_transform->add_child(table_top_bottom_transform);
   auto table_top_bottom = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   table_top_bottom_transform->add_child(table_top_bottom);
   
   // Front face (y=-15, facing -y)
   auto table_top_front_transform = std::make_shared<cg::TransformNode>();
   table_top_front_transform->translate(0.0f, -15.0f, 23.0f);
   table_top_front_transform->rotate_x(90.0f);
   table_top_front_transform->scale(60.0f, 6.0f, 1.0f);
   table_parent_transform->add_child(table_top_front_transform);
   auto table_top_front = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   table_top_front_transform->add_child(table_top_front);
   
   // Back face (y=15, facing +y)
   auto table_top_back_transform = std::make_shared<cg::TransformNode>();
   table_top_back_transform->translate(0.0f, 15.0f, 23.0f);
   table_top_back_transform->rotate_x(-90.0f);
   table_top_back_transform->scale(60.0f, 6.0f, 1.0f);
   table_parent_transform->add_child(table_top_back_transform);
   auto table_top_back = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   table_top_back_transform->add_child(table_top_back);
   
   // Left face (x=-30, facing -x)
   auto table_top_left_transform = std::make_shared<cg::TransformNode>();
   table_top_left_transform->translate(-30.0f, 0.0f, 23.0f);
   table_top_left_transform->rotate_x(90.0f);
   table_top_left_transform->rotate_y(-90.0f);
   table_top_left_transform->scale(30.0f, 6.0f, 1.0f);
   table_parent_transform->add_child(table_top_left_transform);
   auto table_top_left = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   table_top_left_transform->add_child(table_top_left);
   
   // Right face (x=30, facing +x)
   auto table_top_right_transform = std::make_shared<cg::TransformNode>();
   table_top_right_transform->translate(30.0f, 0.0f, 23.0f);
   table_top_right_transform->rotate_x(-90.0f);
   table_top_right_transform->rotate_y(-90.0f);
   table_top_right_transform->scale(30.0f, 6.0f, 1.0f);
   table_parent_transform->add_child(table_top_right_transform);
   auto table_top_right = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   table_top_right_transform->add_child(table_top_right);
   // Table legs (4 legs, radius 3, height 20)
   // Leg positions relative to table center (±27 in x, ±12 in y for 60x30 table)
   float leg_x_offset = 23.0f;
   float leg_y_offset = 12.0f;
   
   // Leg 1: front-left
   auto leg1_transform = std::make_shared<cg::TransformNode>();
   leg1_transform->translate(-leg_x_offset, -leg_y_offset, 10.0f);
   leg1_transform->scale(1.0f, 1.0f, 20.0f);
   table_parent_transform->add_child(leg1_transform);
   auto leg1 = std::make_shared<cg::ConicSurface>(3.0f, 3.0f, 16, 1, position_loc, normal_loc);
   leg1_transform->add_child(leg1);

   // Leg 2: front-right
   auto leg2_transform = std::make_shared<cg::TransformNode>();
   leg2_transform->translate(leg_x_offset, -leg_y_offset, 10.0f);
   leg2_transform->scale(1.0f, 1.0f, 20.0f);
   table_parent_transform->add_child(leg2_transform);
   auto leg2 = std::make_shared<cg::ConicSurface>(3.0f, 3.0f, 16, 1, position_loc, normal_loc);
   leg2_transform->add_child(leg2);

   // Leg 3: back-left
   auto leg3_transform = std::make_shared<cg::TransformNode>();
   leg3_transform->translate(-leg_x_offset, leg_y_offset, 10.0f);
   leg3_transform->scale(1.0f, 1.0f, 20.0f);
   table_parent_transform->add_child(leg3_transform);
   auto leg3 = std::make_shared<cg::ConicSurface>(3.0f, 3.0f, 16, 1, position_loc, normal_loc);
   leg3_transform->add_child(leg3);

   // Leg 4: back-right
   auto leg4_transform = std::make_shared<cg::TransformNode>();
   leg4_transform->translate(leg_x_offset, leg_y_offset, 10.0f);
   leg4_transform->scale(1.0f, 1.0f, 20.0f);
   table_parent_transform->add_child(leg4_transform);
   auto leg4 = std::make_shared<cg::ConicSurface>(3.0f, 3.0f, 16, 1, position_loc, normal_loc);
   leg4_transform->add_child(leg4);

   // ========================================================================
   // TEAPOT on table (child of table hierarchy)
   // ========================================================================
   
   auto teapot_color = std::make_shared<cg::ColorNode>(cg::Color4(0.50754f, 0.50754f, 0.50754f, 1.0f));
   table_parent_transform->add_child(teapot_color);
   
   auto teapot_transform = std::make_shared<cg::TransformNode>();
   teapot_transform->translate(0.0f, 0.0f, 26.0f);
   teapot_transform->scale(2.0f, 2.0f, 2.0f);
   teapot_color->add_child(teapot_transform);
   
   auto teapot = std::make_shared<cg::MeshTeapot>(3, position_loc, normal_loc);
   teapot_transform->add_child(teapot);

   // ========================================================================
   // BOX with CONE on top (hierarchical)
   // ========================================================================
   
   // Parent transform for box and cone
   auto box_parent_transform = std::make_shared<cg::TransformNode>();
   box_parent_transform->translate(80.0f, 80.0f, 0.0f);
   box_parent_transform->rotate_z(45.0f);
   shader->add_child(box_parent_transform);

   // Box (20x20x15)
   auto box_color = std::make_shared<cg::ColorNode>(cg::Color4(0.5f, 0.25f, 0.25f, 1.0f));
   box_parent_transform->add_child(box_color);

   // Box bottom
   auto box_bottom_transform = std::make_shared<cg::TransformNode>();
   box_bottom_transform->translate(0.0f, 0.0f, 0.0f);
   box_bottom_transform->scale(20.0f, 20.0f, 1.0f);
   box_color->add_child(box_bottom_transform);
   auto box_bottom = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   box_bottom_transform->add_child(box_bottom);

   // Box top
   auto box_top_transform = std::make_shared<cg::TransformNode>();
   box_top_transform->translate(0.0f, 0.0f, 15.0f);
   box_top_transform->scale(20.0f, 20.0f, 1.0f);
   box_color->add_child(box_top_transform);
   auto box_top = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   box_top_transform->add_child(box_top);

   // Box front side
   auto box_front_transform = std::make_shared<cg::TransformNode>();
   box_front_transform->translate(0.0f, -10.0f, 7.5f);
   box_front_transform->rotate_x(90.0f);
   box_front_transform->scale(20.0f, 15.0f, 1.0f);
   box_color->add_child(box_front_transform);
   auto box_front = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   box_front_transform->add_child(box_front);

   // Box back side
   auto box_back_transform = std::make_shared<cg::TransformNode>();
   box_back_transform->translate(0.0f, 10.0f, 7.5f);
   box_back_transform->rotate_x(-90.0f);
   box_back_transform->scale(20.0f, 15.0f, 1.0f);
   box_color->add_child(box_back_transform);
   auto box_back = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   box_back_transform->add_child(box_back);

   // Box left side
   auto box_left_transform = std::make_shared<cg::TransformNode>();
   box_left_transform->translate(-10.0f, 0.0f, 7.5f);
   box_left_transform->rotate_x(-90.0f);
   box_left_transform->rotate_y(-90.0f);
   box_left_transform->scale(20.0f, 15.0f, 1.0f);
   box_color->add_child(box_left_transform);
   auto box_left = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   box_left_transform->add_child(box_left);

   // Box right side
   auto box_right_transform = std::make_shared<cg::TransformNode>();
   box_right_transform->translate(10.0f, 0.0f, 7.5f);
   box_right_transform->rotate_x(-90.0f);
   box_right_transform->rotate_y(90.0f);
   box_right_transform->scale(20.0f, 15.0f, 1.0f);
   box_color->add_child(box_right_transform);
   auto box_right = std::make_shared<cg::UnitSquareSurface>(10, position_loc, normal_loc);
   box_right_transform->add_child(box_right);

   // Cone on top of box (radius 4, height 15)
   auto cone_color = std::make_shared<cg::ColorNode>(cg::Color4(0.75164f, 0.60648f, 0.22648f, 1.0f));
   box_parent_transform->add_child(cone_color);
   
   auto cone_transform = std::make_shared<cg::TransformNode>();
   cone_transform->translate(0.0f, 0.0f, 22.5f);
   cone_transform->scale(1.0f, 1.0f, 15.0f);
   cone_color->add_child(cone_transform);
   
   auto cone = std::make_shared<cg::ConicSurface>(4.0f, 0.0f, 32, 1, position_loc, normal_loc);
   cone_transform->add_child(cone);

   // ========================================================================
   // VASE (Surface of Revolution)
   // ========================================================================
   
   auto vase_color = std::make_shared<cg::ColorNode>(cg::Color4(0.95f, 0.35f, 0.65f, 1.0f));
   shader->add_child(vase_color);
   // Profile curve for the vase. Unit width and height, centered at the
    // center of the vase
   std::vector<cg::Point3> vase_profile = {{0.0f, 0.0f, -0.5f},
                                 {0.4f, 0.0f, -0.5f},
                                 {0.6f, 0.0f, -0.45f},
                                 {0.72f, 0.0f, -0.37f},
                                 {0.81f, 0.0f, -0.26f},
                                 {0.82f, 0.0f, -0.18f},
                                 {0.79f, 0.0f, -0.08f},
                                 {0.7f, 0.0f, 0.02f},
                                 {0.55f, 0.0f, 0.13f},
                                 {0.48f, 0.0f, 0.25f},
                                 {0.51f, 0.0f, 0.35f},
                                 {0.53f, 0.0f, 0.41f},
                                 {0.62f, 0.0f, 0.45f},
                                 {0.62f, 0.0f, 0.5f},
                                 {0.65f, 0.0f, 0.5f},
                                 {0.0f, 0.0f, 0.5f}};

   auto vase_transform = std::make_shared<cg::TransformNode>();
   vase_transform->translate(0.0f, 75.0f, 10.0f);
   vase_transform->scale(10.0f, 10.0f, 20.0f);
   vase_color->add_child(vase_transform);
   
   auto vase = std::make_shared<cg::SurfaceOfRevolution>(vase_profile, 32, position_loc, normal_loc);
   vase_transform->add_child(vase);
   std::cout << "Scene construction complete\n";
   g_scene_root->print_graph();
}

void create_window()
{
   // Set OpenGL attributes before creating the window
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
   SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
   SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
   SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
   
   // Create the window
   g_sdl_window = SDL_CreateWindow(
       "Module 6 - 3D Scene",
       800,  // width
       600,  // height
       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
   );
   
   if (!g_sdl_window)
   {
       std::cout << "Error creating window: " << SDL_GetError() << '\n';
       SDL_Quit();
       exit(1);
   }
   
   // Position the window
   SDL_SetWindowPosition(g_sdl_window, 100, 100);
}

void set_gl_args()
{
   // Enable depth testing for hidden surface removal
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   
   // Enable back-face culling
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glFrontFace(GL_CCW);  // Counter-clockwise winding for front faces
   
   // Enable multi-sampling anti-aliasing
   glEnable(GL_MULTISAMPLE);

   glViewport(0, 0, 800, 600);
}


/**
 * Main
 */
int main(int argc, char **argv)
{
    cg::set_root_paths(argv[0]);

    // Print the keyboard commands
    std::cout << "i - Reset to initial view\n";
    std::cout << "R - Roll    5 degrees clockwise   r - Counter-clockwise\n";
    std::cout << "P - Pitch   5 degrees clockwise   p - Counter-clockwise\n";
    std::cout << "H - Heading 5 degrees clockwise   h - Counter-clockwise\n";
    std::cout << "X - Slide camera right            x - Slide camera left\n";
    std::cout << "Y - Slide camera up               y - Slide camera down\n";
    std::cout << "F - Move camera forward           f - Move camera backwards\n";
    std::cout << "V - Faster mouse movement         v - Slower mouse movement\n";
    std::cout << "ESC - Exit Program\n";

    // Initialize SDL
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cout << "Error initializing SDL: " << SDL_GetError() << '\n';
        exit(1);
    }

    // Initialize display mode and window
    // Student to define - window creation.
    create_window();

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

    set_gl_args();

    // Set a fixed perspective projection and view.
    cg::Matrix4x4 projection;
    projection.m00() = 1.608f;
    projection.m11() = 2.145f;
    projection.m22() = -1.007f;
    projection.m23() = -2.007f;
    projection.m32() = -1.0f;
    projection.m33() = 0.0f;
    cg::Matrix4x4 view;
    view.m00() = 1.0f;
    view.m11() = 0.0f;
    view.m12() = 1.0f;
    view.m13() = -20.0f;
    view.m21() = -1.0f;
    view.m22() = 0.0f;
    view.m23() = -100.0f;
    view.m33() = 1.0f;

    // Set the composite projection and viewing matrix
    // These remain fixed in Module 6. Module 7 will develop
    // camera and projection controls.
    g_scene_state.pv = projection * view;

    // Main loop
    while(handle_events())
    {
        display();
        sleep(DRAW_INTERVAL_MILLIS);
    }

    // Destroy OpenGL Context, SDL Window and SDL
    SDL_GL_DestroyContext(g_gl_context);
    SDL_DestroyWindow(g_sdl_window);
    SDL_Quit();

    return 0;
}

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
#include "scene/graphics.hpp"
#include "scene/scene.hpp"

#include "Module6/lighting_shader_node.hpp"

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

    // Profile curve for the vase. Unit width and height, centered at the
    // center of the vase
    std::vector<cg::Point3> v = {{0.0f, 0.0f, -0.5f},
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

    // ===== TEST SCENE: Just a teapot =====
    
    std::cout << "Creating teapot...\n";
    // Set shader as root
    g_scene_root = shader;
    
    // Create a color node for the teapot (gray/silver color)
    auto teapot_color = std::make_shared<cg::ColorNode>(cg::Color4(0.50754f, 0.50754f, 0.50754f, 1.0f));
    shader->add_child(teapot_color);
    
    // Create a transform to scale and position the teapot
    auto teapot_transform = std::make_shared<cg::TransformNode>();
    teapot_transform->scale(3.0f, 3.0f, 3.0f);  // Scale up the teapot
    teapot_transform->translate(0.0f, 50.0f, 20.0f);  // Move it up and forward in view
    teapot_color->add_child(teapot_transform);
    
    // Create the teapot mesh (level 3 subdivision is reasonable)
    auto teapot = std::make_shared<cg::MeshTeapot>(3, position_loc, normal_loc);
    teapot_transform->add_child(teapot);
    std::cout << "Teapot created successfully\n";
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

#include "helpers.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION

// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#else
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#endif

// OpenGL Mathematics Library
#include <glm/glm.hpp> // glm::vec3
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <memory>

#ifdef DEBUG
#include <iostream>
#endif

#include "mesh.h"
#include "my_context.h"
#include "camera.h"

#include "mesh_data.h"

float WIDTH;
float HEIGHT;

std::unique_ptr<MyContext> ctx;

glm::vec2 get_cursor_pos(GLFWwindow* window) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Convert screen position to nds
    float x_nds = (2.0f * xpos) / width - 1.0f;
    float y_nds = 1.0f - (2.0f * ypos) / height;

    glm::vec2 ray_nds = glm::vec2(x_nds, y_nds);

    return ray_nds;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // take into account aspect ratio
    ctx->camera->set_aspect(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Convert screen position to nds
    float x_nds = (2.0f * xpos) / width - 1.0f;
    float y_nds = 1.0f - (2.0f * ypos) / height;


    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        ctx->mouse_ctx.hold();

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        glm::vec2 cursor_pos = get_cursor_pos(window);
        ctx->select(cursor_pos, width, height);
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        ctx->mouse_ctx.release();
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float window_size_factor = 2 * 0.1;

        Optional<MeshEntity> selected = ctx->get_selected();

        // Update the position of the first vertex if the keys 1,2, or 3 are pressed
        switch (key)
        {
            // spawn
        case GLFW_KEY_1:
            ctx->push_mesh_entity({ MeshList::CUBE });
            break;
        case GLFW_KEY_2:
            ctx->push_mesh_entity({ MeshList::BUMPY });
            break;
        case GLFW_KEY_3:
            ctx->push_mesh_entity({ MeshList::BUNNY });
            break;
            // mode
        case GLFW_KEY_M:
            ctx->switch_draw_mode();
            break;
            // program
        case GLFW_KEY_P:
            ctx->switch_program();
            break;
        // switch camera
        case GLFW_KEY_X:
            ctx->switch_camera();
            break;

            // projection
        case GLFW_KEY_C:
            ctx->camera->switch_projection();
            break;
        default:
            // model
            if (selected.has_value()) {
                switch (key) {
                    // despawn
                case GLFW_KEY_R:
                    ctx->mesh_list.erase(ctx->mesh_list.begin() + ctx->mouse_ctx.get_selected());
                    ctx->mouse_ctx.deselect();
                    break;
                    // center to origin
                case GLFW_KEY_O:
                    selected->get().set_to_origin();
                    break;
                    // translate
                case GLFW_KEY_D:
                    selected->get().translate(ctx->camera->get_view(), glm::vec3(window_size_factor, 0.0, 0.0));
                    break;
                case GLFW_KEY_A:
                    selected->get().translate(ctx->camera->get_view(), glm::vec3(-window_size_factor, 0.0, 0.0));
                    break;
                case GLFW_KEY_W:
                    selected->get().translate(ctx->camera->get_view(), glm::vec3(0.0, window_size_factor, 0.0));
                    break;
                case GLFW_KEY_S:
                    selected->get().translate(ctx->camera->get_view(), glm::vec3(0.0, -window_size_factor, 0.0));
                    break;
                    // z axis
                case GLFW_KEY_EQUAL:
                    selected->get().translate(ctx->camera->get_view(), glm::vec3(0.0, 0.0, -window_size_factor));
                    break;
                case GLFW_KEY_MINUS:
                    selected->get().translate(ctx->camera->get_view(), glm::vec3(0.0, 0.0, window_size_factor));
                    break;
                    // scale
                case GLFW_KEY_K:
                    selected->get().scale(ctx->camera->get_view(), MeshEntity::ScaleDir::In, window_size_factor);
                    break;
                case GLFW_KEY_L:
                    selected->get().scale(ctx->camera->get_view(), MeshEntity::ScaleDir::Out, window_size_factor);
                    break;
                    // rotate
                    // x
                case GLFW_KEY_T:
                    selected->get().rotate(ctx->camera->get_view(), -10.f, glm::vec3(1.f, 0.f, 0.f));
                    break;
                case GLFW_KEY_Y:
                    selected->get().rotate(ctx->camera->get_view(), 10.f, glm::vec3(1.f, 0.f, 0.f));
                    break;
                    // y
                case GLFW_KEY_F:
                    selected->get().rotate(ctx->camera->get_view(), -10.f, glm::vec3(0.f, 1.f, 0.f));
                    break;
                case GLFW_KEY_G:
                    selected->get().rotate(ctx->camera->get_view(), 10.f, glm::vec3(0.f, 1.f, 0.f));
                    break;
                    // z
                case GLFW_KEY_H:
                    selected->get().rotate(ctx->camera->get_view(), 10.f, glm::vec3(0.f, 0.f, 1.f));
                    break;
                case GLFW_KEY_J:
                    selected->get().rotate(ctx->camera->get_view(), -10.f, glm::vec3(0.f, 0.f, 1.f));
                    break;
                }
            }
            break;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ctx->mouse_ctx.set_scroll(yoffset);
    double scroll_diff = ctx->mouse_ctx.get_scroll() - ctx->mouse_ctx.get_prev_scroll();

#ifdef DEBUG
    std::cout << "scroll: " << ctx->mouse_ctx.get_scroll() << std::endl;
#endif
    ctx->camera->zoom_protected(ctx->mouse_ctx.get_scroll() > 0 ? Camera::ZoomDir::In : Camera::ZoomDir::Out, glm::abs(scroll_diff / 20.f));
}

int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    int xpos, ypos, width, height;
    glfwGetMonitorWorkarea(primary, &xpos, &ypos, &width, &height);
    WIDTH = static_cast<float>(width); HEIGHT = static_cast<float>(height);

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(WIDTH, HEIGHT, "3D Scene Editor", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

#ifndef __APPLE__
    glewExperimental = true;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // program

#ifdef DEBUG
    std::cout << "DEBUG ENABLED" << std::endl;
#endif

    std::unique_ptr<ShaderProgramCtx> programs = std::unique_ptr<ShaderProgramCtx>{ new ShaderProgramCtx };
    programs->bind(ShaderPrograms::PHONG);

    ctx = std::make_unique<MyContext>(
        MyContext(
            std::move(programs),
            width, 
            height
        )
    );
    ctx->init_mesh_prototypes(std::vector<Mesh>{ UnitCube{}, BumpyCubeMesh{}, BunnyMesh{} });

    // callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // setting the position happens in main instead of in a callback because the callback didnt update frequently enough which caused drift
        ctx->mouse_ctx.set_position(get_cursor_pos(window));

        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // // swivel animation
        // auto camera = dynamic_cast<TrackballCamera*>(&ctx->camera.get_camera());
        // if (camera != nullptr) {
        //     camera->swivel();
        // }

        ctx->draw();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    // TODO: move into ~Program
    for (ShaderProgram& program : *ctx->programs) {
        program.free();
    }

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}

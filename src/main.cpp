// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
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

// Timer
#include <chrono>

#include <memory>

#ifdef DEBUG
    #include <iostream>
#endif

#include "mesh.h"
#include "context.h"
#include "camera.h"

float width = 640.f;
float height = 480.f;

std::unique_ptr<GLContext> ctx;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Convert screen position to nds
    float x_nds = (2.0f * xpos) / width - 1.0f;
    float y_nds = 1.0f - (2.0f * ypos) / height;

    glm::vec2 ray_nds = glm::vec2(x_nds, y_nds);
    glm::vec4 ray_clip = glm::vec4(ray_nds, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(ctx->camera.get_projection()) * ray_clip;
    ray_eye = glm::vec4(glm::vec2(ray_eye), -1.0, 0.0);
    glm::vec3 ray_world = glm::vec3(inverse(ctx->camera.get_view()) * ray_eye);
    ray_world = glm::normalize(ray_world);

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        ctx->mouse_ctx.hold();
        ctx->select(glm::vec3(ray_world));
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        ctx->mouse_ctx.release();
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to nds
    float x_nds = (2.0f * xpos) / width - 1.0f;
    float y_nds = 1.0f - (2.0f * ypos) / height;

    glm::vec2 ray_nds = glm::vec2(x_nds, y_nds);

    // #ifdef DEBUG
    // std::cout << "Ray World: " << ray_world[0] << ' ' << ray_world[1] << ' ' << ray_world[2] << ' ' << std::endl;
    // #endif
    
    ctx->mouse_ctx.set_world_point(ray_nds);
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
            // model
            // translate
            case GLFW_KEY_D:
                if (selected.has_value()) {
                    selected->get().translate(ctx->camera.get_view(), glm::vec3(window_size_factor, 0.0, 0.0));
                }
                break;
            case GLFW_KEY_A:
                if (selected.has_value()) {
                    selected->get().translate(ctx->camera.get_view(), glm::vec3(-window_size_factor, 0.0, 0.0));
                }
                break;
            case GLFW_KEY_W:
                if (selected.has_value()) {
                    selected->get().translate(ctx->camera.get_view(), glm::vec3(0.0, window_size_factor, 0.0));
                }
                break;
            case GLFW_KEY_S:
                if (selected.has_value()) {
                    selected->get().translate(ctx->camera.get_view(), glm::vec3(0.0, -window_size_factor, 0.0));
                }
                break;
            // z axis
            case GLFW_KEY_EQUAL:
                if (selected.has_value()) {
                    selected->get().translate(ctx->camera.get_view(), glm::vec3(0.0, 0.0, -window_size_factor));
                }
                break;
            case GLFW_KEY_MINUS:
                if (selected.has_value()) {
                    selected->get().translate(ctx->camera.get_view(), glm::vec3(0.0, 0.0, window_size_factor));
                }
                break;
            // scale
            case GLFW_KEY_K:
                if (selected.has_value()) {
                    selected->get().scale(ctx->camera.get_view(), MeshEntity::ScaleDir::In, window_size_factor);
                }
                break;
            case GLFW_KEY_L:
                if (selected.has_value()) {
                    selected->get().scale(ctx->camera.get_view(), MeshEntity::ScaleDir::Out, window_size_factor);
                }
                break;
            // rotate
            // x
            case GLFW_KEY_T:
                if (selected.has_value()) {
                    selected->get().rotate(ctx->camera.get_view(), -10.f, glm::vec3(1.f, 0.f, 0.f));
                }
                break;
            case GLFW_KEY_Y:
                if (selected.has_value()) {
                    selected->get().rotate(ctx->camera.get_view(), 10.f, glm::vec3(1.f, 0.f, 0.f));
                }
                break;
            // y
            case GLFW_KEY_F:
                if (selected.has_value()) {
                    selected->get().rotate(ctx->camera.get_view(), -10.f, glm::vec3(0.f, 1.f, 0.f));
                }
                break;
            case GLFW_KEY_G:
                if (selected.has_value()) {
                    selected->get().rotate(ctx->camera.get_view(), 10.f, glm::vec3(0.f, 1.f, 0.f));
                }
                break;
            // z
            case GLFW_KEY_H:
                if (selected.has_value()) {
                    selected->get().rotate(ctx->camera.get_view(), 10.f, glm::vec3(0.f, 0.f, 1.f));
                }
                break;
            case GLFW_KEY_J:
                if (selected.has_value()) {
                    selected->get().rotate(ctx->camera.get_view(), -10.f, glm::vec3(0.f, 0.f, 1.f));
                }
                break;
            // center to origin
            case GLFW_KEY_O:
                if (selected.has_value()) {
                    selected->get().set_to_origin();
                }
                break;
            
            // projection
            case GLFW_KEY_C:
                ctx->camera.switch_projection();
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
    ctx->camera.zoom(ctx->mouse_ctx.get_scroll() > 0 ? Camera::ZoomDir::In : Camera::ZoomDir::Out,  glm::abs(scroll_diff  / 20.f));
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

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(width, height, "3D Scene Editor", NULL, NULL);
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
      if(GLEW_OK != err)
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

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar* vertex_shader =
            R"V0G0N(#version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;

            out vec3 normal;
            out vec3 frag_pos;

            uniform mat4 projection;
            uniform mat4 view_trans;
            uniform mat4 model_trans;

            void main()
            {
                frag_pos = vec3(model_trans * vec4(aPos, 1.0));
                normal = mat3(transpose(inverse(model_trans))) * aNormal;

                gl_Position = projection * view_trans * vec4(frag_pos, 1.0); 
            })V0G0N";
    const GLchar* fragment_shader =
        R"V0G0N(#version 330 core
        in vec3 frag_pos;
        in vec3 normal;

        uniform vec3 triangle_color;

        uniform mat4 model_trans;
        uniform mat4 view_trans;

        out vec4 out_color;

        void main()
        {
            vec3 light_pos = vec3(0.0, 0.0, 20.0);
            vec3 lightColor = vec3(1.0);

            float ambientStrength = 0.2;
            vec3 ambient = ambientStrength * lightColor;
 
            vec3 norm = normalize(normal);
            vec3 lightDir = normalize(light_pos - frag_pos);  
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;

            vec3 result = (ambient + diffuse) * triangle_color;
            out_color = vec4(result, 1.0);
        })V0G0N";
    const GLchar* vertex_shader_normal =
            R"V0G0N(#version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;

            out VS_OUT {
                vec3 normal;
            } vs_out;

            uniform mat4 projection;
            uniform mat4 view_trans;
            uniform mat4 model_trans;

            void main()
            {
                projection;
                gl_Position = view_trans * model_trans * vec4(aPos, 1.0); 
                mat3 normalMatrix = mat3(transpose(inverse(view_trans * model_trans)));
                vs_out.normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
            })V0G0N";
    const GLchar* geometry_shader_normal = 
        R"V0G0N(#version 330 core
            layout (triangles) in;
            layout (line_strip, max_vertices = 6) out;

            in VS_OUT {
                vec3 normal;
            } gs_in[];

            const float MAGNITUDE = 0.1;
            
            uniform mat4 projection;

            void GenerateLine(int index)
            {
                gl_Position = projection * gl_in[index].gl_Position;
                EmitVertex();
                gl_Position = projection * (gl_in[index].gl_Position + 
                                            vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
                EmitVertex();
                EndPrimitive();
            }

            void main()
            {
                GenerateLine(0); // first vertex normal
                GenerateLine(1); // second vertex normal
                GenerateLine(2); // third vertex normal
            })V0G0N";
    const GLchar* fragment_shader_normal =
            R"V0G0N(#version 330 core
            
            uniform vec3 triangle_color;

            out vec4 out_color;

            void main()
            {
                out_color = vec4(triangle_color, 1.0);
            })V0G0N";

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called out_color
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertex_shader,fragment_shader, "", "out_color");
    program.bind();

    Program program_normal;
    program_normal.init(vertex_shader_normal,fragment_shader_normal, geometry_shader_normal, "out_color");

    #ifdef DEBUG
        std::cout << "DEBUG ENABLED" << std::endl;
    #endif
    ctx = std::unique_ptr<GLContext>(new GLContext(program, GLCamera(program, width / height)));
    ctx->init_meshes(std::vector<Mesh>{ BunnyMesh{}, BumpyCubeMesh{}, UnitCube{}, });

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetCursorPosCallback(window, cursor_position_callback);

    glfwSetScrollCallback(window, scroll_callback);

    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ctx->camera.swivel();

        // std::cout << "camera: " << ctx->camera.get_position()[0] << ' ' << ctx->camera.get_position()[1] << ' ' << ctx->camera.get_position()[2] << std::endl;

        // Bind your program
        program.bind();

        ctx->update();

        for (auto& mesh : ctx->mesh_list) {
            mesh.draw();
        }

        // draw normals
        #ifdef DEBUG
        program_normal.bind();

        ctx->update();

        for (auto& mesh : ctx->mesh_list) {
            auto temp = mesh.get_color();
            mesh.set_color(glm::vec3(1.0, 0.0, 0.0));
            mesh.draw();
            mesh.set_color(temp);
        }
        #endif

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}

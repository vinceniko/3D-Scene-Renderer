#include "my_context.h"
#include "mesh_data.h"
#include "cubemap.h"

MyContext::MyContext(int width, int height) :
    // the issue with creating the env->camera first is, due to inheritance, Context is initialized firstpo
    Context {
        width,
        height,
        256,
        256,
    }
{
    set_env(std::make_unique<Environment>(
        std::make_unique<TrackballCamera>(static_cast<float>(width) / height),
        1920,
        PointLights{
            std::make_shared<PointLight>(glm::vec3(2.5f, 1.f, 2.5f)),
            std::make_shared<PointLight>(glm::vec3(-2.5f, 1.f, 2.5f)),
            std::make_shared<PointLight>(glm::vec3(2.5f, 1.f, -2.5f)),
            std::make_shared<PointLight>(glm::vec3(-2.5f, 1.f, -2.5f)),
        },
        std::make_unique<CubeMapEntity>(
            "../data/night_env/",
            true
        )
    ));
    cameras = {
        env->camera.get_camera_ptr(),
        new FreeCamera{ static_cast<float>(width) / height }
    };
    init_mesh_prototypes({ BumpyCubeMesh{}, BunnyMesh{}, MonkeyMesh{} });

    push_mesh_entity({ DefMeshList::QUAD });
    auto& inserted_quad = *(mesh_list.end() - 1);
    inserted_quad->translate(glm::mat4{ 1.f }, glm::vec3(0.f, -1.f, 0.f));
    inserted_quad->rotate(glm::mat4{ 1.f }, -90.f, glm::vec3(1.f, 0.f, 0.f));
    inserted_quad->scale(glm::mat4{ 1.f }, Spatial::ScaleDir::In, 40.f);
    inserted_quad->set_color(glm::vec3(252, 137, 42) / 256.f);
}

void MyContext::switch_cube_map() {
    env->swap_cube_map(curr_cube_map);
}

void MyContext::switch_shader() {
    Optional<MeshEntity> opt_mesh_entity = get_selected();
    if (opt_mesh_entity.has_value()) {
        MeshEntity& mesh_entity = opt_mesh_entity.value().get();
        mesh_entity.set_shader(shaders.cycle());
    }
}

void MyContext::switch_draw_mode() {
    Optional<MeshEntity> opt_mesh_entity = get_selected();
    if (opt_mesh_entity.has_value()) {
        MeshEntity& mesh_entity = opt_mesh_entity.value().get();
        mesh_entity.set_draw_mode(draw_modes.cycle());
    }
}

void MyContext::set_camera(Camera* new_camera) {
    env->camera.set_camera(std::move(std::unique_ptr<Camera>(new_camera)));
}

void MyContext::switch_camera() {
    float aspect = env->camera->get_aspect();
    cameras[camera_idx] = env->camera.get_camera_move().release();  // assign back to array
    camera_idx = (camera_idx + 1) % cameras.size();
    cameras[camera_idx]->set_aspect(aspect);
    set_camera(cameras[camera_idx]);
}

void MyContext::update(std::chrono::duration<float> delta) {
    if (rotate_light) {
        env->dir_light_.set_trans(glm::rotate(env->dir_light_.get_trans(), glm::radians(45.f) * static_cast<float>(delta.count()), glm::vec3(0.f, 1.f, 0.f)));
        env->dir_light_.set_trans(glm::lookAt(env->dir_light_.get_position(), glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f)));
    }

    Context::update(delta);
}
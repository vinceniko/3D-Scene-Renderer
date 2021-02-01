#include "environment.h"

void Environment::bind_static() {
    cube_map_->bind();
}
void Environment::bind_dynamic() {
    cubemap_fbo_.bind();
}

void Environment::buffer() {
    camera.buffer();
    try {
        dir_light_.buffer_shadows();
        buffer_lights();
    }
    catch (const std::runtime_error& e) {
        // doing nothing is acceptable here, shader doesn't have the appropriate light uniform
#ifdef DEBUG
        std::cout << "Light Error: " << e.what() << std::endl;
#endif
    }
}
void Environment::buffer_lights() {
    dir_light_.buffer();
    point_lights_.buffer();
}
void Environment::buffer_shadows() {
    dir_light_.buffer_shadows();
}
void Environment::draw_lights() {
    buffer();
    point_lights_.draw();
}
void Environment::draw_shadows(FBO& main_fbo, MeshEntityList& mesh_list) {
    renderer_->bind(ShaderPrograms::SHADOWS);
    dir_light_.buffer_shadows();
    // disable culling to prevent shadow bias issue
    glDisable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);
    for (auto& mesh : mesh_list) {
        mesh->draw_minimal();
    }
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void Environment::draw_static_scene() {
    bind_static();
    renderer_->bind(ShaderPrograms::PHONG);
    // // not necesary anymore since the point lights are drawn with MeshEntities
    // draw_lights();
    draw_static_cubemap();
}

void Environment::draw_static_cubemap() {
    bind_static();
    renderer_->bind(ShaderPrograms::PHONG);

    glm::mat4 old_view = camera->get_view();
    glm::mat4 w_out_scale = glm::lookAt(camera->get_position(), glm::vec3(glm::inverse(old_view) * glm::vec4(0.f, 0.f, -1.f, 0.f)), glm::vec3(0.f, camera->get_up(), 0.f));
    camera->set_view(w_out_scale);

    ShaderPrograms selected = renderer_->get_selected();
    renderer_->bind(ShaderPrograms::ENV);

    Camera::Projection old_mode = camera->get_projection_mode();
    float old_fov = camera->get_fov();
    glm::mat4 view_w_out_trans = glm::mat3(w_out_scale);
    camera->set_view(view_w_out_trans);
    camera->set_projection_mode(Camera::Projection::Perspective);
    camera->set_fov(fov_);
    camera.buffer();

    cube_map_->draw();

    camera->set_fov(old_fov);
    camera->set_projection_mode(old_mode);
    camera->set_view(old_view);
    renderer_->bind(selected);

    cube_map_->unbind();
}

void Environment::draw_dynamic_cubemap(FBO& main_fbo, MeshEntity& mesh_entity, MeshEntityList& mesh_entities, std::function<void(MeshEntity&)> draw_f) {
    bind_dynamic();

    ShaderPrograms selected = renderer_->get_selected();

    std::array<const glm::vec3, 6> dirs = {
        glm::vec3(10.f, 0.f, 0.f),
        glm::vec3(-10.f, 0.f, 0.f),
        glm::vec3(0.f, 10.f, 0.f),
        glm::vec3(0.f, -10.f, 0.f),
        glm::vec3(0.f, 0.f, 10.f),
        glm::vec3(0.f, 0.f, -10.f),
    };
    std::array<const glm::vec3, 6> up = {
        glm::vec3(0.f, -1.f, 0.f),
        glm::vec3(0.f, -1.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f),
        glm::vec3(0.f, 0.f, -1.f),
        glm::vec3(0.f, -1.f, 0.f),
        glm::vec3(0.f, -1.f, 0.f),
    };
    auto old_view = camera->get_view();
    auto old_fov = camera->get_fov();
    auto old_proj = camera->get_projection_mode();
    auto old_aspect = camera->get_aspect();

    std::unique_ptr<Camera> old_camera = std::move(camera.get_camera_move());
    camera.set_camera(std::make_unique<FreeCamera>(old_camera->get_aspect(), 90.f));

    camera->set_projection_mode(Camera::Projection::Perspective);
    camera->set_fov(90);
    camera->set_aspect(1.0);

    size_t i = 0;
    for (const auto& dir : dirs) {
        cubemap_fbo_.next_face(i);

        // draw env
        renderer_->bind(ShaderPrograms::ENV);
        camera->set_view(glm::lookAt(glm::vec3(0), glm::vec3(dir), up[i]));
        camera.buffer();
        cube_map_->draw();

        glm::mat4 looking_at = glm::lookAt(mesh_entity.get_origin(), mesh_entity.get_origin() + dir, up[i]);
        camera->set_view(looking_at);

        // TODO: the reason the empty spots appear in the reflection of contained reflictive objects is because it is sampling from the same tex that the containing object is drawing

        // draw all other meshes
        for (auto& sec_mesh : mesh_entities) {
            if (sec_mesh.get() != &mesh_entity) {
                draw_f(*sec_mesh);
                cube_map_->bind();
            }
        }

        i++;
    }

    // restore
    camera.set_camera(std::move(old_camera));

    cubemap_fbo_.unbind(main_fbo);

    renderer_->bind(selected);

    cube_map_->unbind();
}

void Environment::set_cube_map(std::unique_ptr<CubeMapEntity> cube_map) {
    cube_map_ = std::move(cube_map);
}
void Environment::swap_cube_map(std::unique_ptr<CubeMapEntity>& cube_map) {
    std::swap(cube_map_, cube_map);
}

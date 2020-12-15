#include "environment.h"

void Environment::draw_static(ShaderProgramCtx& programs) {
    bind_static();
    programs.bind(ShaderPrograms::PHONG);

    draw_lights(programs.get_selected_program());

    glm::mat4 old_view = camera->get_view();
    glm::mat4 w_out_scale = glm::lookAt(camera->get_position(), glm::vec3(glm::inverse(old_view) * glm::vec4(0.f, 0.f, -1.f, 0.f)), glm::vec3(0.f, camera->get_up(), 0.f));
    camera->set_view(w_out_scale);

    ShaderPrograms selected = programs.get_selected();
    programs.bind(ShaderPrograms::ENV);

    Camera::Projection old_mode = camera->get_projection_mode();
    float old_fov = camera->get_fov();
    glm::mat4 view_w_out_trans = glm::mat3(w_out_scale);
    camera->set_view(view_w_out_trans);
    camera->set_projection_mode(Camera::Projection::Perspective);
    camera->set_fov(fov_);
    camera.buffer(programs.get_selected_program());

    cube_map_->draw(programs.get_selected_program());

    camera->set_fov(old_fov);
    camera->set_projection_mode(old_mode);
    camera->set_view(old_view);
    programs.bind(selected);

    cube_map_->unbind();
}

void Environment::draw_dynamic(ShaderProgramCtx& programs, MeshEntity& mesh_entity, MeshEntityList& mesh_entities, std::function<void(MeshEntity&)> draw_f) {
    bind_dynamic();

    ShaderPrograms selected = programs.get_selected();

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
        programs.bind(ShaderPrograms::ENV);
        camera->set_view(glm::lookAt(glm::vec3(0), glm::vec3(dir), up[i]));
        camera.buffer(programs.get_selected_program());
        cube_map_->draw(programs.get_selected_program());

        glm::mat4 looking_at = glm::lookAt(mesh_entity.get_origin(), mesh_entity.get_origin() + dir, up[i]);
        camera->set_view(looking_at);

        // TODO: the reason the empty spots appear in the reflection of contained reflictive objects is because it is sampling from the same tex that the containing object is drawing

        // draw all other meshes
        for (auto& sec_mesh : mesh_entities) {
            if (&sec_mesh != &mesh_entity) {
                draw_f(sec_mesh);
                cube_map_->bind();
            }
        }

        i++;
    }

    // restore
    camera.set_camera(std::move(old_camera));
    
    cubemap_fbo_.unbind();
    
    programs.bind(selected);

    camera.buffer(programs.get_selected_program());

    reset_viewport();
    
    cube_map_->unbind();
}
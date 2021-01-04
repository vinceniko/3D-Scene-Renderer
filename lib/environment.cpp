#include "environment.h"

void Environment::bind_static() {
    cube_map_->bind();
}
void Environment::bind_dynamic() {
    cubemap_fbo_.bind();
}

void Environment::buffer(ShaderProgram& program) {
    camera.buffer(program);
    try {
        dir_light_.buffer_shadows(program);
        buffer_lights(program);
    }
    catch (const std::runtime_error& e) {
        // doing nothing is acceptable here, shader doesn't have the appropriate light uniform
#ifdef DEBUG
        std::cout << "Light Error: " << e.what() << std::endl;
#endif
    }
}
void Environment::buffer_lights(ShaderProgram& program) {
    dir_light_.buffer(program);
    point_lights_.buffer(program);
}
void Environment::buffer_shadows(ShaderProgram& program) {
    dir_light_.buffer_shadows(program);
}
void Environment::draw_lights(ShaderProgram& program) {
    buffer(program);
    point_lights_.draw(program);
}
void Environment::draw_shadows(ShaderProgramCtx& programs, MeshEntityList& mesh_list) {
    programs.bind(ShaderPrograms::SHADOWS);
    depth_fbo_.bind();
    dir_light_.buffer_shadows(programs.get_selected_program());
    // disable culling to prevent shadow bias issue
    glDisable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);
    for (auto& mesh : mesh_list) {
        mesh->draw_minimal(programs.get_selected_program());
    }
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    depth_fbo_.unbind();
    reset_viewport();
}

void Environment::draw_static_scene(ShaderProgramCtx& programs) {
    bind_static();
    programs.bind(ShaderPrograms::PHONG);
    // // not necesary anymore since the point lights are drawn with MeshEntities
    // draw_lights(programs.get_selected_program());
    draw_static_cubemap(programs);
}

void Environment::draw_static_cubemap(ShaderProgramCtx& programs) {
    bind_static();
    programs.bind(ShaderPrograms::PHONG);

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

void Environment::draw_dynamic_cubemap(ShaderProgramCtx& programs, MeshEntity& mesh_entity, MeshEntityList& mesh_entities, std::function<void(MeshEntity&)> draw_f) {
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
            if (sec_mesh.get() != &mesh_entity) {
                draw_f(*sec_mesh);
                cube_map_->bind();
            }
        }

        i++;
    }

    // restore
    camera.set_camera(std::move(old_camera));

    cubemap_fbo_.unbind();

    programs.bind(selected);

    reset_viewport();

    cube_map_->unbind();
}

void Environment::set_width(int width) {
    width_ = width;
}
void Environment::set_height(int height) {
    height_ = height;
}
void Environment::set_viewport(int width, int height) {
    width_ = width;
    height_ = height;
    camera->set_aspect(width, height);
    glViewport(0, 0, width, height);
}
void Environment::reset_viewport() {
    glViewport(0, 0, width_, height_);
}

void Environment::set_cube_map(std::unique_ptr<GL_CubeMapEntity> cube_map) {
    cube_map_ = std::move(cube_map);
}
void Environment::swap_cube_map(std::unique_ptr<GL_CubeMapEntity>& cube_map) {
    std::swap(cube_map_, cube_map);
}

void Environment::draw_depth_map(ShaderProgramCtx& programs) {
    // debug quad
    programs.bind(ShaderPrograms::SHADOW_MAP);

    auto old_trans = camera->get_trans();
    auto old_projection = camera->get_projection_mode();
    auto old_aspect = camera->get_aspect();

    camera->set_projection_mode(Camera::Projection::Ortho);
    camera->set_view(glm::mat4{ 1.f });

    camera.buffer(programs.get_selected_program());
    auto quad = MeshFactory::get().get_mesh_entity(DefMeshList::QUAD);
    quad.translate(glm::mat4{ 1.f }, glm::vec3(-1.0f, 0.5f, -0.01f));
    // quad.scale(glm::mat4{ 1.f }, MeshEntity::ScaleDir::In, 10.f);
    depth_fbo_.get_tex().bind();
    quad.draw_minimal(programs.get_selected_program());
    camera->set_trans(old_trans);
    camera->set_projection_mode(old_projection);
}
void Environment::set_debug_depth_map(bool state) {
    debug_depth_map_ = state;
}
bool Environment::get_debug_depth_map() {
    return debug_depth_map_;
}
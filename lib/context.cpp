#include <cmath>

#include "context.h"

void MouseContext::hold() {
    held_ = true;
}
void MouseContext::hold_select(size_t i) {
    selected_ = i;
#ifdef DEBUG
    std::cout << "object selected: " << selected_ << std::endl;
#endif
    held_ = true;
}
void MouseContext::release() {
    held_ = false;
}
void MouseContext::deselect() {
#ifdef DEBUG
    if (selected_ >= 0)
        std::cout << "object deselected: " << selected_ << std::endl;
#endif
    selected_ = -1;
}
int MouseContext::get_selected() const {
    return selected_;
}
void MouseContext::set_selected(int selected) {
    selected_ = selected;
}
bool MouseContext::is_selected() const {
    return selected_ >= 0;
}
bool MouseContext::is_held() const {
    return held_;
}
void MouseContext::set_position(glm::vec2 world_point) {
    prev_world_point_ = pos_;
    pos_ = world_point;
}
glm::vec2 MouseContext::get_position() const {
    return pos_;
}
glm::vec2 MouseContext::get_prev_position() const {
    return prev_world_point_;
}
void MouseContext::set_scroll(double scroll) {
    prev_scroll_ = scroll_;
    scroll_ = scroll;
}
double MouseContext::get_prev_scroll() const {
    return prev_scroll_;
}
double MouseContext::get_scroll() const {
    return scroll_;
}

Context::Context(std::unique_ptr<Environment> new_env) : env(std::move(new_env)) {
    for (auto& point_light : env->point_lights_) {
        mesh_list.push_back(point_light);
    }
}

int Context::intersected_mesh_perspective(glm::vec3 world_ray) const {
    float min_dist = std::numeric_limits<float>::infinity();
    int closest = -1;
    for (size_t i = 0; i < mesh_list.size(); i++) {
        float distance = mesh_list[i]->intersected_triangles(env->camera->get_position(), world_ray);
        if (distance >= 0) {
            if (min_dist > distance) {
                min_dist = distance;
                closest = i;
            }
        }
    }
    return closest;
}
int Context::intersected_mesh_ortho(glm::vec3 world_pos) const {
    float min_dist = std::numeric_limits<float>::infinity();
    int closest = -1;
    for (size_t i = 0; i < mesh_list.size(); i++) {
        float distance = mesh_list[i]->intersected_triangles(world_pos, glm::inverse(env->camera->get_view()) * glm::vec4(0.0, 0.0, -1.f, 0.f));
        if (distance >= 0) {
            if (min_dist > distance) {
                min_dist = distance;
                closest = i;
            }
        }
    }
    return closest;
}
void Context::select(glm::vec2 cursor_pos, float width, float height) {
    if (env->camera->get_projection_mode() == Camera::Projection::Ortho) {
        glm::vec3 pos_world = env->camera->get_pos_world(cursor_pos, width, height);
        select_ortho(pos_world);
    }
    else {
        glm::vec3 ray_world = env->camera->get_ray_world(cursor_pos, width, height);
        select_perspective(ray_world);
    }
}
void Context::select_perspective(glm::vec3 world_ray) {
    int found = intersected_mesh_perspective(world_ray);
    if (found >= 0 && found != mouse_ctx.get_selected()) {
        deselect();
        mouse_ctx.hold_select(found);
        // mesh_list[found].set_color(glm::vec3(1.f) - mesh_list[found].get_color());
    }
    else if (found >= 0) {
        deselect();
    }
}
void Context::select_ortho(glm::vec3 world_pos) {
    int found = intersected_mesh_ortho(world_pos);
    if (found >= 0 && found != mouse_ctx.get_selected()) {
        deselect();
        mouse_ctx.hold_select(found);
        // mesh_list[found].set_color(glm::vec3(1.f) - mesh_list[found].get_color());
    }
    else if (found >= 0) {
        deselect();
    }
}
void Context::deselect() {
    if (mouse_ctx.is_selected()) {
        // mesh_list[mouse_ctx.get_selected()].set_color(glm::vec3(1.f) - mesh_list[mouse_ctx.get_selected()].get_color());
    }
    mouse_ctx.deselect();
}
Optional<MeshEntity> Context::get_selected() {
    if (mouse_ctx.is_selected()) {
        return { {*mesh_list[mouse_ctx.get_selected()] } };
    }

    return {};
}

void Context::update(std::chrono::duration<float> delta) {
    programs.reload();

    if (mouse_ctx.is_held()) {
        glm::vec2 old_point = mouse_ctx.get_prev_position();
        glm::vec2 new_point = mouse_ctx.get_position();
        env->camera->translate(glm::vec3(new_point, 0.f), glm::vec3(old_point, 0.f));
    }
}

void Context::init_mesh_prototypes(std::vector<Mesh>&& meshes) {
    mesh_factory.push(programs.get_selected_program(), meshes);
}
void Context::push_mesh_entity(std::vector<int>&& ids) {
    for (const auto& id : ids) {
        mesh_list.push_back(std::make_shared<MeshEntity>(mesh_factory.get_mesh_entity(id)));
    }
}

void Context::draw_selected(MeshEntity& mesh_entity) {
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    // glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE); // for outline only, no solid fill
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    draw_w_mode(mesh_entity);

    ShaderPrograms selected = mesh_entity.get_shader();
    programs.bind(ShaderPrograms::OUTLINE);
    Uniform aspect("u_aspect");
    aspect.buffer(programs.get_selected_program(), env->camera->get_aspect());
    env->camera.buffer(programs.get_selected_program());
    // glDisable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDepthFunc(GL_ALWAYS);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    mesh_entity.draw_minimal(programs.get_selected_program());

    glDepthFunc(GL_LESS);
    glStencilMask(0xFF);
    glDisable(GL_STENCIL_TEST);
    glCullFace(GL_BACK);
    // glEnable(GL_CULL_FACE);
    programs.bind(selected);
}

void Context::draw_w_mode(MeshEntity& mesh_entity) {
    programs.bind(mesh_entity.get_shader());
    env->camera.buffer(programs.get_selected_program());
    if (mesh_entity.get_draw_mode() != DrawMode::WIREFRAME_ONLY) {
        draw_surfaces(mesh_entity);
    }
    if (mesh_entity.get_draw_mode() == DrawMode::WIREFRAME || mesh_entity.get_draw_mode() == DrawMode::WIREFRAME_ONLY) {
        draw_wireframes(mesh_entity);
    }
    else if (mesh_entity.get_draw_mode() == DrawMode::DRAW_NORMALS) {
        draw_normals(mesh_entity);
    }
}
void Context::draw(MeshEntity& mesh_entity, MeshEntityList& mesh_entities) {
    if (mesh_entity.get_dyn_reflections() && (mesh_entity.get_shader() == ShaderPrograms::REFLECT || mesh_entity.get_shader() == ShaderPrograms::REFRACT)) {
        env->draw_dynamic_cubemap(programs, mesh_entity, mesh_entities, [&](MeshEntity& sec_mesh) {
            draw_w_mode(sec_mesh);
        });
    }
    else if (mesh_entity.get_shader() == ShaderPrograms::REFLECT || mesh_entity.get_shader() == ShaderPrograms::REFRACT) {
        env->bind_static();
    }
    // else {
    //     env->depth_fbo_.get_tex().bind();
    // }
    draw_static(mesh_entity);
}

void Context::draw_static(MeshEntity& mesh_entity) {
    if (get_selected().has_value() && &mesh_entity == &get_selected().value().get()) {
        draw_selected(mesh_entity);
    }
    else {
        draw_w_mode(mesh_entity);
    }
}

void Context::swap_selected_mesh(const uint32_t idx) {
    uint32_t i = 0;
    for (auto& mesh_entity : mesh_list) {
        if (get_selected().has_value() && mesh_entity.get() == &get_selected().value().get()) {
            std::swap(mesh_list[i], mesh_list[idx]);
            mouse_ctx.set_selected(idx);
        }
        i++;
    }
}

void Context::draw() {
    programs.bind(ShaderPrograms::SHADOWS);
    env->draw_shadows(programs, mesh_list);

    // swap selected to end of drawing list
    swap_selected_mesh(mesh_list.size() - 1.0);
    for (auto& mesh_entity : mesh_list) {
        draw(*mesh_entity, mesh_list);
    }
    
    env->draw_static_scene(programs);
    
    if (env->get_debug_depth_map()) {
        env->draw_depth_map(programs);
    }
}
void Context::draw_surfaces(MeshEntity& mesh_entity) {
    programs.bind(mesh_entity.get_shader());
    // TODO: check if shader has attached uniform at compile time elsewhere
    if (mesh_entity.get_shader() == ShaderPrograms::PHONG || mesh_entity.get_shader() == ShaderPrograms::FLAT || mesh_entity.get_shader() == ShaderPrograms::REFLECT || mesh_entity.get_shader() == ShaderPrograms::REFRACT) {
        // bind the depth map as well for env mapped objs
        if (mesh_entity.get_shader() == ShaderPrograms::REFLECT || mesh_entity.get_shader() == ShaderPrograms::REFRACT) {
            // * don't need to bind the cubemap texture here because it is already bound after rendering to it
            env->depth_fbo_.get_tex().bind(GL_TEXTURE1); // bind the depthmap to the second texture slot
            Uniform("u_skybox").buffer(programs.get_selected_program(), 0);
            Uniform("u_shadow_map").buffer(programs.get_selected_program(), 1);
        }
        env->buffer_lights(programs.get_selected_program());
        env->buffer_shadows(programs.get_selected_program());
        env->debug_shadows_.buffer(programs.get_selected_program());
    }
    mesh_entity.draw(programs.get_selected_program());
}
void Context::draw_surfaces() {
    for (auto& mesh : mesh_list) {
        draw_surfaces(*mesh);
    }
}
void Context::draw_wireframes(MeshEntity& mesh_entity) {
    ShaderPrograms selected = mesh_entity.get_shader();

    programs.bind(ShaderPrograms::DEF_SHADER);

    if (env->camera->get_projection_mode() == Camera::Projection::Perspective) {
        float min_zoom = 1.f / std::pow(2, 8);  // to prevent z-fighting

        auto temp = env->camera->get_view();
        // minimally scale the view to draw on top
        env->camera->scale_view(Camera::ScaleDir::Out, min_zoom);
        env->camera.buffer(programs.get_selected_program());
        mesh_entity.draw_wireframe(programs.get_selected_program());
        env->camera->set_view(temp);
    } else {
        env->camera.buffer(programs.get_selected_program());
        auto old_trans = mesh_entity.get_trans();
        mesh_entity.scale(env->camera->get_view(), Spatial::ScaleDir::In, 1.f / std::pow(2, 8));
        mesh_entity.draw_wireframe(programs.get_selected_program());
        mesh_entity.set_trans(old_trans);
    }

    programs.bind(selected);
}
void Context::draw_wireframes() {
    for (auto& mesh : mesh_list) {
        draw_wireframes(*mesh);
    }
}
void Context::draw_normals(MeshEntity& mesh_entity) {
    ShaderPrograms selected = mesh_entity.get_shader();

    programs.bind(ShaderPrograms::NORMALS);;
    env->camera.buffer(programs.get_selected_program());

    auto temp = mesh_entity.get_color();
    mesh_entity.set_color(glm::vec3(1.0, 0.0, 0.0));
    mesh_entity.draw(programs.get_selected_program());
    mesh_entity.set_color(temp);

    programs.bind(selected);

    draw_wireframes(mesh_entity);
}
void Context::draw_normals() {
    for (auto& mesh : mesh_list) {
        draw_normals(*mesh);
    }
}

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

Context::Context(int width, int height, int base_width, int base_height) :
    base_width_(base_width),
    base_height_(base_height) {
    renderer = std::make_unique<DefRenderer>();
    set_global_renderer(renderer.get());
    mesh_factory = std::make_unique<DefMeshFactory>();
    set_global_mesh_factory(mesh_factory.get());

    auto aspect = base_width / (static_cast<float>(width) / height);
    main_fbo_ = std::make_unique<FBO>(0, width, height);
    offscreen_fbo_ = std::make_unique<Offscreen_FBO>(base_width, aspect);
    offscreen_fbo_msaa_ = std::make_unique<Offscreen_FBO_Multisample>(base_width, aspect);
    depth_fbo_ = std::make_unique<Depth_FBO>(1024, 1024);
    debug_shadows_ = std::make_unique<DebugShadows>();
}

void Context::set_env(std::unique_ptr<Environment>&& new_env) {
    env = std::move(new_env);
    // set_viewport(width_, height_);
    for (auto& point_light : env->point_lights_) {
        mesh_list.push_back(point_light);
    }
}

void Context::set_viewport(int width, int height) {
    float aspect = static_cast<float>(width) / height;
    env->camera->set_aspect(aspect);

    float base_aspect = static_cast<float>(base_width_) / base_height_;
    if (!isnan(base_aspect)) {
        int height_ = base_width_ / aspect;
        int base_height = base_width_ * 1.f / base_aspect;
        int width_ = base_height * aspect;
        if (width_ < height_) {
            offscreen_fbo_->resize(width_, base_height);
            offscreen_fbo_msaa_->resize(width_, base_height);
        }
        else {
            offscreen_fbo_->resize(base_width_, height_);
            offscreen_fbo_msaa_->resize(base_width_, height_);
        }
        main_fbo_->resize(width, height);
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
    renderer->reload();

    if (mouse_ctx.is_held()) {
        glm::vec2 old_point = mouse_ctx.get_prev_position();
        glm::vec2 new_point = mouse_ctx.get_position();
        env->camera->translate(glm::vec3(new_point, 0.f), glm::vec3(old_point, 0.f));
    }
}

void Context::init_mesh_prototypes(std::vector<Mesh>&& meshes) {
    MESH_FACTORY->push(meshes);
}
void Context::push_mesh_entity(std::vector<int>&& ids) {
    for (const auto& id : ids) {
        mesh_list.push_back(std::make_shared<MeshEntity>(MESH_FACTORY->get_mesh_entity(id)));
    }
}

void Context::draw_selected_to_stencil(MeshEntity& mesh_entity) {
    glEnable(GL_STENCIL_TEST);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    // glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE); // for outline only, no solid fill
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    draw_w_mode(mesh_entity);

    glDisable(GL_STENCIL_TEST);
}

void Context::draw_selected(MeshEntity& mesh_entity) {
    glEnable(GL_STENCIL_TEST);
    ShaderPrograms selected = mesh_entity.get_shader();
    renderer->bind(ShaderPrograms::OUTLINE);
    Uniform aspect("u_aspect");
    aspect.buffer(env->camera->get_aspect());
    env->camera.buffer();
    glDisable(GL_CULL_FACE); // disable to render selected quads better (quads have weird normals)
    // glCullFace(GL_FRONT); // culling works for selected objects with correct normals
    glDepthFunc(GL_ALWAYS);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    mesh_entity.draw_minimal();

    glDepthFunc(GL_LESS);
    glStencilMask(0xFF);
    glDisable(GL_STENCIL_TEST);
    // glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    renderer->bind(selected);
}

void Context::draw_w_mode(MeshEntity& mesh_entity) {
    renderer->bind(mesh_entity.get_shader());
    env->camera.buffer();
    if (mesh_entity.get_draw_mode() != DrawMode::WIREFRAME_ONLY) {
        draw_surfaces(mesh_entity);
    }
    if (mesh_entity.get_draw_mode() == DrawMode::WIREFRAME || mesh_entity.get_draw_mode() == DrawMode::WIREFRAME_ONLY) {
        draw_wireframe(mesh_entity);
    }
    else if (mesh_entity.get_draw_mode() == DrawMode::DRAW_NORMALS) {
        draw_normals(mesh_entity);
    }
}
void Context::draw(FBO& main_fbo, MeshEntity& mesh_entity, MeshEntityList& mesh_entities) {
    if (mesh_entity.get_dyn_reflections() && (mesh_entity.get_shader() == ShaderPrograms::REFLECT || mesh_entity.get_shader() == ShaderPrograms::REFRACT)) {
        env->draw_dynamic_cubemap(main_fbo, mesh_entity, mesh_entities, [&](MeshEntity& sec_mesh) {
            draw_w_mode(sec_mesh);
            });
    }
    else if (mesh_entity.get_shader() == ShaderPrograms::REFLECT || mesh_entity.get_shader() == ShaderPrograms::REFRACT) {
        env->bind_static();
    }
    // else {
    //     env->depth_fbo_->get_tex().bind();
    // }
    draw_static(mesh_entity);
}

void Context::draw_static(MeshEntity& mesh_entity) {
    if (get_selected().has_value() && &mesh_entity == &get_selected().value().get()) {
        draw_selected_to_stencil(mesh_entity);
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

void Context::draw_grid() {
    auto quad = MESH_FACTORY->get_mesh_entity(DefMeshList::QUAD);
    quad.rotate(glm::mat4{ 1.f }, -90.f, glm::vec3(1.f, 0.f, 0.f));
    quad.scale(glm::mat4{ 1.f }, Spatial::ScaleDir::In, 20.f);
    renderer->bind(ShaderPrograms::GRID);
    env->camera.buffer();
    glDisable(GL_CULL_FACE);
    quad.draw_minimal();
    glEnable(GL_CULL_FACE);
}

void Context::draw() {
    FBO* draw_fbo = offscreen_fbo_.get();
    if (msaa_use_) {
        draw_fbo = offscreen_fbo_msaa_.get();
    }
    draw_fbo->bind();
    // main_fbo_->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    depth_fbo_->bind();
    // env->draw_shadows(main_fbo_, mesh_list);
    env->draw_shadows(*draw_fbo, mesh_list);
    // depth_fbo_->unbind(*main_fbo_.get());
    depth_fbo_->unbind(*draw_fbo);

    // swap selected to end of drawing list
    uint32_t selected_idx = mesh_list.size() - 1.0;
    swap_selected_mesh(selected_idx);
    for (auto& mesh_entity : mesh_list) {
        draw(*draw_fbo, *mesh_entity, mesh_list);
        // draw(main_fbo_, *mesh_entity, mesh_list);
    }

    env->draw_static_scene();

    if (msaa_use_) {
        draw_fbo->unbind(*offscreen_fbo_.get());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        draw_fbo->blit(*offscreen_fbo_.get());
    }

    draw_offscreen(*offscreen_fbo_.get());

    if (get_selected().has_value()) {
        auto& mesh_entity = *mesh_list[selected_idx];
        draw_selected(mesh_entity);
    }

    offscreen_fbo_->unbind(*main_fbo_.get());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    offscreen_fbo_->blit(*main_fbo_.get());
    if (fxaa_use_) {
        draw_fxaa(*offscreen_fbo_.get());
    }

    if (draw_grid_) {
        draw_grid();
    }

    if (debug_depth_map_) {
        draw_depth_map();
    }
}

void Context::draw_offscreen(Offscreen_FBO& draw_fbo) {
    draw_fbo.bind_offscreen();
    // glDepthFunc(GL_ALWAYS);
    glDisable(GL_DEPTH_TEST); // not writing to depth in shader
    auto quad = MESH_FACTORY->get_mesh_entity(DefMeshList::QUAD);
    quad.draw_none();
    // glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
}

void Context::draw_fxaa(Offscreen_FBO& draw_fbo) {
    renderer->bind(ShaderPrograms::FXAA);
    Uniform("u_offscreen_tex").buffer(0);
    draw_fbo.get_tex().bind(GL_TEXTURE0);
    glDisable(GL_DEPTH_TEST); // not writing to depth in shader
    auto quad = MESH_FACTORY->get_mesh_entity(DefMeshList::QUAD);
    Uniform("inverseScreenSize.x").buffer(1.f / draw_fbo.get_width());
    Uniform("inverseScreenSize.y").buffer(1.f / draw_fbo.get_height());
    quad.draw_none();
    // glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
}

void Context::draw_surfaces(MeshEntity& mesh_entity) {
    renderer->bind(mesh_entity.get_shader());
    // TODO: check if shader has attached uniform at compile time elsewhere
    if (mesh_entity.get_shader() == ShaderPrograms::PHONG || mesh_entity.get_shader() == ShaderPrograms::FLAT || mesh_entity.get_shader() == ShaderPrograms::REFLECT || mesh_entity.get_shader() == ShaderPrograms::REFRACT) {
        // bind the depth map as well for env mapped objs
        env->buffer_lights();
        env->buffer_shadows();
        debug_shadows_->buffer();
    }
    if (mesh_entity.get_shader() == ShaderPrograms::REFLECT || mesh_entity.get_shader() == ShaderPrograms::REFRACT) {
        // * don't need to bind the cubemap texture here because it is already bound after rendering to it
        depth_fbo_->get_tex().bind(GL_TEXTURE1); // bind the depthmap to the second texture slot
        Uniform("u_skybox").buffer(0);
        Uniform("u_shadow_map").buffer(1);
        glCullFace(GL_BACK);
        mesh_entity.draw_minimal();
    }
    else {
        mesh_entity.draw();
    }
    depth_fbo_->get_tex().bind(); // bind back to first texture slot
}
void Context::draw_surfaces() {
    for (auto& mesh : mesh_list) {
        draw_surfaces(*mesh);
    }
}
void Context::draw_wireframe(MeshEntity& mesh_entity) {
    ShaderPrograms selected = mesh_entity.get_shader();

    renderer->bind(ShaderPrograms::DEF_SHADER);

    if (env->camera->get_projection_mode() == Camera::Projection::Perspective) {
        float min_zoom = 1.f / std::pow(2, 8);  // to prevent z-fighting

        auto temp = env->camera->get_view();
        // minimally scale the view to draw on top
        env->camera->scale_view(Camera::ScaleDir::Out, min_zoom);
        env->camera.buffer();
        mesh_entity.draw_wireframe();
        env->camera->set_view(temp);
    }
    else {
        env->camera.buffer();
        auto old_trans = mesh_entity.get_trans();
        mesh_entity.scale(env->camera->get_view(), Spatial::ScaleDir::In, 1.f / std::pow(2, 8));
        mesh_entity.draw_wireframe();
        mesh_entity.set_trans(old_trans);
    }

    renderer->bind(selected);
}
void Context::draw_wireframes() {
    for (auto& mesh : mesh_list) {
        draw_wireframe(*mesh);
    }
}
void Context::draw_normals(MeshEntity& mesh_entity) {
    ShaderPrograms selected = mesh_entity.get_shader();

    renderer->bind(ShaderPrograms::NORMALS);;
    env->camera.buffer();

    auto temp = mesh_entity.get_color();
    mesh_entity.set_color(glm::vec3(1.0, 0.0, 0.0));
    mesh_entity.draw();
    mesh_entity.set_color(temp);

    renderer->bind(selected);

    draw_wireframe(mesh_entity);
}
void Context::draw_normals() {
    for (auto& mesh : mesh_list) {
        draw_normals(*mesh);
    }
}

void Context::draw_depth_map() {
    // debug quad
    glDisable(GL_DEPTH_TEST);
    renderer->bind(ShaderPrograms::SHADOW_MAP);

    auto old_trans = env->camera->get_trans();
    auto old_projection = env->camera->get_projection_mode();
    auto old_aspect = env->camera->get_aspect();

    env->camera->set_projection_mode(Camera::Projection::Ortho);
    env->camera->set_view(glm::mat4{ 1.f });
    // camera->set_aspect(1.f);

    env->camera.buffer();
    auto quad = MESH_FACTORY->get_mesh_entity(DefMeshList::QUAD);
    quad.translate(env->camera->get_trans(), glm::vec3(-0.5f, 0.5f, -0.01f));
    // quad.scale(glm::mat4{ 1.f }, MeshEntity::ScaleDir::In, 10.f);
    depth_fbo_->get_tex().bind();
    quad.draw_minimal();
    env->camera->set_trans(old_trans);
    env->camera->set_projection_mode(old_projection);
    // camera->set_aspect(old_aspect);
    glEnable(GL_DEPTH_TEST);
}

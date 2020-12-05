#include "my_context.h"
#include "mesh_data.h"

MyContext::MyContext(int width, int height) :
// the issue with creating the env.camera first is, due to inheritance, Context is initialized firstpo
Context(std::make_shared<TrackballCamera>(TrackballCamera{ static_cast<float>(width) / height }), width, height),
cameras{
    env.camera.get_camera_ptr(),
    std::make_shared<FreeCamera>(FreeCamera{ static_cast<float>(width) / height })
} {
    init_mesh_prototypes(std::vector<Mesh>{ BumpyCubeMesh{}, BunnyMesh{}, TorusMesh{}, MonkeyMesh{}, SphereMesh{} }); 
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

void MyContext::set_camera(std::shared_ptr<Camera> new_camera) {
    env.camera.set_camera(new_camera);
}
void MyContext::switch_camera() {
    camera_idx = (camera_idx + 1) % cameras.size();
    env.camera.set_camera(cameras[camera_idx]);
}

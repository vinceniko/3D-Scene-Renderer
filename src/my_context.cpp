#include "my_context.h"
#include "mesh_data.h"

MyContext::MyContext(std::unique_ptr<ShaderProgramCtx> programs, float width, float height) :
// the issue with creating the env.camera first is, due to inheritance, Context is initialized firstpo
Context(std::move(programs), std::make_shared<TrackballCamera>(TrackballCamera{ width / height })),
cameras{
    env.camera.get_camera_ptr(),
    std::make_shared<FreeCamera>(FreeCamera{ width / height })
} {
    init_mesh_prototypes(std::vector<Mesh>{ BumpyCubeMesh{}, BunnyMesh{}, TorusMesh{}, MonkeyMesh{} }); 
}

void MyContext::switch_program() {
    programs->bind(shaders[(shader_idx += 1) %= shaders.size()]);
}
void MyContext::set_camera(std::shared_ptr<Camera> new_camera) {
    env.camera.set_camera(new_camera);
}
void MyContext::switch_camera() {
    camera_idx = (camera_idx + 1) % cameras.size();
    env.camera.set_camera(cameras[camera_idx]);
}

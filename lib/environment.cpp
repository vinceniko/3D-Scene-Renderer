#include "environment.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

CubeMap::CubeMapFace CubeMap::decode_face(const std::string& kind) {
    if (kind == "posz") {
        return CubeMapFace::BACK;
    }
    else if (kind == "negz") {
        return CubeMapFace::FRONT;
    }
    else if (kind == "negx") {
        return CubeMapFace::LEFT;
    }
    else if (kind == "posx") {
        return CubeMapFace::RIGHT;
    }
    else if (kind == "posy") {
        return CubeMapFace::TOP;
    }
    else if (kind == "negy") {
        return CubeMapFace::BOTTOM;
    }
    throw std::runtime_error("Error decoding CubeMap face: " + kind);
}

uint32_t GLCubeMap::gl_decode_face(CubeMap::CubeMapFace face) {
    switch (face) {
    case CubeMap::BACK:
        return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
    case CubeMap::FRONT:
        return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
    case CubeMap::LEFT:
        return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
    case CubeMap::RIGHT:
        return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    case CubeMap::TOP:
        return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
    case CubeMap::BOTTOM:
        return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
    }
    throw std::runtime_error("Error translating CubeMap face to GL face");
}

CubeMap::CubeMapFace CubeMap::parse_path_name(const std::string& path_name) {
    // index of extension
    size_t ext_idx = path_name.rfind('.');
    if (ext_idx == std::string::npos) {
        throw "Error finding extension name of file";
    }
    // ex: "negex"
    constexpr int face_size = 4;

    std::string face_str = path_name.substr(ext_idx - face_size, face_size);

    return decode_face(face_str);
}

uint32_t GLCubeMap::gl_decode_face(const std::string& path_name) {
    return gl_decode_face(parse_path_name(path_name));
}

void GLCubeMap::load(const std::string& dir_path, bool flip) {
    glGenTextures(1, &tex_id_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id_);

    stbi_set_flip_vertically_on_load(flip);
    for (auto& tex_path : std::filesystem::directory_iterator(dir_path)) {
        int width, height, n_chan;
        unsigned char* data = stbi_load(tex_path.path().c_str(), &width, &height, &n_chan, 0);
        if (data) {
            glTexImage2D(
                gl_decode_face(tex_path.path()),
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
        }
        else {
            stbi_image_free(data);
            throw std::runtime_error("Error loading texture data for cube map");
        }
        stbi_image_free(data);
    }
    stbi_set_flip_vertically_on_load(false);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GLCubeMap::draw(ShaderProgramCtx& programs) {
    glDepthFunc(GL_LEQUAL);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id_);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    cube_entity_.draw_no_color(programs);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    glDepthFunc(GL_LESS);
}

void Environment::draw(ShaderProgramCtx& programs) {
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

    camera.buffer(programs);

    cube_map_.draw(programs);

    camera->set_fov(old_fov);
    camera->set_projection_mode(old_mode);
    camera->set_view(old_view);
    programs.bind(selected);
}
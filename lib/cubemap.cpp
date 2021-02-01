#include "cubemap.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void CubeMapTex::init() {
    bind();

    for (int i = 0; i < 6; i++) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

#ifdef DEBUG
        check_gl_error();
#endif
}

CubeMapLoader::CubeMapFace CubeMapLoader::decode_face(const std::string& kind) {
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

uint32_t CubeMapEntity::gl_decode_face(CubeMapLoader::CubeMapFace face) {
    switch (face) {
    case CubeMapLoader::BACK:
        return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
    case CubeMapLoader::FRONT:
        return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
    case CubeMapLoader::LEFT:
        return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
    case CubeMapLoader::RIGHT:
        return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    case CubeMapLoader::TOP:
        return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
    case CubeMapLoader::BOTTOM:
        return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
    }
    throw std::runtime_error("Error translating CubeMap face to GL face");
}

CubeMapLoader::CubeMapFace CubeMapLoader::parse_path_name(const std::string& path_name) {
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

uint32_t CubeMapEntity::gl_decode_face(const std::string& path_name) {
    return gl_decode_face(parse_path_name(path_name));
}

void CubeMapEntity::init(const std::string& dir_path, bool flip) {
    bind();

    stbi_set_flip_vertically_on_load(flip);
    for (auto& tex_path : std::filesystem::directory_iterator(dir_path)) {
        int n_chan;
        unsigned char* data = stbi_load(tex_path.path().string().c_str(), &width_, &width_, &n_chan, 0);
        if (data) {
            glTexImage2D(
                gl_decode_face(tex_path.path().string()),
                0, GL_RGB, width_, width_, 0, GL_RGB, GL_UNSIGNED_BYTE, data
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
}

void CubeMapEntity::draw() {
    glDepthFunc(GL_LEQUAL);

    glActiveTexture(GL_TEXTURE0);

    bind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    cube_entity_.draw_minimal();

    glDepthFunc(GL_LESS);

    unbind();
}
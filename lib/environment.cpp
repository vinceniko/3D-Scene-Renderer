#include "environment.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void GLCubeMap::load(const std::string& dir_path) {
    glGenTextures(1, &tex_id_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id_);

    for (auto& tex_path : std::filesystem::directory_iterator(dir_path)) {
        stbi_set_flip_vertically_on_load(true);

        int width, height, n_chan;
        unsigned char* data = stbi_load(tex_path.path().c_str(), &width, &height, &n_chan, 0);
        if (data) {
            glTexImage2D(
                get_face(tex_path.path()),
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
        }
        else {
            stbi_image_free(data);
            throw std::runtime_error("Error loading texture data for cube map");
        }
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
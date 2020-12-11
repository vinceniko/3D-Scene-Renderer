#include <vector>
#include <sstream>

#include "renderer.h"
#include "spatial.h"

struct LightTrait {
    glm::vec3 color_ = glm::vec3(0.5);
    float strength_ = 1.f;

    LightTrait(glm::vec3 color) : color_(color) {}
    LightTrait(float strength) : strength_(strength) {}
    LightTrait(glm::vec3 color, float strength) : color_(color), strength_(strength) {}

    glm::vec3 get_trait() {
        return color_ * strength_;
    }
};

struct Light : public Spatial {
    glm::vec3 light_color = glm::vec3(0.5);

    LightTrait ambient{ light_color, 0.5 };
    LightTrait specular{ light_color, 0.5 };
    float shininess = pow(2, 7);

    std::string uniform_prefix;

    Uniform u_light_color;
    Uniform u_ambient;
    Uniform u_specular;
    Uniform u_shininess;

    Light(std::string&& kind) : uniform_prefix(kind) {
        u_light_color.name_ = uniform_prefix + ".light_color";
        u_ambient.name_ = uniform_prefix + ".ambient";
        u_specular.name_ = uniform_prefix + ".specular";
        u_shininess.name_ = uniform_prefix + ".shininess";
    }
    void buffer(ShaderProgram& program) {
        u_light_color.buffer(program, light_color);
        u_ambient.buffer(program, ambient.get_trait());
        u_specular.buffer(program, specular.get_trait());
        u_shininess.buffer(program, shininess);
    }
};

struct DirLight : public Light {    
    Uniform u_direction;

    DirLight() : Light("dir_light") {
        u_direction.name_ = uniform_prefix + ".direction";
    }
    void buffer(ShaderProgram& program) {
        Light::buffer(program);
        u_direction.buffer(program, look_direction());
    }
};

struct PointLight : public Light {
    float constant;
    float linear;
    float quadratic;

    Uniform u_constant;
    Uniform u_linear;
    Uniform u_quadratic;

    Uniform u_position;

    PointLight() : Light("point_light") {
        u_constant.name_ = uniform_prefix + ".constant";
        u_linear.name_ = uniform_prefix + ".linear";
        u_quadratic.name_ = uniform_prefix + ".quadratic";

        u_position.name_ = uniform_prefix + ".position";
    }
    void buffer(ShaderProgram& program) {
        Light::buffer(program);
        u_constant.buffer(program, constant);
        u_linear.buffer(program, linear);
        u_quadratic.buffer(program, quadratic);

        u_position.buffer(program, get_position());
    }
};

struct PointLights : public std::vector<PointLight> {
    void buffer(ShaderProgram& program) {
        uint32_t i = 0;
        for (auto light : *this) {
            std::ostringstream ss;
            ss << "[" << i << "]";
            light.uniform_prefix += ss.str();
            light.buffer(program);

            i++;
        }
    }
};
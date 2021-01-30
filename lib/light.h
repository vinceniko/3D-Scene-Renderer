#pragma once

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

struct LightTraits {
    glm::vec3 color_ = glm::vec3(0.5);

    LightTrait ambient_{ color_, 0.0 };
    LightTrait diffuse_{ color_, 0.0 };
    LightTrait specular_{ color_, 0.0 };

    float shininess_ = 1.f;

    LightTraits(float ambient, float diffuse, float specular, float shininess_factor) : LightTraits(glm::vec3(1.f), ambient, diffuse, specular, shininess_factor) {}
    LightTraits(glm::vec3 color, float ambient, float diffuse, float specular, float shininess_factor) : color_(color) {
        ambient_.strength_ = ambient;
        diffuse_.strength_ = diffuse;
        specular_.strength_ = specular;
        shininess_ = pow(2, shininess_factor);
    }
};

struct Light {
    LightTraits light_traits_;

    std::string uniform_prefix_;

    Uniform u_ambient_;
    Uniform u_diffuse_;
    Uniform u_specular_;
    Uniform u_shininess_;

    glm::mat4 projection_;
    Uniform u_light_vp_{ "u_light_vp" };

    Light(std::string&& kind, LightTraits light_traits, glm::mat4 projection = glm::mat4{ 1.f }) : uniform_prefix_(kind), light_traits_(light_traits), projection_(projection) {}
    void buffer() {
        u_ambient_.name_ = uniform_prefix_ + ".ambient";
        u_diffuse_.name_ = uniform_prefix_ + ".diffuse";
        u_specular_.name_ = uniform_prefix_ + ".specular";
        u_shininess_.name_ = uniform_prefix_ + ".shininess";

        u_ambient_.buffer(light_traits_.ambient_.get_trait());
        u_diffuse_.buffer(light_traits_.diffuse_.get_trait());
        u_specular_.buffer(light_traits_.specular_.get_trait());
        u_shininess_.buffer(light_traits_.shininess_);
    }
    void buffer_shadows(glm::mat4 light_vp) {
        u_light_vp_.buffer(light_vp);
    }
    void set_color(glm::vec3 color) {
        light_traits_.ambient_.color_ = color;
        light_traits_.diffuse_.color_ = color;
        light_traits_.specular_.color_ = color;
    }
    void set_strength(float strength) {
        light_traits_.ambient_.strength_ = strength;
        light_traits_.diffuse_.strength_ = strength;
        light_traits_.specular_.strength_ = strength;
    }
};

struct DirLight : public Light, public Spatial {
    Uniform u_direction_;

    DirLight() : DirLight(-glm::vec3(4.f, 4.f, 0.f), LightTraits{ glm::vec3(0.3f), 0.01f, 0.5f, 1.0f, 0 }) {}
    DirLight(glm::vec3 direction, LightTraits light_traits) : Light("dir_light", light_traits, glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.01f, 20.f)) {
        set_trans(glm::lookAt(-direction, direction, glm::vec3(0.f, 0.f, 1.f)));
    }
    void buffer() {
        Light::buffer();

        u_direction_.name_ = uniform_prefix_ + ".direction";

        u_direction_.buffer(look_direction());
    }
    void buffer_shadows() {
        glm::mat4 light_vp = projection_ * get_trans();
        Light::buffer_shadows(light_vp);
    }
};

struct Attenuation {
    float constant;
    float linear;
    float quadratic;
};

const Attenuation ATTENUATION_7 = Attenuation{ 1.0, 0.7, 1.8 };
const Attenuation ATTENUATION_20 = Attenuation{ 1.0, 0.35, 0.44 };
const Attenuation ATTENUATION_50 = Attenuation{ 1.0, 0.14, 0.07 };
const Attenuation ATTENUATION_100 = Attenuation{ 1.0, 0.045, 0.0075 };
const Attenuation ATTENUATION_200 = Attenuation{ 1.0, 0.022, 0.0019 };
const Attenuation ATTENUATION_600 = Attenuation{ 1.0, 0.007, 0.0002 };

struct PointLight : public Light, public MeshEntity {
    Attenuation attenuation_ = ATTENUATION_50;

    Uniform u_constant;
    Uniform u_linear;
    Uniform u_quadratic;

    Uniform u_position;

    PointLight() : PointLight(glm::vec3(0.f), LightTraits(glm::vec3(1.f), 0.1, 1.0, 1.0, 7)) {}
    PointLight(glm::vec3 position) : PointLight(position, LightTraits(glm::vec3(1.f), 0.1, 1.0, 1.0, 7)) {}
    PointLight(glm::vec3 position, LightTraits light_traits) : PointLight(position, light_traits, ATTENUATION_50) {}
    PointLight(glm::vec3 position, LightTraits light_traits, Attenuation attenuation) : PointLight(position, light_traits, attenuation, MeshFactory::get().get_mesh_entity(DefMeshList::SPHERE)) {}
    PointLight(glm::vec3 position, LightTraits light_traits, Attenuation attenuation, MeshEntity&& model) : Light("point_light", light_traits), attenuation_(attenuation), MeshEntity(std::move(model)) {
        scale(glm::mat4{ 1.f }, Spatial::ScaleDir::Out, 0.5);
        translate(glm::mat4{ 1.f }, position);
        MeshEntity::set_color(glm::vec3{ 1.f });
    }
    void buffer() {
        Light::buffer();

        u_constant.name_ = uniform_prefix_ + ".constant";
        u_linear.name_ = uniform_prefix_ + ".linear";
        u_quadratic.name_ = uniform_prefix_ + ".quadratic";

        u_position.name_ = uniform_prefix_ + ".position";

        u_constant.buffer(attenuation_.constant);
        u_linear.buffer(attenuation_.linear);
        u_quadratic.buffer(attenuation_.quadratic);

        u_position.buffer(get_origin());
    }
};

struct PointLights : public std::vector<std::shared_ptr<PointLight>> {
    using std::vector<std::shared_ptr<PointLight>>::vector;

    Uniform u_num_lights{"u_num_point_lights"};

    PointLights(PointLights&& point_lights) : vector(point_lights) {}
    void buffer() {
        uint32_t i = 0;
        for (auto& light_ptr : *this) {
            auto& light = *light_ptr;
            auto old_prefix = light.uniform_prefix_;

            std::ostringstream ss;
            ss << "s[" << i << "]";
            light.uniform_prefix_ += ss.str();

            light.buffer();
            light.uniform_prefix_ = old_prefix;
            
            i++;
        }
        u_num_lights.buffer(static_cast<int>(size()));
    }
    void draw() {
        for (auto& light : *this) {
            light->draw();
        }
    }
};

struct DebugShadows {
    Uniform u_debug{ "u_debug_shadows" };
    bool debug_ = false;

    void buffer() {
        try {
            u_debug.buffer(debug_);
        }
        catch (const std::runtime_error& e) {
#ifdef DEBUG
            std::cout << "Shadow Debug Error: " << e.what() << std::endl;
#endif
        }
    }
};
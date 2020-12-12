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

struct Light {
    glm::vec3 light_color = glm::vec3(0.5);

    LightTrait ambient_{ light_color, 0.0 };
    LightTrait diffuse_{ light_color, 0.0 };
    LightTrait specular_{ light_color, 0.0 };
    float shininess_ = pow(2, 7);

    std::string uniform_prefix_;

    Uniform u_ambient_;
    Uniform u_diffuse_;
    Uniform u_specular_;
    Uniform u_shininess_;

    Light(std::string&& kind) : uniform_prefix_(kind) {}
    void buffer(ShaderProgram& program) {
        u_ambient_.name_ = uniform_prefix_ + ".ambient";
        u_diffuse_.name_ = uniform_prefix_ + ".diffuse";
        u_specular_.name_ = uniform_prefix_ + ".specular";
        u_shininess_.name_ = uniform_prefix_ + ".shininess";

        u_ambient_.buffer(program, ambient_.get_trait());
        u_diffuse_.buffer(program, diffuse_.get_trait());
        u_specular_.buffer(program, specular_.get_trait());
        u_shininess_.buffer(program, shininess_);
    }
    void set_color(glm::vec3 color) {
        ambient_.color_ = color;
        diffuse_.color_ = color;
        specular_.color_ = color;
    }
    void set_strength(float strength) {
        ambient_.strength_ = strength;
        diffuse_.strength_ = strength;
        specular_.strength_ = strength;
    }
};

struct DirLight : public Light, public Spatial {
    Uniform u_direction_;

    DirLight() : DirLight(glm::vec3(0.f, -1.f, -1.f), glm::vec3(1.f, 1.f, 1.f)) {}
    DirLight(glm::vec3 direction, glm::vec3 color) : Light("dir_light") {
        set_color(color);
        ambient_.strength_ = 0.2;
        diffuse_.strength_ = 0.2;
        specular_.strength_ = 0.2;
        shininess_ = pow(2, 3);

        set_trans(glm::lookAt(glm::vec3(0.f), direction, glm::vec3(0.f, 0.f, 1.f)));
    }
    void buffer(ShaderProgram& program) {
        Light::buffer(program);
        
        u_direction_.name_ = uniform_prefix_ + ".direction";
        
        u_direction_.buffer(program, look_direction());
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

struct PointLight : public Light {
    MeshEntity model;

    Attenuation attenuation = ATTENUATION_50;

    Uniform u_constant;
    Uniform u_linear;
    Uniform u_quadratic;

    Uniform u_position;

    PointLight() : Light("point_light"), model(MeshFactory::get().get_mesh_entity(DefMeshList::CUBE)) {
        ambient_.strength_ = 0.2;
        diffuse_.strength_ = 1.0;
        specular_.strength_ = 0.75;
        
        model.scale(glm::mat4{ 1.f }, Spatial::ScaleDir::Out, 1.5);
        model.set_color(glm::vec3{ 1.f });
    }
    PointLight(glm::vec3 position) : PointLight() {
        model.translate(glm::mat4{ 1.f }, position);
    }
    void buffer(ShaderProgram& program) {
        Light::buffer(program);
        
        u_constant.name_ = uniform_prefix_ + ".constant";
        u_linear.name_ = uniform_prefix_ + ".linear";
        u_quadratic.name_ = uniform_prefix_ + ".quadratic";

        u_position.name_ = uniform_prefix_ + ".position";

        u_constant.buffer(program, attenuation.constant);
        u_linear.buffer(program, attenuation.linear);
        u_quadratic.buffer(program, attenuation.quadratic);

        u_position.buffer(program, model.get_origin());
    }
    void draw(ShaderProgram& program) {
        model.draw(program);
    }
};

struct PointLights : public std::vector<PointLight> {
    using std::vector<PointLight>::vector;
    void buffer(ShaderProgram& program) {
        uint32_t i = 0;
        for (auto light : *this) {
            std::ostringstream ss;
            ss << "s[" << i << "]";
            light.uniform_prefix_ += ss.str();
            light.buffer(program);

            i++;
        }
    }
    void draw(ShaderProgram& program) {
        for (auto light : *this) { 
            light.draw(program);
        }
    }
};
#ifndef __MODULE5_SHADER_SRC_HPP__
#define __MODULE5_SHADER_SRC_HPP__

const char *simple_light_vert =
    R"(
#version 410 core
layout (location = 0) in vec3 vtx_position;
layout (location = 1) in vec3 vtx_normal;
layout (location = 0) smooth out vec4 color;

uniform vec3 material_color;
uniform mat4 pvm_matrix;
uniform mat4 model_matrix;
uniform mat4 normal_matrix;

void main() 
{
    vec3 light_position = vec3(0.0, -100.0, 50.0f);
    vec3 N = normalize(vec3(normal_matrix * vec4(vtx_normal, 0.0)));
    vec4 v = model_matrix * vec4(vtx_position, 1.0);
    vec3 L = normalize(vec3(light_position - vec3(v)));
    color = vec4(material_color * max(dot(L, N), 0.0), 1.0);
    gl_Position = pvm_matrix * vec4(vtx_position, 1.0);
}
)";

const char *simple_light_frag =
    R"(
#version 410 core
layout (location = 0) smooth in vec4 color;
layout (location = 0) out vec4 fragColor;
void main() 
{
    fragColor = color;
}
)";

#endif

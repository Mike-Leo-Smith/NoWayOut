#version 410 core

#define M_PI        3.14159265358979323846264338327950288f   /* pi             */
#define M_PI_2      1.57079632679489661923132169163975144f   /* pi/2           */
#define M_PI_4      0.785398163397448309615660845819875721f  /* pi/4           */
#define M_1_PI      0.318309886183790671537767526745028724f  /* 1/pi           */
#define M_2_PI      0.636619772367581343075535053490057448f  /* 2/pi           */

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

layout (location = 0) out vec3 FragColor;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;
uniform int has_diffuse_texture;
uniform int has_specular_texture;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float roughness;
uniform vec3 camera_position;

void main() {
    vec3 V = normalize(camera_position - Position);
    vec3 N = normalize(Normal);
    vec3 Kd = has_diffuse_texture == 0 ? diffuse_color : pow(texture(diffuse_texture, TexCoord).rgb, vec3(2.2f));
    vec3 Ks = has_specular_texture == 0 ? specular_color : pow(texture(specular_texture, TexCoord).rgb, vec3(2.2f));
    FragColor = pow(Kd, vec3(1.0f / 2.2f));
}

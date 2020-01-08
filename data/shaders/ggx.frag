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
uniform vec3 light_direction;
uniform vec3 light_emission;

float DistributionGGX(vec3 m, vec3 n, float alpha)
{
    float cos_theta_m = dot(m, n);

    if (cos_theta_m <= 0.0f) {
        return 0.0f;
    }
    float root = alpha / (cos_theta_m * cos_theta_m * (alpha * alpha - 1.0f) + 1.0f);

    return root * root * M_1_PI;
}

float G1(vec3 v, vec3 m, vec3 n, float alpha) {

    float v_dot_n = dot(v, n);
    float v_dot_m = dot(v, m);

    if (v_dot_n * v_dot_m <= 0.0f) {
        return 0.0f;
    }

    float sqr_cos_theta_v = max(abs(v_dot_n * v_dot_n), 0.0001f);
    float sqr_tan_theta_v = (1.0f - sqr_cos_theta_v) / sqr_cos_theta_v;

    return 2.0f / (1.0f + sqrt(1.0f + alpha * alpha * sqr_tan_theta_v));
}

float Geo(vec3 i, vec3 o, vec3 m, vec3 n, float alpha) {
    return G1(i, m, n, alpha) * G1(o, m, n, alpha);
}

float sqr(float x) {
    return x * x;
}

float Fresnel(float HdotV, float eta) {
    float c = HdotV;
    float g = sqrt(eta * eta - 1.0f + c * c);
    return 0.5f * sqr(g - c) / sqr(g + c) * (1.0f + sqr(c * (g + c) - 1.0f) / sqr(c * (g - c) - 1.0f));
}

void main() {
    vec3 L = normalize(light_direction);
    vec3 V = normalize(camera_position - Position);
    vec3 N = normalize(Normal);
    vec3 Kd = has_diffuse_texture == 0 ? diffuse_color : pow(texture(diffuse_texture, TexCoord).rgb, vec3(2.2f));
    vec3 Ks = has_specular_texture == 0 ? specular_color : pow(texture(specular_texture, TexCoord).rgb, vec3(2.2f));
    float Alpha = min(max(roughness, 0.01f), 1.0f);

    float NdotL = dot(N, L);
    float NdotV = dot(N, V);

    vec3 Li = light_emission;
    vec3 Lo = vec3(0.0);
    if (NdotL > 0.0f) {
        vec3 specular = vec3(0.0f);
        float specular_strength = max(max(Ks.r, Ks.g), Ks.b);
        if (specular_strength > 0.0f) {
            vec3 H = normalize(V + L);
            float D = DistributionGGX(H, N, Alpha);
            float G = Geo(V, L, H, N, Alpha);
            float F = Fresnel(dot(H, V), 1.5f);
            float nominator = D * G * F;
            float denominator = 4.0f * abs(NdotV);
            specular = Ks * nominator / max(denominator, 0.001f);
        }
        vec3 diffuse = (1.0f - specular_strength) * Kd * abs(NdotL) * M_1_PI;
//        float shadow = ShadowCalculation();
        Lo = (diffuse + specular) * Li;
    }
    Lo += 0.15f * Kd;  // ambient
    FragColor = pow(Lo, vec3(1.0f / 2.2f));
    FragColor = N * 0.5f + 0.5f;
}

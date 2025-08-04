#version 460

#extension GL_EXT_debug_printf: enable

// Simple fragment shader for animated model demo; for now just ultra basic gooch-style shading

/* INPUTS */
layout (location = 0) in vec4 vPos;
layout (location = 1) in vec4 vNormal;

struct Light {
    vec4 position;
    vec4 color;
};

#define MAX_LIGHTS 10
layout (binding = 3) uniform LightingData {
    vec3 modelUnlitColor;
    vec3 warmColor;
    vec3 coolColor;

    uint lightCount;
    Light lights[MAX_LIGHTS];
};

layout (binding = 4) uniform CameraData {
    vec3 cameraPosition;
};

/* OUTPUTS */
layout (location = 0) out vec4 outColor;

vec3 lit(vec3 l, vec3 n, vec3 v) {
    vec3 r_l = reflect(-l, n);
    float s = clamp(100.0 * dot(r_l, v) - 97.0, 0.0, 1.0);

    vec3 highlightColor = vec3(2, 2, 2);
    return mix(warmColor, highlightColor, s);
}

void main() {
    vec3 view = normalize(cameraPosition - vPos.xyz);
    outColor = vec4(modelUnlitColor, 1.0);

    for (uint i = 0u; i < lightCount; i++) {
        vec3 light = normalize(lights[i].position.xyz - vPos.xyz);
        float normalDotLight = clamp(dot(vNormal.xyz, light), 0.0, 1.0);
        outColor.rgb += normalDotLight * lights[i].color.rgb * lit(light, vNormal.xyz, view);
    }
}
#version 460 core

in vec3 vNormal;
in vec3 vWorldPos;
in vec2 vUV;
in mat3 TBN;

out vec4 FragColor;

uniform vec3 uCamPos;
uniform vec3 uLightDir = vec3(1.0,1.0,1.0);
uniform vec3 uBaseColor = vec3(0.75, 0.75, 0.85);

uniform vec3 uKa;         // ambient (material)
uniform vec3 uKd;         // diffuse (material)
uniform vec3 uKs;         // specular (material)
uniform float uShininess; // Ns
uniform float uOpacity;   // alpha
uniform bool matToggle;

uniform vec3 uAmbientLight = vec3(1.0,1.0,1.0);

uniform sampler2D uNormalMap;
uniform bool uHasNormalMap;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uCamPos - vWorldPos);
    vec3 H = normalize(L + V);

    vec3 Nw;
    if(uHasNormalMap){
        vec3 nTs = texture(uNormalMap, vUV).rgb * 2.0 - 1.0;
        Nw = normalize(TBN * nTs);
    }else{
        Nw = normalize(TBN[2]);
    }

    //Blinn-Phong
    float diff = max(dot(N, L), 0.0);
    float spec = (diff > 0.0) ? pow(max(dot(N, H), 0.0), uShininess) : 0.0;

    vec3 ambient = uKa * uAmbientLight;
    vec3 diffuse = diff * uKd;
    vec3 specular = spec * uKs;
    vec3 color = ambient + diffuse + specular;
    if(matToggle){
        color = uBaseColor * (0.15 + 0.85 * diff);
    }
    

    FragColor = vec4(color, uOpacity);
}
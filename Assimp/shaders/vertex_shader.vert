#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat3 uNormalMat;

out vec3 vNormal;
out vec3 vWorldPos;
out vec2 vUV;
out mat3 TBN;

void main()
{   
    vec3 N = normalize(uNormalMat * aNormal);
    vec3 T = normalize(uNormalMat * aTangent);
    vec3 B = normalize(uNormalMat * aBitangent);

    T = normalize(T - dot(N,T) * N);
    B = normalize(cross(N, T));

    TBN = mat3(T,B,N);
    vUV = aUV;

    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = normalize(uNormalMat * aNormal);

    gl_Position = uProj * uView * worldPos;
}
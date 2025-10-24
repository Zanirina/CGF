#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform vec3 uLight1Pos;
uniform vec3 uLight2Pos;
uniform vec3 uLightAmbient;
uniform vec3 uLightDiffuse;
uniform vec3 uLightSpecular;
uniform vec3 uMatAmbient;
uniform vec3 uMatDiffuse;
uniform vec3 uMatSpecular;
uniform float uMatShininess;
uniform vec3 uViewPos;

out vec3 vertexColor;

void main() {
    // compute world-space
    vec3 fragPos = vec3(uModel * vec4(aPos,1.0));
    vec3 normal = normalize(mat3(transpose(inverse(uModel))) * aNormal);

    // light1
    vec3 L1 = normalize(uLight1Pos - fragPos);
    float diff1 = max(dot(normal, L1), 0.0);
    vec3 R1 = reflect(-L1, normal);
    float spec1 = pow(max(dot(normalize(uViewPos - fragPos), R1), 0.0), uMatShininess);

    // light2
    vec3 L2 = normalize(uLight2Pos - fragPos);
    float diff2 = max(dot(normal, L2), 0.0);
    vec3 R2 = reflect(-L2, normal);
    float spec2 = pow(max(dot(normalize(uViewPos - fragPos), R2), 0.0), uMatShininess);

    vec3 ambient = uLightAmbient * uMatAmbient;
    vec3 diffuse = (diff1 + diff2) * uLightDiffuse * uMatDiffuse;
    vec3 specular = (spec1 + spec2) * uLightSpecular * uMatSpecular;

    vertexColor = ambient + diffuse + specular;
    gl_Position = uMVP * vec4(aPos, 1.0);
}

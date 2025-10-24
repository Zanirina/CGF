#version 330 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

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

void main() {
    vec3 norm = normalize(Normal);

    // light1
    vec3 L1 = normalize(uLight1Pos - FragPos);
    float diff1 = max(dot(norm, L1), 0.0);
    vec3 R1 = reflect(-L1, norm);
    float spec1 = pow(max(dot(normalize(uViewPos - FragPos), R1), 0.0), uMatShininess);

    // light2
    vec3 L2 = normalize(uLight2Pos - FragPos);
    float diff2 = max(dot(norm, L2), 0.0);
    vec3 R2 = reflect(-L2, norm);
    float spec2 = pow(max(dot(normalize(uViewPos - FragPos), R2), 0.0), uMatShininess);

    vec3 ambient = uLightAmbient * uMatAmbient;
    vec3 diffuse = (diff1 + diff2) * uLightDiffuse * uMatDiffuse;
    vec3 specular = (spec1 + spec2) * uLightSpecular * uMatSpecular;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}

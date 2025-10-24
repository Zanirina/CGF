#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform vec3 uLightPos;
uniform vec3 uViewPos;

out vec3 vertexColor;

void main() {
    vec3 fragPos = vec3(uModel * vec4(aPos, 1.0));
    vec3 normal = normalize(mat3(transpose(inverse(uModel))) * aNormal);
    vec3 lightDir = normalize(uLightPos - fragPos);
    vec3 viewDir = normalize(uViewPos - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);

    vec3 color = vec3(0.2) + diff * vec3(0.7) + spec * vec3(1.0);
    vertexColor = color;
    gl_Position = uMVP * vec4(aPos, 1.0);
}

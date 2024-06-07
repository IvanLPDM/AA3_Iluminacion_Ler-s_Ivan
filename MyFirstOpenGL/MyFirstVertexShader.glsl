#version 440 core

layout(location = 0) in vec3 posicion;
layout(location = 1) in vec2 uvsVertexShader;

out vec2 uvsGeometryShader;

uniform mat4 translationMatrix;
uniform mat4 rotationMatrix;
uniform mat4 scaleMatrix;

void main() {

    uvsGeometryShader = uvsVertexShader;

    mat4 model = translationMatrix * rotationMatrix * scaleMatrix;

    gl_Position = model * vec4(posicion, 1.0);
}
#version 440 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 uvsGeometryShader[];
in vec3 normalsGeometryShader[];

out vec2 uvsFragmentShader;
out vec3 normalsFragmentShader;
out vec4 primitivePosition;

uniform mat4 translationMatrix;
uniform mat4 rotationMatrix;
uniform mat4 scaleMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(){

mat4 model = translationMatrix * rotationMatrix * scaleMatrix;

	for(int i = 0; i < gl_in.length(); i++){
		gl_Position = projectionMatrix * viewMatrix * gl_in[i].gl_Position;
		uvsFragmentShader = uvsGeometryShader[i];
		normalsFragmentShader = normalsGeometryShader[i];
		EmitVertex();
	}

	primitivePosition = model * ((gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) * 0.33);

	EndPrimitive();
}
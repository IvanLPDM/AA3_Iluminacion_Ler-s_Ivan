#version 440 core

uniform vec2 windowSize;
uniform sampler2D textureSampler;
uniform vec3 color;

in vec2 uvsFragmentShader;
in vec3 normalsFragmentShader;
in vec4 primitivePosition;

out vec4 fragColor;

void main() {

        vec2 adjustedTexCoord = vec2(uvsFragmentShader.x, 1.0 - uvsFragmentShader.y);
        vec4 baseColor = texture(textureSampler, adjustedTexCoord);

        vec3 sourceLight = vec3( 0, 0, 1);
        vec3 lightDirection = normalize(sourceLight - primitivePosition.xyz);

        float sourceLightAngle = dot(normalsFragmentShader, lightDirection);

        //vec4 ambientColor = vec4(0.5, 1.0, 0.5, 1.0);

        baseColor = vec4(baseColor.r * color.x, baseColor.g * color.y, baseColor.b * color.z, baseColor.a);

        //fragColor = baseColor * ambientColor;

        fragColor = vec4(baseColor.rgb * sourceLightAngle, 1.0);
}
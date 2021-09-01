#version 330 core
out vec4 FragColor;

in vec4 projectionPos;
in vec2 texcoord;
in vec3 worldFragPos;

#pragma include light.glsl
#pragma include fog.glsl

uniform sampler2D refraction;
uniform sampler2D dudvMap;
uniform sampler2D reflection;
uniform sampler2D normalMap;

uniform float time;
uniform vec3 viewPos;

uniform Light sun;
uniform Fog fog;

const float DISTORTION_SCALE = 0.01;
const float WAVE_SCALE = 0.03;

void main()
{
    FragColor = texture(refraction, texcoord);
}
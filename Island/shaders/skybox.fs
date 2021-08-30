#version 330 core
out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube skyboxDiurnal;
uniform samplerCube skyboxNocturnal;

uniform float coeDiurnal;

void main()
{
    vec3 color = mix(texture(skyboxNocturnal, texCoords), texture(skyboxDiurnal, texCoords), coeDiurnal).rgb;
    color.x = pow(color.x, 1.0 / 2.2);
    color.y = pow(color.y, 1.0 / 2.2);
    color.z = pow(color.z, 1.0 / 2.2);

    FragColor = vec4(color, 1.0);
}
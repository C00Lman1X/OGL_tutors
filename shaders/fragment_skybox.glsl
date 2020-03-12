#version 330 core
out vec4 FragColor;
in vec3 TexCoords;

uniform samplerCube skybox;

uniform bool evening;

void main()
{
    FragColor = texture(skybox, TexCoords);
    
    if(evening)
    {
        vec4 second_layer = FragColor;
        second_layer.r = second_layer.r * 1.5;
        second_layer.b = second_layer.b * 0.5;

        FragColor = FragColor * second_layer;
    }
}
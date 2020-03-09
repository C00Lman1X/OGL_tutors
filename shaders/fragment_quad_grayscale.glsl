#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec4 srcColor = texture(screenTexture, TexCoords);
    float average = 0.2126 * srcColor.r + 0.7152 * srcColor.g + 0.0722 * srcColor.b;
    FragColor = vec4(average, average, average, 1.0);
}
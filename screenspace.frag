#version 420
in vec2 v2fTexCoord;
out vec3 oColor;
layout(binding = 0) uniform sampler2D raytracerImage;
void main()
{
    oColor = texture( raytracerImage, v2fTexCoord ).rgb;
}
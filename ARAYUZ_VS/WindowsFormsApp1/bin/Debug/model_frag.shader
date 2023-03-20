#version 330 core
in vec2 texCoord;

out vec4 color;

uniform sampler2D texture_diffuse1;

void main(){
    vec2 t = texCoord;
    t.y = 1 - t.y;
    t.x = 1 - t.x;

    color =  texture(texture_diffuse1, t);
}
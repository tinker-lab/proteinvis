#version 400

uniform sampler2D textureSampler;
in vec2 texture_coordinates;
out vec4 fragment_colour; // final colour of surface

void main () {

	fragment_colour = texture( textureSampler, texture_coordinates );
	
}
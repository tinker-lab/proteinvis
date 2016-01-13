#version 400

in vec2 texture_coordinates;
in vec4 colorVal;
uniform bool has_lambertian_texture;
uniform sampler2D lambertian_texture;

out vec4 fragment_colour; // final colour of surface

void main () {

	vec4 lambertianColor = colorVal;
	if (has_lambertian_texture) {
		lambertianColor.a = texture2D(lambertian_texture, texture_coordinates).a;
	}
	fragment_colour = lambertianColor;
		
}
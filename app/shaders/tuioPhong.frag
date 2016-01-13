#version 400

in vec3 position_world, normal_world;
in vec2 texture_coordinates;
uniform vec3 eye_world;

// fixed point light properties
vec3 light_position_world  = vec3 (0.0, 1.0, 4.0);
vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
vec3 Ld = vec3 (0.7, 0.7, 0.7); // dull white diffuse light colour
vec3 La = vec3 (0.2, 0.2, 0.2); // grey ambient colour
  
// surface reflectance
vec3 Ks = vec3 (1.0, 1.0, 1.0); // fully reflect specular light
vec3 Kd = vec3 (1.0, 0.5, 0.0); // orange diffuse surface reflectance
vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light
float specular_exponent = 100.0; // specular 'power'

uniform sampler2D koalaTextureSampler;


out vec4 fragment_colour; // final colour of surface

void main () {
	// ambient intensity
	vec3 Ia = La * Ka;

	// diffuse intensity
	// raise light position to eye space
	vec3 distance_to_light_world = light_position_world - position_world;
	vec3 direction_to_light_world = normalize (distance_to_light_world);
	float dot_prod = dot (direction_to_light_world, normalize(normal_world));
	dot_prod = max (dot_prod, 0.0);
	vec3 Id = Ld * Kd * dot_prod; // final diffuse intensity
	
	// specular intensity
	vec3 surface_to_viewer_world = normalize (eye_world - position_world);
	vec3 half_way_world = normalize (surface_to_viewer_world + direction_to_light_world);
	float dot_prod_specular = dot (half_way_world, normal_world);
	float specular_factor = pow (dot_prod_specular, specular_exponent);
	
	vec3 Is = Ls * Ks * specular_factor; // final specular intensity
		
	// final colour
	fragment_colour = vec4 (Is + Id + Ia, 1.0);

	fragment_colour = vec4(texture( koalaTextureSampler, texture_coordinates ).rgb, 1.0 );
	
}
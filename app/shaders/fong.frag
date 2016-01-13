#version 400

in vec3 position_world, normal_world;
uniform vec3 eye_world;

// fixed point light properties
vec3 light_position_world1  = vec3 (0.5, 4.0, 2.0);
vec3 light_position_world2  = vec3 (-0.5, 4.0, 2.0);
vec3 Ls = vec3 (0.7, 0.7, 0.7); // white specular colour
vec3 Ld = vec3 (0.5, 0.5, 0.5); // dull white diffuse light colour
vec3 La = vec3 (0.1, 0.1, 0.1); // grey ambient colour
  
// surface reflectance
vec3 Ks = vec3 (1.0, 1.0, 1.0); // fully reflect specular light
vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light
float specular_exponent = 25.0; // specular 'power'

uniform sampler2D textureSampler;
in vec2 texture_coordinates;

out vec4 fragment_colour; // final colour of surface


vec3 computeLightContribution(vec3 lightPosition, vec4 Kd, vec3 surface_to_viewer_world) {
	// ambient intensity
	vec3 Ia = La * Ka;

	vec3 distance_to_light_world = lightPosition - position_world;
	vec3 direction_to_light_world = normalize (distance_to_light_world);
	float ndotl = dot (normalize(normal_world), direction_to_light_world);
	ndotl = max (ndotl, 0.0);

	vec3 Id = Ld * Kd.rgb * ndotl; // final diffuse intensity

	// specular intensity
	vec3 half_way_world = normalize (surface_to_viewer_world + direction_to_light_world);
	float ldoth = max(dot (half_way_world, normal_world), 0.0);
	float specular_factor = pow (ldoth, specular_exponent);
	
	vec3 Is = Ls * Ks * specular_factor; // final specular intensity

	return Ia+Id+Is;
}


void main () {
	
	vec4 Kd = texture(textureSampler, texture_coordinates ); 
	vec3 surface_to_viewer_world = normalize (eye_world - position_world);

	vec3 light1_contribution = computeLightContribution(light_position_world1, Kd, surface_to_viewer_world);
	vec3 light2_contribution = computeLightContribution(light_position_world2, Kd, surface_to_viewer_world);

	
	// final colour
	fragment_colour = vec4 (light1_contribution + light2_contribution , Kd.a);
	
}
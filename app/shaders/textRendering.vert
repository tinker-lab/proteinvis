#version 400

in layout (location = 0) vec2 vertex_position;
in layout (location = 1) vec2 vertex_texcoord;
in layout (location = 2) vec4 vertex_color;
uniform mat4 projection_mat, view_mat, model_mat;
out vec2 texture_coordinates;
out vec4 colorVal;

void main () {
	vec3 position_world = vec3 (model_mat * vec4 (vertex_position, 0.0, 1.0));
	texture_coordinates = vertex_texcoord;
	colorVal = vertex_color;
	gl_Position = projection_mat * view_mat * vec4 (position_world, 1.0);
}
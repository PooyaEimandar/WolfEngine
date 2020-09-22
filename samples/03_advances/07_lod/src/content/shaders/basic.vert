#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec3 i_norm;
layout(location = 2) in vec2 i_uv;

layout(binding = 0) uniform U0
{
	mat4	world;
	mat4	view;
	mat4	projection;
	vec4	camera_pos;//w is padding
} u0;

layout(binding = 1) uniform U1
{
	uint	texture_max_mip_maps_max_level;
	float	bounding_sphere_radius;
	float	padding_0;
	float	padding_1;
} u1;

out gl_PerVertex
{
  vec4 gl_Position;
};

layout (location = 0) out vec3 o_norm;
layout (location = 1) out vec2 o_uv;
layout (location = 2) out flat uint o_texture_mip_map_level;
layout (location = 3) out vec3 o_color;
layout (location = 4) out vec3 o_camera_pos;
layout (location = 5) out vec3 o_world_pos;

void main() 
{
	vec4 _pos = vec4(i_pos, 1.0);
	vec4 _world_pos = u0.world * _pos;
	mat4 _world_view = u0.view * u0.world;

	gl_Position = u0.projection * u0.view * _world_pos;
	o_norm =  normalize( ( vec4(i_norm, 0.0)  * _world_view ).xyz );
	o_uv = i_uv;
	o_camera_pos = u0.camera_pos.xyz;
	o_world_pos = _world_pos.xyz;

	//set texture mip map based on position
	o_texture_mip_map_level = u1.texture_max_mip_maps_max_level - 1;
	float _distance_from_cam = distance(u0.camera_pos, vec4(u0.world[3][0], u0.world[3][1], u0.world[3][2], 1.0));
	float _bs = u1.bounding_sphere_radius * 10; 
	for(int i = 0; i < u1.texture_max_mip_maps_max_level; ++i)
	{
		if (_distance_from_cam <= _bs * (i + 1))
		{
			o_texture_mip_map_level = i;
			break;
		}
	}

	if (gl_InstanceIndex == 0)
	{
		//this is ref model
		o_color = vec3(1.0, 0.0, 0.0);
	}
	else
	{
		//this is instance model
		o_color = vec3(0.0, 1.0, 0.0);
	}
}

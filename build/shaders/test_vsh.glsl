#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( row_major ) uniform;

layout( binding = 0 ) uniform material_uniforms
{
    mat4 world_matrix;
    mat4 view_matrix;
    mat4 projection_matrix;
}
uniforms;

layout( location = 0 ) in vec3 vertex_position;
layout( location = 1 ) in vec3 vertex_normal;
layout( location = 2 ) in vec4 vertex_color;
layout( location = 3 ) in vec4 vertex_uv;

layout( location = 0 ) out vec4 out_color;

void main()
{
    // just pass the color along
    vec4 position = vec4( vertex_position.xyz, 1.0 );
    // position.z    = 0;

    position = uniforms.world_matrix * position;
    // position = uniforms.view_matrix * position;
    position = uniforms.projection_matrix * position;

    gl_Position = position;

    // out_color   = vec4( vertex_position.xy * 4.0 - 1.0, 1.0, 1.0 );
    out_color = vertex_color;
}
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec3 vertex_position;
layout( location = 1 ) in vec3 vertex_normal;
layout( location = 2 ) in vec4 vertex_color;
layout( location = 3 ) in vec4 vertex_uv;

layout( location = 0 ) out vec4 out_color;

void main()
{
    // just pass the color along
    gl_Position = vec4( vertex_position.xy, 0.0, 1.0 );
    out_color   = vec4( vertex_position.xy * 4.0 - 1.0, 1.0, 1.0 );
}
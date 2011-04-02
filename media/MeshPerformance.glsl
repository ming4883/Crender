-- Scene.Vertex
in vec4 i_vertex;
in vec2 i_texcoord0;

out vec2 v_texcoord;

uniform mat4 u_worldViewProjMtx;

void main() {
	gl_Position = u_worldViewProjMtx * i_vertex;
	v_texcoord = i_texcoord0;
}

-- Scene.Fragment
in vec2 v_texcoord;

out vec4 o_fragColor;

uniform sampler2D u_tex;

void main() {
	
	vec4 color = texture(u_tex, v_texcoord);	
	o_fragColor = color;
}

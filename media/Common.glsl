-- Bg.Vertex
in vec4 i_vertex;
in vec2 i_texcoord0;

out vec2 v_texcoord;

void main() {
	gl_Position = i_vertex;
	v_texcoord = i_texcoord0;
}

-- Bg.Fragment
in vec2 v_texcoord;

out vec4 o_fragColor;
uniform vec4 u_colors[4];

void main() {
	vec4 c0 = mix(u_colors[0], u_colors[1], v_texcoord.x);
	vec4 c1 = mix(u_colors[2], u_colors[3], v_texcoord.x);
	o_fragColor = mix(c0, c1, v_texcoord.y);
}

-- Bg.Vertex.20
attribute vec4 i_vertex;
attribute vec2 i_texcoord0;

varying vec2 v_texcoord;

void main() {
	gl_Position = i_vertex;
	v_texcoord = i_texcoord0;
}

-- Bg.Fragment.20
varying vec2 v_texcoord;

uniform vec4 u_colors[4];

void main() {
	vec4 c0 = mix(u_colors[0], u_colors[1], v_texcoord.x);
	vec4 c1 = mix(u_colors[2], u_colors[3], v_texcoord.x);
	gl_FragColor = mix(c0, c1, v_texcoord.y);
}

-- Ui.Vertex
in vec4 i_vertex;
in vec2 i_texcoord0;

out vec2 v_texcoord;

void main() {
	gl_Position = i_vertex;
	v_texcoord = i_texcoord0;
}

-- Ui.Fragment
in vec2 v_texcoord;

out vec4 o_fragColor;

uniform sampler2D u_tex;

void main() {
	o_fragColor = texture(u_tex, v_texcoord);
}

-- Text.Fragment
in vec2 v_texcoord;

out vec4 o_fragColor;

uniform sampler2D u_tex;
uniform vec4 u_textColor;

void main() {
	o_fragColor = vec4(u_textColor.xyz, u_textColor.w * texture(u_tex, v_texcoord).x);
}

-- Scene.Vertex
attribute vec4 i_vertex;
attribute vec3 i_normal;
attribute vec2 i_texcoord0;

varying vec2 v_texcoord;

uniform mat4 u_worldMtx;
uniform mat4 u_worldViewMtx;
uniform mat4 u_worldViewProjMtx;
uniform mat4 u_textureMtx;

void main() {
	gl_Position = u_worldViewProjMtx * i_vertex;
	v_texcoord = (u_textureMtx * vec4(i_texcoord0, 1, 1)).xy;
}

-- Scene.Fragment
precision mediump float;

varying vec2 v_texcoord;

uniform sampler2D u_tex;

void main() {
	vec4 tex = texture2D(u_tex, v_texcoord);
	gl_FragColor = tex;
}

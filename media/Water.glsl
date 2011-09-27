-- Scene.Vertex
attribute vec4 i_vertex;
attribute vec3 i_normal;
attribute vec2 i_texcoord0;

varying vec3 v_normal;
varying vec3 v_pos;
varying vec2 v_texcoord;
varying vec4 v_refractionMap;

uniform mat4 u_worldMtx;
uniform mat4 u_worldViewMtx;
uniform mat4 u_worldViewProjMtx;
uniform mat4 u_refractionMapTexMtx;

void main() {
	gl_Position = u_worldViewProjMtx * i_vertex;
	v_normal = (u_worldMtx * vec4(i_normal, 0)).xyz;
	v_pos = (u_worldMtx * i_vertex).xyz;
	v_texcoord = i_texcoord0;
	v_refractionMap = u_refractionMapTexMtx * vec4(v_pos, 1);
}

-- Scene.Fragment
varying vec3 v_normal;
varying vec3 v_pos;
varying vec2 v_texcoord;
varying vec4 v_refractionMap;

uniform vec4 u_matDiffuse;
uniform vec4 u_matSpecular;
uniform float u_matShininess;
uniform vec4 u_refractionMapParam;

uniform sampler2D u_tex;

void main() {
	vec3 n = normalize(v_normal.xyz);
	vec3 l = normalize(vec3(0,10,10) - v_pos.xyz);
	vec3 h = normalize(l + vec3(0, 0, 1));
	
	if(false == gl_FrontFacing)
		n *= -1;
		
	vec3 sm = v_refractionMap.xyz / v_refractionMap.www;
	
	float ndl = max(0, dot(n, l)) * 0.8 + 0.2;
	float ndh = max(0, dot(n, h));
	ndh = pow(ndh, u_matShininess);
	
	vec4 color = u_matDiffuse * texture2D(u_tex, v_texcoord);
	color.xyz *= ndl;
	color.xyz += u_matSpecular.xyz * ndh;
	
	gl_FragColor = color;
}

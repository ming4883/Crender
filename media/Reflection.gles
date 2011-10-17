
-- Scene.Vertex
attribute vec4 i_vertex;
attribute vec3 i_normal;
attribute vec2 i_texcoord0;

//varying vec3 v_normal;
//varying vec3 v_pos;
varying vec2 v_texcoord;

uniform mat4 u_worldMtx;
uniform mat4 u_worldViewMtx;
uniform mat4 u_worldViewProjMtx;

void main() {
	gl_Position = u_worldViewProjMtx * i_vertex;
	//v_normal = (u_worldMtx * vec4(i_normal, 0)).xyz;
	//v_pos = (u_worldMtx * i_vertex).xyz;
	v_texcoord = i_texcoord0;
}

-- Scene.Fragment
precision mediump float;

//varying vec3 v_normal;
//varying vec3 v_pos;
varying vec2 v_texcoord;

uniform vec3 u_camPos;
uniform vec4 u_matDiffuse;
uniform vec4 u_matSpecular;
uniform float u_matShininess;

uniform sampler2D u_tex;

void main() {
	vec4 tex = texture2D(u_tex, v_texcoord);
	gl_FragColor = tex;
}

-- SceneWater.Vertex
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

void main() {
	gl_Position = u_worldViewProjMtx * i_vertex;
	v_normal = (u_worldMtx * vec4(i_normal, 0)).xyz;
	v_pos = (u_worldMtx * i_vertex).xyz;
	v_texcoord = i_texcoord0;
	v_refractionMap = (u_worldViewProjMtx * i_vertex);
}

-- SceneWater.Fragment
precision mediump float;

varying vec3 v_normal;
varying vec3 v_pos;
varying vec2 v_texcoord;
varying vec4 v_refractionMap;

uniform vec4 u_matDiffuse;
uniform vec4 u_matSpecular;
uniform float u_matShininess;
uniform vec4 u_refractionMapParam;
uniform vec3 u_camPos;

uniform sampler2D u_refract;
uniform sampler2D u_water;
uniform sampler2D u_flow;

void main() {
	
	vec2 flow = texture2D(u_flow, v_texcoord).xy * 2.0 - 1.0;
	float halfcycle = u_refractionMapParam.y;
	float phase0 = u_refractionMapParam.z;
	float phase1 = u_refractionMapParam.w;
	float texscale = 1.0;
	
	vec4 normal0 = texture2D(u_water, (v_texcoord * texscale) + flow * phase0);
	vec4 normal1 = texture2D(u_water, (v_texcoord * texscale) + flow * phase1);
	float flowLerp = (abs(halfcycle - phase0) / halfcycle);
	
	vec4 water = mix(normal0, normal1, flowLerp);
	water.xyz = water.xyz * 2.0 - 1.0;
	vec3 norm = normalize( vec3(water.x, 0.5, water.y) );
	
	vec2 refracoord = (v_refractionMap.xy / v_refractionMap.ww) * 0.5 + 0.5;
	refracoord += norm.xz * u_refractionMapParam.xx;
	
	vec4 refle = texture2D(u_refract, refracoord);
	
	float d = dot(norm, normalize(u_camPos - v_pos));
	d = max(d, 0.0);
	d = pow(d, 2.0) * 0.125;

	gl_FragColor = mix(refle, u_matDiffuse, d);
}

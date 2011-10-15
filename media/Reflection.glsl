
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

uniform sampler2D u_water;
uniform sampler2D u_refract;

void main() {
	
	vec2 watercoord = v_texcoord * u_refractionMapParam.zz + vec2(0.0, u_refractionMapParam.w);
	vec4 water = texture2D(u_water, watercoord);
	vec3 norm = normalize( water.xzy * 2.0 - 1.0 );
	
	vec2 refracoord = (v_refractionMap.xy / v_refractionMap.ww) * 0.5 + 0.5;
	refracoord += norm.xz * u_refractionMapParam.xy;
	
	vec4 refra = texture2D(u_refract, refracoord);
	refra.xyz *= u_matDiffuse.xyz;
	
	float d = dot(norm, normalize(u_camPos - v_pos));
	d = max(d, 0.0);
	d = pow(d, 8.0);
	
	// add some lighting
	vec3 l = normalize(vec3(-5.0, 5.0, 0.0) - v_pos);
	vec3 h = normalize(l + normalize(u_camPos - v_pos));
	float ndh = max(0.0, dot(norm, h));
	ndh = pow(ndh, u_matShininess);
	
	vec4 refle = vec4(1.0, 1.0, 1.0, 1.0);

	gl_FragColor = mix(refra, refle, d) + u_matSpecular * ndh;
	
	/*
	vec2 refracoord = (v_refractionMap.xy / v_refractionMap.ww) * 0.5 + 0.5;
	vec4 refra = texture2D(u_refract, refracoord);
	refra.xyz *= u_matDiffuse.xyz;
	gl_FragColor = refra;
	*/
}

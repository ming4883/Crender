-- Process.Vertex
attribute vec3 i_vertex;
varying vec2 v_texcrd;

void main(void) {
	gl_Position = vec4(i_vertex, 1.0);
	v_texcrd = i_vertex.xy * 0.5 + 0.5;
}

-- Init.Fragment
precision mediump float;

varying vec2 v_texcrd;

void main(void) {
	gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
}

-- Copy.Fragment
precision mediump float;

varying vec2 v_texcrd;
uniform sampler2D u_buffer;

void main(void) {
	gl_FragColor = texture2D(u_buffer, v_texcrd);
}

-- Step.Fragment
precision mediump float;

varying vec2 v_texcrd;
uniform vec2 u_delta;
uniform sampler2D u_curr;
uniform sampler2D u_last;

void main(void) {
	vec2 dx = vec2(u_delta.x, 0.0);
	vec2 dy = vec2(0.0, u_delta.y);
	vec4 curr = texture2D(u_curr, v_texcrd);
	vec4 last = texture2D(u_last, v_texcrd);
	vec4 s;
	s.x = texture2D(u_curr, v_texcrd - dx - dy).x;
	s.y = texture2D(u_curr, v_texcrd - dx + dy).x;
	s.z = texture2D(u_curr, v_texcrd + dx - dy).x;
	s.w = texture2D(u_curr, v_texcrd + dx + dy).x;
	
	float average = dot(s, vec4(0.25, 0.25, 0.25, 0.25));
	
	// http://www.lonesock.net/article/verlet.html
	// xi+1 = xi + (xi - xi-1) + a * dt * dt
	
	float v = curr.x - last.x;
	float a = average - curr.x;
	v -= v / 64.0; // dumping
	float next = curr.x + v + a;
	
	gl_FragColor = vec4(next, 0, 0, 0);
}

-- Normal.Fragment
precision mediump float;

varying vec2 v_texcrd;
uniform vec2 u_delta;
uniform sampler2D u_buffer;

void main(void) {
	vec2 dx = vec2(u_delta.x, 0.0);
	vec2 dy = vec2(0.0, u_delta.y);
	vec4 s;
	s.x = texture2D(u_buffer, v_texcrd - dx - dy).x;
	s.y = texture2D(u_buffer, v_texcrd - dx + dy).x;
	s.z = texture2D(u_buffer, v_texcrd + dx - dy).x;
	s.w = texture2D(u_buffer, v_texcrd + dx + dy).x;
	
	vec2 n;
	n.x = dot(s, vec4(0.25, 0.25,-0.25,-0.25)); // sobelX
	n.y = dot(s, vec4(0.25,-0.25,+0.25,-0.25)); // sobelY
	
	vec3 norm = normalize( vec3(n.x, 1.0 / 512.0, n.y) );
	
	// output
	gl_FragColor = vec4(norm * 0.5 + 0.5, 0);
}

-- AddDrop.Fragment
precision mediump float;

const float PI = 3.141592653589793;

varying vec2 v_texcrd;
uniform vec2 u_center;
uniform float u_radius;
uniform float u_strength;
uniform sampler2D u_buffer;

void main(void) {
	vec4 curr = texture2D(u_buffer, v_texcrd);
	
	float drop = max(0.0, 1.0 - length(u_center - v_texcrd) / u_radius);
	drop = 0.5 - cos(drop * PI) * 0.5;
	
	float next = curr.x + drop * u_strength;
	
	// output
	gl_FragColor = vec4(next, 0, 0, 0);
}

-- Scene.Vertex
attribute vec4 i_vertex;
attribute vec3 i_normal;
attribute vec2 i_texcoord0;

varying vec3 v_normal;
varying vec3 v_pos;
varying vec2 v_texcoord;

uniform mat4 u_worldMtx;
uniform mat4 u_worldViewMtx;
uniform mat4 u_worldViewProjMtx;

void main() {
	gl_Position = u_worldViewProjMtx * i_vertex;
	v_normal = (u_worldMtx * vec4(i_normal, 0)).xyz;
	v_pos = (u_worldMtx * i_vertex).xyz;
	v_texcoord = i_texcoord0;
}

-- Scene.Fragment
precision mediump float;

varying vec3 v_normal;
varying vec3 v_pos;
varying vec2 v_texcoord;

uniform vec3 u_camPos;
uniform vec4 u_matDiffuse;
uniform vec4 u_matSpecular;
uniform float u_matShininess;

uniform sampler2D u_tex;

void main() {
	vec3 n = normalize(v_normal.xyz);
	vec3 l = normalize(vec3(-2.0, 2.0, 0.0) - v_pos);
	vec3 h = normalize(l + normalize(u_camPos - v_pos));
	
	if(false == gl_FrontFacing)
		n *= -1.0;

	float ndl = max(0.0, dot(n, l));
	float ndh = max(0.0, dot(n, h));
	ndh = pow(ndh, u_matShininess);
	
	vec4 tex = texture2D(u_tex, v_texcoord);
	vec4 color = (u_matDiffuse * ndl * tex);
	color.xyz += (u_matSpecular * ndh * tex).xyz;
	
	//gl_FragColor = color;
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
	
	vec4 water = texture2D(u_water, v_texcoord);
	water = water * 2.0 - 1.0;
	vec3 norm = normalize( water.xyz );
	//vec3 norm = ( water.xyz );
	
	vec2 refracoord = (v_refractionMap.xy / v_refractionMap.ww) * 0.5 + 0.5;
	refracoord += norm.xz * u_refractionMapParam.xy;
	
	vec4 refra = texture2D(u_refract, refracoord);
	refra.xyz *= u_matDiffuse.xyz;
	
	float d = dot(norm, normalize(u_camPos - v_pos));
	d = max(d, 0.0);
	d = pow(d, 4.0);
	
	vec4 refle = vec4(1.0, 1.0, 1.0, 1.0);
	/*
	// add some lighting
	vec3 l = normalize(vec3(-5.0, 5.0, 0.0) - v_pos);
	vec3 h = normalize(l + normalize(u_camPos - v_pos));
	float ndh = max(0.0, dot(norm, h));
	ndh = pow(ndh, u_matShininess);

	gl_FragColor = mix(refra, refle, d) + u_matSpecular * ndh;
	*/
	gl_FragColor = mix(refra, refle, d);
}

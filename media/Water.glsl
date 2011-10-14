-- Process.Vertex
attribute vec3 i_vertex;
varying vec2 v_texcrd;

void main(void) {
	gl_Position = vec4(i_vertex, 1.0);
	v_texcrd = i_vertex.xy * 0.5 + 0.5;
}

-- Init.Fragment
precision highp float;

varying vec2 v_texcrd;

void main(void) {
	gl_FragColor.x = 0.0;
	gl_FragColor.y = 0.0;
	gl_FragColor.z = 0.0;
	gl_FragColor.w = 1.0;
}

-- Step.Fragment
precision highp float;

varying vec2 v_texcrd;
uniform vec2 u_delta;
uniform sampler2D u_buffer;

void main(void) {
	vec2 dx = vec2(u_delta.x, 0.0);
	vec2 dy = vec2(0.0, u_delta.y);
	vec4 curr = texture2D(u_buffer, v_texcrd);
	vec4 s;
	s.x = texture2D(u_buffer, v_texcrd - dx - dy).x;
	s.y = texture2D(u_buffer, v_texcrd - dx + dy).x;
	s.z = texture2D(u_buffer, v_texcrd + dx - dy).x;
	s.w = texture2D(u_buffer, v_texcrd + dx + dy).x;
	
	float average = dot(s, vec4(0.25, 0.25, 0.25, 0.25));
	
	// change the velocity to move toward the average
	curr.y += (average - curr.x) * 2.0;
	
	// apply some dumping to the velocity
	curr.y *= 0.985;
	
	// move the displacement along the velocity
	curr.x += curr.y;
	
	curr.z = dot(s, vec4(0.25, 0.25,-0.25,-0.25)); // sobelX
	curr.w = dot(s, vec4(0.25,-0.25,+0.25,-0.25)); // sobelY
	
	// output
	gl_FragColor = curr;
}

-- AddDrop.Fragment
precision highp float;

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
	
	curr.x += drop * u_strength;
	
	// output
	gl_FragColor = curr;
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
precision highp float;

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
precision highp float;

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
	vec3 norm = normalize( vec3(water.z, 0.01, water.w) );
	
	vec2 refracoord = (v_refractionMap.xy / v_refractionMap.ww) * 0.5 + 0.5;
	refracoord += norm.xz * u_refractionMapParam.xy;
	
	vec4 refra = texture2D(u_refract, refracoord);
	refra.xyz *= u_matDiffuse.xyz;
	
	float d = dot(norm, normalize(u_camPos - v_pos));
	d = max(d, 0.0);
	d = pow(d, 4.0);
	
	// add some lighting
	vec3 l = normalize(vec3(-5.0, 5.0, 0.0) - v_pos);
	vec3 h = normalize(l + normalize(u_camPos - v_pos));
	float ndh = max(0.0, dot(norm, h));
	ndh = pow(ndh, u_matShininess);
	
	vec4 refle = vec4(1.0, 1.0, 1.0, 1.0);

	gl_FragColor = mix(refra, refle, d) + u_matSpecular * ndh;
}

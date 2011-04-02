-- Scene.Vertex
in vec4 i_vertex;
in vec3 i_normal;
in vec2 i_texcoord0;

out vec3 v_normal;
out vec3 v_pos;
out vec2 v_texcoord;

uniform mat4 u_worldViewMtx;
uniform mat4 u_worldViewProjMtx;

void main() {
	gl_Position = u_worldViewProjMtx * i_vertex;
	v_normal = (u_worldViewMtx * vec4(i_normal, 0)).xyz;
	v_pos = (u_worldViewMtx * i_vertex).xyz;
	v_texcoord = i_texcoord0;
}

-- Scene.Fragment
in vec3 v_normal;
in vec3 v_pos;
in vec2 v_texcoord;

out vec4 o_fragColor;

uniform vec4 u_matDiffuse;
uniform vec4 u_matSpecular;
uniform float u_matShininess;

void main() {
	
	vec3 n = normalize(v_normal.xyz);
	vec3 l = normalize(vec3(0,10,10) - v_pos.xyz);
	vec3 h = normalize(l + vec3(0, 0, 1));
	
	if(false == gl_FrontFacing)
		n *= -1.0;
		
	float ndl = max(0.0, dot(n, l)) * 0.8 + 0.2;
	float ndh = max(0.0, dot(n, h));
	ndh = pow(ndh, u_matShininess);
	
	vec4 color = u_matDiffuse;
	color.xyz *= ndl;
	color.xyz += u_matSpecular.xyz * ndh;
	
	o_fragColor = color;
}

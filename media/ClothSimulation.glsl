-- Scene.Vertex
in vec4 i_vertex;
in vec3 i_normal;
in vec2 i_texcoord0;

out vec3 v_normal;
out vec3 v_pos;
out vec2 v_texcoord;
out vec4 v_shadowMap;

uniform mat4 u_worldMtx;
uniform mat4 u_worldViewMtx;
uniform mat4 u_worldViewProjMtx;
uniform mat4 u_shadowMapTexMtx;

void main() {
	gl_Position = u_worldViewProjMtx * i_vertex;
	v_normal = (u_worldMtx * vec4(i_normal, 0)).xyz;
	v_pos = (u_worldMtx * i_vertex).xyz;
	v_texcoord = i_texcoord0;
	v_shadowMap = u_shadowMapTexMtx * vec4(v_pos, 1);
}

-- Scene.Fragment
in vec3 v_normal;
in vec3 v_pos;
in vec2 v_texcoord;
in vec4 v_shadowMap;

out vec4 o_fragColor;

uniform vec4 u_matDiffuse;
uniform vec4 u_matSpecular;
uniform float u_matShininess;
uniform vec4 u_shadowMapParam;

uniform sampler2D u_tex;
uniform sampler2D u_shadowMapTex;

float computeShadowTerm(vec3 sm, vec2 offset)
{
	float smTexel = u_shadowMapParam[0];
	float smBias = u_shadowMapParam[1];
	
	vec4 smDepth;
	vec2 samples[4] = vec2[4](
		vec2(0, 0),
		vec2(0, 1),
		vec2(1, 0),
		vec2(1, 1)
	);
	
	for(int i=0; i<4; ++i) {
		vec2 uv = sm.xy + (samples[i] + offset) * smTexel;
		smDepth[i] = texture(u_shadowMapTex, uv).x;
	}

	return dot(vec4(lessThan(sm.zzzz - smBias, smDepth)), vec4(0.25));
}

void main() {
	vec3 n = normalize(v_normal.xyz);
	vec3 l = normalize(vec3(0,10,10) - v_pos.xyz);
	vec3 h = normalize(l + vec3(0, 0, 1));
	
	if(false == gl_FrontFacing)
		n *= -1;
		
	vec3 sm = v_shadowMap.xyz / v_shadowMap.www;
	float shadowTerm = 0;
	shadowTerm += computeShadowTerm(sm, vec2(-1,-1));
	shadowTerm += computeShadowTerm(sm, vec2(-1, 1));
	shadowTerm += computeShadowTerm(sm, vec2( 1,-1));
	shadowTerm += computeShadowTerm(sm, vec2( 1, 1));
	shadowTerm *= 0.25;
	
	float ndl = max(0, dot(n, l)) * shadowTerm * 0.8 + 0.2;
	float ndh = max(0, dot(n, h)) * shadowTerm;
	ndh = pow(ndh, u_matShininess);
	
	vec4 color = u_matDiffuse * texture(u_tex, v_texcoord);
	color.xyz *= ndl;
	color.xyz += u_matSpecular.xyz * ndh;
	
	o_fragColor = color;
}

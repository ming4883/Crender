-- Scene.Vertex
struct vs_in {
	float4 vertex : POSITION;
	float3 normal : NORMAL;
	float2 texcoord0 : TEXCOORD0;
};

struct vs_out {
	float4 vertex : POSITION;
	float3 normal : TEXCOORD0;
	float3 pos : TEXCOORD1;
	float2 texcoord : TEXCOORD2;
	float4 refractionMap : TEXCOORD3;
};

uniform float4x4 u_worldMtx;
uniform float4x4 u_worldViewMtx;
uniform float4x4 u_worldViewProjMtx;
uniform float4x4 u_refractionMapTexMtx;

vs_out main(vs_in i) {
	vs_out o;

	o.vertex = mul(u_worldViewProjMtx, i.vertex);
	o.normal = mul(u_worldMtx, float4(i.normal, 0)).xyz;
	o.pos = mul(u_worldMtx, i.vertex).xyz;
	o.texcoord = i.texcoord0;
	o.refractionMap = mul(u_refractionMapTexMtx, float4(o.pos, 1));

	return o;
}

-- Scene.Fragment
struct ps_in {
	float3 normal : TEXCOORD0;
	float3 pos : TEXCOORD1;
	float2 texcoord : TEXCOORD2;
	float4 refractionMap : TEXCOORD3;
	float facing : VFACE;
};

struct ps_out {
	float4 fragColor : COLOR0;
};

uniform float4 u_matDiffuse;
uniform float4 u_matSpecular;
uniform float u_matShininess;
uniform float4 u_refractionMapParam;

uniform sampler2D u_tex;

ps_out main(ps_in i) {
	ps_out o;

	float3 n = normalize(i.normal.xyz);
	float3 l = normalize(float3(0,10,10) - i.pos.xyz);
	float3 h = normalize(l + float3(0, 0, 1));
	
	n *= -i.facing;
	
	float3 sm = i.refractionMap.xyz / i.refractionMap.www;
		
	float ndl = max(0, dot(n, l)) * 0.8 + 0.2;
	float ndh = max(0, dot(n, h));
	ndh = pow(ndh, u_matShininess);
	
	float4 color = u_matDiffuse * tex2D(u_tex, i.texcoord);
	color.xyz *= ndl;
	color.xyz += u_matSpecular.xyz * ndh;
	
	o.fragColor = color;

	return o;
}
